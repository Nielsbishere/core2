#include "system/windows_file_system.hpp"
#include "system/system.hpp"
#include "system/log.hpp"
#include "error/ocore.hpp"
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

	WFileSystem::WFileSystem(): LocalFileSystem(getWorkingDirectory()) {
		initFiles();
		initLut();
		initFileWatcher();
	}

	WFileSystem::~WFileSystem() {
		running = false;
		thread.wait();
	}

	bool WFileSystem::readVirtual(const FileInfo &file, Buffer &buffer, usz size, usz offset) const {
		file;
		buffer;
		size;
		offset;
		return false;		//TODO:
	}

	String getPath(FILE_NOTIFY_INFORMATION *fni) {

		usz len = fni->FileNameLength / 2;
		String path(len + 2, '/'), newPath;
		path[0] = '.';

		for (usz i = 0; i < len; ++i)
			path[i + 2] = fni->FileName[i] == L'\\' ? '/' : c8(fni->FileName[i]);

		return path;
	}

	void WFileSystem::watchFileSystem(WFileSystem *fs) {

		HANDLE directory = CreateFileA(
			fs->getLocalPath().c_str(), GENERIC_READ, 
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 
			NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL
		);

		if (directory == INVALID_HANDLE_VALUE)
			System::log()->fatal(errors::fs::invalid);

		constexpr usz bufferSize = 16_MiB;
		u8 *buffer = oic::System::allocator()->allocArray<u8>(bufferSize);

		HANDLE iocp = CreateIoCompletionPort(directory, NULL, NULL, 1);
		HRESULT hr = GetLastError();

		if(hr != S_OK)
			System::log()->fatal(errors::fs::illegal);

		OVERLAPPED overlapped;
		ZeroMemory(&overlapped, sizeof(overlapped));

		while (fs->running) {

			if(!ReadDirectoryChangesW(
				directory, buffer, bufferSize, TRUE,
				FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
				NULL, &overlapped, NULL
			))
				System::log()->fatal(errors::fs::invalid);

			DWORD returned{};
			LPOVERLAPPED po;
			ULONG_PTR key;

			if (!GetQueuedCompletionStatus(iocp, &returned, &key, &po, 1000)) {

				hr = GetLastError();

				if (hr == WAIT_TIMEOUT)
					continue;

				System::log()->fatal(errors::fs::illegal);
			}

			FILE_NOTIFY_INFORMATION *fni = (FILE_NOTIFY_INFORMATION*)buffer;
			struct stat st;

			System::begin();

			do {

				String newPath;
				String path = getPath(fni);

				switch (fni->Action) {

					case FILE_ACTION_REMOVED:

						if (!fs->exists(path))
							System::log()->fatal(errors::fs::invalid);

						fs->rem(path);
						break;

					case FILE_ACTION_ADDED:

						stat(path.c_str(), &st);

						fs->add(path, !S_ISREG(st.st_mode));
						break;

					case FILE_ACTION_MODIFIED:

						if (!fs->exists(path))
							System::log()->fatal(errors::fs::invalid);

						fs->upd(path);
						break;

					case FILE_ACTION_RENAMED_OLD_NAME:

						fni = (FILE_NOTIFY_INFORMATION*)((u8*)fni + fni->NextEntryOffset);

						if (fni->Action != FILE_ACTION_RENAMED_NEW_NAME)
							System::log()->fatal(errors::fs::illegal);

						newPath = getPath(fni);

						fs->mov(path, newPath);
						break;

					default:
						System::log()->fatal(errors::fs::illegal);

				}

				fni = fni->NextEntryOffset ? (FILE_NOTIFY_INFORMATION*)((u8*)fni + fni->NextEntryOffset) : nullptr;

			} while (fni);

			System::end();

		}

		CloseHandle(iocp);
		CloseHandle(directory);
		free(buffer);
	}

	void WFileSystem::initFileWatcher() {
		running = true;
		thread = std::async(watchFileSystem, this);
	}
	
	void WFileSystem::initFiles_(const String &ou, FileHandle parent) {

		//Find all files

		WIN32_FIND_DATAA data {};
		HANDLE file = FindFirstFileA((ou + "/*").c_str(), &data);

		if (file == INVALID_HANDLE_VALUE)
			System::log()->fatal(errors::fs::nonExistent);

		List<String> directories, files;
		directories.reserve(16);
		files.reserve(16);

		do {

			//Skip .. and .
			if (data.cFileName[0] == '.' && data.cFileName[2] == '\0' && (data.cFileName[1] == '.' || data.cFileName[1] == '\0'))
				continue;

			if(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				directories.push_back(data.cFileName);
			else
				files.push_back(data.cFileName);

		} while (FindNextFileA(file, &data));

		FindClose(file);

		//Init all directories

		FileInfo *p = &get(parent, true);
		FileHandle folderHint = p->folderHint = size(true);

		for (String &dir : directories) {

			FileInfo subdir = {
				p->path + "/" + dir, dir,
				0, 0, nullptr, 0,
				p->id, size(true),
				0, 0, 0,
				p->access,
				true
			};

			initFile(subdir);
			this->files(true).push_back(subdir);

			p = &get(parent, true);

		}

		//Init all files

		p->fileHint = size(true);

		for (String &fil : files) {

			FileInfo subfil = {
				p->path + "/" + fil, fil,
				0, 0, nullptr, 0,
				p->id, size(true),
				0, 0, 0,
				p->access,
				false
			};

			initFile(subfil);
			this->files(true).push_back(subfil);

			p = &get(parent, true);
		}

		//Recursive

		p->fileEnd = size(true);

		FileHandle i = folderHint;

		for (String &dir : directories) {
			initFiles_(ou + "/" + dir, i);
			++i;
		}

	}

	void WFileSystem::initFiles() {
		initFile(get(0, true));
		initFiles_(getLocalPath(), 0);
	}

}