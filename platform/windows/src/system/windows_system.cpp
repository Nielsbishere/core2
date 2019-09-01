#include "system/windows_system.hpp"

namespace oic {

	namespace windows {

		//WindowsSystem::WindowsSystem(): System(&wfileSystem, &wallocator, &wviewportManager, &wlog) { }

		WindowsSystem::WindowsSystem():
			System(&wfileSystem, (oic::Allocator*)nullptr, &wviewportManager, &wlog) {}

		const WindowsSystem WindowsSystem::windowsSystem = WindowsSystem();

	}

	System *System::system = (System*) &windows::WindowsSystem::windowsSystem;

}