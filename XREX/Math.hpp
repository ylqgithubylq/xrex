#pragma once
#include "BasicType.hpp"

#include "Vector.hpp"
#include "Matrix.hpp"
#include "Quaternion.hpp"

#include "Geometry.hpp"

#include <cmath>

namespace XREX
{

	double const D_PI = 3.14159265358979323846264338327950288419716939937510582097494459230781640628620;
	//double const D_PI = std::atan(1.0) * 4;
	float const PI = static_cast<float>(D_PI);
	float const RadianToDegree = static_cast<float>(180.0 / D_PI);
	float const DegreeToRadian = static_cast<float>(D_PI / 180.0);

	template <typename T>
	inline T RadianFromDegree(T degree);
	template <>
	inline float RadianFromDegree<float>(float degree)
	{
		return DegreeToRadian * degree;
	}
	inline float RadianFromDegree(int degree)
	{
		return DegreeToRadian * degree;
	}

	template <typename T>
	inline T DegreeFromRadian(T radian);
	template <>
	inline float DegreeFromRadian<float>(float radian)
	{
		return RadianToDegree * radian;
	}
	inline float DegreeFromRadian(int radian)
	{
		return RadianToDegree * radian;
	}

	template <typename T>
	inline bool Equal(T const& left, T const& right)
	{
		return left == right;
	}
	template <>
	inline bool Equal<float>(float const& left, float const& right)
	{
		return std::abs(left - right) <= std::numeric_limits<float>::epsilon();
	}
	template <>
	inline bool Equal<double>(double const& left, double const& right)
	{
		return std::abs(left - right) <= std::numeric_limits<double>::epsilon();
	}

	template <typename T>
	inline T Square(T number)
	{
		return number * number;
	}
	/*
	 *	Have fast float hack implementation.
	 */
	template <typename T>
	T ReciprocalSqrt(T number);

	/*
	 *	@return: angle in [0, PI] in radians.
	 */
	template <typename T, uint32 N>
	T Angle(VectorT<T, N> const& from, VectorT<T, N> const& to);

	template <typename T>
	QuaternionT<T> QuaternionFromMatrix(Matrix4T<T> const& rotationMatrix);
	template <typename T>
	Matrix4T<T> MatrixFromQuaternion(QuaternionT<T> const& quaternion);

	/*
	 *	for 3 dimension vector, affine division will be done on the result vector.
	 */
	template <typename T, uint32 N>
	VectorT<T, N> Transform(Matrix4T<T> const& matrix, VectorT<T, N> const& vector, T const& lastComponent = T(1));

	template <typename T>
	VectorT<T, 3> RotateByQuaternion(QuaternionT<T> const& quaternion, VectorT<T, 3> const& vector);

	template <typename T, uint32 N>
	VectorT<T, N> TransformDirection(Matrix4T<T> const& matrix, VectorT<T, N> const& vector);

	template <typename T>
	Matrix4T<T> TranslationMatrix(T const& x, T const& y, T const& z);
	template <typename T>
	Matrix4T<T> TranslationMatrix(VectorT<T, 3> const& v);

	template <typename T>
	Matrix4T<T> ScalingMatrix(T const& s);
	template <typename T>
	Matrix4T<T> ScalingMatrix(T const& sx, T const& sy, T const& sz);
	template <typename T>
	Matrix4T<T> ScalingMatrix(VectorT<T, 3> const& s);

	template <typename T>
	Matrix4T<T> RotationMatrixX(T const& angleX);
	template <typename T>
	Matrix4T<T> RotationMatrixY(T const& angleY);
	template <typename T>
	Matrix4T<T> RotationMatrixZ(T const& angleZ);
	template <typename T>
	Matrix4T<T> RotationMatrix(T const& angle, T const& x, T const& y, T const& z);
	template <typename T>
	Matrix4T<T> RotationMatrix(T const& angle, VectorT<T, 3> const& axis);
	template <typename T>
	Matrix4T<T> RotationMatrixFromTo(VectorT<T, 3> const& from, VectorT<T, 3> const& to);

	template <typename T>
	QuaternionT<T> RotationQuaternion(T const& angle, T const& x, T const& y, T const& z);
	template <typename T>
	QuaternionT<T> RotationQuaternion(T const& angle, VectorT<T, 3> const& axis);
	template <typename T>
	QuaternionT<T> RotationQuaternionFromTo(VectorT<T, 3> const& from, VectorT<T, 3> const& to);

	/*
	 *	@to: face to direction in world space.
	 *	@up: up direction in world space as reference.
	 *	@localFront: front direction in local space.
	 *	@localUp: up reference direction in local space.
	 *	@return: matrix only contains rotation component.
	 */
	template <typename T>
	Matrix4T<T> FaceToMatrix(VectorT<T, 3> const& to, VectorT<T, 3> const& up, VectorT<T, 3> const& localFront, VectorT<T, 3> const& localUp);
	/*
	 *	@to: face to direction in world space.
	 *	@up: up direction in world space as reference.
	 *	@localFront: front direction in local space.
	 *	@localUp: up reference direction in local space.
	 */
	template <typename T>
	QuaternionT<T> FaceToQuaternion(VectorT<T, 3> const& to, VectorT<T, 3> const& up, VectorT<T, 3> const& localFront, VectorT<T, 3> const& localUp);

	/*
	 *	Creating view matrix of camera. With assumption of camera front is +z.
	 *	@eye: eye position.
	 *	@to: look to direction.
	 *	@up: up direction in world space.
	 *	@return: view matrix.
	 */
	template <typename T>
	Matrix4T<T> LookToViewMatrix(VectorT<T, 3> const& eye, VectorT<T, 3> const& to, VectorT<T, 3> const& up);


	template <typename T>
	Matrix4T<T> FrustumMatrix(T const& fieldOfView, T const& aspectRatio, T const& near, T const& far);
	template <typename T>
	Matrix4T<T> FrustumMatrix(T const& top, T const& bottom, T const& left, T const& right, T const& near, T const& far);

}
