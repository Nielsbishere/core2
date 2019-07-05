#include "system/windows_file_system.hpp"
#include "system/system.hpp"
#include "system/log.hpp"
#include "error/ocore.hpp"
#include <Windows.h>
#include <codecvt>

namespace oic {

	String getWorkingDirectory() {
		String directory(MAX_PATH + 1, '\0');
		DWORD size = GetCurrentDirectoryA(MAX_PATH + 1, directory.data());
		return directory.substr(0, usz(size));
	}

	WFileSystem::WFileSystem(): LocalFileSystem(getWorkingDirectory()) {
		initFiles();
		resetLut();
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
		return false;
	}

	void WFileSystem::watchFileSystem(WFileSystem *fs) {

		HANDLE directory = CreateFileA(
			fs->getLocalPath().c_str(), GENERIC_READ, 
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 
			NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL
		);

		if (directory == INVALID_HANDLE_VALUE)
			System::log()->fatal(errors::fs::invalid);

		while (fs->running) {

			u8 buffer[sizeof(FILE_NOTIFY_INFORMATION) + MAX_PATH * 2]{};
			DWORD returned{};

			if(!ReadDirectoryChangesW(
				directory, buffer, sizeof(buffer), TRUE,
				FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SECURITY,
				&returned, NULL, NULL
			))
				System::log()->fatal(errors::fs::invalid);

			FILE_NOTIFY_INFORMATION &fni = *(FILE_NOTIFY_INFORMATION*)buffer;

			usz len = fni.FileNameLength / 2;
			String path(len + 2, '/');
			path[0] = '.';

			for (usz i = 0; i < len; ++i)
				path[i + 2] = fni.FileName[i] == '\\' ? '/' : c8(fni.FileName[i]);

			switch (fni.Action) {

				case FILE_ACTION_REMOVED:

					if (!fs->exists(path))
						System::log()->fatal(errors::fs::invalid);

					fs->notifyFileChange(path, true);
					break;

				case FILE_ACTION_ADDED:

					fs->notifyFileChange(path, false);
					break;

				case FILE_ACTION_MODIFIED:

					if (!fs->exists(path))
						System::log()->fatal(errors::fs::invalid);

					fs->notifyFileChange(path, false);
					break;

				case FILE_ACTION_RENAMED_NEW_NAME:
				case FILE_ACTION_RENAMED_OLD_NAME:

					//fs->notifyFileChange(path, false);
					break;

				default:
					System::log()->fatal(errors::fs::invalid);

			}

			int dbg = 0;
			dbg;

		}

		CloseHandle(directory);
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

		//Init all directories

		FileInfo *p = &get(parent, true);
		FileHandle folderHint = p->folderHint = localSize();

		for (String &dir : directories) {

			FileInfo subdir = {
				p->path + "/" + dir,
				0, 0, nullptr, 0,
				p->id, localSize(),
				0, 0, 0,
				FileAccess::READ_WRITE,
				true
			};

			initFile(subdir);
			addLocal(subdir);

			p = &get(parent, true);

		}

		//Init all files

		p->fileHint = localSize();

		for (String &fil : files) {

			FileInfo subfil = {
				p->path + "/" + fil,
				0, 0, nullptr, 0,
				p->id, localSize(),
				0, 0, 0,
				FileAccess::READ_WRITE,
				false
			};

			initFile(subfil);
			addLocal(subfil);

			p = &get(parent, true);
		}

		//Recursive

		p->fileEnd = localSize();

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