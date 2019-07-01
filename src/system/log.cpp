#include "system/log.hpp"

namespace oic {

	void Log::println(const String &str) {
		debug(str);
	}

	void Log::printStackTrace(usz skip) {
		printStackTrace(captureStackTrace(skip + 1));
	}

	void Log::println(const String &str, LogLevel level) {

		switch (level) {

			case LogLevel::FATAL:
				fatal(str);
				break;

			case LogLevel::ERROR:
				error(str);
				break;

			case LogLevel::WARN:
				warn(str);
				break;

			case LogLevel::PERFORMANCE:
				performance(str);
				break;

			default:
				debug(str);
		}
	}

}