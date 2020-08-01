#pragma once
#include "vec.hpp"
#include "system/log.hpp"
#include "system/system.hpp"

//Classes for handling plain data in 1D, 2D and 3D

namespace oic {

	//1D grid
	//x

	template<
		typename T,
		typename = std::enable_if_t<std::is_trivial_v<T> || std::is_arithmetic_v<T>>
	>
	class Grid1D {

		usz w{};
		T *data{};
		bool ownsData = true;

	public:

		inline usz size() const { return w; }
		inline usz dataSize() const { return w * sizeof(T); }

		inline const T *begin() const { return data; }
		inline const T *end() const { return data + w; }

		inline T *begin() { return data; }
		inline T *end() { return data + w; }

		Grid1D() {}
		~Grid1D() { 
			if(ownsData) delete[] data;
			data = nullptr; w = {};
		}
		
		Grid1D(usz w): w(w), data(new T[w]{}){}

		Grid1D(const u8 *buffer, usz bytes):
			ownsData(false), hw(bytes / sizeof(T)), data((T*)buffer)
		{
			if (bytes % sizeof(T))
				oic::System::log()->fatal("Invalid buffer size passed to Grid1D");
		}

		template<usz W, typename = std::enable_if_t<W != 0>>
		Grid1D(const T(&dat)[W]) : data(new T[W]), w(W) {
			std::memcpy(data, dat, dataSize());
		}

		template<template<typename> typename Arr>
		Grid1D(const Arr<T> &list) : 
			data(
				list.size() ? new T[list.size()] :
				nullptr
			), w(list.size()) {
			std::memcpy(data, list.data(), dataSize());
		}

		Grid1D(const Grid1D &g): w(g.w), data(g.data), ownsData(g.ownsData) {
			if (g.ownsData && g.data) {
				data = new T[g.w];
				std::memcpy(data, g.data, dataSize());
			}
		}

		Grid1D(Grid1D &&g): w(g.w), data(g.data), ownsData(g.ownsData) {
			g.data = nullptr;
			g.w = {};
		}

		inline Grid1D &operator=(const Grid1D &g) {

			if(ownsData)
				delete[] data;

			w = g.w;
			ownsData = g.ownsData;

			if (g.ownsData && g.data) {
				data = new T[w];
				std::memcpy(data, g.data, dataSize());
			} 
			else data = g.data;

			return *this;
		}

		inline Grid1D &operator=(Grid1D &&g) {

			if(ownsData)
				delete[] data;

			w = g.w;
			data = g.data;
			ownsData = g.ownsData;

			g.data = nullptr;
			g.w = {};

			return *this;
		}

		inline T &operator[](usz i)	{ return data[i]; }
		inline const T &operator[](usz i) const { return data[i]; }

		inline Buffer buffer() const { return Buffer((u8*)begin(), (u8*)end()); }
	};

	//2D Grid
	//y, x

	template<
		typename T,
		typename = std::enable_if_t<std::is_trivial_v<T> || std::is_arithmetic_v<T>>
	>
	class Grid2D {

		Vec2usz hw;
		T *data{};
		bool ownsData = true;

	public:

		inline auto linearSize() const { return hw[0] * hw[1]; }
		inline Vec2usz size() const { return hw; }
		inline usz dataSize() const { return linearSize() * sizeof(T); }

		inline const T *begin() const { return data; }
		inline const T *end() const { return data + linearSize(); }

		inline T *begin() { return data; }
		inline T *end() { return data + linearSize(); }

		Grid2D() {}
		~Grid2D() { 
			if(ownsData) delete[] data; 
			data = nullptr; hw = {};
		}

		Grid2D(Vec2usz hw): hw(hw), data(new T[hw[0] * hw[1]]{}){}

		Grid2D(const u8 *buffer, usz bytes, usz W):
			ownsData(false), hw(bytes / sizeof(T) / W, W), data((T*)buffer)
		{
			if (bytes % sizeof(T) || bytes % (sizeof(T) * W))
				oic::System::log()->fatal("Invalid buffer size passed to Grid2D");
		}

		template<usz H, usz W, typename = std::enable_if_t<W != 0 && H != 0>>
		Grid2D(const T(&dat)[H][W]) : hw(H, W), data(new T[H*W]) {
			std::memcpy(data, dat, dataSize());
		}

		template<template<typename> typename Arr>
		Grid2D(const Arr<T> &list, usz W) : 
			data(
				list.size() ? new T[list.size()] :
				nullptr
			), hw(list.size() / W, W) {

			if (linearSize() != list.size())
				oic::System::log()->fatal("Invalid list size in Grid2D");

			std::memcpy(data, list.data(), dataSize());
		}

		Grid2D(const Grid2D &g): hw(g.hw), data(g.data), ownsData(g.ownsData) {
			if (g.data && g.ownsData) {
				data = new T[g.linearSize()];
				std::memcpy(data, g.data, dataSize());
			}
		}

		Grid2D(Grid2D &&g): hw(g.hw), data(g.data), ownsData(g.ownsData) {
			g.data = nullptr;
			g.hw = {};
		}

		inline Grid2D &operator=(const Grid2D &g) {

			hw = g.hw;

			if(ownsData) 
				delete[] data;

			ownsData = g.ownsData;

			if (g.ownsData && g.data) {
				data = new T[linearSize()];
				std::memcpy(data, g.data, dataSize());
			}
			else data = g.data;

			return *this;
		}

		inline Grid2D &operator=(Grid2D &&g) {

			if(ownsData) 
				delete[] data;

			ownsData = g.ownsData;

			hw = g.hw;
			data = g.data;

			g.data = nullptr;
			g.hw = {};

			return *this;
		}

		inline usz linearIndex(const Vec2usz &yx) const {
			return yx[0] * hw[1] + yx[1];
		}

		inline T &operator[](const Vec2usz &yx) { return data[linearIndex(yx)]; }
		inline const T &operator[](const Vec2usz &yx) const	{ return data[linearIndex(yx)]; }

		inline Buffer buffer() const { return Buffer((u8*)begin(), (u8*)end()); }
	};

	//3D Grid
	//z, y, x

	template<
		typename T,
		typename = std::enable_if_t<std::is_trivial_v<T> || std::is_arithmetic_v<T>>
	>
	class Grid3D {

		Vec3usz lhw;
		T *data{};
		bool ownsData = true;

	public:

		inline auto linearSize() const { return lhw[0] * lhw[2] * lhw[3]; }
		inline Vec3usz size() const { return lhw; }
		inline usz dataSize() const { return linearSize() * sizeof(T); }

		inline const T *begin() const { return data; }
		inline const T *end() const { return data + linearSize(); }

		inline T *begin() { return data; }
		inline T *end() { return data + linearSize(); }

		Grid3D() {}
		~Grid3D() { 
			if(ownsData) delete[] data;
			data = nullptr; lhw = {};
		}

		Grid3D(Vec3usz lhw): lhw(lhw), data(new T[lhw[0] * lhw[1] * lwh[2]]{}){}

		Grid3D(const u8 *buffer, usz bytes, Vec2usz hw):
			ownsData(false), lhw(bytes / sizeof(T) / hw[0] / hw[1], hw[0], hw[1]), data((T*)buffer)
		{
			if (bytes % sizeof(T) || bytes % (sizeof(T) * hw[1]) || bytes % (sizeof(T) * hw[0] * hw[1]))
				oic::System::log()->fatal("Invalid buffer size passed to Grid3D");
		}

		template<usz H, usz W, usz L, typename = std::enable_if_t<W != 0 && H != 0 && L != 0>>
		Grid3D(const T(&dat)[L][H][W]) : 
			data(new T[L*H*W]), lhw{ L, H, W } {
			std::memcpy(data, dat, dataSize());
		}

		template<template<typename> typename Arr>
		Grid3D(const Arr<T> &list, usz W, usz H) : 
			data(
				list.size() ? new T[list.size()] :
				nullptr
			), lhw(list.size() / H / W, list.size() / W, W) {

			if (linearSize() != list.size())
				oic::System::log()->fatal("Invalid list size in Grid3D");

			std::memcpy(data, list.data(), dataSize());
		}

		Grid3D(const Grid3D &g): lhw(g.lhw), data(g.data) ownsData(g.ownsData) {
			if (g.ownsData && g.data) {
				data = new T[g.linearSize()];
				std::memcpy(data, g.data, dataSize());
			}
		}

		Grid3D(Grid3D &&g): lhw(g.lhw), data(g.data), ownsData(g.ownsData) {
			g.data = nullptr;
			g.lhw = {};
		}

		inline Grid3D &operator=(const Grid3D &g) {

			if(ownsData)
				delete[] data;

			lhw = g.lhw;
			ownsData = g.ownsData;

			if (g.data && g.ownsData) {
				data = new T[linearSize()];
				std::memcpy(data, g.data, dataSize());
			}
			else data = g.data;

			return *this;
		}

		inline Grid3D &operator=(Grid3D &&g) {

			if(ownsData)
				delete[] data;

			ownsData = g.ownsData;

			lhw = g.lhw;
			data = g.data;

			g.data = nullptr;
			g.lhw = {};

			return *this;
		}

		inline usz linearIndex(const Vec3usz &zyx) const {
			return zyx[0] * lhw[1] * lhw[2] + zyx[1] * lhw[2] + zyx[2];
		}

		inline T &operator[](const Vec3usz &zyx) { return data[linearIndex(zyx)]; }
		inline const T &operator[](const Vec3usz &zyx) const { return data[linearIndex(zyx)]; }

		inline Buffer buffer() const { return Buffer((u8*)begin(), (u8*)end()); }
	};

}