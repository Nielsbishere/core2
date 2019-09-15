#pragma once
#include "types/types.hpp"

namespace oic {

	struct Hash {

		//Result of a hash as base64 string
		struct HashString {

			c8 c[12] {};
			operator String() { return c; }
		};

		//Generate 64-bit uint from string (constexpr)
		template<usz i>
		static inline constexpr u64 doHash(const c8 c[i]);

		//Generate readable 64-bit uint from string (constexpr)
		template<usz i>
		static inline constexpr HashString hash(const c8 c[i]);

		//Generate 64-bit uint from string
		static inline u64 doHash(const String &str);

		//Generate readable 64-bit uint from string
		static inline HashString hash(const String &str);

	};

	//Implementations

	template<usz i>
	inline constexpr u64 Hash::doHash(const c8 c[i]) {

		u64 hash = 0x406BA8208FCAB43F;

		for (const c8 cc : c)
			hash = ((hash << 5) + hash) + cc;

		return hash;
	}

	template<usz i>
	inline constexpr Hash::HashString Hash::hash(const c8 c[i]) {

		constexpr c8 mapping[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz*#";

		const u64 hash = doHash(c);

		HashString result {};

		for (usz j = 0; j < sizeof(HashString::c) - 1; ++j)
			result.c[j] = mapping[(hash >> (6 * j)) & 0x3F];

		return result;
	}

	inline u64 Hash::doHash(const String &str) {

		u64 hash = 0x406BA8208FCAB43F;

		for (const char cc : str)
			hash = ((hash << 5) + hash) + cc;

		return hash;
	}

	inline Hash::HashString Hash::hash(const String &str) {

		constexpr c8 mapping[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz*#";

		const u64 hash = doHash(str);

		HashString result {};

		for (usz j = 0; j < sizeof(HashString::c) - 1; ++j)
			result.c[j] = mapping[(hash >> (6 * j)) & 0x3F];

		return result;
	}
}

//For enabling hashing for obfuscation

#ifndef NDEBUG
	#define NAME(x) String(x)
#else
	#define NAME(x) String(oic::Hash::hash(x))
#endif