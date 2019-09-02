#include "utils/timer.hpp"
#include <chrono>

namespace oic {

	ns Timer::now() {
		using namespace std::chrono;
		return duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
	}

	String Timer::formatSeconds(ns time) {
		return Log::concat(time / 1_s, ".", Log::num(time % 1_s / 1000, 6));
	}

	//h:m:s.mus
	String Timer::formatDuration(ns time) {
		return Log::concat(
			Log::num(time / 1_h % 60), ":", Log::num(time / 1_m % 60, 2), ":", Log::num(time / 1_s % 60, 2), ".",
			Log::num(time % 1_s / 1000, 6)
		);
	}
}