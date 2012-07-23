/*
*	Code partially come from KlayGE, with some simplification and improvement.
*/

#pragma once

#include "Math.hpp"
#include "MathHelper.hpp"

#include <array>

//float ReciprocalSqrt(float number);

/*
 *	Immutable type.
 */
template <typename T, uint32 N>
class Vector
{
	template <typename U, uint32 M>
	friend class Vector;

	template <typename T>
	friend class Matrix4;

	typedef std::array<T, N> ContainerType;

public:
	static uint32 const Dimension = N;

	typedef T ValueType;

	typedef ValueType* Pointer;
	typedef ValueType const * ConstPointer;

	typedef ValueType& Reference;
	typedef ValueType const & ConstReference;


public:
	/*
	 *	Create an uninitialized Vector.
	 */
	Vector()
	{
	}
	explicit Vector(T const * rhs)
	{
		MathHelper::VectorHelper<T, N>::DoCopy(&values_[0], rhs);
	}
	Vector(Vector const & rhs)
		: values_(rhs.values_)
	{
	}
	template <typename U, uint32 M>
	explicit Vector(Vector<U, M> const & rhs)
	{
		static_assert(M >= N, "");

		MathHelper::VectorHelper<T, N>::DoCopy(&values_[0], &rhs[0]);
	}
	explicit Vector(T const & rhs)
	{
		MathHelper::VectorHelper<T, N>::DoAssign(&values_[0], rhs);
	}
	Vector(T const & x, T const & y)
	{
		static_assert(Dimension == 2, "");

		values_[0] = x;
		values_[1] = y;
	}
	Vector(T const & x, T const & y, T const & z)
	{
		static_assert(Dimension == 3, "");

		values_[0] = x;
		values_[1] = y;
		values_[2] = z;
	}
	Vector(T const & x, T const & y, T const & z, T const & w)
	{
		static_assert(Dimension == 4, "");

		values_[0] = x;
		values_[1] = y;
		values_[2] = z;
		values_[3] = w;
	}
	Vector& operator=(Vector const & rhs)
	{
		if (this != &rhs)
		{
			values_ = rhs.values_;
		}
		return *this;
	}
	template <typename U, uint32 M>
	Vector& operator=(Vector<U, M> const & rhs)
	{
		static_assert(M >= N, "");

		MathHelper::VectorHelper<T, N>::DoCopy(&values_[0], &rhs.values_[0]);
		return *this;
	}


	static Vector const & Zero()
	{
		static Vector<T, N> zero(T(0));
		return zero;
	}


	ConstReference operator[](uint32 index) const
	{
		return values_[index];
	}

	ConstReference X() const
	{
		static_assert(Dimension >= 1, "");
		return values_[0];
	}

	ConstReference Y() const
	{
		static_assert(Dimension >= 2, "");
		return values_[1];
	}

	ConstReference Z() const
	{
		static_assert(Dimension >= 3, "");
		return values_[2];
	}

	ConstReference W() const
	{
		static_assert(Dimension >= 4, "");
		return values_[3];
	}


	friend Vector operator+(Vector const & lhs, Vector const & rhs)
	{
		Vector temp;
		MathHelper::VectorHelper<T, N>::DoAdd(&temp.values_[0], &lhs.values_[0], &rhs.values_[0]);
		return temp;
	}

	friend Vector operator-(Vector const & lhs, Vector const & rhs)
	{
		Vector temp;
		MathHelper::VectorHelper<T, N>::DoSubtract(&temp.values_[0], &lhs.values_[0], &rhs.values_[0]);
		return temp;
	}

	friend Vector operator*(Vector const & lhs, Vector const & rhs)
	{
		Vector temp;
		MathHelper::VectorHelper<T, N>::DoMultiply(&temp.values_[0], &lhs.values_[0], &rhs.values_[0]);
		return temp;
	}

	friend Vector operator*(Vector const & lhs, T const & rhs)
	{
		Vector temp;
		MathHelper::VectorHelper<T, N>::DoScale(&temp.values_[0], &lhs.values_[0], rhs);
		return temp;
	}
	friend Vector operator*(T const & lhs, Vector const & rhs)
	{
		Vector temp;
		MathHelper::VectorHelper<T, N>::DoScale(&temp.values_[0], &rhs.values_[0], lhs);
		return temp;
	}

	friend Vector operator/(Vector const & lhs, Vector const & rhs)
	{
		Vector temp;
		MathHelper::VectorHelper<T, N>::DoDivide(&temp.values_[0], &lhs.values_[0], &rhs.values_[0]);
		return temp;
	}

	friend Vector operator/(Vector const & lhs, T const & rhs)
	{
		Vector temp;
		MathHelper::VectorHelper<T, N>::DoScale(&temp.values_[0], &lhs.values_[0], T(1) / rhs);
		return temp;
	}

	Vector const & operator+() const
	{
		return *this; 
	}
	Vector operator-() const
	{
		Vector temp;
		MathHelper::VectorHelper<T, N>::DoNegate(&temp.values_[0], &values_[0]);
		return temp;
	}

	friend bool operator==(Vector const & lhs, Vector const & rhs)
	{
		return lhs.values_ == rhs.values_;
		//return MathHelper::VectorHelper<T, N>::DoEqual(&lhs[0], &rhs[0]);
	}

	friend bool	operator!=(Vector const & lhs, Vector const & rhs)
	{
		return lhs.values_ != rhs.values_;
	}

	Vector Normalize() const // float only
	{
		return *this * ReciprocalSqrt(LengthSquared());
	}

	ValueType Length() const // float only
	{
		return ValueType(1) / ReciprocalSqrt(LengthSquared());
	}

	ValueType LengthSquared() const
	{
		return Dot(*this, *this);
	}

	friend ValueType Dot(Vector const & lhs, Vector const & rhs)
	{
		return MathHelper::VectorHelper<T, N>::DoDot(&lhs.values_[0], &rhs.values_[0]);
	}

	ConstPointer GetArray() const
	{
		return &values_[0];
	}

private:
	ContainerType values_;
};

template <typename T>
Vector<T, 3> Cross(Vector<T, 3> const & lhs, Vector<T, 3> const & rhs)
{
	return Vector<T, 3>(lhs.Y() * rhs.Z() - lhs.Z() * rhs.Y(),
		lhs.Z() * rhs.X() - lhs.X() * rhs.Z(),
		lhs.X() * rhs.Y() - lhs.Y() * rhs.X());
}


typedef Vector<float, 1> floatV1;
typedef Vector<float, 2> floatV2;
typedef Vector<float, 3> floatV3;
typedef Vector<float, 4> floatV4;


