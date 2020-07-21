#pragma once
#include "flp.hpp"
#include "units.hpp"

using f16 = oic::flp<u16, 10, 5>;

constexpr u16 operator ""_f16(long double test) { return ((f16)(f32)test).value; }

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