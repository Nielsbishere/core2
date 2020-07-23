#pragma once
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#include <cstdint>
#include <limits>
#include <string>
#include <codecvt>
#include <vector>
#include <array>
#include <unordered_map>
#include <bitset>

//Types

using i8  = std::int8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using u8  = std::uint8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using f64 = double;
using f32 = float;

using c8 = char;
using c16 = char16_t;
using c32 = char32_t;

using usz = std::size_t;
using isz = std::ptrdiff_t;

using ns = u64;

//Casts for constants

constexpr u8 operator ""_u8(unsigned long long test) { return (u8)test; }
constexpr i8 operator ""_i8(unsigned long long test) { return (i8)test; }

constexpr u16 operator ""_u16(unsigned long long test) { return (u16)test; }
constexpr i16 operator ""_i16(unsigned long long test) { return (i16)test; }

constexpr u32 operator ""_u32(unsigned long long test) { return (u32)test; }
constexpr i32 operator ""_i32(unsigned long long test) { return (i32)test; }

constexpr u64 operator ""_u64(unsigned long long test) { return (u64)test; }
constexpr i64 operator ""_i64(unsigned long long test) { return (i64)test; }

constexpr usz operator ""_usz(unsigned long long test) { return (usz)test; }
constexpr isz operator ""_isz(unsigned long long test) { return (isz)test; }

constexpr f32 operator ""_f32(long double test) { return (f32)test; }
constexpr f64 operator ""_f64(long double test) { return (f64)test; }

//Limits

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

constexpr usz usz_BYTES = sizeof(usz);
constexpr usz usz_BITS = usz_BYTES << 3;

constexpr usz usz_MIN = 0;
constexpr usz usz_MAX = usz(usz_BITS == 64 ? u64_MAX : u32_MAX);
constexpr isz isz_MIN = isz(usz_BITS == 64 ? i64_MIN : i32_MIN);
constexpr isz isz_MAX = isz(usz_BITS == 64 ? i64_MAX : i32_MAX);

constexpr usz usz_BYTE_SHIFT = usz_BYTES == 8 ? 3 : 2;	//1 << usz_BYTE_SHIFT == usz_BYTES
constexpr usz usz_BIT_SHIFT = usz_BYTES == 8 ? 6 : 5;	//1 << usz_BIT_SHIFT == usz_BYTES

//Constants

static constexpr f64 PI_CONST = 3.141592653589793;
static constexpr f64 TO_DEG = 180 / PI_CONST;
static constexpr f64 TO_RAD = PI_CONST / 180;

//Containers

using String = std::string;
using WString = std::u16string;

static inline WString fromUTF8(const String &str) {
	return std::wstring_convert<std::codecvt_utf8_utf16<c16>, c16>().from_bytes(str);
}

static inline String fromUTF16(const WString &str) {
	return std::wstring_convert<std::codecvt_utf8_utf16<c16>, c16>().to_bytes(str);
}

template<typename T>
using List = std::vector<T>;

using Buffer = std::vector<u8>;

template<typename T, usz siz>
using Array = std::array<T, siz>;

template<typename K, typename V>
using HashMap = std::unordered_map<K, V>;

template<typename First, typename Second>
using Pair = std::pair<First, Second>;

template<usz siz>
using Bitset = std::bitset<siz>;