#pragma once
#include "types/types.hpp"

namespace oic {

	class Allocator {

	public:

		enum RangeHint {

			//Normally is on heap
			//Regularly used and normally behaves like malloc
			HEAP = 0,

			//Commit onto free space
			COMMIT = 1,

			//If set; will only reserve the range, not allocate
			//The reserved range has to be deallocated, not the allocated ones
			RESERVE = 2,

			//Whenever you need the memory on the free space right away
			//Commit only works if it has been reserved
			//Reserve makes sure it can't be reserved again
			COMMIT_RESERVE = COMMIT | RESERVE
		};

		virtual ~Allocator() {}

		template<typename T = u8, typename ...args>
		T *allocRange(usz address, usz count, RangeHint hint, args &...arg);

		template<typename T = u8, typename ...args>
		T *allocArray(usz count, args &...arg);

		//Allocate an object somewhere in memory
		//Initialize it with the params given
		template<typename T = u8, typename ...args>
		T *alloc(args &...arg);

		template<typename T = u8>
		void freeRange(T *&t, usz count);

		template<typename T>
		void free(T *&t);

		template<typename T>
		void freeArray(T *&t, usz count);

	protected:

		virtual void *alloc(usz size, RangeHint hint, usz addressHint = 0) = 0;
		virtual void free(void *v, usz size, bool isRange = false) = 0;

	};

	template<typename T, typename ...args>
	T *Allocator::alloc(args &...arg) {

		T *addr = (T*)alloc(sizeof(T), HEAP);

		if constexpr (std::is_class_v<T> || sizeof...(arg) != 0)
			::new T(addr)(arg...);

		return (T*)addr;
	}

	template<typename T, typename ...args>
	T *Allocator::allocArray(usz count, args &...arg) {

		T *addr = (T*)alloc(sizeof(T) * count, HEAP);

		if constexpr (std::is_class_v<T> || sizeof...(arg) != 0)
			for(usz i = 0; i < count; ++i)
				::new T(addr + i)(arg...);

		return addr;
	}

	template<typename T, typename ...args>
	T *Allocator::allocRange(usz address, usz count, RangeHint hint, args &...arg) {

		T *addr = (T*)alloc(sizeof(T) * count, hint, address);

		hint; ((arg), ...);

		if constexpr (std::is_class_v<T> || sizeof...(arg) != 0)
			if (!(hint & RESERVE))
				for(usz i = 0; i < count; ++i)
					::new T(addr + i)(arg...);

		return addr;
	}

	template<typename T>
	void Allocator::free(T *&t) {

		if constexpr (std::is_class_v<T> && std::is_destructible_v<T>)
			t->~T();

		free(t, sizeof(T));
		t = nullptr;
	}

	template<typename T>
	void Allocator::freeArray(T *&t, usz count) {

		if constexpr (std::is_class_v<T> && std::is_destructible_v<T>)
			for (usz i = 0; i < count; ++i)
				t[i].~T();

		free(t, sizeof(T) * count);
		t = nullptr;
	}

	template<typename T>
	void Allocator::freeRange(T *&t, usz count) {

		if constexpr (std::is_class_v<T> && std::is_destructible_v<T>)
			for (usz i = 0; i < count; ++i)
				t[i].~T();

		free(t, sizeof(T) * count, true);
		t = nullptr;
	}

}