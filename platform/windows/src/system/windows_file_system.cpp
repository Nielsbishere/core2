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

	class WVirtualFile : public File {

	private:

		HGLOBAL file{};
		const c8 *data{};

		virtual ~WVirtualFile() { 
			if (file) {
				UnlockResource(file);
				FreeResource(file);
				file = nullptr;
				data = nullptr;
			}
		}

	public:

		WVirtualFile(FileSystem *fs, const FileInfo &f, ns, ns): File(fs, f) {

			if (!f.hasAccess(FileAccess::WRITE) && f.dataExt) {

				HRSRC rc = (HRSRC) f.dataExt;

				file = LoadResource(nullptr, rc);

				if (file) {

					data = (const c8*)LockResource(file);

					if (!data) {
						FreeResource(rc);
						file = nullptr;
					}

					else isOpen = true;
				}
			}

			if(!isOpen)
				System::log()->fatal("File can't be opened");
		}

		bool read(void *v, FileSize size, FileSize offset) const final override {

			if (offset + size > f.fileSize) {
				System::log()->fatal("File read is out of bounds");
				return false;
			}

			std::memcpy(v, data + offset, size);
			return true;
		}

		//Write to file isn't allowed with virtual files

		bool write(const void*, FileSize, FileSize) final override { return false; }
		bool resize(FileSize) final override { return false; }
	};

	File *WFileSystem::openVirtual(const FileInfo &info) {
		return new WVirtualFile(this, info, 0, 0);
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

		//Get root file

		HRSRC root = FindResourceA(nullptr, "0", RT_RCDATA);
		if (!root) return;

		HGLOBAL rootHandle = LoadResource(nullptr, root);
		if (!rootHandle) return;

		const c8 *data = (const c8*)LockResource(rootHandle);
		if (!data) { FreeResource(rootHandle); return; }

		usz end = strlen(data);

		List<FileInfo> files;
		files.push_back(virtualFiles[0]);

		files[0].fileHint = u32_MAX;
		files[0].fileEnd = u32_MAX;
		files[0].folderHint = u32_MAX;

		HashMap<u32, u32> unorderedIdToId;

		usz prev{}, i0{}, i1{};

		for (usz i = 0; i < end; ++i) {

			c8 c = data[i];

			//Get first two spaces

			if (c == ' ') {
				if (!i0)		i0 = i;
				else if(!i1)	i1 = i;
			}

			//Flush file

			if (c == '\n') {

				usz k = data[i - 1] == '\r' ? i - 1 : i;

				bool isFile = data[k - 1] == '|';
				String name(data + i1 + 1, data + k - isFile);

				u32 parent = std::stoi(String(data + i0 + 1, data + i1));

				String primaryId = String(data + prev, data + i0);
				u32 primaryIdi = std::stoi(primaryId);

				//Windows can't handle numbers as file names
				//So we just use an underscore

				String primary = '_' + primaryId;

				auto file = isFile ?
					FindResourceA(nullptr, primary.c_str(), RT_RCDATA) : nullptr;

				HRESULT hr = GetLastError();
				
				if (FAILED(hr)) {
					oic::System::log()->warn("Missing symbol " + primary);
					continue;
				}

				//Find the last file that has the same parent
				//And insert after; otherwise, insert at end
				//If it's a folder, stop when it first encounters a file

				u32 j = 1;
				bool inFolder{};

				for (; j < u32(files.size()); ++j)

					if (!isFile && inFolder && !files[j].isFolder())
						break;

					else if (files[j].parent == parent)
						inFolder = true;

					else if (inFolder)
						break;

				//Correct ids

				for (auto &map : unorderedIdToId)
					if (map.second >= j)
						++map.second;

				//Insert this entry

				unorderedIdToId[primaryIdi] = j;

				files.insert(files.begin() + j, FileInfo{
					name,
					name,
					0,
					file,
					isFile ? SizeofResource(nullptr, file) : 0,
					parent,
					u32_MAX,
					u32_MAX,
					u32_MAX,
					isFile ? FileFlags::VIRTUAL_FILE : FileFlags::VIRTUAL_FOLDER
				});

				i0 = i1 = 0;
				prev = i + 1;
			}
		}

		//Update references to parents and file hint if needed

		for(u32 i = 1; i < u32(files.size()); ++i) {

			auto &f = files[i];

			//Lookup real parent

			u32 parent = unorderedIdToId[f.parent];

			f.parent = parent;
			auto &p = files[parent];

			if (f.isFolder()) {

				//Since folders are ordered first, we only have to set folderHint and fileEnd to i + 1

				if (p.folderHint == u32_MAX)
					p.folderHint = i;

				p.fileHint = p.fileEnd = i + 1;
			}
			else {

				if (p.folderHint == u32_MAX)
					p.folderHint = i;

				if (p.fileHint == u32_MAX)
					p.fileHint = i;

				p.fileEnd = i + 1;
			}

		}

		//Done with parsing root file

		virtualFiles = files;

		UnlockResource(rootHandle);
		FreeResource(rootHandle);

	}

}