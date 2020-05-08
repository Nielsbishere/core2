#pragma once
#include "system/log.hpp"
#include <ctime>

#ifdef _WIN32
	#include <intrin.h>
	#define localtime_r(t, tp) localtime_s(&tp, &t)
#else
	#include <x86intrin.h>
#endif

namespace oic {

	class Timer {

	public:

		static inline ns getElapsed(ns time) { return now() - time; }

		//Time since Unix Epoch (1970)
		static ns now();

		static inline u64 getElapsedClocks(u64 clocks) { return getClocks() - clocks; }
		static inline u64 getClocks() { return __rdtsc(); }

		//s.mus
		String formatSeconds(ns time);

		//h:m:s.mus
		String formatDuration(ns time);

	};

}