#pragma once

#include "Math.hpp"
#include "Vector.hpp"
#include "MathHelper.hpp"

template <typename T>
class QuaternionT
{
	template <typename U>
	friend class QuaternionT;

public:
	static uint32 const Dimension = 4;

	typedef T ValueType;

	typedef ValueType* Pointer;
	typedef ValueType const * ConstPointer;

	typedef ValueType& Reference;
	typedef ValueType const & ConstReference;

public:
	static QuaternionT const Identity;
public:

	/*
	 *	Create an uninitialized QuaternionT.
	 */
	QuaternionT()
	{
	}
	QuaternionT(QuaternionT const & rhs)
		: values_(rhs.values_)
	{
	}
	template <typename U>
	explicit QuaternionT(QuaternionT<U> const & rhs)
		: values_(rhs.values_)
	{
	}
	explicit QuaternionT(VectorT<T, 4> const & rhs)
		: values_(rhs)
	{
	}
	QuaternionT(VectorT<T, 3> const & axis, T const & s)
		: values_(axis.X(), axis.Y(), axis.Z(), s)
	{
	}
	QuaternionT(T const & x, T const & y, T const & z, T const & s)
		: values_(x, y, z, s)
	{
	}

	QuaternionT& operator=(QuaternionT const & rhs)
	{
		if (this != &rhs)
		{
			values_ = rhs.values_;
		}
		return *this;
	}
	template <typename U>
	QuaternionT& operator=(QuaternionT<U> const & rhs)
	{
		values_ = rhs.values_;
		return *this;
	}

	/*
	 *	Rotation component is at index 3.
	 */
	ConstReference operator[](uint32 index) const
	{
		return values_[index];
	}

	ConstReference X() const
	{
		return values_.X();
	}

	ConstReference Y() const
	{
		return values_.Y();
	}

	ConstReference Z() const
	{
		return values_.Z();
	}

	ConstReference S() const
	{
		return values_.W();
	}

	friend QuaternionT operator+(QuaternionT const & lhs, QuaternionT const & rhs)
	{
		return QuaternionT(lhs.values_ + rhs.values_);
	}

	friend QuaternionT operator-(QuaternionT const & lhs, QuaternionT const & rhs)
	{
		return QuaternionT(lhs.values_ - rhs.values_);

	}

	friend QuaternionT operator*(QuaternionT const & lhs, QuaternionT const & rhs)
	{
		// see Mathematics for 3D Game Programming and Computer Graphics, 3rd. 4.6.1 Quaternions Mathematics
		return QuaternionT(
			lhs.X() * rhs.S() - lhs.Y() * rhs.Z() + lhs.Z() * rhs.Y() + lhs.S() * rhs.X(),
			lhs.X() * rhs.Z() + lhs.Y() * rhs.S() - lhs.Z() * rhs.X() + lhs.S() * rhs.Y(),
			lhs.Y() * rhs.X() - lhs.X() * rhs.Y() + lhs.Z() * rhs.S() + lhs.S() * rhs.Z(),
			lhs.S() * rhs.S() - lhs.X() * rhs.X() - lhs.Y() * rhs.Y() - lhs.Z() * rhs.Z());
	}

	friend QuaternionT operator*(QuaternionT const & lhs, T const & rhs)
	{
		return QuaternionT(lhs.values_ * rhs);
	}
	friend QuaternionT operator*(T const & lhs, QuaternionT const & rhs)
	{
		return QuaternionT(lhs * rhs.values_);
	}

	friend QuaternionT operator/(QuaternionT const & lhs, T const & rhs)
	{
		return QuaternionT(lhs.values_ / rhs);
	}

	QuaternionT const & operator+() const
	{
		return *this; 
	}
	QuaternionT operator-() const
	{
		return QuaternionT(-values_);
	}

	friend bool operator==(QuaternionT const & lhs, QuaternionT const & rhs)
	{
		return lhs.values_ == rhs.values_;
	}

	friend bool	operator!=(QuaternionT const & lhs, QuaternionT const & rhs)
	{
		return lhs.values_ != rhs.values_;
	}

	QuaternionT Normalize() const // float only
	{
		return QuaternionT(values_.Normalize());
	}

	QuaternionT Conjugate() const
	{
		return QuaternionT(-values_.X(), -values_.Y(), -values_.Z(), values_.W());
	}

	QuaternionT Inverse() const
	{
		return QuaternionT(Conjugate() / LengthSquared());
	}

	ValueType Length() const // float only
	{
		return values_.Length();
	}

	ValueType LengthSquared() const
	{
		return values_.LengthSquared();
	}

	VectorT<T, 3> const & Axis() const
	{
		// check offsets equality
		assert(&(reinterpret_cast<VectorT<T, 4>*>(nullptr)->X()) == &(reinterpret_cast<VectorT<T, 4>*>(nullptr)->operator [](0)));
		assert(&(reinterpret_cast<VectorT<T, 4>*>(nullptr)->Z()) == &(reinterpret_cast<VectorT<T, 4>*>(nullptr)->operator [](2)));
		// evil hack, first three component are the axis.
		return *reinterpret_cast<VectorT<T, 3> const *>(&values_);
	}

private:
	VectorT<T, Dimension> values_;
};

template <typename T>
QuaternionT<T> const QuaternionT<T>::Identity = QuaternionT(VectorT<T, 3>(T(0)), T(1));

typedef QuaternionT<float> floatQuaternion;


