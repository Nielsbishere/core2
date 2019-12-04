#pragma once
#include "types.hpp"
#include "utils/math.hpp"
#include "system/system.hpp"
#include "system/log.hpp"

namespace oic {

	//Optimal bitset implementation

	template<usz count, typename SizeType = usz, typename = std::enable_if<std::is_arithmetic_v<SizeType>>>
	struct Bitset {

	public:

		static constexpr usz
			sizeType = sizeof(SizeType) * 8,
			bufferSize = usz(Math::max(Math::ceil(count / f64(sizeType)), 1.0)),
			bufferEnd = bufferSize - 1,
			unusedBits = bufferSize * sizeType - count;

		static constexpr SizeType
			mask = SizeType((1 << (count % sizeType)) - 1),
			unusedMask = SizeType((1 << (sizeType - unusedBits)) - 1);

		Bitset();
		explicit Bitset(bool b);
		Bitset(SizeType value);

		template<usz i = 0, typename T, typename ...args>
		explicit inline Bitset(T v, args... arg): data{ SizeType(v), SizeType(arg)... } { }

		template<typename ...args>
		explicit inline Bitset(bool b, args... arg): data{} {

			if constexpr (sizeof...(args) > count)
				static_assert("Too many identifiers specified");

			set<count - 1>(b, arg...);
		}

		Bitset(const Bitset &other) = default;
		Bitset(Bitset &&other) = default;
		Bitset &operator=(const Bitset &other) = default;
		Bitset &operator=(Bitset &&other) = default;

		bool operator==(const Bitset &other) const;
		bool operator!=(const Bitset &other) const;
		bool operator>=(const Bitset &other) const;
		bool operator<=(const Bitset &other) const;
		bool operator>(const Bitset &other) const;
		bool operator<(const Bitset &other) const;

		Bitset &operator++();
		Bitset &operator--();

		bool operator||(const Bitset &other) const;
		bool operator&&(const Bitset &other) const;

		Bitset operator>>(usz shift) const;
		Bitset operator<<(usz shift) const;
		Bitset &operator>>=(usz shift);
		Bitset &operator<<=(usz shift);

		//TODO: +, -, *, /, %

		SizeType &at(usz i);
		SizeType at(usz i) const;
		bool operator[](usz i) const;

		void clear(usz i);
		void set(usz i);
		void set(usz i, bool b);

		void clear();
		void set();

		bool nonZero() const;
		bool operator!() const;

		Bitset operator~() const;
		Bitset &not();

		Bitset &operator&=(const Bitset &b);
		Bitset &operator|=(const Bitset &b);
		Bitset &operator^=(const Bitset &b);

		Bitset operator&(const Bitset &b) const;
		Bitset operator|(const Bitset &b) const;
		Bitset operator^(const Bitset &b) const;

		template<typename T, typename = std::enable_if<std::is_unsigned_v<T> && std::is_integral_v<T>>>
		explicit operator T() const {

			if constexpr (bufferSize != 1)
				static_assert("operator T() is only valid if the number of bits is less than the represented data type's bits");

			return T(data[0]);
		}

	protected:

		template<usz i, typename ...args>
		inline void set(bool b, args... arg) {
			set(i, b);
			set<i - 1>(arg...);
		}

		template<usz i>
		inline void set(bool b) {
			set(i, b);
		}

	private:

		SizeType data[bufferSize];

	};

	//Sized bitsets
	//Only the Bitset8 is guaranteed to maintain big endian values
	//Regular bitset is optimized for speed, not storage

	template<usz count>
	using Bitset8 = Bitset<count, u8>;

	template<usz count>
	using Bitset16 = Bitset<count, u16>;

	template<usz count>
	using Bitset32 = Bitset<count, u32>;

	template<usz count>
	using Bitset64 = Bitset<count, u64>;

	//Implementation

	template<usz count, typename SizeType, typename V>
	Bitset<count, SizeType, V>::Bitset(SizeType value): data { SizeType(value & mask) } {

		if constexpr (bufferSize != 1)
			static_assert("Bitset(SizeType) is only valid if the number of bits is less than the represented data type's bits");

		if (value > mask)
			System::log()->fatal("The Bitset didn't have enough bits to represent the value");

	}

	template<usz count, typename SizeType, typename V>
	SizeType &Bitset<count, SizeType, V>::at(usz i) {

		if (i >= bufferSize)
			System::log()->fatal("The index was out of bounds");

		return data[i];
	}

	template<usz count, typename SizeType, typename V>
	SizeType Bitset<count, SizeType, V>::at(usz i) const {
		return ((Bitset*) this)->at(i);
	}

	template<usz count, typename SizeType, typename V>
	bool Bitset<count, SizeType, V>::operator[](usz i) const {

		if (i >= count)
			System::log()->fatal("The index was out of bounds");

		const SizeType &v = data[bufferEnd - i / sizeType];
		const SizeType bmask = 1 << (i % sizeType);

		return v & bmask;
	}

	template<usz count, typename SizeType, typename V>
	void Bitset<count, SizeType, V>::clear(usz i) {

		if (i >= count)
			System::log()->fatal("The index was out of bounds");

		SizeType &v = data[bufferEnd - i / sizeType];
		const SizeType bmask = 1 << (i % sizeType);

		v &= ~bmask;
	}

	template<usz count, typename SizeType, typename V>
	void Bitset<count, SizeType, V>::set(usz i) {

		if (i >= count)
			System::log()->fatal("The index was out of bounds");

		SizeType &v = data[bufferEnd - i / sizeType];
		const SizeType bmask = 1 << (i % sizeType);

		v |= bmask;
	}

	template<usz count, typename SizeType, typename V>
	void Bitset<count, SizeType, V>::set(usz i, bool b) {

		if (i >= count)
			System::log()->fatal("The index was out of bounds");

		SizeType &v = data[bufferEnd - i / sizeType];
		const SizeType bmask = 1 << (i % sizeType);

		v &= ~bmask;		//Clear
		v |= b * bmask;		//Set

	}

	template<usz count, typename SizeType, typename V>
	void Bitset<count, SizeType, V>::set() {
		memset(data, 0xFF, sizeof(data));
		data[0] &= unusedMask;
	}

	template<usz count, typename SizeType, typename V>
	void Bitset<count, SizeType, V>::clear() {
		memset(data, 0x00, sizeof(data));
	}

	template<usz count, typename SizeType, typename V>
	Bitset<count, SizeType, V>::Bitset(): data {} {}

	template<usz count, typename SizeType, typename V>
	Bitset<count, SizeType, V>::Bitset(bool b) : data {} {
		if (b)
			set();
	}

	template<usz count, typename SizeType, typename V>
	bool Bitset<count, SizeType, V>::nonZero() const {
		return *this != Bitset();
	}

	template<usz count, typename SizeType, typename V>
	bool Bitset<count, SizeType, V>::operator!() const {
		return *this == Bitset();
	}

	template<usz count, typename SizeType, typename V>
	bool Bitset<count, SizeType, V>::operator==(const Bitset &other) const {
		return memcmp(data, other.data, sizeof(data)) == 0;
	}

	template<usz count, typename SizeType, typename V>
	bool Bitset<count, SizeType, V>::operator!=(const Bitset &other) const {
		return memcmp(data, other.data, sizeof(data)) != 0;
	}

	template<usz count, typename SizeType, typename V>
	Bitset<count, SizeType, V> &Bitset<count, SizeType, V>::operator&=(const Bitset &b) {

		for (usz i = 0; i < bufferSize; ++i)
			data[i] &= b.data[i];

		return *this;
	}

	template<usz count, typename SizeType, typename V>
	Bitset<count, SizeType, V> &Bitset<count, SizeType, V>::operator|=(const Bitset &b) {

		for (usz i = 0; i < bufferSize; ++i)
			data[i] |= b.data[i];

		return *this;
	}

	template<usz count, typename SizeType, typename V>
	Bitset<count, SizeType, V> &Bitset<count, SizeType, V>::operator^=(const Bitset &b) {

		for (usz i = 0; i < bufferSize; ++i)
			data[i] ^= b.data[i];

		return *this;
	}

	template<usz count, typename SizeType, typename V>
	Bitset<count, SizeType, V> Bitset<count, SizeType, V>::operator&(const Bitset &b) const {

		Bitset result = *this;

		for (usz i = 0; i < bufferSize; ++i)
			result.data[i] &= b.data[i];

		return result;
	}

	template<usz count, typename SizeType, typename V>
	Bitset<count, SizeType, V> Bitset<count, SizeType, V>::operator|(const Bitset &b) const {

		Bitset result = *this;

		for (usz i = 0; i < bufferSize; ++i)
			result.data[i] |= b.data[i];

		return result;
	}

	template<usz count, typename SizeType, typename V>
	Bitset<count, SizeType, V> Bitset<count, SizeType, V>::operator^(const Bitset &b) const {

		Bitset result = *this;

		for (usz i = 0; i < bufferSize; ++i)
			result.data[i] ^= b.data[i];

		return result;
	}

	template<usz count, typename SizeType, typename V>
	Bitset<count, SizeType, V> Bitset<count, SizeType, V>::operator~() const {
		Bitset result = *this;
		result.not();
		return result;
	}

	template<usz count, typename SizeType, typename V>
	Bitset<count, SizeType, V> &Bitset<count, SizeType, V>::not() {

		for (usz i = 0; i < bufferSize; ++i)
			data[i] = ~data[i];

		return *this;
	}

	template<usz count, typename SizeType, typename V>
	bool Bitset<count, SizeType, V>::operator<=(const Bitset &other) const {

		if constexpr(std::is_same_v<SizeType, u8>)
			return memcmp(data, other.data, sizeof(data)) <= 0;
		else {
			for (usz i = 0; i < bufferSize; ++i)
				if (data[i] > other.data[i])
					return false;
				else if (data[i] < other.data[i])
					return true;

			return true;
		}
	}

	template<usz count, typename SizeType, typename V>
	bool Bitset<count, SizeType, V>::operator>=(const Bitset &other) const {

		if constexpr (std::is_same_v<SizeType, u8>)
			return memcmp(data, other.data, sizeof(data)) >= 0;
		else {
			for (usz i = 0; i < bufferSize; ++i)
				if (data[i] < other.data[i])
					return false;
				else if (data[i] > other.data[i])
					return true;

			return true;
		}
	}

	template<usz count, typename SizeType, typename V>
	bool Bitset<count, SizeType, V>::operator<(const Bitset &other) const {

		if constexpr(std::is_same_v<SizeType, u8>)
			return memcmp(data, other.data, sizeof(data)) <= 0;
		else {
			for (usz i = 0; i < bufferSize; ++i)
				if (data[i] > other.data[i])
					return false;
				else if (data[i] < other.data[i])
					return true;

			return false;
		}
	}

	template<usz count, typename SizeType, typename V>
	bool Bitset<count, SizeType, V>::operator>(const Bitset &other) const {

		if constexpr (std::is_same_v<SizeType, u8>)
			return memcmp(data, other.data, sizeof(data)) >= 0;
		else {
			for (usz i = 0; i < bufferSize; ++i)
				if (data[i] < other.data[i])
					return false;
				else if (data[i] > other.data[i])
					return true;

			return false;
		}
	}

	template<usz count, typename SizeType, typename V>
	bool Bitset<count, SizeType, V>::operator||(const Bitset &other) const {
		return nonZero() || other.nonZero();
	}

	template<usz count, typename SizeType, typename V>
	bool Bitset<count, SizeType, V>::operator&&(const Bitset &other) const {
		return nonZero() && other.nonZero();
	}

	template<usz count, typename SizeType, typename V>
	Bitset<count, SizeType, V> &Bitset<count, SizeType, V>::operator++() {

		for (usz i = bufferEnd; i != usz_MAX; --i)
			if (++data[i])		//Add till the carry isn't set
				return *this;

		return *this;
	}

	template<usz count, typename SizeType, typename V>
	Bitset<count, SizeType, V> &Bitset<count, SizeType, V>::operator--() {

		for (usz i = bufferEnd; i != usz_MAX; --i)
			if (--data[i] != usz_MAX)		//Remove till the carry isn't set
				return *this;

		return *this;
	}

	template<usz count, typename SizeType, typename V>
	Bitset<count, SizeType, V> Bitset<count, SizeType, V>::operator>>(usz shift) const {

		Bitset result;

		usz q{};
		usz o = shift / sizeType;
		usz f = shift % sizeType;

		if (o < bufferSize)
			result.data[o] = data[q] >> f;

		++o;
		++q;

		SizeType bmask = SizeType((1 << f) - 1);

		for (; o < bufferSize; ++o, ++q)
			result.data[o] = ((data[q - 1] & bmask) << f) | (data[q] >> f);

		return result;
	}

	template<usz count, typename SizeType, typename V>
	Bitset<count, SizeType, V> &Bitset<count, SizeType, V>::operator>>=(usz shift) {
		return *this = operator>>(shift);
	}

	template<usz count, typename SizeType, typename V>
	Bitset<count, SizeType, V> Bitset<count, SizeType, V>::operator<<(usz shift) const {

		Bitset result;

		usz q = shift / sizeType;
		usz o{};
		usz f = shift % sizeType;

		SizeType bmask = SizeType((1 << f) - 1);

		for (; q < bufferEnd; ++o, ++q)
			result.data[o] = f == 0 ? data[q] : ((data[q] & bmask) << f) | (data[q + 1] >> f);

		if(q < bufferSize)
			result.data[o] = data[bufferEnd] << f;

		return result;
	}

	template<usz count, typename SizeType, typename V>
	Bitset<count, SizeType, V> &Bitset<count, SizeType, V>::operator<<=(usz shift) {
		return *this = operator<<(shift);
	}

}