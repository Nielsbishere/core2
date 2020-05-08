#include "system/windows_file_system.hpp"
#include "system/system.hpp"
#include "system/log.hpp"
#include "system/allocator.hpp"

#include <Windows.h>
#include <codecvt>
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)

namespace oic {

	String getWorkingDirectory() {
		String directory(MAX_PATH + 1, '\0');
		DWORD size = GetCurrentDirectoryA(MAX_PATH + 1, directory.data());
		return directory.substr(0, usz(size));
	}

	WFileSystem::WFileSystem() : LocalFileSystem(getWorkingDirectory()) {
		initFiles();
		initLut();
	}

	WFileSystem::~WFileSystem() {
		for (auto &thr : threads) {
			running[thr.first] = false;
			thr.second.wait();
		}
	}

	File *WFileSystem::openVirtual(const FileInfo &) {
		//TODO: Virtual files
		oic::System::log()->fatal("Virtual files not supported yet");
		return nullptr;
	}

	String getPath(FILE_NOTIFY_INFORMATION *fni) {

		usz len = fni->FileNameLength / 2;
		String path(len, '/'), newPath;

		for (usz i = 0; i < len; ++i)
			path[i] = fni->FileName[i] == L'\\' ? '/' : c8(fni->FileName[i]);

		return path;
	}

	void WFileSystem::watchFileSystem(WFileSystem *fs, const String &subPath) {

		const String watchPath = fs->getLocalPath() + "/" + subPath;

		HANDLE directory = CreateFileA(
			watchPath.c_str(), GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL
		);

		if (directory == INVALID_HANDLE_VALUE)
			System::log()->fatal("The folder path is invalid");

		constexpr usz bufferSize = 4_MiB;
		u8 *buffer = oic::System::allocator()->allocArray<u8>(bufferSize);

		HANDLE iocp = CreateIoCompletionPort(directory, NULL, NULL, 1);
		HRESULT hr = GetLastError();

		if (hr != S_OK)
			System::log()->fatal("The io completion port couldn't be created");

		OVERLAPPED overlapped;
		ZeroMemory(&overlapped, sizeof(overlapped));

		while (fs->running[subPath]) {

			if (!ReadDirectoryChangesW(
				directory, buffer, bufferSize, TRUE,
				FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
				NULL, &overlapped, NULL
			))
				System::log()->fatal("Couldn't read directory changes");

			DWORD returned {};
			LPOVERLAPPED po;
			ULONG_PTR key;

			if (!GetQueuedCompletionStatus(iocp, &returned, &key, &po, 1000)) {

				hr = GetLastError();

				if (hr == WAIT_TIMEOUT)
					continue;

				System::log()->fatal("The queued status is illegal");
			}

			FILE_NOTIFY_INFORMATION *fni = (FILE_NOTIFY_INFORMATION *)buffer;
			struct stat st;

			System::files()->lock();

			do {

				String newPath;
				String path = subPath + "/" + getPath(fni);

				switch (fni->Action) {

				case FILE_ACTION_REMOVED:
					fs->remove(path, true);
					break;

				case FILE_ACTION_ADDED:

					stat(path.c_str(), &st);

					fs->add(path, !S_ISREG(st.st_mode), true);
					break;

				case FILE_ACTION_MODIFIED:

					if (!fs->exists(path))
						System::log()->fatal("The modified file path is invalid");

					fs->update(path);
					break;

				case FILE_ACTION_RENAMED_OLD_NAME:

					fni = (FILE_NOTIFY_INFORMATION*)((u8*)fni + fni->NextEntryOffset);

					if (fni->Action != FILE_ACTION_RENAMED_NEW_NAME)
						System::log()->fatal("The rename action requires an old and new name in that sequence");

					newPath = subPath + "/" + getPath(fni);

					fs->mov(path, newPath, true);
					break;

				default:
					System::log()->fatal("The file operation is illegal");

				}

				fni = fni->NextEntryOffset ? (FILE_NOTIFY_INFORMATION *)((u8 *)fni + fni->NextEntryOffset) : nullptr;

			} while (fni);

			System::files()->unlock();

		}

		CloseHandle(iocp);
		CloseHandle(directory);
		free(buffer);
	}

	void WFileSystem::startFileWatcher(const String &path) {
		running[path] = true;
		threads[path] = std::move(std::async(watchFileSystem, this, path));
	}

	void WFileSystem::endFileWatcher(const String &path) {
		running[path] = false;
		threads[path] = std::move(std::async(watchFileSystem, this, path));
	}

	template<bool includeFiles, bool includeFolders>
	inline List<String> findFileObjects(const String &path) {

		//Find all files

		WIN32_FIND_DATAA data{};
		HANDLE file = FindFirstFileA((path + "/*").c_str(), &data);

		if (file == INVALID_HANDLE_VALUE)
			System::log()->fatal("Invalid folder to scan");

		List<String> objs;
		objs.reserve(32);

		do {

			//Skip .. and .
			if (data.cFileName[0] == '.' && data.cFileName[2] == '\0' && (data.cFileName[1] == '.' || data.cFileName[1] == '\0'))
				continue;

			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

				if constexpr(includeFolders)
					objs.push_back(data.cFileName);

			} else if constexpr(includeFiles)
				objs.push_back(data.cFileName);

		} while (FindNextFileA(file, &data));

		FindClose(file);
		return objs;
	}
	
	List<String> WFileSystem::localDirectories(const String &path) const {
		return findFileObjects<false, true>(path);
	}

	List<String> WFileSystem::localFileObjects(const String &path) const {
		return findFileObjects<true, true>(path);
	}

	List<String> WFileSystem::localFiles(const String &path) const {
		return findFileObjects<true, false>(path);
	}

	void WFileSystem::initFiles() {
		//TODO: Get virtual files
	}

}