#pragma once
#include "types/types.hpp"
#include <sstream>

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

		static constexpr usz maxStackTrace = 128;
		using StackTrace = Array<void *, maxStackTrace>;

		Log() = default;
		virtual ~Log() = default;

		//TODO: Add ability to print with specs; e.g. no next line, no date, etc.

		virtual void print(LogLevel level, const String &str) = 0;

		template<LogLevel level, typename ...args>
		inline void println(const args &...arg);

		template<typename ...args>
		inline void println(LogLevel level, const args &...arg);

		template<typename ...args>
		inline void debug(const String &str);

		template<typename ...args>
		inline void performance(const String &str);

		template<typename ...args>
		inline void warn(const String &str);

		template<typename ...args>
		inline void error(const String &str);

		template<typename ...args>
		inline void fatal(const String &str);

		template<typename ...args>
		static inline String concat(const args &...arg);

		//Convert an integer to string
		template<usz base = 10, typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
		static inline String num(T val, usz minSize = 0);

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

	template<typename ...args>
	String Log::concat(const args &...arg) {
		std::stringstream ss;
		((ss << arg), ...);
		return ss.str();
	}

	template<LogLevel level, typename ...args>
	void Log::println(const args &...arg){
		const String str = concat(arg..., "\n");
		print(level, str);
	}

	template<typename ...args>
	inline void Log::println(LogLevel level, const args &...arg) {
		const String str = concat(arg..., "\n");
		print(level, str);
	}
	
	template<usz base, typename T, typename>
	static inline String Log::num(T val, usz minSize) {

		static_assert(base <= 64, "Only supported up to base64");

		static constexpr c8 chars[65] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz#*";

		String str;

		bool sign = false;

		if constexpr(std::is_signed_v<T>)
			if (val < 0) {
				sign = true;
				val = -val;
			}

		if (val == 0)
			str = "0";
		else {

			str.reserve(minSize == 0 ? 16 : minSize);

			do {
				const T remainder = val % base;
				str.insert(str.begin(), chars[remainder]);
				val /= base;

			} while (val != 0);
		}

		if (str.size() >= minSize)
			return (sign ? "-" : "") + str;

		return (sign ? "-" : "") + String(minSize - str.size(), '0') + str;
	}

	template<typename ...args>
	inline void Log::debug(const String &str) {
		println<LogLevel::DEBUG>(str);
	}

	template<typename ...args>
	inline void Log::performance(const String &str) {
		println<LogLevel::PERFORMANCE>(str);
	}

	template<typename ...args>
	inline void Log::warn(const String &str) {
		println<LogLevel::ERROR>(str);
	}

	template<typename ...args>
	inline void Log::error(const String &str) {
		println<LogLevel::ERROR>(str);
	}

	template<typename ...args>
	inline void Log::fatal(const String &str) {
		println<LogLevel::FATAL>(str);
	}

	//For full debugging; all fatal errors (and errors) will have origins in files
	//oic::System::log()->fatal(error, ..., ...) will evaluate to error, ..., ..., " at {FILE}::{FUNC}:{LINE}"

	#ifndef NDEBUG
		#define fatal(...) println<oic::LogLevel::FATAL>(__VA_ARGS__, " at " __FILE__ "::", __func__, ":", std::to_string(__LINE__))
	#endif

	//Asserts

	#define oicAssert(error, ...) if(!(__VA_ARGS__)) oic::System::log()->fatal("Assert failed: " error);
}
