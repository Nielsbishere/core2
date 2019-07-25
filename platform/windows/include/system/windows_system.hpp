#pragma once
#include "system/system.hpp"
#include "system/windows_log.hpp"
#include "system/windows_file_system.hpp"

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

		WFileSystem wfileSystem;
		//WAllocator wallocator;
		//WViewportManager wviewportManager;
		WLog wlog;

	public:

		static const WindowsSystem windowsSystem;

	};

}