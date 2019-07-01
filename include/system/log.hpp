#pragma once
#include "types/types.hpp"

namespace oic {

	enum class LogLevel {
		DEBUG,
		PERFORMANCE,
		WARN,
		ERROR,
		FATAL
	};

	class Log {

	public:

		static constexpr usz maxStackTrace = 48;
		using StackTrace = Array<void *, maxStackTrace>;

		Log() = default;
		virtual ~Log() = default;

		virtual void debug(const String &str) = 0;
		virtual void performance(const String &str) = 0;
		virtual void warn(const String &str) = 0;
		virtual void error(const String &str) = 0;
		virtual void fatal(const String &str) = 0;

		void println(const String &str, LogLevel level);
		void println(const String &str);

		template<LogLevel level = LogLevel::DEBUG>
		inline void println(const String &str);

		//!Used to print the current stacktrace
		//@param[in] skip How many function calls to skip (0 by default)
		void printStackTrace(usz skip = 0);

		//!Used to capture the current stacktrace
		//@param[in] skip How many function calls to skip (0 by default)
		//@return Array<void*, maxStackTrace> functionCalls; last index is nullptr or at maxStackTrace - 1.
		virtual StackTrace captureStackTrace(usz skip = 0) = 0;

		//!Used to print a captured stack trace
		virtual void printStackTrace(const StackTrace &stackTrace) = 0;

	};

	template<LogLevel level>
	void Log::println(const String &str){

		if constexpr (level == LogLevel::DEBUG)
			debug(str);
		else if constexpr (level == LogLevel::PERFORMANCE)
			performance(str);
		else if constexpr (level == LogLevel::WARN)
			warn(str);
		else if constexpr (level == LogLevel::ERROR)
			error(str);
		else
			fatal(str);
	}


}