#pragma once
#include "data_types.hpp"

namespace oic {

	//A reference to a pointer with a size
	//This can be used for things like serialization to specify there's a limit to where the array ends

	template<typename T>
	class ListRef {

		T *ptr;
		usz siz;

	public:

		ListRef(T *ptr, usz siz): ptr(ptr), siz(siz){}
		ListRef() : ListRef({}, 0) {}

		~ListRef() = default;
		ListRef(const ListRef&) = default;
		ListRef(ListRef&&) = default;
		ListRef &operator=(const ListRef&) = default;
		ListRef &operator=(ListRef&&) = default;

		inline bool operator==(const ListRef &o) const { return ptr == o.ptr && siz == o.siz; }
		inline bool operator!=(const ListRef &o) const { return !operator==(o); }

		inline usz size() const { return siz; }
		inline bool empty() const { return !siz; }

		inline T *begin() const { return ptr; }
		inline T *end() const { return ptr + siz; }

		inline T &operator[](const usz &i) const { return ptr[i]; }

		static constexpr bool isConst = std::is_const_v<T>;

	};

}