#include "XREX.hpp"

#include "Math.hpp"
#include "MathHelper.hpp"

// Copy from KlayGE
// From Quake III. But the magic number is from http://www.lomont.org/Math/Papers/2003/InvSqrt.pdf
template<>
float ReciprocalSqrt<float>(float number)
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
T Angle(VectorT<T, N> const & from, VectorT<T, N> const & to)
{
	return std::acos(Dot(from, to) / (from.Length() * to.Length()));
}
template float Angle(floatV3 const & from, floatV3 const & to);
template float Angle(floatV2 const & from, floatV2 const & to);

template <typename T>
QuaternionT<T> QuaternionFromMatrix(Matrix4T<T> const & rotationMatrix)
{
	assert(false); // have problems, I have no idea where it goes wrong.

	// see Real-Time Rendering, 3rd. 4.3.2 Quaternion Transforms.
	// code modified from From paper 'Quaternion to Matrix and Back', by ID software, 2005.
	T x, y, z, w;

	// check the diagonal
	if (rotationMatrix(0, 0) + rotationMatrix(1, 1) + rotationMatrix(2, 2) > 0)
	{
		T const t(rotationMatrix(0, 0) + rotationMatrix(1, 1) + rotationMatrix(2, 2));
		T const s = ReciprocalSqrt(t) * T(0.5);
		w = s * t;
		x = (rotationMatrix(1, 2) - rotationMatrix(2, 1)) * s;
		y = (rotationMatrix(2, 0) - rotationMatrix(0, 2)) * s;
		z = (rotationMatrix(0, 1) - rotationMatrix(1, 0)) * s;
	}
	else if (rotationMatrix(0, 0) > rotationMatrix(1, 1) && rotationMatrix(0, 0) > rotationMatrix(2, 2))
	{
		T const t = +rotationMatrix(0, 0) - rotationMatrix(1, 1) - rotationMatrix(2, 2) + T(1);
		T s = ReciprocalSqrt(t) * T(0.5);
		x = s * t;
		y = (rotationMatrix(0, 1) + rotationMatrix(1, 0)) * s;
		z = (rotationMatrix(2, 0) + rotationMatrix(0, 2)) * s;
		w = (rotationMatrix(1, 2) - rotationMatrix(2, 1)) * s;
	}
	else if (rotationMatrix(1, 1) > rotationMatrix(2, 2))
	{ 
		T const t = -rotationMatrix(0, 0) + rotationMatrix(1, 1) - rotationMatrix(2, 2) + T(1);
		T const s = ReciprocalSqrt(t) * T(0.5);

		y = s * t;
		x = (rotationMatrix(0, 1) + rotationMatrix(1, 0)) * s;
		w = (rotationMatrix(2, 0) - rotationMatrix(0, 2)) * s;
		z = (rotationMatrix(1, 2) + rotationMatrix(2, 1)) * s;
	}
	else
	{ 
		T const t = -rotationMatrix(0, 0) - rotationMatrix(1, 1) + rotationMatrix(2, 2) + T(1);
		T const s = ReciprocalSqrt(t) * T(0.5);

		z = s * t;
		w = (rotationMatrix(0, 1) - rotationMatrix(1, 0)) * s;
		x = (rotationMatrix(2, 0) + rotationMatrix(0, 2)) * s;
		y = (rotationMatrix(1, 2) + rotationMatrix(2, 1)) * s;
	}

	// return QuaternionT<T>(x, y, z, w).Normalize();
	return QuaternionT<T>(x, y, z, w);
}
template floatQ QuaternionFromMatrix(floatM44 const & rotationMatrix);


template <typename T>
Matrix4T<T> MatrixFromQuaternion(QuaternionT<T> const & quaternion)
{
	// see Real-Time Rendering, 3rd. 4.3.2 Quaternion Transforms
	// or Mathematics for 3D Game Programming and Computer Graphics, 3rd. 4.6.2 Rotations with Quaternions

	T const x2(quaternion.X() + quaternion.X());
	T const y2(quaternion.Y() + quaternion.Y());
	T const z2(quaternion.Z() + quaternion.Z());

	T const xx2(quaternion.X() * x2);
	T const xy2(quaternion.X() * y2);
	T const xz2(quaternion.X() * z2);
	T const yy2(quaternion.Y() * y2);
	T const yz2(quaternion.Y() * z2);
	T const zz2(quaternion.Z() * z2);

	T const wx2(quaternion.W() * x2);
	T const wy2(quaternion.W() * y2);
	T const wz2(quaternion.W() * z2);

	return Matrix4T<T>(
		1 - yy2 - zz2, xy2 + wz2, xz2 - wy2, 0,
		xy2 - wz2, 1 - xx2 - zz2, yz2 + wx2, 0,
		xz2 + wy2, yz2 - wx2, 1 - xx2 - yy2, 0,
		0, 0, 0, 1);
}
template floatM44 MatrixFromQuaternion(floatQ const & quaternion);

template <typename T, uint32 N>
VectorT<T, N> Transform(Matrix4T<T> const & matrix, VectorT<T, N> const & vector, T const & lastComponent)
{
	VectorT<T, N> temp;
	MathHelper::TransformHelper<T, N>::Do(const_cast<T*>(&temp[0]), &matrix[0], &vector[0], lastComponent);
	return temp;
}
template floatV2 Transform(floatM44 const & matrix, floatV2 const & vector, float const & lastComponent);
template floatV3 Transform(floatM44 const & matrix, floatV3 const & vector, float const & lastComponent);
template floatV4 Transform(floatM44 const & matrix, floatV4 const & vector, float const & lastComponent);

template <typename T>
VectorT<T, 3> RotateByQuaternion(QuaternionT<T> const & quaternion, VectorT<T, 3> const & vector)
{
	// see Mathematics for 3D Game Programming and Computer Graphics, 3rd. 4.6.2 Rotations with Quaternions

	// result = a*vector + b*quaternion + c(quaternion.V CROSS vector)
	// where
	//  a = quaternion.W^2 - (quaternion.V DOT quaternion.V)
	//  b = 2*(quaternion.V DOT vector)
	//  c = 2*quaternion.W
	T const a(quaternion.W() * quaternion.W() - quaternion.V().LengthSquared());
	T const b(2 * Dot(quaternion.V(), vector));
	T const c(quaternion.W() + quaternion.W());

	// Must store this, because result may alias vector
	VectorT<T, 3> crossV(Cross(quaternion.V(), vector)); // quaternion.V CROSS vector

	return a * vector + b * quaternion.V() + c * crossV;
}
template floatV3 RotateByQuaternion(floatQ const & quaternion, floatV3 const & vector);

template <typename T, uint32 N>
VectorT<T, N> TransformNormal(Matrix4T<T> const & matrix, VectorT<T, N> const & vector)
{
	return Transform(matrix, vector, T(0));
}
template floatV2 TransformNormal(floatM44 const & matrix, floatV2 const & vector);
template floatV3 TransformNormal(floatM44 const & matrix, floatV3 const & vector);
template floatV4 TransformNormal(floatM44 const & matrix, floatV4 const & vector);


template <typename T>
Matrix4T<T> TranslationMatrix(T const & x, T const & y, T const & z)
{
	return Matrix4T<T>(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		x, y, z, 1);
}
template floatM44 TranslationMatrix(float const & x, float const & y, float const & z);

template <typename T>
Matrix4T<T> TranslationMatrix(VectorT<T, 3> const & v)
{
	return TranslationMatrix(v.X(), v.Y(), v.Z());
}
template floatM44 TranslationMatrix(floatV3 const & v);

template <typename T>
Matrix4T<T> ScalingMatrix(T const & s)
{
	return ScalingMatrix(s, s, s);
}
template floatM44 ScalingMatrix(float const & s);

template <typename T>
Matrix4T<T> ScalingMatrix(T const & sx, T const & sy, T const & sz)
{
	return Matrix4T<T>(
		sx, 0, 0, 0,
		0, sy, 0, 0,
		0, 0, sz, 0,
		0, 0, 0, 1);
}
template floatM44 ScalingMatrix(float const & sx, float const & sy, float const & sz);

template <typename T>
Matrix4T<T> ScalingMatrix(VectorT<T, 3> const & s)
{
	return ScalingMatrix(s.X(), s.Y(), s.Z());
}
template floatM44 ScalingMatrix(floatV3 const & s);

template <typename T>
Matrix4T<T> RotationMatrixX(T const & angleX)
{
	float sx = std::sin(angleX);
	float cx = std::cos(angleX);
	return Matrix4T<T>(
		1, 0, 0, 0,
		0, cx, -sx, 0,
		0, sx, cx, 0,
		0, 0, 0, 1);
}
template floatM44 RotationMatrixX(float const & angleX);

template <typename T>
Matrix4T<T> RotationMatrixY(T const & angleY)
{
	float sy = std::sin(angleY);
	float cy = std::cos(angleY);
	return Matrix4T<T>(
		cy, 0, sy, 0,
		0, 1, 0, 0,
		-sy, 0, cy, 0,
		0, 0, 0, 1);
}
template floatM44 RotationMatrixY(float const & angleY);

template <typename T>
Matrix4T<T> RotationMatrixZ(T const & angleZ)
{
	float sz = std::sin(angleZ);
	float cz = std::cos(angleZ);
	return Matrix4T<T>(
		cz, -sz, 0, 0,
		sz, cz, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);
}
template floatM44 RotationMatrixZ(float const & angleZ);

template <typename T>
Matrix4T<T> RotationMatrix(T const & angle, T const & x, T const & y, T const & z)
{
	return RotationMatrix(angle, VectorT<T, 3>(x, y, z));
}
template floatM44 RotationMatrix(float const & angle, float const & x, float const & y, float const & z);

template <typename T>
Matrix4T<T> RotationMatrix(T const & angle, VectorT<T, 3> const & axis)
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
template floatM44 RotationMatrix(float const & angle, floatV3 const & axis);

template <typename T>
Matrix4T<T> RotationMatrixFromTo(VectorT<T, 3> const & from, VectorT<T, 3> const & to)
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

	// Rotation matrix build from two vectors, see 'Real-time Rendering, 3rd', 4.3.2, 'Rotation from One Vector to Another'
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
template floatM44 RotationMatrixFromTo(floatV3 const & from, floatV3 const & to);


template <typename T>
QuaternionT<T> RotationQuaternion(T const & angle, T const & x, T const & y, T const & z)
{
	return RotationQuaternion(angle, VectorT<T, 3>(x, y, z));
}
template floatQ RotationQuaternion(float const & angle, float const & x, float const & y, float const & z);

template <typename T>
QuaternionT<T> RotationQuaternion(T const & angle, VectorT<T, 3> const & axis)
{
	if (Equal<T>(axis.LengthSquared(), 0))
	{
		return QuaternionT<T>::Identity;
	}
	T halfAngle = T(0.5) * angle;
	T sha = std::sin(halfAngle);
	T cha = std::cos(halfAngle);
	return QuaternionT<T>(sha * axis.Normalize(), cha);
}
template floatQ RotationQuaternion(float const & angle, floatV3 const & axis);

template <typename T>
QuaternionT<T> RotationQuaternionFromTo(VectorT<T, 3> const & from, VectorT<T, 3> const & to)
{
	VectorT<T, 3> uFrom = from.Normalize();
	VectorT<T, 3> uTo = to.Normalize();

	T cos = Dot(uFrom, uTo);
	if (Equal<T>(cos, 1))
	{
		return QuaternionT<T>::Identity;
	}
	else if (Equal<T>(cos, -1))
	{
		return QuaternionT<T>(1, 0, 0, 0);
	}
	VectorT<T, 3> axis = Cross(uFrom, uTo);

	// 	sin(A/2)=+-((1-cosA)/2)^(1/2), here only positive
	// 	cos(A/2)=+-((1+cosA)/2)^(1/2), here only positive
	T sha = std::sqrt((1 - cos) * 0.5);
	T cha = std::sqrt((1 + cos) * 0.5);
	return QuaternionT<T>(sha * axis, cha);
}
template floatQ RotationQuaternionFromTo(floatV3 const & from, floatV3 const & to);

template <typename T>
Matrix4T<T> FrustumMatrix(T const & fieldOfView, T const & aspectRatio, T const & near, T const & far)
{
	T top = std::tan(fieldOfView * T(0.5)) * near;
	T bottom = -top;
	T right = top * aspectRatio;
	T left = -right;
	return FrustumMatrix<T>(top, bottom, left, right, near, far);
}
template floatM44 FrustumMatrix(float const & fieldOfView, float const & aspectRatio, float const & near, float const & far);

template <typename T>
Matrix4T<T> FrustumMatrix(T const & top, T const & bottom, T const & left, T const & right, T const & near, T const & far)
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
template floatM44 FrustumMatrix(float const & top, float const & bottom, float const & left, float const & right, float const & near, float const & far);

template <typename T>
Matrix4T<T> FaceToMatrix(VectorT<T, 3> const & front, VectorT<T, 3> const & self, VectorT<T, 3> const & to, VectorT<T, 3> const & up)
{
	// TODO have problems, not correct
	// code below calculate the matrix using -z as the front, so rotate front direction axis to -z first
	QuaternionT<T> rotateToNegtiveZ = RotationQuaternionFromTo(VectorT<T, 3>(0, 0, -1), front);
	VectorT<T, 3> zAxis = RotateByQuaternion(rotateToNegtiveZ, (self - to).Normalize());
	VectorT<T, 3> xAxis = Cross(up, zAxis).Normalize();
	VectorT<T, 3> yAxis = Cross(zAxis, xAxis);

	if (self == to)
	{
		return Matrix4T<T>::Identity;
	}
	if (up.LengthSquared() == 0)
	{
		return Matrix4T<T>::Zero;
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
template floatM44 FaceToMatrix(floatV3 const & front, floatV3 const & self, floatV3 const & to, floatV3 const & up);

template <typename T>
QuaternionT<T> FaceToQuaternion(VectorT<T, 3> const & front, VectorT<T, 3> const & self, VectorT<T, 3> const & to, VectorT<T, 3> const & up)
{
	QuaternionT<T> rotation = RotationQuaternionFromTo(front, to);
	VectorT<T, 3> upRotated = RotateByQuaternion(rotation, up).Normalize();
	VectorT<T, 3> frontRotated = RotateByQuaternion(rotation, front);
	VectorT<T, 3> normalOfFrontRotatedAndUp = Cross(frontRotated, up).Normalize();

	T angleToRotate = Dot((upRotated - Dot(upRotated, normalOfFrontRotatedAndUp) * normalOfFrontRotatedAndUp).Normalize(), upRotated);
	QuaternionT<T> adjust = RotationQuaternion(-angleToRotate, frontRotated);
	// assert(false); // TODO not verified, seems have problem
	return adjust * rotation;
}
template floatQ FaceToQuaternion(floatV3 const & front, floatV3 const & self, floatV3 const & to, floatV3 const & up);


template <typename T>
Matrix4T<T> LookAtMatrix(VectorT<T, 3> const & eye, VectorT<T, 3> const & at, VectorT<T, 3> const & up)
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
template floatM44 LookAtMatrix(floatV3 const & eye, floatV3 const & at, floatV3 const & up);
