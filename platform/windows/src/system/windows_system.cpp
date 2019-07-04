#include "system/windows_system.hpp"

namespace oic::windows {

	//WindowsSystem::WindowsSystem(): System(&wfileSystem, &wallocator, &wviewportManager, &wlog) { }

	WindowsSystem::WindowsSystem() : 
		System(&wfileSystem, (oic::Allocator*)nullptr, (oic::ViewportManager*)nullptr, &wlog) {}

	const WindowsSystem WindowsSystem::windowsSystem = WindowsSystem();

}