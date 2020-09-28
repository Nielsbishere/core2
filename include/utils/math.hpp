#pragma once
#include "types/types.hpp"
#include <cmath>

namespace oic {

	struct Math {

		static constexpr f64
			PI = 3.14159265358979323846,
			PI2 = PI * 2,
			PI0_5 = PI * 0.5,
			radToDeg = 180 / Math::PI,
			degToRad = Math::PI / 180;

		template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
		static constexpr inline T abs(T v) {
			return v * (T(v >= 0) * 2 - 1);
		}

		template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
		static constexpr inline T pow2(T v) {
			return v * v;
		}

		template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
		static constexpr inline T toDegrees(T rad) {
			return rad * T(radToDeg);
		}

		template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
		static constexpr inline T toRadians(T rad) {
			return rad * T(degToRad);
		}

		template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
		static constexpr inline T floor(T v) {
			return T(i64(v));
		}

		template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
		static constexpr inline T fract(T v) {
			return v - floor(v);
		}

		template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
		static constexpr inline T ceil(T v) {
			return floor(v) + (v > 0 && v != floor(v));
		}

		template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
		static constexpr inline T min(T a, T b) {
			return a <= b ? a : b;
		}

		template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
		static constexpr inline T max(T a, T b) {
			return a >= b ? a : b;
		}

		template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
		static constexpr inline T clamp(T v, T a, T b) {
			return v < a ? a : (v > b ? b : v);
		}

		template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
		static inline T log(T v) {
			return T(std::log10(v));
		}

		template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
		static inline T log2(T v) {
			return T(std::log2(v));
		}

	};

}