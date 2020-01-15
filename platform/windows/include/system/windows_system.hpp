#pragma once
#include "system/system.hpp"
#include "system/windows_log.hpp"
#include "system/windows_file_system.hpp"
#include "system/windows_viewport_manager.hpp"
#include "system/windows_allocator.hpp"

namespace oic::windows {

	//!Windows implementation of a system
	class WindowsSystem : public System {

	private:

		WindowsSystem();
		~WindowsSystem() {
			wviewportManager.clear();
		}

		void sleep(ns time) final override;

		WFileSystem wfileSystem;
		WAllocator wallocator;
		WViewportManager wviewportManager;
		WLog wlog;

	public:

		static const WindowsSystem windowsSystem;

		void (*ntDelayExecution)(bool alertable, void *largeInteger);

	};

}