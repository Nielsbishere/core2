#pragma once
#include "data_types.hpp"

namespace oic {
	//Generate a signed version of the unsigned integer

	template<typename T> struct Signed {};
	template<> struct Signed<u8> { using v = i8; };
	template<> struct Signed<u16> { using v = i16; };
	template<> struct Signed<u32> { using v = i32; };
	template<> struct Signed<u64> { using v = i64; };

	template<typename T>
	using Signed_v = typename Signed<T>::v;

	template<typename T, usz mantissaBits, usz exponentBits>
	struct flp;

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

	//Floating point properties

	template<typename T>
	struct flp_properties_of {};

	template<typename T, usz mantissaBits, usz exponentBits>
	struct flp_properties_of<oic::flp<T, mantissaBits, exponentBits>> { static constexpr usz mantissa = mantissaBits, exponent = exponentBits; };

	template<>
	struct flp_properties_of<f32> { static constexpr usz mantissa = 23, exponent = 8; };

	template<>
	struct flp_properties_of<f64> { static constexpr usz mantissa = 52, exponent = 11; };

	template<typename T>
	static constexpr usz flp_mantissa_of = flp_properties_of<T>::mantissa;

	template<typename T>
	static constexpr usz flp_exponent_of = flp_properties_of<T>::exponent;

	//Get signed or unsigned ints from size (used for flps)

	template<usz T>
	struct uint_with_size_of {};

	template<usz T>
	struct int_with_size_of {};

	template<typename T>
	using uint_with_size_of_t = typename uint_with_size_of<sizeof(T)>::type;

	template<typename T>
	using int_with_size_of_t = typename int_with_size_of<sizeof(T)>::type;

	template<> struct uint_with_size_of<1> { using type = u8; };
	template<> struct uint_with_size_of<2> { using type = u16; };
	template<> struct uint_with_size_of<4> { using type = u32; };
	template<> struct uint_with_size_of<8> { using type = u64; };
	template<> struct int_with_size_of<1> { using type = i8; };
	template<> struct int_with_size_of<2> { using type = i16; };
	template<> struct int_with_size_of<4> { using type = i32; };
	template<> struct int_with_size_of<8> { using type = i64; };

}