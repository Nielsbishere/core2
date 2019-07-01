#pragma once
#include "system/system.hpp"
#include "system/wlog.hpp"

namespace oic::windows {

	class LocalFileSystem;
	class Allocator;
	class ViewportManager;
	class Log;

	//!Windows implementation of a system
	class WindowsSystem : public System {

	private:

		WindowsSystem();
		~WindowsSystem() = default;

		//WLocalFileSystem wfileSystem;
		//WAllocator wallocator;
		//WViewportManager wviewportManager;
		WLog wlog;

		static const WindowsSystem windowsSystem;

	};

}