#pragma once
#include "system/local_file_system.hpp"
#include <future>

namespace oic {

	class WFileSystem : public LocalFileSystem {

	public:

		WFileSystem();
		~WFileSystem();

	protected:

		bool readVirtual(const FileInfo &file, Buffer &buffer, usz size, usz offset) const final override;
		void initFileWatcher() final override;
		void initFiles() final override;

		void initFiles_(const String &ou, FileHandle file);

		static void watchFileSystem(WFileSystem *fs);

		std::future<void> thread;
		bool running = false;

	};

}