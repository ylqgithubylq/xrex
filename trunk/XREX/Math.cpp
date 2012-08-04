#include "XREX.hpp"

#include "Math.hpp"
#include "MathHelper.hpp"

// Copy from KlayGE
// From Quake III. But the magic number is from http://www.lomont.org/Math/Papers/2003/InvSqrt.pdf
float ReciprocalSqrt(float number)
{
	float const threehalfs = 1.5f;

	float x2 = number * 0.5f;
	union FNI
	{
		float f;
		int32 i;
	} fni;
	fni.f = number;											// evil floating point bit level hacking
	fni.i = 0x5f375a86 - (fni.i >> 1);						// what the fuck?
	fni.f = fni.f * (threehalfs - (x2 * fni.f * fni.f));	// 1st iteration
	fni.f = fni.f * (threehalfs - (x2 * fni.f * fni.f));	// 2nd iteration, this can be removed

	return fni.f;
}

template <typename T, uint32 N>
VectorT<T, N> Transform(Matrix4T<T>& matrix, VectorT<T, N>& vector, T lastComponent)
{
	VectorT<T, N> temp;
	MathHelper::TransformHelper<T, N>::Do(const_cast<T*>(&temp[0]), &matrix[0], &vector[0], lastComponent);
	return temp;
}
template floatV2 Transform(floatM44& matrix, floatV2& vector, float lastComponent);
template floatV3 Transform(floatM44& matrix, floatV3& vector, float lastComponent);
template floatV4 Transform(floatM44& matrix, floatV4& vector, float lastComponent);

template <typename T, uint32 N>
VectorT<T, N> TransformNormal(Matrix4T<T>& matrix, VectorT<T, N>& vector)
{
	return Transform(matrix, vector, T(0));
}
template floatV2 TransformNormal(floatM44& matrix, floatV2& vector);
template floatV3 TransformNormal(floatM44& matrix, floatV3& vector);
template floatV4 TransformNormal(floatM44& matrix, floatV4& vector);


template <typename T>
Matrix4T<T> Translation(T const & x, T const & y, T const & z)
{
	return Matrix4T<T>(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		x, y, z, 1);
}
template floatM44 Translation(float const & x, float const & y, float const & z);

template <typename T>
Matrix4T<T> Translation(VectorT<T, 3> const & v)
{
	return Translation(v.X(), v.Y(), v.Z());
}
template floatM44 Translation(floatV3 const & v);

template <typename T>
Matrix4T<T> Scaling(T const & s)
{
	return Scaling(s, s, s);
}
template floatM44 Scaling(float const & s);

template <typename T>
Matrix4T<T> Scaling(T const & sx, T const & sy, T const & sz)
{
	return Matrix4T<T>(
		sx, 0, 0, 0,
		0, sy, 0, 0,
		0, 0, sz, 0,
		0, 0, 0, 1);
}
template floatM44 Scaling(float const & sx, float const & sy, float const & sz);

template <typename T>
Matrix4T<T> Scaling(VectorT<T, 3> const & s)
{
	return Scaling(s.X(), s.Y(), s.Z());
}
template floatM44 Scaling(floatV3 const & s);

template <typename T>
Matrix4T<T> RotationX(T const & angleX)
{
	float sx = std::sin(angleX);
	float cx = std::cos(angleX);
	return Matrix4T<T>(
		1, 0, 0, 0,
		0, cx, -sx, 0,
		0, sx, cx, 0,
		0, 0, 0, 1);
}
template floatM44 RotationX(float const & angleX);

template <typename T>
Matrix4T<T> RotationY(T const & angleY)
{
	float sy = std::sin(angleY);
	float cy = std::cos(angleY);
	return Matrix4T<T>(
		cy, 0, sy, 0,
		0, 1, 0, 0,
		-sy, 0, cy, 0,
		0, 0, 0, 1);
}
template floatM44 RotationY(float const & angleY);

template <typename T>
Matrix4T<T> RotationZ(T const & angleZ)
{
	float sz = std::sin(angleZ);
	float cz = std::cos(angleZ);
	return Matrix4T<T>(
		cz, -sz, 0, 0,
		sz, cz, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);
}
template floatM44 RotationZ(float const & angleZ);

template <typename T>
Matrix4T<T> Rotation(T const & angle, T const & x, T const & y, T const & z)
{
	return Rotation(angle, VectorT<T, 3>(x, y, z));
}
template floatM44 Rotation(float const & angle, float const & x, float const & y, float const & z);

template <typename T>
Matrix4T<T> Rotation(T const & angle, VectorT<T, 3> const & axis)
{
	// 0 vector
	if (axis.LengthSquared() == 0)
	{
		return Matrix4T<T>::Zero;
	}
	VectorT<T, 3> naxis = axis.Normalize();
	T ux = naxis.X();
	T uy = naxis.Y();
	T uz = naxis.Z();

	Matrix4T<T> temp;
	T* resultArray = const_cast<T*>(temp.GetArray());

	// Rotation matrix, see http://en.wikipedia.org/wiki/Rotation_matrix#Rotation_matrix_from_axis_and_angle
	// for details, see 'Mathematics for 3D Game Programming and Computer Graphics, 3rd', 4.3.1
	T sin = std::sin(angle);
	T cos = std::cos(angle);
	T t = 1 - cos;

	resultArray[0] = ux * ux * t + cos;
	resultArray[1] = uy * ux * t + uz * sin;
	resultArray[2] = uz * ux * t - uy * sin;
	resultArray[3] = 0;

	resultArray[4] = ux * uy * t - uz * sin;
	resultArray[5] = uy * uy * t + cos;
	resultArray[6] = uz * uy * t + ux * sin;
	resultArray[7] = 0;

	resultArray[8] = ux * uz * t + uy * sin;
	resultArray[9] = uy * uz * t - ux * sin;
	resultArray[10] = uz * uz * t + cos;
	resultArray[11] = 0;

	resultArray[12] = 0;
	resultArray[13] = 0;
	resultArray[14] = 0;
	resultArray[15] = 1;

	return temp;
}
template floatM44 Rotation(float const & angle, floatV3 const & axis);

template <typename T>
Matrix4T<T> RotationFromTo(VectorT<T, 3> const & from, VectorT<T, 3> const & to)
{
	// 0 vector
	if (from.LengthSquared() == 0 || to.LengthSquared() == 0)
	{
		return Matrix4T<T>::Zero;
	}
	VectorT<T, 3> ufrom = from.Normalize();
	VectorT<T, 3> uto = to.Normalize();

	Matrix4T<T> temp;
	T* resultArray = const_cast<T*>(temp.GetArray());

	// Rotation matrix build from to vectors, see 'Real-time Rendering, 3rd', 4.3.2, 'Rotation from One VectorT to Another'
	T vx = ufrom.Y() * uto.Z() - ufrom.Z() * uto.Y();
	T vy = ufrom.Z() * uto.X() - ufrom.X() * uto.Z();
	T vz = ufrom.X() * uto.Y() - ufrom.Y() * uto.X();
	T e = ufrom.X() * uto.X() + ufrom.Y() * uto.Y() + ufrom.Z() * uto.Z();
	T h = T(1) / (T(1) + e);

	resultArray[0] = e + h * vx * vx;
	resultArray[1] = h * vx * vy + vz;
	resultArray[2] = h * vx * vz - vy;
	resultArray[3] = 0;

	resultArray[4] = h * vx * vy - vz;
	resultArray[5] = e + h * vy * vy;
	resultArray[6] = h * vy * vz + vx;
	resultArray[7] = 0;

	resultArray[8] = h * vx * vz + vy;
	resultArray[9] = h * vy * vz - vx;
	resultArray[10] = e + h * vz * vz;
	resultArray[11] = 0;

	resultArray[12] = 0;
	resultArray[13] = 0;
	resultArray[14] = 0;
	resultArray[15] = 1;

	return temp;
}
template floatM44 RotationFromTo(floatV3 const & from, floatV3 const & to);

template <typename T>
Matrix4T<T> Frustum(T const & fieldOfView, T const & aspectRatio, T const & near, T const & far)
{
	T top = std::tan(fieldOfView * T(0.5)) * near;
	T bottom = -top;
	T right = top * aspectRatio;
	T left = -right;
	return Frustum<T>(top, bottom, left, right, near, far);
}
template floatM44 Frustum(float const & fieldOfView, float const & aspectRatio, float const & near, float const & far);

template <typename T>
Matrix4T<T> Frustum(T const & top, T const & bottom, T const & left, T const & right, T const & near, T const & far)
{
	Matrix4T<T> temp;
	T* resultArray = const_cast<T*>(temp.GetArray());

	// for details, see 'Real-time Rendering, 3rd', 4.6.2
	// or 'Mathematics for 3D Game Programming and Computer Graphics, 3rd', 5.5.1
	T rl = (right - left), tb = (top - bottom), fn = (far - near);
	resultArray[0] = (near * 2) / rl;
	resultArray[1] = 0;
	resultArray[2] = 0;
	resultArray[3] = 0;
	resultArray[4] = 0;
	resultArray[5] = (near * 2) / tb;
	resultArray[6] = 0;
	resultArray[7] = 0;
	resultArray[8] = (right + left) / rl;
	resultArray[9] = (top + bottom) / tb;
	resultArray[10] = -(far + near) / fn;
	resultArray[11] = -1;
	resultArray[12] = 0;
	resultArray[13] = 0;
	resultArray[14] = -(far * near * 2) / fn;
	resultArray[15] = 0;

	return temp;
};
template floatM44 Frustum(float const & top, float const & bottom, float const & left, float const & right, float const & near, float const & far);

template <typename T>
Matrix4T<T> FaceTo(VectorT<T, 3> const & front, VectorT<T, 3> const & self, VectorT<T, 3> const & to, VectorT<T, 3> const & up)
{
	// code below calculate the matrix using -z as the front, so rotate -z axis to front direction first
	floatM44 rotateToNegtiveZ = RotationFromTo(floatV3(0, 0, -1), front);
	VectorT<T, 3> zAxis = Transform(rotateToNegtiveZ, (self - to).Normalize());
	VectorT<T, 3> xAxis = Cross(up, zAxis).Normalize();
	VectorT<T, 3> yAxis = Cross(zAxis, xAxis);

	if (self == to)
	{
		return Matrix4T<T>::Identity;
	}
	if (up.LengthSquared() == 0)
	{
		return Matrix4T<T>::Identity;
	}

	Matrix4T<T> temp;
	T* resultArray = const_cast<T*>(temp.GetArray());

	resultArray[0] = xAxis.X();
	resultArray[1] = xAxis.Y();
	resultArray[2] = xAxis.Z();
	resultArray[3] = 0;
	resultArray[4] = yAxis.X();
	resultArray[5] = yAxis.Y();
	resultArray[6] = yAxis.Z();
	resultArray[7] = 0;
	resultArray[8] = zAxis.X();
	resultArray[9] = zAxis.Y();
	resultArray[10] = zAxis.Z();
	resultArray[11] = 0;
	resultArray[12] = 0;
	resultArray[13] = 0;
	resultArray[14] = 0;
	resultArray[15] = 1;

	return temp;
}
template floatM44 FaceTo(floatV3 const & front, floatV3 const & self, floatV3 const & to, floatV3 const & up);

template <typename T>
Matrix4T<T> LookAt(VectorT<T, 3> const & eye, VectorT<T, 3> const & at, VectorT<T, 3> const & up)
{
	// -z is front direction
	VectorT<T, 3> zAxis((eye - at).Normalize());
	VectorT<T, 3> xAxis(Cross(up, zAxis).Normalize());
	VectorT<T, 3> yAxis(Cross(zAxis, xAxis));

	if (eye == at)
	{
		return Matrix4T<T>::Identity;
	}
	if (up.LengthSquared() == 0)
	{
		return Matrix4T<T>::Identity;
	}

	Matrix4T<T> temp;
	T* resultArray = const_cast<T*>(temp.GetArray());

	resultArray[0] = xAxis.X();
	resultArray[1] = yAxis.X();
	resultArray[2] = zAxis.X();
	resultArray[3] = 0;
	resultArray[4] = xAxis.Y();
	resultArray[5] = yAxis.Y();
	resultArray[6] = zAxis.Y();
	resultArray[7] = 0;
	resultArray[8] = xAxis.Z();
	resultArray[9] = yAxis.Z();
	resultArray[10] = zAxis.Z();
	resultArray[11] = 0;
	resultArray[12] = -Dot(xAxis, eye);
	resultArray[13] = -Dot(yAxis, eye);
	resultArray[14] = -Dot(zAxis, eye);
	resultArray[15] = 1;

	return temp;
}
template floatM44 LookAt(floatV3 const & eye, floatV3 const & at, floatV3 const & up);
