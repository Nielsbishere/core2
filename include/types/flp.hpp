#pragma once
#include "template_helpers.hpp"

namespace oic {

	//IEEE754 Floating points
	//TODO: Operator overloading

	template<typename T, usz mantissaBits, usz exponentBits>
	struct flp {

		static constexpr u64
			myExponentBits = exponentBits,
			myMantissaBits = mantissaBits,
			signOff = mantissaBits + exponentBits,
			exponentMask = (T(1) << exponentBits) - 1,
			mantissaMask = (T(1) << mantissaBits) - 1;

		static constexpr int_with_size_of_t<T> exponentStart = (1 << (exponentBits - 1)) - 1;

		T value;

		constexpr inline T mantissa() const { return value & mantissaMask; }
		constexpr inline T rawExponent() const { return (value >> mantissaBits) & exponentMask; }
		constexpr inline auto exponent() const { return (int_with_size_of_t<T>) rawExponent() - exponentStart; }
		constexpr inline bool sign() const { return value >> signOff; }

		constexpr inline void clearSign() { value &= ~(T(1) << signOff); }
		constexpr inline void setSign() { value |= (T(1) << signOff); }
		constexpr inline void sign(bool b) { if (b) setSign(); else clearSign(); }

		constexpr inline void mantissa(T mantissa) { value &= ~mantissaBits; value |= mantissa; }
		constexpr inline void rawExponent(T exponent) { value &= ~(exponentMask << mantissaBits); value |= exponent << mantissaBits; }

		constexpr inline flp(const flp &other) : value(other.value) {}
		constexpr inline flp(flp &&other) : value(other.value) {}

		constexpr inline flp &operator=(const flp &other) { value = other.value; return *this; }
		constexpr inline flp &operator=(flp &&other) { value = other.value; return *this; }

		template<template<typename T2, usz, usz> typename Tflp, typename T2, usz Mantissa, usz Exp>
		constexpr inline flp(const Tflp<T2, Mantissa, Exp> &val) : value {} { _init(val); }

		template<typename T2, typename = std::enable_if_t<std::is_floating_point_v<T2>>>
		constexpr inline flp(const T2 &_value): value{} { _init(*(const flp<uint_with_size_of_t<T2>, flp_mantissa_of<T2>, flp_exponent_of<T2>>*) &_value); }

		constexpr inline flp(): value{} {}

		constexpr inline T &getValue() { return value; }
		constexpr inline const T &getValue() const { return value; }

		//Cast to bigger type

		template<template<typename T2, usz, usz> typename Tflp, typename T2, usz Mantissa, usz Exp>
		constexpr inline Tflp<T2, Mantissa, Exp> cast(const Tflp<T2, Mantissa, Exp>&) const {

			static_assert(Mantissa >= mantissaBits && Exp >= exponentBits, "Can only convert up to more precise types, use the constructor otherwise");

			Tflp<T2, Mantissa, Exp> res{};

			//Maintain sign

			res.sign(sign());

			//NaN

			if (rawExponent() == exponentMask && mantissa())
				res.value |= (res.exponentMask << Mantissa) | res.mantissaMask;

			//Inf

			else if (rawExponent() == exponentMask)
				res.value |= res.exponentMask << Mantissa;

			//Only calculate if not -0 or 0

			else if (mantissa() != 0 || rawExponent() != 0) {
				T2 mant = T2(mantissa()) << (res.myMantissaBits - mantissaBits);
				T2 exp = T2((exponent() + res.exponentStart) << Mantissa);
				res.value |= mant | exp;
			}

			return res;
		}

		template<typename T>
		constexpr inline T cast() const {
			return cast(T{});
		}

		template<typename T2, typename = std::enable_if_t<std::is_floating_point_v<T2>>>
		constexpr inline operator T2() const {
			auto res = cast<flp<uint_with_size_of_t<T2>, flp_mantissa_of<T2>, flp_exponent_of<T2>>>();
			return *(const T2*) &res.value;
		}

	private:

		template<template<typename T2, usz, usz> typename Tflp, typename T2, usz Mantissa, usz Exp>
		constexpr inline void _init(const Tflp<T2, Mantissa, Exp> &val) {

			static_assert(Mantissa >= mantissaBits && Exp >= exponentBits, "Can only convert down from more precise types, use the cast otherwise");

			//Maintain sign

			sign(val.sign());

			//Zero, so we can easily skip some checks

			if (val.mantissa() == 0 && val.rawExponent() == 0)
				return;

			//Calculate new exponents and mantissa

			auto vmantissa = val.mantissa();
			auto vexponent = val.exponent();

			auto normalizedExponent = vexponent + exponentStart;

			//A number so small it can't be contained, collapses to 0 or -0
			if (normalizedExponent < 0)
				return;

			//NaN
			else if (val.rawExponent() == val.exponentMask && vmantissa) {
				value |= (exponentMask << mantissaBits) | mantissaMask;
				return;
			}

			//A number so big it can't be contained, collapses to inf or -inf
			else if (normalizedExponent >= exponentMask - 1 && (normalizedExponent >= exponentMask || vmantissa > mantissaMask << (val.myMantissaBits - mantissaBits))) {
				value |= exponentMask << mantissaBits;
				return;
			}

			//Regular numbers or DeN
			value |= (T(normalizedExponent) << mantissaBits) | (vmantissa >> (val.myMantissaBits - mantissaBits));
		}

	};

}