#include "system/windows_system.hpp"

namespace oic::windows {

	WindowsSystem::WindowsSystem(): System() { }

	LocalFileSystem *WindowsSystem::getFiles() const final override { return &fileSystem; }
	Allocator *WindowsSystem::getAllocator() const final override { return &allocator; }
	WindowManager *WindowsSystem::getWindowManager() const final override { return &viewportManager; }
	Log *WindowsSystem::getLog() const final override { return &log; }

	const WindowSystem WindowsSystem::windowsSystem = WindowsSystem();

}