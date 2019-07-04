#include "system/windows_file_system.hpp"
#include "system/system.hpp"
#include "system/log.hpp"
#include <Windows.h>

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

		Array<HANDLE, 3> notifiers{};

		Array<DWORD, 3> notifyFilters {
			FILE_NOTIFY_CHANGE_DIR_NAME,
			FILE_NOTIFY_CHANGE_FILE_NAME,
			FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION
		};

		for (usz i = 0, j = notifyFilters.size(); i != j; ++i) {

			notifiers[i] = FindFirstChangeNotificationA(
				fs->getLocalPath().c_str(), TRUE, notifyFilters[i]
			);

			if (notifiers[i] == INVALID_HANDLE_VALUE)
				System::log()->fatal("Couldn't create file watcher");
		}

		while (fs->running) {

			DWORD type = WaitForMultipleObjects(DWORD(notifiers.size()), notifiers.data(), FALSE, 1000);

			for(HANDLE n : notifiers)
				FindNextChangeNotification(n);

			if (type == WAIT_TIMEOUT)	//Look for next file change
				continue;

			int dbg = 0;
			dbg;

		}

		for (HANDLE n : notifiers)
			FindCloseChangeNotification(n);
	}

	void WFileSystem::initFileWatcher() {
		running = true;
		thread = std::async(watchFileSystem, this);
	}

	void WFileSystem::initFiles() {
		int dbg = 0;
		dbg;
	}

}