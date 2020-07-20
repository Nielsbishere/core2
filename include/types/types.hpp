#pragma once
#include <cstdint>
#include <limits>

//Types

using i8  = std::int8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using u8  = std::uint8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using f32 = float;
using f64 = double;

using c8 = char;
using c16 = char16_t;

using usz = std::size_t;
using isz = std::ptrdiff_t;

static constexpr f64 PI_CONST = 3.141592653589793;
static constexpr f64 TO_DEG = 180 / PI_CONST;
static constexpr f64 TO_RAD = PI_CONST / 180;

//Casts

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

//Bytes & bits

constexpr usz KiB = 1024;
constexpr usz MiB = KiB * KiB;
constexpr usz GiB = KiB * MiB;
constexpr usz TiB = KiB * GiB;

constexpr usz Kib = 128;
constexpr usz Mib = Kib * Kib;
constexpr usz Gib = Kib * Mib;
constexpr usz Tib = Kib * Gib;

constexpr usz operator ""_KiB(unsigned long long test) { return (usz)test * KiB; }
constexpr usz operator ""_MiB(unsigned long long test) { return (usz)test * MiB; }
constexpr usz operator ""_GiB(unsigned long long test) { return (usz)test * GiB; }
constexpr usz operator ""_TiB(unsigned long long test) { return (usz)test * TiB; }

constexpr usz operator ""_Kib(unsigned long long test) { return (usz)test * Kib; }
constexpr usz operator ""_Mib(unsigned long long test) { return (usz)test * Mib; }
constexpr usz operator ""_Gib(unsigned long long test) { return (usz)test * Gib; }
constexpr usz operator ""_Tib(unsigned long long test) { return (usz)test * Tib; }

//Base10 constants

constexpr usz operator ""_K(unsigned long long test) { return (usz)test * 1'000; }
constexpr usz operator ""_M(unsigned long long test) { return (usz)test * 1'000'000; }
constexpr usz operator ""_B(unsigned long long test) { return (usz)test * 1'000'000'000; }

//Conversion to radians from degrees
constexpr f64 operator ""_deg(long double test) { return f64(test * TO_RAD); }
constexpr f64 operator ""_deg(unsigned long long test) { return i64(test) * TO_RAD; }

//Conversion to degrees from radians
constexpr f64 operator ""_rad(long double test) { return f64(test * TO_DEG); }
constexpr f64 operator ""_rad(unsigned long long test) { return i64(test) * TO_DEG; }

//Time

constexpr u64 operator ""_d(unsigned long long test) { return test * 8'640'000'000'000; }	//days to ns
constexpr u64 operator ""_h(unsigned long long test) { return test * 360'000'000'000; }		//hours to ns
constexpr u64 operator ""_m(unsigned long long test) { return test * 60'000'000'000; }		//mins to ns
constexpr u64 operator ""_s(unsigned long long test) { return test * 1'000'000'000; }		//seconds to ns
constexpr u64 operator ""_ms(unsigned long long test) { return test * 1'000'000; }			//mili seconds to ns
constexpr u64 operator ""_mus(unsigned long long test) { return test * 1'000; }				//micro seconds to ns
using ns = u64;

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

//Handling proper pointer destruction (prefer over delete)

template<typename T>
static inline void destroy(T *&ptr) {
	delete ptr;
	ptr = nullptr; 
}

template<typename ...args>
static inline void destroy(args *&...arg) {
	(destroy(arg), ...);
}

//Strings

#include <string>
#include <codecvt>

using String = std::string;
using WString = std::u16string;

static inline WString fromUTF8(const String &str) {
	return std::wstring_convert<std::codecvt_utf8_utf16<c16>, c16>().from_bytes(str);
}

static inline String fromUTF16(const WString &str) {
	return std::wstring_convert<std::codecvt_utf8_utf16<c16>, c16>().to_bytes(str);
}

//Helper if type is a string

template<typename T>
struct is_string {
	static constexpr bool value = false;
};

template<>
struct is_string<WString> {
	static constexpr bool value = true;
};

template<>
struct is_string<c16*> {
	static constexpr bool value = true;
};

template<usz N>
struct is_string<c16(&)[N]> {
	static constexpr bool value = true;
};

template<>
struct is_string<String> {
	static constexpr bool value = true;
};

template<>
struct is_string<c8*> {
	static constexpr bool value = true;
};

template<usz N>
struct is_string<c8(&)[N]> {
	static constexpr bool value = true;
};

template<typename T>
static constexpr bool is_string_v = is_string<T>::value;

//

//Containers

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

template<typename First, typename Second>
using Pair = std::pair<First, Second>;

#include <bitset>

template<usz siz>
using Bitset = std::bitset<siz>;

//Generate a signed version of the unsigned integer

template<typename T> struct Signed {};
template<> struct Signed<u8> { using v = i8; };
template<> struct Signed<u16> { using v = i16; };
template<> struct Signed<u32> { using v = i32; };
template<> struct Signed<u64> { using v = i64; };

template<typename T>
using Signed_v = typename Signed<T>::v;