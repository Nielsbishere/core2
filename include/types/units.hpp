#pragma once
#include "data_types.hpp"

//Byte conversions

constexpr usz KiB = 1024;
constexpr usz MiB = KiB * KiB;
constexpr usz GiB = KiB * MiB;
constexpr usz TiB = KiB * GiB;

constexpr usz Kib = 128;
constexpr usz Mib = Kib * Kib;
constexpr usz Gib = Kib * Mib;
constexpr usz Tib = Kib * Gib;

constexpr usz KB = 1000;
constexpr usz MB = KB * KB;
constexpr usz GB = KB * MB;
constexpr usz TB = KB * GB;

constexpr usz Kb = 125;
constexpr usz Mb = Kb * Kb;
constexpr usz Gb = Kb * Mb;
constexpr usz Tb = Kb * Gb;

constexpr usz operator ""_KiB(unsigned long long test) { return (usz)test * KiB; }
constexpr usz operator ""_MiB(unsigned long long test) { return (usz)test * MiB; }
constexpr usz operator ""_GiB(unsigned long long test) { return (usz)test * GiB; }
constexpr usz operator ""_TiB(unsigned long long test) { return (usz)test * TiB; }

constexpr usz operator ""_Kib(unsigned long long test) { return (usz)test * Kib; }
constexpr usz operator ""_Mib(unsigned long long test) { return (usz)test * Mib; }
constexpr usz operator ""_Gib(unsigned long long test) { return (usz)test * Gib; }
constexpr usz operator ""_Tib(unsigned long long test) { return (usz)test * Tib; }

constexpr usz operator ""_KB(unsigned long long test) { return (usz)test * KB; }
constexpr usz operator ""_MB(unsigned long long test) { return (usz)test * MB; }
constexpr usz operator ""_GB(unsigned long long test) { return (usz)test * GB; }
constexpr usz operator ""_TB(unsigned long long test) { return (usz)test * TB; }

constexpr usz operator ""_Kb(unsigned long long test) { return (usz)test * Kb; }
constexpr usz operator ""_Mb(unsigned long long test) { return (usz)test * Mb; }
constexpr usz operator ""_Gb(unsigned long long test) { return (usz)test * Gb; }
constexpr usz operator ""_Tb(unsigned long long test) { return (usz)test * Tb; }

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