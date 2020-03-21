#pragma once
#include "types.hpp"

//The vectors in this class AREN'T SIMD vectors
//This is to allow flexibility with padding;
//	a Vec2[] on the CPU would be expected to be 16 bytes
//	on the GPU, this can be either 16 or 32 bytes, depending on the padding
//However, some of the vector functions on types that have good SIMD functions can be implemented

template<typename T, usz N>
struct TVecStorage {
	T arr[N];
};

template<typename T>
struct TVecStorage<T, 2> {
	union {
		T arr[2];
		struct { T x, y; };
	};
};

template<typename T>
struct TVecStorage<T, 3> {
	union {
		T arr[3];
		struct { T x, y, z; };
	};
};

template<typename T>
struct TVecStorage<T, 4> {
	union {
		T arr[4];
		struct { T x, y, z, w; };
	};
};

//Generic vector
template<typename T, usz N>
struct Vec : public TVecStorage<T, N> {

	using TVecStorage<T, N>::TVecStorage;
	using TVecStorage<T, N>::arr;

	using Type = T;
	
	//Initialization
	
	Vec(const Vec &other) = default;
	Vec(Vec &&other) = default;
	Vec &operator=(const Vec &other) = default;
	Vec &operator=(Vec &&other) = default;
	~Vec() = default;
	
	constexpr inline Vec() : TVecStorage<T, N>{} {
		static_assert(std::is_arithmetic_v<T> && !std::is_same_v<T, bool>, "Vec only allowed on arithmetic types");
	}
	
	constexpr inline Vec(const T(&t)[N]) : TVecStorage<T, N>{ t } {}
	
	template<typename ...args>
	constexpr inline Vec(const T &t, const args &...arg) : TVecStorage<T, N>{{{ t, T(arg)... }}} {
		static_assert(sizeof...(args) + 1 <= N, "Invalid number of initializers for Vec");
	}

	//Arithmetic functions
	
	constexpr inline Vec(const T &t): TVecStorage<T, N>{} {
		for (usz i = 0; i < N; ++i) arr[i] = t;
	}
	
	constexpr inline Vec &operator+=(const Vec &other) {
		for (usz i = 0; i < N; ++i) arr[i] += other.arr[i];
		return *this;
	}
	
	constexpr inline Vec &operator-=(const Vec &other) {
		for (usz i = 0; i < N; ++i) arr[i] -= other.arr[i];
		return *this;
	}
	
	constexpr inline Vec &operator*=(const Vec &other) {
		for (usz i = 0; i < N; ++i) arr[i] *= other.arr[i];
		return *this;
	}
	
	constexpr inline Vec &operator/=(const Vec &other) {
		for (usz i = 0; i < N; ++i) arr[i] /= other.arr[i];
		return *this;
	}
	
	constexpr inline Vec &operator%=(const Vec &other) {
	
		if constexpr(std::is_integral_v<T>)
			for (usz i = 0; i < N; ++i) arr[i] %= other.arr[i];
		else
			for (usz i = 0; i < N; ++i) {
				arr[i] /= other.arr[i];
				arr[i] -= std::floor(arr[i]);
				arr[i] *= other.arr[i];
			}
	
		return *this;
	}
	
	constexpr inline Vec operator-() const {
		Vec res;
		for (usz i = 0; i < N; ++i) res.arr[i] = -arr[i];
		return res;
	}
	
	constexpr inline Vec operator+(const Vec &other) const { return Vec(*this) += other; }
	constexpr inline Vec operator-(const Vec &other) const { return Vec(*this) -= other; }
	constexpr inline Vec operator*(const Vec &other) const { return Vec(*this) *= other; }
	constexpr inline Vec operator/(const Vec &other) const { return Vec(*this) /= other; }
	constexpr inline Vec operator%(const Vec &other) const { return Vec(*this) %= other; }
	
	constexpr inline Vec &operator++() const { return operator+=(1); }
	constexpr inline Vec &operator--() const { return operator-=(1); }
	
	constexpr inline const T &operator[](const usz i) const { return arr[i]; }
	constexpr inline T &operator[](const usz i) { return arr[i]; }
	
	inline const T *begin() const { return arr; }
	inline const T *end() const { return arr + N; }
	inline T *begin() { return arr; }
	inline T *end() { return arr + N; }
	
	static constexpr inline usz size() { return N; }
	
	//Math functions
	
	constexpr inline T sum() const {
		T v{};
		for (usz i = 0; i < N; ++i) v += arr[i];
		return v;
	}
	
	constexpr inline T squaredMagnitude() const {
		T v{};
		for (usz i = 0; i < N; ++i) v += arr[i] * arr[i];
		return v;
	}
	
	constexpr inline T magnitude() const {
		return T(std::sqrt(f64(squaredMagnitude())));
	}
	
	constexpr inline Vec normalize() const {
		return *this / magnitude();
	}
	
	template<typename T2 = T>
	constexpr inline T2 prod() const {
		T2 v = T2(1);
		for (usz i = 0; i < N; ++i) v *= arr[i];
		return v;
	}
	
	constexpr inline T dot(const Vec &other) const {
		T v{};
		for (usz i = 0; i < N; ++i) v += arr[i] * other.arr[i];
		return v;
	}
	
	constexpr inline bool any() const {
		for (usz i = 0; i < N; ++i) if (arr[i]) return true;
		return false;
	}
	
	constexpr inline bool all() const {
		bool b = true;
		for (usz i = 0; i < N; ++i) b &= bool(arr[i]);
		return b;
	}
	
	constexpr inline bool none() const { return !any(); }
	constexpr inline bool operator!() const { return !all(); }
	
	constexpr inline Vec cos() const {
		Vec res;
		for (usz i = 0; i < N; ++i) res.arr[i] = T(std::cos(f64(arr[i])));
		return res;
	}
	
	constexpr inline Vec sin() const {
		Vec res;
		for (usz i = 0; i < N; ++i) res.arr[i] = T(std::sin(f64(arr[i])));
		return res;
	}
	
	constexpr inline Vec tan() const {
		Vec res;
		for (usz i = 0; i < N; ++i) res.arr[i] = T(std::tan(f64(arr[i])));
		return res;
	}
	
	constexpr inline Vec sqrt() const {
		Vec res;
		for (usz i = 0; i < N; ++i) res.arr[i] = T(std::sqrt(f64(arr[i])));
		return res;
	}
	
	constexpr inline Vec pow2() const {
		Vec res;
		for (usz i = 0; i < N; ++i) res.arr[i] = T(std::pow(f64(arr[i]), 2));
		return res;
	}
	
	constexpr inline Vec pow(T t) const {
		Vec res;
		for (usz i = 0; i < N; ++i) res.arr[i] = T(std::pow(f64(arr[i]), t));
		return res;
	}
	
	constexpr inline Vec min(const Vec &other) const {
		Vec res;
		for (usz i = 0; i < N; ++i) res.arr[i] = std::min(arr[i], other.arr[i]);
		return res;
	}
	
	constexpr inline Vec max(const Vec &other) const {
		Vec res;
		for (usz i = 0; i < N; ++i) res.arr[i] = std::max(arr[i], other.arr[i]);
		return res;
	}
	
	constexpr inline Vec clamp(const Vec &min, const Vec &max) const {
		Vec res;
		for (usz i = 0; i < N; ++i) res.arr[i] = std::clamp(arr[i], min.arr[i], max.arr[i]);
		return res;
	}
	
	constexpr inline Vec inverse() const {
		Vec res;
		for (usz i = 0; i < N; ++i) res.arr[i] = T(1) / arr[i];
		return res;
	}
	
	template<typename T2, usz N2>
	constexpr inline Vec<T2, N2> cast() const {
	
		Vec<T2, N2> res{};
	
		for (usz i = 0; i < N2 && i < N; ++i)
			res.arr[i] = T2(arr[i]);
	
		return res;
	}
	
	template<typename T2>
	constexpr inline T2 cast() const {
	
		T2 res{};
	
		for (usz i = 0; i < T2::size() && i < N; ++i)
			res.arr[i] = typename T2::Type(arr[i]);
	
		return res;
	}
	
	//Floating point functions
	
	constexpr inline Vec isnan() const {
	
		static_assert(std::is_floating_point_v<T>, "Vec<T,N>::isnan only exists on floating points");
	
		Vec res;
		for (usz i = 0; i < N; ++i) res.arr[i] = T(std::isnan(arr[i]));
		return res;
	}
	
	constexpr inline Vec isinf() const {
	
		static_assert(std::is_floating_point_v<T>, "Vec<T,N>::isinf only exists on floating points");
	
		Vec res;
		for (usz i = 0; i < N; ++i) res.arr[i] = T(std::isinf(arr[i]));
		return res;
	}
	
	constexpr inline Vec lerp(const Vec &destination, const Vec &percentage) const {
	
		static_assert(std::is_floating_point_v<T>, "Vec<T,N>::lerp only exists on floating points");
	
		Vec res;
		for (usz i = 0; i < N; ++i) res.arr[i] = arr[i] * percentage.arr[i] + destination.arr[i] * (1 - percentage.arr[i]);
		return res;
	}
	
	constexpr inline Vec floor() const {
	
		static_assert(std::is_floating_point_v<T>, "Vec<T,N>::floor only exists on floating points");
	
		Vec res;
		for (usz i = 0; i < N; ++i) res.arr[i] = std::floor(arr[i]);
		return res;
	}
	
	constexpr inline Vec round() const {
	
		static_assert(std::is_floating_point_v<T>, "Vec<T,N>::round only exists on floating points");
	
		Vec res;
		for (usz i = 0; i < N; ++i) res.arr[i] = std::round(arr[i]);
		return res;
	}
	
	constexpr inline Vec ceil() const {
	
		static_assert(std::is_floating_point_v<T>, "Vec<T,N>::ceil only exists on floating points");
	
		Vec res;
		for (usz i = 0; i < N; ++i) res.arr[i] = std::ceil(arr[i]);
		return res;
	}
	
	constexpr inline Vec fract() const {
	
		static_assert(std::is_floating_point_v<T>, "Vec<T,N>::fract only exists on floating points");
	
		Vec res;
		for (usz i = 0; i < N; ++i) res.arr[i] = arr[i] - std::floor(arr[i]);
		return res;
	}
	
	constexpr inline Vec radToDeg() const {
		static_assert(std::is_floating_point_v<T>, "Vec<T,N>::radToDeg only exists on floating points");
		return operator*(TO_DEG);
	}
	
	constexpr inline Vec degToRad() const {
		static_assert(std::is_floating_point_v<T>, "Vec<T,N>::degToRad only exists on floating points");
		return operator*(TO_RAD);
	}
	
	//Bitwise operator
	
	constexpr inline Vec &operator<<=(const usz v) {
	
		static_assert(std::is_integral_v<T>, "Vec<T,N>::operator<<= only exists on integers");
	
		for (usz i = 0; i < N; ++i) arr[i] <<= v;
		return *this;
	}
	
	constexpr inline Vec &operator>>=(const usz v) {
	
		static_assert(std::is_integral_v<T>, "Vec<T,N>::operator>>= only exists on integers");
	
		for (usz i = 0; i < N; ++i) arr[i] >>= v;
		return *this;
	}
	
	constexpr inline Vec &operator&=(const Vec &other) {
	
		static_assert(std::is_integral_v<T>, "Vec<T,N>::operator&= only exists on integers");
	
		for (usz i = 0; i < N; ++i) arr[i] &= other.arr[i];
		return *this;
	}
	
	constexpr inline Vec &operator^=(const Vec &other) {
	
		static_assert(std::is_integral_v<T>, "Vec<T,N>::operator^= only exists on integers");
	
		for (usz i = 0; i < N; ++i) arr[i] ^= other.arr[i];
		return *this;
	}

	constexpr inline Vec &operator|=(const Vec &other) {

		static_assert(std::is_integral_v<T>, "Vec<T,N>::operator|= only exists on integers");

		for (usz i = 0; i < N; ++i) arr[i] |= other.arr[i];
		return *this;
	}

	constexpr inline Vec operator~() const {

		static_assert(std::is_integral_v<T>, "Vec<T,N>::operator~ only exists on integers");

		Vec res;
		for (usz i = 0; i < N; ++i) res.arr[i] = ~arr[i];
		return res;
	}

	constexpr inline Vec operator>>(const usz v) const { return Vec(*this) >>= v; }
	constexpr inline Vec operator<<(const usz v) const { return Vec(*this) <<= v; }
	constexpr inline Vec operator|(const Vec &other) const { return Vec(*this) |= other; }
	constexpr inline Vec operator&(const Vec &other) const { return Vec(*this) &= other; }
	constexpr inline Vec operator^(const Vec &other) const { return Vec(*this) ^= other; }

	//Comparison functions

	constexpr inline bool operator==(const Vec &other) const {
		for (usz i = 0; i < N; ++i) if (arr[i] != other.arr[i]) return false;
		return true;
	}

	constexpr inline bool operator!=(const Vec &other) const {
		for (usz i = 0; i < N; ++i) if (arr[i] != other.arr[i]) return true;
		return false;
	}

	constexpr inline Vec eq(const Vec &other) const {
		Vec res;
		for (usz i = 0; i < N; ++i) res.arr[i] = T(arr[i] == other.arr[i]);
		return res;
	}

	constexpr inline Vec neq(const Vec &other) const {
		Vec res;
		for (usz i = 0; i < N; ++i) res.arr[i] = T(arr[i] != other.arr[i]);
		return res;
	}

	constexpr inline Vec operator>(const Vec &other) const {
		Vec res;
		for (usz i = 0; i < N; ++i) res.arr[i] = T(arr[i] > other.arr[i]);
		return res;
	}

	constexpr inline Vec operator<(const Vec &other) const {
		Vec res;
		for (usz i = 0; i < N; ++i) res.arr[i] = T(arr[i] < other.arr[i]);
		return res;
	}

	constexpr inline Vec operator>=(const Vec &other) const {
		Vec res;
		for (usz i = 0; i < N; ++i) res.arr[i] = T(arr[i] >= other.arr[i]);
		return res;
	}

	constexpr inline Vec operator<=(const Vec &other) const {
		Vec res;
		for (usz i = 0; i < N; ++i) res.arr[i] = T(arr[i] <= other.arr[i]);
		return res;
	}

	inline Buffer toData() const {
		return Buffer((u8*)arr, (u8*)(arr + N));
	}

	inline Buffer toGPUData() const {
		return toData();
	}

};

//Vec2 (TVec<T,2> with extra functionality)

template<typename T>
struct Vec2 : public Vec<T, 2> {

	using Vec<T, 2>::Vec;
	using TVecStorage<T, 2>::x;
	using TVecStorage<T, 2>::y;

	constexpr inline Vec2(const Vec<T, 2> &dat) : Vec<T, 2>(dat) {}
	constexpr inline Vec2(Vec<T, 2> &&dat) : Vec<T, 2>(dat) {}

	constexpr inline Vec2 swap() const { return { y, x }; }
	constexpr inline Vec2 yx() const { return { y, x }; }
	constexpr inline T aspect() const { return x / y; }

	constexpr inline T cross(const Vec2 &other) const {
		return x * other.y - y * other.x;
	}

	constexpr inline Vec2 reflect(const Vec2 &normal) const {
		return operator-(normal * (dot(normal) * 2));
	}

};

//Vec3 (TVec<T,3> with extra functionality)

template<typename T>
struct Vec3 : public Vec<T, 3> {

	using Vec<T, 3>::Vec;
	using TVecStorage<T, 3>::x;
	using TVecStorage<T, 3>::y;
	using TVecStorage<T, 3>::z;

	constexpr inline Vec3(const Vec<T, 3> &dat) : Vec<T, 3>(dat) {}
	constexpr inline Vec3(Vec<T, 3> &&dat) : Vec<T, 3>(dat) {}

	constexpr inline Vec3 yxz() const { return { y, x, z }; }
	constexpr inline Vec3 yzx() const { return { y, z, x }; }
	constexpr inline Vec3 zyx() const { return { z, y, x }; }
	constexpr inline Vec3 zxy() const { return { z, x, y }; }
	constexpr inline Vec3 xzy() const { return { x, z, y }; }

	constexpr inline Vec2<T> xy() const { return { x, y }; }
	constexpr inline Vec2<T> xz() const { return { x, z }; }
	constexpr inline Vec2<T> yx() const { return { y, x }; }
	constexpr inline Vec2<T> yz() const { return { y, z }; }
	constexpr inline Vec2<T> zx() const { return { z, x }; }
	constexpr inline Vec2<T> zy() const { return { z, y }; }

	constexpr inline Vec3 cross(const Vec3 &other) const {
		return {
			y * other.z - z * other.y,
			z * other.x - x * other.z,
			x * other.y - y * other.x
		};
	}

	constexpr inline Vec3 reflect(const Vec3 &normal) const {
		return operator-(normal * (dot(normal) * 2));
	}

};

//Vec4 (TVec<T,4> with extra functionality)

template<typename T>
struct Vec4 : public Vec<T, 4> {

	using Vec<T, 4>::Vec;
	using TVecStorage<T, 4>::x;
	using TVecStorage<T, 4>::y;
	using TVecStorage<T, 4>::z;
	using TVecStorage<T, 4>::w;

	constexpr inline Vec4(const Vec<T, 4> &dat) : Vec<T, 4>(dat) {}
	constexpr inline Vec4(Vec<T, 4> &&dat) : Vec<T, 4>(dat) {}

	constexpr inline Vec4 yxzw() const { return { y, x, z, w }; }
	constexpr inline Vec4 yzxw() const { return { y, z, x, w }; }
	constexpr inline Vec4 zyxw() const { return { z, y, x, w }; }
	constexpr inline Vec4 zxyw() const { return { z, x, y, w }; }
	constexpr inline Vec4 xzyw() const { return { x, z, y, w }; }

	constexpr inline Vec4 yxwz() const { return { y, x, w, z }; }
	constexpr inline Vec4 yzwx() const { return { y, z, w, x }; }
	constexpr inline Vec4 zywx() const { return { z, y, w, x }; }
	constexpr inline Vec4 zxwy() const { return { z, x, w, y }; }
	constexpr inline Vec4 xzwy() const { return { x, z, w, y }; }
	constexpr inline Vec4 xywz() const { return { x, y, w, z }; }

	constexpr inline Vec4 ywxz() const { return { y, w, x, z }; }
	constexpr inline Vec4 ywzx() const { return { y, w, z, x }; }
	constexpr inline Vec4 zwyx() const { return { z, w, y, x }; }
	constexpr inline Vec4 zwxy() const { return { z, w, x, y }; }
	constexpr inline Vec4 xwzy() const { return { x, w, z, y }; }
	constexpr inline Vec4 xwyz() const { return { x, w, y, z }; }

	constexpr inline Vec4 wyxz() const { return { w, y, x, z }; }
	constexpr inline Vec4 wyzx() const { return { w, y, z, x }; }
	constexpr inline Vec4 wzyx() const { return { w, z, y, x }; }
	constexpr inline Vec4 wzxy() const { return { w, z, x, y }; }
	constexpr inline Vec4 wxzy() const { return { w, x, z, y }; }
	constexpr inline Vec4 wxyz() const { return { w, x, y, z }; }

};

//Type definitions

using Vec2u8  = Vec2<u8>;
using Vec2u16 = Vec2<u16>;
using Vec2u32 = Vec2<u32>;
using Vec2u64 = Vec2<u64>;
using Vec2usz = Vec2<usz>;

using Vec2i8  = Vec2<i8>;
using Vec2i16 = Vec2<i16>;
using Vec2i32 = Vec2<i32>;
using Vec2i64 = Vec2<i64>;
using Vec2isz = Vec2<isz>;

using Vec2f32 = Vec2<f32>;
using Vec2f64 = Vec2<f64>;

using Vec3u8  = Vec3<u8>;
using Vec3u16 = Vec3<u16>;
using Vec3u32 = Vec3<u32>;
using Vec3u64 = Vec3<u64>;
using Vec3usz = Vec3<usz>;

using Vec3i8  = Vec3<i8>;
using Vec3i16 = Vec3<i16>;
using Vec3i32 = Vec3<i32>;
using Vec3i64 = Vec3<i64>;
using Vec3isz = Vec3<isz>;

using Vec3f32 = Vec3<f32>;
using Vec3f64 = Vec3<f64>;

using Vec4u8  = Vec4<u8>;
using Vec4u16 = Vec4<u16>;
using Vec4u32 = Vec4<u32>;
using Vec4u64 = Vec4<u64>;
using Vec4usz = Vec4<usz>;

using Vec4i8  = Vec4<i8>;
using Vec4i16 = Vec4<i16>;
using Vec4i32 = Vec4<i32>;
using Vec4i64 = Vec4<i64>;
using Vec4isz = Vec4<isz>;

using Vec4f32 = Vec4<f32>;
using Vec4f64 = Vec4<f64>;
