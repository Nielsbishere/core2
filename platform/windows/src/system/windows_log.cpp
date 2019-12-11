#include "system/windows_log.hpp"
#include "system/system.hpp"
#include <exception>
#include <ctime>

//Unfortunately before Windows 10 it doesn't support printing colors into console using printf
//We also use Windows dependent stack tracing and std::exception
#include <Windows.h>
#include <signal.h>
#include <DbgHelp.h>
#include <comdef.h>

#pragma comment(lib, "DbgHelp.lib")
#undef fatal

namespace oic::windows {

	//Printing colored text

	template<bool outputToDebugConsole = false>
	void print(const c8 *cstr, WORD color) {

		//Get thread
		u32 thread = GetCurrentThreadId();

		//Get time
		time_t t {};
		time(&t);

		//Get readable time
		struct tm timeInfo {};
		localtime_s(&timeInfo, &t);

		LARGE_INTEGER counter, freq;
		QueryPerformanceCounter(&counter);
		QueryPerformanceFrequency(&freq);

		u32 ns = u32((counter.QuadPart / (freq.QuadPart / 1000000)) % 1000000);

		//Set color
		HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(handle, color);

		//Print text
		printf("[%u %02i:%02i:%02i.%06u] %s", thread, timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec, ns, cstr);

		if constexpr (outputToDebugConsole)
			OutputDebugStringA(cstr);
	}

	//Printing text based on log level

	void WLog::print(LogLevel level, const String &str) {

		static const WORD colors[] = {
			2,	/* green */
			3,	/* cyan */
			14,	/* yellow */
			4,	/* red */
			12	/* bright red */
		};

		WORD color = colors[usz(level)];

		if (level != LogLevel::DEBUG) {

			HRESULT hr = GetLastError();
			String err;

			if (hr != S_OK)
				err = std::system_category().message(hr);

			windows::print<true>((str + err + "\n").c_str(), color);
		} else
			windows::print(str.c_str(), color);

		if (level == LogLevel::FATAL) {

			Log::printStackTrace(1);

			#ifndef NDEBUG
				DebugBreak();
			#endif

			throw std::runtime_error(str.c_str());
		}
	}

	//Capture stacktrace

	Log::StackTrace WLog::captureStackTrace(usz skip) {

		StackTrace stack{};
		RtlCaptureStackBackTrace(DWORD(1 + skip), DWORD(stack.size()), stack.data(), NULL);
		return stack;
	}

	void WLog::printStackTrace(const StackTrace &stackTrace) {

		//Obtain process

		HANDLE process = GetCurrentProcess();
		HMODULE processModule = GetModuleHandleA(NULL);

		struct CapturedStackTrace {

			//Module and symbol

			String mod{}, sym{};

			//File and line don't have to be specified, for external calls

			String fil{};
			u32 lin{};


		} captured[Log::maxStackTrace]{};

		usz stackCount{};

		if (!SymInitialize(process, NULL, TRUE))
			goto noSymbols;

		for (usz i = 0; i < stackTrace.size() && stackTrace[i]; ++i, ++stackCount) {

			usz addr = usz(stackTrace[i]);

			//Get module name

			usz moduleBase = SymGetModuleBase(process, addr);

			c8 modulePath[MAX_PATH + 1]{};
			if (!moduleBase || !GetModuleFileNameA((HINSTANCE) moduleBase, modulePath, MAX_PATH))
				goto error;

			c8 symbolData[sizeof(IMAGEHLP_SYMBOL) + MAX_PATH + 1]{};

			PIMAGEHLP_SYMBOL symbol = (PIMAGEHLP_SYMBOL)symbolData;
			symbol->SizeOfStruct = sizeof(symbolData);
			symbol->MaxNameLength = MAX_PATH;

			c8 *symbolName = symbol->Name;

			if(!SymGetSymFromAddr(process, addr, NULL, symbol))
				goto error;

			DWORD offset{};
			IMAGEHLP_LINE line{};
			line.SizeOfStruct = sizeof(line);

			SymGetLineFromAddr(process, addr, &offset, &line);	//Can fail, meaning that line is null

			if (line.FileName && strlen(line.FileName) > MAX_PATH)
				goto error;

			CapturedStackTrace &capture = captured[i];
			capture.mod = modulePath;
			capture.sym = symbolName;

			if (moduleBase == usz(processModule))
				capture.mod = capture.mod.substr(capture.mod.find_last_of('\\') + 1);

			if(line.FileName)
				capture.fil = line.FileName;

			capture.lin = line.LineNumber;

		}

		printf("Stacktrace:\n");

		for (usz i = 0; i < stackCount; ++i) {

			CapturedStackTrace &capture = captured[i];

			if(capture.lin)
				printf("%p: %s!%s (%s, Line %u)\n", stackTrace[i], capture.mod.c_str(), capture.sym.c_str(), capture.fil.c_str(), capture.lin);
			else
				printf("%p: %s!%s\n", stackTrace[i], capture.mod.c_str(), capture.sym.c_str());
		}

		goto end;

		//Fallback for executable with no debug symbols

	error:

		#ifndef NDEBUG
			printf("No symbols available (%s)\n", _com_error(GetLastError()).ErrorMessage());
		#endif

	noSymbols:

		printf("Stacktrace:\n");

		for(usz i = 0; i < stackTrace.size() && stackTrace[i]; ++i)
			printf("%p\n", stackTrace[i]);

		//Cleanup

	end:

		SymCleanup(process);

	}

	//Handle crash signals

	void WLog::sigFunc(int signal) {

		const c8 *msg{};

		switch (signal) {

			case SIGABRT:
				msg = "Abort was called";
				break;

			case SIGFPE:
				msg = "Floating point error occurred";
				break;

			case SIGILL:
				msg = "Illegal instruction";
				break;

			case SIGINT:
				msg = "Interrupt was called";
				break;

			case SIGSEGV:
				msg = "Segfault";
				break;

			case SIGTERM:
				msg = "Terminate was called";
				break;

			default:
				msg = "Undefined instruction";
				break;

		}

		//Outputting to console is not technically allowed by the Windows docs
		//If this signal is triggered from the wrong thread it might cause stackoverflow
		//For debugging purposed however, this is very useful
		//Turn this off by defining __NO_SIGNAL_HANDLING__

		windows::print<true>(msg, 12 /* bright red */);
		System::log()->printStackTrace(1);
		exit(signal);
	}

	//Use our custom signal handler

	WLog::WLog() {
		#ifndef __NO_SIGNAL_HANDLING__
			signal(SIGABRT, WLog::sigFunc);
			signal(SIGFPE, WLog::sigFunc);
			signal(SIGILL, WLog::sigFunc);
			signal(SIGINT, WLog::sigFunc);
			signal(SIGSEGV, WLog::sigFunc);
			signal(SIGTERM, WLog::sigFunc);
		#endif
	}

}