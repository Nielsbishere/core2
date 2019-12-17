#pragma once
#include "vec.hpp"

//Helper for generating matrices
//All rotations and fovs are in radians

//Matrix storage 

template<typename T, usz W, usz H>
struct TMatStorage {
	union {
		T f[W * H];
		T m[W][H];
		Vec<T, W> axes[H];
	};
	TMatStorage() : f {} {}
};

template<typename T>
struct TMatStorage<T, 2, 2> {
	union {
		T f[4];
		T m[2][2];
		Vec2<T> axes[2];
		struct { Vec2<T> x, y; };
	};

	TMatStorage() : f {} {}
};

template<typename T>
struct TMatStorage<T, 3, 3> {
	union {

		T f[9];
		T m[3][3];
		Vec3<T> axes[3];

		struct {
			Vec3<T> xAxis, yAxis, zAxis;
		};
		struct { 
			Vec2<T> x; T wx; 
			Vec2<T> y; T wy;
			Vec2<T> pos; T one;
		};
	};

	TMatStorage() : f {} {}
};

template<typename T>
struct TMatStorage<T, 4, 4> {
	union {

		T f[16];
		T m[4][4];
		Vec4<T> axes[4];

		struct {
			Vec4<T> xAxis, yAxis, zAxis, pos4;
		};
		struct { 
			Vec3<T> x; T wx; 
			Vec3<T> y; T wy;
			Vec3<T> z; T wz;
			Vec3<T> pos; T one;
		};
	};

	TMatStorage() : f {} {}
};

//Col major matrix base

template<typename T, usz W, usz H>
struct Mat : public TMatStorage<T, W, H> {

	//Constants and types

	static constexpr usz N = W * H, diagonalN = W < H ? W : H;

	using Type = T;
	using Diagonal = Vec<T, diagonalN>;
	using Horizontal = Vec<T, W>;
	using Vertical = Vec<T, H>;

	using TMatStorage<T, W, H>::m;
	using TMatStorage<T, W, H>::f;
	using TMatStorage<T, W, H>::axes;

	//Constructors

	Mat(const Mat&) = default;
	Mat(Mat&&) = default;
	Mat &operator=(const Mat&) = default;
	Mat &operator=(Mat&&) = default;

	//Identity or scale matrix
	constexpr inline Mat(const Diagonal &scale = 1) {
		for (usz i = 0; i < diagonalN; ++i) m[i][i] = scale[i];
	}

	//Value matrix
	constexpr inline Mat(const T &t) {
		for (usz i = 0; i < N; ++i) f[i] = t;
	}

	//TODO: Constructor with T[N] and T[W][H] and T, T, T, ...

	//Empty matrix
	static constexpr inline Mat nil() { return Mat(0); }

	//Access to rows, cols and values

	static constexpr usz horizontal() { return W; }
	static constexpr usz vertical() { return H; }

	Horizontal &operator[](const usz y) { return axes[y]; }
	const Horizontal &operator[](const usz y) const { return axes[y]; }

	T &operator[](const Vec2usz &xy) { return m[xy.y][xy.x]; }
	const T &operator[](const Vec2usz &xy) const { return axes[xy.y][xy.x]; }

	Vertical getVertical(const usz x) const {
		Vertical res;
		for (usz i = 0; i < W; ++i) res[i] = m[i][x];
		return res;
	}

	//Arithmetic overloads

	Mat &operator+=(const Mat &other) {
		for (usz i = 0; i < H; ++i) axes[i] += other.axes[i];
		return *this;
	}

	Mat &operator-=(const Mat &other) {
		for (usz i = 0; i < H; ++i) axes[i] -= other.axes[i];
		return *this;
	}

	Mat &operator/=(const Mat &other) {
		for (usz i = 0; i < H; ++i) axes[i] /= other.axes[i];
		return *this;
	}

	Mat &operator%=(const Mat &other) {
		for (usz i = 0; i < H; ++i) axes[i] %= other.axes[i];
		return *this;
	}

	Mat operator+(const Mat &other) const { return Mat(*this) += other; }
	Mat operator-(const Mat &other) const { return Mat(*this) -= other; }
	Mat operator/(const Mat &other) const { return Mat(*this) /= other; }
	Mat operator%(const Mat &other) const { return Mat(*this) %= other; }

	//Since Matrix multiply is different, mulVal can be used to perform regular multiplications on the values

	Mat &mulVal(const Mat &other) {
		for (usz i = 0; i < H; ++i) axes[i] *= other.axes[i];
		return *this;
	}

	static inline Mat mulVal(const Mat &a, const Mat &b) { return Mat(a).mulVal(b); }
	static inline Mat scale(const Diagonal &diag) { return Mat(diag); }

	//Comparison

	constexpr bool operator==(const Mat &other) const {
		return std::memcmp(f, other.f, sizeof(other)) == 0;
	}

	constexpr bool operator!=(const Mat &other) const {
		return std::memcmp(f, other.f, sizeof(other));
	}

	//Matrix math

	//TODO: Inverse, determinant
	//TODO: Transform vector
	//TODO: Matrix multiplication

	constexpr inline Mat<T, H, W> transpose() const {

		Mat<T, H, W> res{};

		for (usz j = 0; j < H && j < W; ++j)
			for (usz i = 0; i < W && i < H; ++i)
				res.m[i][j] = m[j][i];

		return res;
	}

	//Helpers

	inline Buffer toData() const {
		return Buffer((u8*)f, (u8*)(f + N));
	}

	template<typename T2, usz W2, usz H2>
	constexpr inline Mat<T2, W2, H2> cast() const {

		Mat<T2, W2, H2> res{};

		for (usz j = 0; j < H && j < H2; ++j)
			for (usz i = 0; i < W && i < W2; ++i)
				res.m[j][i] = T2(m[j][i]);

		return res;
	}

	template<typename T2>
	constexpr inline T2 cast() const {

		T2 res{};

		for (usz j = 0; j < H && j < T2::vertical(); ++j)
			for (usz i = 0; i < W && i < T2::horizontal(); ++i)
				res.m[j][i] = typename T2::Type(m[j][i]);

		return res;
	}

};

//Helper functions that carry to multiple types of matrices
template<typename Mat, typename T>
struct TMatHelper {

	static constexpr inline Mat rotateZ(T v) {

		static_assert(
			std::is_floating_point_v<typename Mat::Type>, 
			"Can't call TMatHelper<Mat>::rotateZ if T isn't a floating point"
		);

		static_assert(
			Mat::horizontal() > 1 && Mat::vertical() > 1, 
			"Can't call TMatHelper<Mat>::rotateZ if Mat's dimensions are less than 2x2"
		);

		Mat res{};
		res[0][0] = std::cos(v);	res[0][1] = -std::sin(v);
		res[1][0] = std::sin(v);	res[1][1] = std::cos(v);
		return res;
	}
};

//2x2 matrix

template<typename T>
struct Mat2x2 : public Mat<T, 2, 2> {

	using Mat<T, 2, 2>::Mat;

	constexpr inline Mat2x2(const Mat<T, 2, 2> &dat) : Mat<T, 2, 2>(dat) {}
	constexpr inline Mat2x2(Mat<T, 2, 2> &&dat) : Mat<T, 2, 2>(dat) {}

	//Rotate z
	static constexpr inline Mat2x2 rotateZ(T v) { return TMatHelper<Mat2x2, T>::rotateZ(v); }

	//Pad to vec4s
	inline Buffer toGPUData() const {

		Buffer result(0x20);	//2 vec4s

		std::memcpy(result.data() + 0x00, f + 0x00, 0x08);
		std::memcpy(result.data() + 0x10, f + 0x08, 0x08);
		return result;
	}
};

//3x3 matrix

template<typename T>
struct Mat3x3 : public Mat<T, 3, 3> {

	using Mat<T, 3, 3>::Mat;

	constexpr inline Mat3x3(const Mat<T, 3, 3> &dat) : Mat<T, 3, 3>(dat) {}
	constexpr inline Mat3x3(Mat<T, 3, 3> &&dat) : Mat<T, 3, 3>(dat) {}

	//TODO: rotateY3D, rotateZ3D, translate2D, transform2D, orientation3D, perspective/ortho

	//Rotate z
	static constexpr Mat3x3 rotateZ(T v) { return TMatHelper<Mat3x3, T>::rotateZ(v); }

	//Pad to vec4s
	inline Buffer toGPUData() const {

		Buffer result(0x30);	//3 vec4s

		std::memcpy(result.data() + 0x00, f + 0x00, 0x0C);
		std::memcpy(result.data() + 0x10, f + 0x0C, 0x0C);
		std::memcpy(result.data() + 0x20, f + 0x18, 0x0C);
		return result;
	}
};

//4x4 matrix

template<typename T>
struct Mat4x4 : public Mat<T, 4, 4> {

	using Mat<T, 4, 4>::Mat;

	constexpr inline Mat4x4(const Mat<T, 4, 4> &dat) : Mat<T, 4, 4>(dat) {}
	constexpr inline Mat4x4(Mat<T, 4, 4> &&dat) : Mat<T, 4, 4>(dat) {}

	//Rotate z
	static constexpr inline Mat4x4 rotateZ(T v) { return TMatHelper<Mat4x4, T>::rotateZ(v); }

	//Helper functions

	static constexpr inline Mat4x4 translate(const Vec3<T> &pos) { 
		Mat4x4 res{};
		res.pos = pos;
		return res;
	}

	static inline Mat4x4 perspective(T fov, T asp, T n) {

		static_assert(
			std::is_floating_point_v<T>, 
			"Can't call Mat4x4<T>::perspective if T isn't a floating point"
		);

		T scale = T(1 / tan(fov / 2));

		Mat4x4 res(
			Vec4<T>(scale / asp, -scale, 0, 0)
		);

		res.m[2][3] = -1;
		res.m[3][2] = 2 * n;
		return res;
	}

	//TODO: Ortho
	//TODO: Rotate X, Y

	static constexpr inline Mat4x4 rotate(const Vec3<T> &rot) {
		return rotateX(rot.x) * rotateY(rot.y) * rotateZ(rot.z);
	}

	static constexpr inline Mat4x4 transform(
		const Vec3<T> &pos, const Vec3<T> &rot, const Vec3<T> &scl
	) {
		return translate(pos) * rotate(rot) * scale(scl);
	}

	static constexpr inline Mat4x4 view(
		const Vec3<T> &pos, const Vec3<T> &rot
	) {
		return translate(-pos) * rotate(rot);
	}

	static constexpr inline Mat4x4 lookAt(
		const Vec3<T> &eye, const Vec3<T> &center, const Vec3<T> &up
	) {
		Vec3<T> z = (eye - center).normalize();
		Vec3<T> x = (up.cross(z)).normalize();
		Vec3<T> y = (z.cross(x)).normalize();

		Mat4x4 res;
		res.x = x;
		res.y = y;
		res.z = z;
		res.pos = -eye; res.one = 1;
		return res;
	}

	static constexpr inline Mat4x4 lookDirection(
		const Vec3<T> &eye, const Vec3<T> &dir, const Vec3<T> &up
	) {
		Vec3<T> z = dir.normalize();
		Vec3<T> x = (up.cross(z)).normalize();
		Vec3<T> y = (z.cross(x)).normalize();

		Mat4x4 res;
		res.x = x;
		res.y = y;
		res.z = z;
		res.pos = -eye; res.one = 1;
		return res;
	}

	//Get gpu data
	inline Buffer toGPUData() const {
		return toData();
	}
};

//Types

//2x2 matrices (2d orientation)

using Mat2x2i16 = Mat2x2<i16>;
using Mat2x2u16 = Mat2x2<u16>;

using Mat2x2i16 = Mat2x2<i16>;
using Mat2x2u16 = Mat2x2<u16>;

using Mat2x2b32 = Mat2x2<u32>;
using Mat2x2f32 = Mat2x2<f32>;
using Mat2x2i32 = Mat2x2<i32>;
using Mat2x2u32 = Mat2x2<u32>;

using Mat2x2i64 = Mat2x2<i64>;
using Mat2x2u64 = Mat2x2<u64>;
using Mat2x2f64 = Mat2x2<f64>;

//3x3 matrices (3d orientation or 2d orientation + translation)

using Mat3x3i16 = Mat3x3<i16>;
using Mat3x3u16 = Mat3x3<u16>;

using Mat3x3i16 = Mat3x3<i16>;
using Mat3x3u16 = Mat3x3<u16>;

using Mat3x3b32 = Mat3x3<u32>;
using Mat3x3f32 = Mat3x3<f32>;
using Mat3x3i32 = Mat3x3<i32>;
using Mat3x3u32 = Mat3x3<u32>;

using Mat3x3i64 = Mat3x3<i64>;
using Mat3x3u64 = Mat3x3<u64>;
using Mat3x3f64 = Mat3x3<f64>;

//4x4 matrices (3d orientation + translation)

using Mat4x4i16 = Mat4x4<i16>;
using Mat4x4u16 = Mat4x4<u16>;

using Mat4x4i16 = Mat4x4<i16>;
using Mat4x4u16 = Mat4x4<u16>;

using Mat4x4b32 = Mat4x4<u32>;
using Mat4x4f32 = Mat4x4<f32>;
using Mat4x4i32 = Mat4x4<i32>;
using Mat4x4u32 = Mat4x4<u32>;

using Mat4x4i64 = Mat4x4<i64>;
using Mat4x4u64 = Mat4x4<u64>;
using Mat4x4f64 = Mat4x4<f64>;