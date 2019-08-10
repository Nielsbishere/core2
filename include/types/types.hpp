#pragma once
#include <stdint.h>
#include <limits>
#include <ctime>

using i8 = int8_t;
using u8 = uint8_t;

using i16 = int16_t;
using u16 = uint16_t;

using i32 = int32_t;
using u32 = uint32_t;

using i64 = int64_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

using c8 = char;
using c16 = char16_t;
using c32 = char32_t;

using usz = size_t;

constexpr u8 operator ""_u8(unsigned long long test) { return (u8)test; }
constexpr i8 operator ""_i8(unsigned long long test) { return (i8)test; }

constexpr u16 operator ""_u16(unsigned long long test) { return (u16)test; }
constexpr i16 operator ""_i16(unsigned long long test) { return (i16)test; }

constexpr u32 operator ""_u32(unsigned long long test) { return (u32)test; }
constexpr i32 operator ""_i32(unsigned long long test) { return (i32)test; }

constexpr u64 operator ""_u64(unsigned long long test) { return (u64)test; }
constexpr i64 operator ""_i64(unsigned long long test) { return (i64)test; }

constexpr usz operator ""_usz(unsigned long long test) { return (usz)test; }

constexpr f32 operator ""_f32(long double test) { return (f32)test; }
constexpr f64 operator ""_f64(long double test) { return (f64)test; }

constexpr usz KiB = 1024;
constexpr usz MiB = KiB * KiB;
constexpr usz GiB = KiB * MiB;
constexpr usz TiB = KiB * GiB;

constexpr usz operator ""_KiB(unsigned long long test) { return (usz)test * KiB; }
constexpr usz operator ""_MiB(unsigned long long test) { return (usz)test * MiB; }
constexpr usz operator ""_GiB(unsigned long long test) { return (usz)test * GiB; }
constexpr usz operator ""_TiB(unsigned long long test) { return (usz)test * TiB; }

constexpr u8 u8_MAX = 0xFF_u8;
constexpr u8 u8_MIN = 0_u8;
constexpr u16 u16_MAX = 0xFFFF_u16;
constexpr u16 u16_MIN = 0_u16;
constexpr u32 u32_MAX = 0xFFFFFFFF_u32;
constexpr u32 u32_MIN = 0_u32;
constexpr u64 u64_MAX = 0xFFFFFFFFFFFFFFFF_u64;
constexpr u64 u64_MIN = 0_u64;

constexpr i8 i8_MAX = 0x7F_i8;
constexpr i8 i8_MIN = 0x80_i8;
constexpr i16 i16_MAX = 0x7FFF_i16;
constexpr i16 i16_MIN = 0x8000_i16;
constexpr i32 i32_MAX = 0x7FFFFFFF_i32;
constexpr i32 i32_MIN = 0x80000000_i32;
constexpr i64 i64_MAX = 0x7FFFFFFFFFFFFFFF_i64;
constexpr i64 i64_MIN = 0x8000000000000000_i64;

constexpr f32 f32_MIN = std::numeric_limits<f32>::min();
constexpr f32 f32_MAX = std::numeric_limits<f32>::max();
constexpr f64 f64_MIN = std::numeric_limits<f64>::min();
constexpr f64 f64_MAX = std::numeric_limits<f64>::max();

constexpr usz usz_MIN = 0;
constexpr usz usz_MAX = usz(sizeof(usz) == 8 ? u64_MAX : u32_MAX);		//if usz is u32, will cut from u64_MAX to u32_MAX

class gbool {

public:

	gbool(u32 val) : val(val) {}
	gbool(bool b = false) : val(b) {}

	operator bool() { return val != 0U; }

private:

	u32 val;

};

template<typename T>
static inline void destroy(T *&ptr) {
	if (ptr) {
		delete ptr;
	    ptr = nullptr; 
    }
}

//Temporary; Might get replaced by custom types

#include <string>

using String = std::string;

#include <vector>

template<typename T>
using List = std::vector<T>;

using Buffer = std::vector<u8>;

#include <array>

template<typename T, usz siz>
using Array = std::array<T, siz>;

#include <unordered_map>

template<typename K, typename V>
using HashMap = std::unordered_map<K, V>;

//Generate a signed version of the unsigned integer

template<typename T> struct Signed {};
template<> struct Signed<u8> { using v = i8; };
template<> struct Signed<u16> { using v = i16; };
template<> struct Signed<u32> { using v = i32; };
template<> struct Signed<u64> { using v = i64; };

template<typename T>
using Signed_v = typename Signed<T>::v;