#include "system/log.hpp"

namespace oic {

	void Log::printStackTrace(usz skip) {
		printStackTrace(captureStackTrace(skip + 1));
	}

}