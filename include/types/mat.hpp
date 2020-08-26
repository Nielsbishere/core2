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

	constexpr inline TMatStorage(): f{} {}

	template<typename ...args>
	constexpr inline TMatStorage(const Vec<T, W> &axis, const args &...arg): axes{ axis, arg... } {}
};

template<typename T>
struct TMatStorage<T, 2, 2> {

	union {
		T f[4];
		T m[2][2];
		Vec2<T> axes[2];
		struct { Vec2<T> x, y; };
	};

	constexpr inline TMatStorage(): f{} {}

	template<typename ...args>
	constexpr inline TMatStorage(const Vec2<T> &axis, const args &...arg): axes{ axis, arg... } {}
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

	constexpr inline TMatStorage(): f{} {}

	template<typename ...args>
	constexpr inline TMatStorage(const Vec3<T> &axis, const args &...arg): axes{ axis, arg... } {}
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

	constexpr inline TMatStorage(): f{} {}

	template<typename ...args>
	constexpr inline TMatStorage(const Vec4<T> &axis, const args &...arg): axes{ axis, arg... } {}
};

template<typename T>
struct TMatStorage<T, 4, 3> {

	union {

		T f[12];
		T m[4][3];
		Vec3<T> axes[4];

		struct {
			Vec3<T> x, y, z, pos;
		};
	};

	constexpr inline TMatStorage(): f{} {}

	template<typename ...args>
	constexpr inline TMatStorage(const Vec3<T> &axis, const args &...arg): axes{ axis, arg... } {}
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

	using TMatStorage<T, W, H>::TMatStorage;
	using TMatStorage<T, W, H>::m;
	using TMatStorage<T, W, H>::f;
	using TMatStorage<T, W, H>::axes;

	static constexpr usz Height = H, Width = W;

	//Constructors

	Mat(const Mat&) = default;
	Mat(Mat&&) = default;
	Mat &operator=(const Mat&) = default;
	Mat &operator=(Mat&&) = default;

	//Scale matrix
	explicit constexpr inline Mat(const Diagonal &scale) {
		for (usz i = 0; i < diagonalN; ++i) m[i][i] = scale[i];
	}

	//Identity
	constexpr inline Mat(): Mat(Diagonal(1)) { }

	//
	template<typename ...args>
	constexpr inline Mat(const Vec<T, W> &axis, const args &...arg): TMatStorage<T, W, H>{ axis, arg... } {}

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

	Vertical &operator[](const usz i) { return axes[i]; }
	constexpr const Vertical &operator[](const usz i) const { return axes[i]; }

	T &operator[](const Vec2usz &xy) { return m[xy.x][xy.y]; }
	constexpr const T &operator[](const Vec2usz &xy) const { return axes[xy.x][xy.y]; }

	constexpr Horizontal getHorizontal(const usz j) const {
		Vertical res;
		for (usz i = 0; i < W; ++i) res[i] = m[i][j];
		return res;
	}

	constexpr const Vertical &getVertical(const usz i) const { return axes[i]; }

	//Arithmetic overloads

	constexpr Mat &operator+=(const Mat &other) {
		for (usz i = 0; i < W; ++i) axes[i] += other.axes[i];
		return *this;
	}

	constexpr Mat &operator-=(const Mat &other) {
		for (usz i = 0; i < W; ++i) axes[i] -= other.axes[i];
		return *this;
	}

	constexpr Mat &operator/=(const Mat &other) {
		for (usz i = 0; i < W; ++i) axes[i] /= other.axes[i];
		return *this;
	}

	constexpr Mat &operator%=(const Mat &other) {
		for (usz i = 0; i < W; ++i) axes[i] %= other.axes[i];
		return *this;
	}

	constexpr Mat operator+(const Mat &other) const { return Mat(*this) += other; }
	constexpr Mat operator-(const Mat &other) const { return Mat(*this) -= other; }
	constexpr Mat operator/(const Mat &other) const { return Mat(*this) /= other; }
	constexpr Mat operator%(const Mat &other) const { return Mat(*this) %= other; }

	//Since Matrix multiply is different, mulVal can be used to perform regular multiplications on the values

	constexpr Mat &mulVal(const Mat &other) {
		for (usz i = 0; i < W; ++i) axes[i] *= other.axes[i];
		return *this;
	}

	static constexpr inline Mat mulVal(const Mat &a, const Mat &b) { return Mat(a).mulVal(b); }
	static constexpr inline Mat scale(const Diagonal &diag) { return Mat(diag); }

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

	constexpr inline Vec<T, H> operator*(const Vec<T, W> &other) const {

		Vec<T, H> res{};

		for (usz i = 0; i < W; ++i)
			for (usz j = 0; j < H; ++j)
				res[j] += m[i][j] * other[i];

		return res;
	}

	constexpr inline Mat operator*(const Mat &other) const {

		Mat res;

		for (usz i = 0; i < W; ++i)
			for (usz j = 0; j < H; ++j)
				res.m[i][j] = getHorizontal(j).dot(other.getVertical(i));

		return res;
	}

	constexpr inline Mat &operator*=(const Mat &other) { return *this = *this * other; }

	constexpr inline Mat<T, H, W> transpose() const {

		Mat<T, H, W> res{};

		for (usz i = 0; i < W && i < H; ++i)
			for (usz j = 0; j < H && j < W; ++j)
				res.m[j][i] = m[i][j];

		return res;
	}

	//Helpers

	inline Buffer toData() const {
		return Buffer((u8*)f, (u8*)(f + N));
	}

	template<typename T2, usz W2, usz H2>
	constexpr inline Mat<T2, W2, H2> cast() const {

		Mat<T2, W2, H2> res{};

		for (usz i = 0; i < W && i < W2; ++i)
			for (usz j = 0; j < H && j < H2; ++j)
				res.m[i][j] = T2(m[i][j]);

		return res;
	}

	template<typename T2>
	constexpr inline T2 cast() const {

		T2 res{};

		for (usz i = 0; i < W && i < T2::horizontal(); ++i)
			for (usz j = 0; j < H && j < T2::vertical(); ++j)
				res.m[i][j] = typename T2::Type(m[i][j]);

		return res;
	}

};

//Helper functions that carry to multiple types of matrices
template<typename Mat, typename T>
struct TMatHelper {

	static constexpr inline Mat rotateX(T v) {

		static_assert(
			std::is_floating_point_v<typename Mat::Type>, 
			"Can't call TMatHelper<Mat>::rotateX if T isn't a floating point"
		);

		static_assert(
			Mat::horizontal() > 2 && Mat::vertical() > 2, 
			"Can't call TMatHelper<Mat>::rotateX if Mat's dimensions are less than 3x3"
		);

		Mat res{};
		res[1][1] = std::cos(v);	res[2][1] = -std::sin(v);
		res[1][2] = std::sin(v);	res[2][2] = std::cos(v);
		return res;
	}

	static constexpr inline Mat rotateY(T v) {

		static_assert(
			std::is_floating_point_v<typename Mat::Type>, 
			"Can't call TMatHelper<Mat>::rotateY if T isn't a floating point"
		);

		static_assert(
			Mat::horizontal() > 2 && Mat::vertical() > 2, 
			"Can't call TMatHelper<Mat>::rotateY if Mat's dimensions are less than 3x3"
		);

		Mat res{};
		res[0][0] = std::cos(v);	res[0][2] = -std::sin(v);
		res[2][0] = std::sin(v);	res[2][2] = std::cos(v);
		return res;
	}

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
	using Mat<T, 2, 2>::f;

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
	using Mat<T, 3, 3>::f;

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

template<typename T, usz W, usz H, typename = std::enable_if_t<W == 4 && (H == 3 || W == 4)>>
struct MatOT : public Mat<T, W, H> {

	using Mat<T, W, H>::Mat;
	using Mat<T, W, H>::f;
	using Mat<T, W, H>::toData;

	constexpr inline MatOT(const Mat<T, W, H> &dat) : Mat<T, W, H>(dat) { }
	constexpr inline MatOT(Mat<T, W, H> &&dat) : Mat<T, W, H>(dat) {}

	//Rotation
	static constexpr inline MatOT rotateX(T v) { return TMatHelper<MatOT, T>::rotateX(v); }
	static constexpr inline MatOT rotateY(T v) { return TMatHelper<MatOT, T>::rotateY(v); }
	static constexpr inline MatOT rotateZ(T v) { return TMatHelper<MatOT, T>::rotateZ(v); }

	//Helper functions

	static constexpr inline MatOT translate(const Vec3<T> &pos) { 
		MatOT res{};
		res.pos = pos;
		return res;
	}

	static inline MatOT perspective(T fov, T asp, T n, T f) {

		static_assert(
			std::is_floating_point_v<T>, 
			"Can't call MatOT::perspective if T isn't a floating point"
		);

		static_assert(H == 4, "MatOT::perspective only allowed on 4x4 matrices");

		T scale = T(1 / tan(fov / 2));

		MatOT res(
			Vec4<T>(scale / asp, -scale, n / (f - n), 0)
		);

		res.m[2][3] = -1;
		res.m[3][2] = f * n / (f - n);
		return res;
	}

	//TODO: Ortho

	static constexpr inline MatOT rotate(const Vec3<T> &rot) {
		return rotateX(rot.x) * rotateY(rot.y) * rotateZ(rot.z);
	}

	static constexpr inline MatOT scale(const Vec3<T> &scl) {

		if constexpr (H == 4)
			return Mat4x4<T>(Vec4<T>(scl.x, scl.y, scl.z, 1));
		else
			return Mat4x3<T>(Vec3<T>(scl.x, scl.y, scl.z));
	}

	static constexpr inline MatOT transform(
		const Vec3<T> &pos, const Vec3<T> &rot, const Vec3<T> &scl
	) {
		return translate(pos) * rotate(rot) * scale(scl);
	}

	static constexpr inline MatOT view(
		const Vec3<T> &pos, const Vec3<T> &rot
	) {
		return translate(-pos) * rotate(rot);
	}

	static constexpr inline MatOT lookAt(
		const Vec3<T> &eye, const Vec3<T> &center, const Vec3<T> &up
	) {
		Vec3<T> z = (eye - center).normalize();
		Vec3<T> x = (up.cross(z)).normalize();
		Vec3<T> y = (z.cross(x)).normalize();

		MatOT res;
		res.x = x;
		res.y = y;
		res.z = z;
		res.pos = -eye; 
		
		if constexpr (H == 4)
			res.one = 1;

		return res;
	}

	static constexpr inline MatOT lookDirection(
		const Vec3<T> &eye, const Vec3<T> &dir, const Vec3<T> &up
	) {
		Vec3<T> z = dir.normalize();
		Vec3<T> x = (z.cross(up)).normalize();
		Vec3<T> y = (x.cross(z)).normalize();

		MatOT res;
		res.x = x;
		res.y = y;
		res.z = z;
		res.pos = -eye;

		if constexpr (H == 4)
			res.one = 1;

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

template<typename T>
using Mat4x4 = MatOT<T, 4, 4>;

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

//4x3 matrices (3d orientation + translation)

template<typename T>
using Mat4x3 = MatOT<T, 4, 3>;

using Mat4x3i16 = Mat4x3<i16>;
using Mat4x3u16 = Mat4x3<u16>;

using Mat4x3i16 = Mat4x3<i16>;
using Mat4x3u16 = Mat4x3<u16>;

using Mat4x3b32 = Mat4x3<u32>;
using Mat4x3f32 = Mat4x3<f32>;
using Mat4x3i32 = Mat4x3<i32>;
using Mat4x3u32 = Mat4x3<u32>;

using Mat4x3i64 = Mat4x3<i64>;
using Mat4x3u64 = Mat4x3<u64>;
using Mat4x3f64 = Mat4x3<f64>;

namespace oic {

	template<typename T>
	struct is_matrix { static constexpr bool value = false; };

	template<typename T, usz W, usz H>
	struct is_matrix<Mat<T, W, H>> { static constexpr bool value = true; };

	template<typename T>
	struct is_matrix<Mat2x2<T>> { static constexpr bool value = true; };

	template<typename T>
	struct is_matrix<Mat3x3<T>> { static constexpr bool value = true; };

	template<typename T>
	struct is_matrix<Mat4x4<T>> { static constexpr bool value = true; };

	template<typename T>
	struct is_matrix<Mat4x3<T>> { static constexpr bool value = true; };

	template<typename T>
	static constexpr bool is_matrix_v = is_matrix<T>::value;
}
