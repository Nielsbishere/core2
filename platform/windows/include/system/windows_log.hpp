#pragma once
#include "system/log.hpp"

namespace oic::windows {

	class WLog : public oic::Log {

	public:

		WLog();

		void debug(const String &str) final override;
		void performance(const String &str) final override;
		void warn(const String &str) final override;
		void error(const String &str) final override;
		void fatal(const String &str) final override;

		StackTrace captureStackTrace(usz skip = 0) final override;
		void printStackTrace(const StackTrace &stackTrace) final override;

	private:

		static void sigFunc(int signal);

	};

}