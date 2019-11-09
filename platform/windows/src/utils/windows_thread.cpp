#include "utils/thread.hpp"
#include <Windows.h>

namespace oic {

	usz Thread::getCurrentId() {
		return GetCurrentProcessId();
	}
}