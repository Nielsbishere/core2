#pragma once
#include "types/types.hpp"
#include <random>

namespace oic {

	class Random {

	public:

		//Create randomly seeded random
		Random(): rd(), g((u64(rd()) << 32) | rd()) { g.discard(700_K); }

		//Create seeded random
		Random(u64 seed): g(seed) {}

		//Pick a random range
		//[min, max] for T = int (including max)
		//[min, max> for T = float (excluding max)
		template<typename T>
		T range(const T min, T max) {

			using Gen = std::conditional_t<
				std::is_floating_point_v<T>, std::uniform_real_distribution<T>, std::uniform_int_distribution<T>
			>;

			return Gen(min, max)(g);
		}

	private:

		std::random_device rd;
		std::mt19937_64 g;

	};
}