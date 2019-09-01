#include "system/windows_system.hpp"
#include <Windows.h>

namespace oic {

	namespace windows {

		WindowsSystem::WindowsSystem(): System(&wfileSystem, &wallocator, &wviewportManager, &wlog) {

			HMODULE ntdll = GetModuleHandleA("ntdll.dll");

			if (!ntdll)
				oic::System::log()->fatal("Ntdll not found");

			*((void**)&ntDelayExecution) = GetProcAddress(ntdll, "NtDelayExecution");

			if (!ntDelayExecution)
				oic::System::log()->fatal("Sleep function not found");
		}

		const WindowsSystem WindowsSystem::windowsSystem = WindowsSystem();

		void WindowsSystem::sleep(u64 time) {
			LARGE_INTEGER interval;
			interval.QuadPart = -i64(time / 100);
			ntDelayExecution(false, &interval);
		}
	}

	System *System::system = (System*) &windows::WindowsSystem::windowsSystem;

}