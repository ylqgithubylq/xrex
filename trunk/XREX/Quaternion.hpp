#pragma once

#include "Math.hpp"
#include "Vector.hpp"
#include "MathHelper.hpp"

namespace XREX
{

	/*
	 *	Immutable type.
	 */
	template <typename T>
	class QuaternionT
	{
		template <typename U>
		friend class QuaternionT;

	public:
		static uint32 const Dimension = 4;

		typedef T ValueType;

		typedef ValueType* Pointer;
		typedef ValueType const* ConstPointer;

		typedef ValueType& Reference;
		typedef ValueType const& ConstReference;

	public:
		/*
		 *	Used for making error intentionally.
		 */
		static QuaternionT const Zero;
		static QuaternionT const Identity;
	public:

		/*
		 *	Create an uninitialized QuaternionT.
		 */
		QuaternionT()
		{
		}
		QuaternionT(QuaternionT const& right)
			: values_(right.values_)
		{
		}
		template <typename U>
		explicit QuaternionT(QuaternionT<U> const& right)
			: values_(right.values_)
		{
		}
		explicit QuaternionT(VectorT<T, 4> const& right)
			: values_(right)
		{
		}
		QuaternionT(VectorT<T, 3> const& axis, T const& w)
			: values_(axis.X(), axis.Y(), axis.Z(), w)
		{
		}
		QuaternionT(T const& x, T const& y, T const& z, T const& w)
			: values_(x, y, z, w)
		{
		}

		QuaternionT& operator =(QuaternionT const& right)
		{
			if (this != &right)
			{
				values_ = right.values_;
			}
			return *this;
		}
		template <typename U>
		QuaternionT& operator =(QuaternionT<U> const& right)
		{
			values_ = right.values_;
			return *this;
		}

		/*
		 *	Rotation component is at index 3.
		 */
		T const& operator [](uint32 index) const
		{
			return values_[index];
		}

		T const& X() const
		{
			return values_.X();
		}

		T const& Y() const
		{
			return values_.Y();
		}

		T const& Z() const
		{
			return values_.Z();
		}

		T const& W() const
		{
			return values_.W();
		}

		VectorT<T, 3> const& V() const
		{
			// check offsets equality
			assert(&(reinterpret_cast<VectorT<T, 4>*>(nullptr)->X()) == &(reinterpret_cast<VectorT<T, 4>*>(nullptr)->operator [](0)));
			assert(&(reinterpret_cast<VectorT<T, 4>*>(nullptr)->Z()) == &(reinterpret_cast<VectorT<T, 4>*>(nullptr)->operator [](2)));
			// evil hack, first three component are the axis.
			return *reinterpret_cast<VectorT<T, 3> const*>(&values_);
		}

		friend QuaternionT operator +(QuaternionT const& left, QuaternionT const& right)
		{
			return QuaternionT(left.values_ + right.values_);
		}

		friend QuaternionT operator -(QuaternionT const& left, QuaternionT const& right)
		{
			return QuaternionT(left.values_ - right.values_);

		}

		friend QuaternionT operator *(QuaternionT const& left, QuaternionT const& right)
		{
			// see Mathematics for 3D Game Programming and Computer Graphics, 3rd. 4.6.1 Quaternions Mathematics
			return QuaternionT(
				left.W() * right.X() + left.X() * right.W() + left.Y() * right.Z() - left.Z() * right.Y(),
				left.W() * right.Y() + left.Y() * right.W() + left.Z() * right.X() - left.X() * right.Z(),
				left.W() * right.Z() + left.Z() * right.W() + left.X() * right.Y() - left.Y() * right.X(),
				left.W() * right.W() - left.X() * right.X() - left.Y() * right.Y() - left.Z() * right.Z());
		}

		friend QuaternionT operator *(QuaternionT const& left, T const& right)
		{
			return QuaternionT(left.values_ * right);
		}
		friend QuaternionT operator *(T const& left, QuaternionT const& right)
		{
			return QuaternionT(left * right.values_);
		}

		friend QuaternionT operator /(QuaternionT const& left, T const& right)
		{
			return QuaternionT(left.values_ / right);
		}

		QuaternionT const& operator +() const
		{
			return *this; 
		}
		QuaternionT operator -() const
		{
			return QuaternionT(-values_);
		}

		friend bool operator ==(QuaternionT const& left, QuaternionT const& right)
		{
			return left.values_ == right.values_;
		}

		friend bool	operator !=(QuaternionT const& left, QuaternionT const& right)
		{
			return left.values_ != right.values_;
		}

		QuaternionT Normalize() const
		{
			return QuaternionT(values_.Normalize());
		}

		QuaternionT Conjugate() const
		{
			return QuaternionT(-V(), values_.W());
		}

		QuaternionT Inverse() const
		{
			return QuaternionT(Conjugate() * (T(1) / LengthSquared()));
		}

		T Length() const
		{
			return values_.Length();
		}

		T LengthSquared() const
		{
			return values_.LengthSquared();
		}


	private:
		VectorT<T, Dimension> values_;
	};

	template <typename T>
	QuaternionT<T> const QuaternionT<T>::Zero = QuaternionT(VectorT<T, 3>(T(0)), T(0));

	template <typename T>
	QuaternionT<T> const QuaternionT<T>::Identity = QuaternionT(VectorT<T, 3>(T(0)), T(1));

	typedef QuaternionT<float> floatQ;

}
