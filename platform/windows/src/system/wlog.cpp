#include "system/wlog.hpp"
#include "system/system.hpp"
#include "error/ocore.hpp"
#include <exception>

//Unfortunately before Windows 10 it doesn't support printing colors into console using printf
//We also use Windows dependent stack tracing and std::exception
#include <Windows.h>
#include <signal.h>
#include <DbgHelp.h>
#include <comdef.h>

#pragma comment(lib, "DbgHelp.lib")

namespace oic::windows {

	//Printing colored text

	template<bool outputToDebugConsole = false>
	void print(const c8 *cstr, WORD color) {

		HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(handle, color);
		printf("%s\n", cstr);

		if constexpr (outputToDebugConsole)
			OutputDebugStringA(cstr);
	}

	//Printing text based on log level

	void WLog::debug(const String &str) {
		print(str.c_str(), 2 /* green */);
	}

	void WLog::performance(const String &str) {
		print(str.c_str(), 3 /* cyan */);
	}

	void WLog::warn(const String &str) {
		print<true>(str.c_str(), 14 /* yellow */);
	}

	void WLog::error(const String &str) {
		print<true>(str.c_str(), 4 /* red */);
	}

	void WLog::fatal(const String &str) {
		print<true>(str.c_str(), 12 /* bright red */);
		Log::printStackTrace(1);
		throw std::exception(str.c_str());
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
				msg = errors::sig::abrt;
				break;

			case SIGFPE:
				msg = errors::sig::fpe;
				break;

			case SIGILL:
				msg = errors::sig::ill;
				break;

			case SIGINT:
				msg = errors::sig::interupt;
				break;

			case SIGSEGV:
				msg = errors::sig::segv;
				break;

			case SIGTERM:
				msg = errors::sig::term;
				break;

			default:
				msg = errors::sig::undef;
				break;

		}

		//Outputting to console is not technically allowed by the Windows docs
		//If this signal is triggered from the wrong thread it might cause stackoverflow
		//For debugging purposed however, this is very useful
		//Turn this off by defining __NO_SIGNAL_HANDLING__

		print<true>(msg, 12 /* bright red */);
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