#pragma once
#include "system/local_file_system.hpp"
#include <future>

namespace oic {

	class WFileSystem : public LocalFileSystem {

	public:

		WFileSystem();
		~WFileSystem();

	protected:

		File *openVirtual(const FileInfo &file) final override;

		void startFileWatcher(const String &location) final override;
		void endFileWatcher(const String &location) final override;
		void initFiles() final override;

		List<String> localDirectories(const String &path) const final override;
		List<String> localFileObjects(const String &path) const final override;
		List<String> localFiles(const String &path) const final override;

		static void watchFileSystem(WFileSystem *fs, const String &path);

		HashMap<String, std::future<void>> threads;
		HashMap<String, bool> running;

	};

}