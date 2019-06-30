#pragma once
#include "system/system.hpp"

namespace oic::windows {

	class LocalFileSystem;
	class Allocator;
	class ViewportManager;
	class Log;

	//!Windows implementation of a system
	class WindowsSystem {

	protected:

		LocalFileSystem *getFiles() const final override;
		Allocator *getAllocator() const final override;
		ViewportManager *getViewportManager() const final override;
		Log *getLog() const final override;

	private:

		WindowsSystem();
		~WindowsSystem() = default;

		WLocalFileSystem fileSystem;
		WAllocator allocator;
		WViewportManager viewportManager;
		WLog log;

		static const WindowsSystem windowsSystem;

	};

}