#include "system/windows_allocator.hpp"
#include "system/windows_system.hpp"
#include <Windows.h>

namespace oic::windows {

	void *WAllocator::alloc(usz size, RangeHint hint, usz addressHint) {

		void *addr;

		if (hint == HEAP)
			addr = ::malloc(size);
		else {

			DWORD type{};

			if (hint & RESERVE) type |= MEM_RESERVE;
			if (hint & COMMIT) type |= MEM_COMMIT;

			addr = VirtualAlloc(
				LPVOID(addressHint), size, type, PAGE_READWRITE
			);
		}

		if (!addr)
			oic::System::log()->fatal("Couldn't allocate memory");

		return addr;
	}

	void WAllocator::free(void *v, usz, bool isRange) {

		if (isRange)
			VirtualFree(v, 0, MEM_RELEASE);
		else
			::free(v);
	}

}