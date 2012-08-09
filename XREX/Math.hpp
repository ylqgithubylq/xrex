#pragma once
#include "BasicType.hpp"

#include "Vector.hpp"
#include "Matrix.hpp"
#include "Quaternion.hpp"

#include <cmath>

double const D_PI = 3.14159265358979323846264338327950288419716939937510582097494459230781640628620;
//double const D_PI = std::atan(1.0) * 4;
float const PI = static_cast<float>(D_PI);
float const RadianToDegree = static_cast<float>(180.0 / D_PI);
float const DegreeToRadian = static_cast<float>(D_PI / 180.0);

template <typename T>
inline T ToRadian(T degree);
template <>
inline float ToRadian<float>(float degree)
{
	return DegreeToRadian * degree;
}

template <typename T>
inline T ToDegree(T radian);
template <>
inline float ToDegree<float>(float radian)
{
	return RadianToDegree * radian;
}

template <typename T>
inline bool	Equal(T const & lhs, T const & rhs)
{
	return (lhs == rhs);
}
template <>
inline bool	Equal<float>(float const & lhs, float const & rhs)
{
	return (std::abs(lhs - rhs) <= std::numeric_limits<float>::epsilon());
}
template <>
inline bool Equal<double>(double const & lhs, double const & rhs)
{
	return (std::abs(lhs - rhs) <= std::numeric_limits<double>::epsilon());
}

/*
 *	Have fast float hack implementation.
 */
template<typename T>
T ReciprocalSqrt(T number);

// TODO QuaternionFromMatrix, MatrixFromQuaternion

template <typename T, uint32 N>
VectorT<T, N> Transform(Matrix4T<T> const & matrix, VectorT<T, N> const & vector, T const & lastComponent = T(1));

template <typename T>
VectorT<T, 3> RotateByQuaternion(QuaternionT<T> const & quaternion, VectorT<T, 3> const & vector);

template <typename T, uint32 N>
VectorT<T, N> TransformNormal(Matrix4T<T> const & matrix, VectorT<T, N> const & vector);

template <typename T>
Matrix4T<T> Translation(T const & x, T const & y, T const & z);
template <typename T>
Matrix4T<T> Translation(VectorT<T, 3> const & v);

template <typename T>
Matrix4T<T> Scaling(T const & s);
template <typename T>
Matrix4T<T> Scaling(T const & sx, T const & sy, T const & sz);
template <typename T>
Matrix4T<T> Scaling(VectorT<T, 3> const & s);

template <typename T>
Matrix4T<T> RotationX(T const & angleX);
template <typename T>
Matrix4T<T> RotationY(T const & angleY);
template <typename T>
Matrix4T<T> RotationZ(T const & angleZ);
template <typename T>
Matrix4T<T> Rotation(T const & angle, T const & x, T const & y, T const & z);
template <typename T>
Matrix4T<T> Rotation(T const & angle, VectorT<T, 3> const & axis);
template <typename T>
Matrix4T<T> RotationFromTo(VectorT<T, 3> const & from, VectorT<T, 3> const & to);

template <typename T>
QuaternionT<T> RotationQuaternion(T const & angle, T const & x, T const & y, T const & z);
template <typename T>
QuaternionT<T> RotationQuaternion(T const & angle, VectorT<T, 3> const & axis);

template <typename T>
Matrix4T<T> Frustum(T const & fieldOfView, T const & aspectRatio, T const & near, T const & far);
template <typename T>
Matrix4T<T> Frustum(T const & top, T const & bottom, T const & left, T const & right, T const & near, T const & far);

// TODO Quaternion version of RotationFromTo and FaceTo

/*
 *	Return matrix only contains rotation component.
 */
template <typename T>
Matrix4T<T> FaceTo(VectorT<T, 3> const & front, VectorT<T, 3> const & self, VectorT<T, 3> const & to, VectorT<T, 3> const & up);

template <typename T>
Matrix4T<T> LookAt(VectorT<T, 3> const & eye, VectorT<T, 3> const & at, VectorT<T, 3> const & up);
