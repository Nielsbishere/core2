#pragma once
#include "types/types.hpp"

namespace oic {

	//Magic numbers of FNV-1a
	template<typename T> struct FNV {};

	template<> struct FNV<u64> { 
		static constexpr u64 prime = 0x00000100000001B3, offset = 0xcbf29ce484222325;
	};

	template<> struct FNV<u32> { 
		static constexpr u32 prime = 0x01000193, offset = 0x811c9dc5;
	};

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

		template<typename T>
		static inline void fnv1a(T &seed, T a);

		static inline u64 hash64(const u64 a, const u64 b, u64 seed = FNV<u64>::offset);
		static inline u32 hash32(const u32 a, const u32 b, u32 seed = FNV<u32>::offset);
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

	template<typename T>
	inline void Hash::fnv1a(T &seed, T a) {
		seed = (seed ^ a) * FNV<T>::prime;
	}

	inline u64 Hash::hash64(const u64 a, const u64 b, u64 seed) { 
		fnv1a(seed, a);
		fnv1a(seed, b);
		return seed;
	}

	inline u32 Hash::hash32(const u32 a, const u32 b, u32 seed) { 
		fnv1a(seed, a);
		fnv1a(seed, b);
		return seed;
	}
}

//For enabling hashing for obfuscation

#ifndef NDEBUG
	#define NAME(x) String(x)
	#define VIRTUAL_FILE(x) String(x)
#else
	#define NAME(x) String(oic::Hash::hash(x))
	#define VIRTUAL_FILE(x) String(x)						//TODO:
#endif