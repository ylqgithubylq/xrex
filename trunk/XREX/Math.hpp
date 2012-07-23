#pragma once
#include "BasicType.hpp"

#include "Vector.hpp"
#include "Matrix.hpp"

double const D_PI = 3.14159265358979323846264338327950288419716939937510582097494459230781640628620;
//double const D_PI = std::atan(1.0) * 4;
float const PI = static_cast<float>(D_PI);
float const RADIAN_TO_DEGREE = static_cast<float>(180.0 / D_PI);
float const DEGREE_TO_RADIAN = static_cast<float>(D_PI / 180.0);


float ReciprocalSqrt(float number);


template <typename T, uint32 N>
Vector<T, N> Transform(Matrix4<T>& matrix, Vector<T, N>& vector, T lastComponent = T(1));

template <typename T, uint32 N>
Vector<T, N> TransformNormal(Matrix4<T>& matrix, Vector<T, N>& vector);

template <typename T>
Matrix4<T> Translation(T const & x, T const & y, T const & z);
template <typename T>
Matrix4<T> Translation(Vector<T, 3> const & v);

template <typename T>
Matrix4<T> Scaling(T const & s);
template <typename T>
Matrix4<T> Scaling(T const & sx, T const & sy, T const & sz);
template <typename T>
Matrix4<T> Scaling(Vector<T, 3> const & s);

template <typename T>
Matrix4<T> RotationX(T const & angleX);
template <typename T>
Matrix4<T> RotationY(T const & angleY);
template <typename T>
Matrix4<T> RotationZ(T const & angleZ);
template <typename T>
Matrix4<T> Rotation(T const & angle, T const & x, T const & y, T const & z);
template <typename T>
Matrix4<T> Rotation(T const & angle, Vector<T, 3> const & axis);
template <typename T>
Matrix4<T> RotationFromTo(Vector<T, 3> const & from, Vector<T, 3> const & to);

template <typename T>
Matrix4<T> Frustum(T const & fieldOfView, T const & aspectRatio, T const & near, T const & far);
template <typename T>
Matrix4<T> Frustum(T const & top, T const & bottom, T const & left, T const & right, T const & near, T const & far);

template <typename T>
Matrix4<T> LookAt(Vector<T, 3> const & eye, Vector<T, 3> const & at, Vector<T, 3> const & up);
// TODO look at