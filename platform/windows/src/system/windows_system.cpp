#include "system/windows_system.hpp"

namespace oic::windows {

	//WindowsSystem::WindowsSystem(): System(&wfileSystem, &wallocator, &wviewportManager, &wlog) { }

	WindowsSystem::WindowsSystem() : 
		System((oic::LocalFileSystem*)nullptr, (oic::Allocator*)nullptr, (oic::ViewportManager*)nullptr, (oic::Log*)&wlog) {}

	const WindowsSystem WindowsSystem::windowsSystem = WindowsSystem();

}