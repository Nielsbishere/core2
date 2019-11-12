#pragma once
#include "system/log.hpp"

namespace oic::windows {

	class WLog : public oic::Log {

	public:

		WLog();

		void print(LogLevel level, const String &str) final override;

		StackTrace captureStackTrace(usz skip = 0) final override;
		void printStackTrace(const StackTrace &stackTrace) final override;

	private:

		static void sigFunc(int signal);

	};

}