/*
*	Code partially come from KlayGE, with some simplification and improvement.
*/

#pragma once

#include "Math.hpp"
#include "MathHelper.hpp"

#include <array>


namespace XREX
{

	/*
	 *	Immutable type.
	 */
	template <typename T, uint32 N>
	class VectorT
	{
		static_assert(N >= 1 && N <= 4, "");

		template <typename U, uint32 M>
		friend class VectorT;

		// Matrix knows the storage details
		template <typename T>
		friend class Matrix4;

	public:
		static uint32 const Dimension = N;

		typedef T ValueType;

		typedef ValueType* Pointer;
		typedef ValueType const* ConstPointer;

		typedef ValueType& Reference;
		typedef ValueType const& ConstReference;

	public:
		static VectorT const Zero;

	public:
		/*
		 *	Create an uninitialized VectorT.
		 */
		VectorT()
		{
		}
		explicit VectorT(T const* rhs)
		{
			MathHelper::VectorHelper<T, N>::DoCopy(&values_[0], rhs);
		}
		VectorT(VectorT const& rhs)
			: values_(rhs.values_)
		{
		}
	private:
		struct SmallerSizeTag
		{
		};
		struct LargerSizeTag
		{
		};
		template <typename U, uint32 M>
		void DoConstructFromOtherSizedVector(VectorT<U, M> const& rhs, SmallerSizeTag)
		{
			MathHelper::VectorHelper<T, M>::DoCopy(&values_[0], &rhs[0]);
			MathHelper::VectorHelper<T, N - M>::DoAssign(&values_[M], 0); // fill rest with 0
		}
		template <typename U, uint32 M>
		void DoConstructFromOtherSizedVector(VectorT<U, M> const& rhs, LargerSizeTag)
		{
			MathHelper::VectorHelper<T, N>::DoCopy(&values_[0], &rhs[0]);
		}
	public:
		template <typename U, uint32 M>
		explicit VectorT(VectorT<U, M> const& rhs)
		{
			DoConstructFromOtherSizedVector(rhs, std::conditional<M >= N, LargerSizeTag, SmallerSizeTag>::type());
		}
		explicit VectorT(T const& rhs)
		{
			MathHelper::VectorHelper<T, N>::DoAssign(&values_[0], rhs);
		}
		VectorT(T const& x, T const& y)
		{
			static_assert(Dimension == 2, "Dimension 2 only");

			values_[0] = x;
			values_[1] = y;
		}
		VectorT(T const& x, T const& y, T const& z)
		{
			static_assert(Dimension == 3, "Dimension 3 only");

			values_[0] = x;
			values_[1] = y;
			values_[2] = z;
		}
		VectorT(T const& x, T const& y, T const& z, T const& w)
		{
			static_assert(Dimension == 4, "Dimension 4 only");

			values_[0] = x;
			values_[1] = y;
			values_[2] = z;
			values_[3] = w;
		}
		VectorT& operator =(VectorT const& rhs)
		{
			if (this != &rhs)
			{
				values_ = rhs.values_;
			}
			return *this;
		}
		template <typename U, uint32 M>
		VectorT& operator =(VectorT<U, M> const& rhs)
		{
			DoConstructFromOtherSizedVector(rhs, std::conditional<M >= N, LargerSizeTag, SmallerSizeTag>::type());
			return *this;
		}


		T const& operator [](uint32 index) const
		{
			assert(index < Dimension);
			return values_[index];
		}

		T const& X() const
		{
			static_assert(Dimension >= 1, "");
			return values_[0];
		}

		T const& Y() const
		{
			static_assert(Dimension >= 2, "");
			return values_[1];
		}

		T const& Z() const
		{
			static_assert(Dimension >= 3, "");
			return values_[2];
		}

		T const& W() const
		{
			static_assert(Dimension >= 4, "");
			return values_[3];
		}


		friend VectorT operator +(VectorT const& lhs, VectorT const& rhs)
		{
			VectorT temp;
			MathHelper::VectorHelper<T, N>::DoAdd(&temp.values_[0], &lhs.values_[0], &rhs.values_[0]);
			return temp;
		}

		friend VectorT operator -(VectorT const& lhs, VectorT const& rhs)
		{
			VectorT temp;
			MathHelper::VectorHelper<T, N>::DoSubtract(&temp.values_[0], &lhs.values_[0], &rhs.values_[0]);
			return temp;
		}

		friend VectorT operator *(VectorT const& lhs, VectorT const& rhs)
		{
			VectorT temp;
			MathHelper::VectorHelper<T, N>::DoMultiply(&temp.values_[0], &lhs.values_[0], &rhs.values_[0]);
			return temp;
		}

		friend VectorT operator *(VectorT const& lhs, T const& rhs)
		{
			VectorT temp;
			MathHelper::VectorHelper<T, N>::DoScale(&temp.values_[0], &lhs.values_[0], rhs);
			return temp;
		}
		friend VectorT operator *(T const& lhs, VectorT const& rhs)
		{
			VectorT temp;
			MathHelper::VectorHelper<T, N>::DoScale(&temp.values_[0], &rhs.values_[0], lhs);
			return temp;
		}

		friend VectorT operator /(VectorT const& lhs, VectorT const& rhs)
		{
			VectorT temp;
			MathHelper::VectorHelper<T, N>::DoDivide(&temp.values_[0], &lhs.values_[0], &rhs.values_[0]);
			return temp;
		}

		friend VectorT operator /(VectorT const& lhs, T const& rhs)
		{
			VectorT temp;
			MathHelper::VectorHelper<T, N>::DoScale(&temp.values_[0], &lhs.values_[0], T(1) / rhs);
			return temp;
		}

		VectorT const& operator +() const
		{
			return *this; 
		}
		VectorT operator -() const
		{
			VectorT temp;
			MathHelper::VectorHelper<T, N>::DoNegate(&temp.values_[0], &values_[0]);
			return temp;
		}

		friend bool operator ==(VectorT const& lhs, VectorT const& rhs)
		{
			return lhs.values_ == rhs.values_;
			//return MathHelper::VectorHelper<T, N>::DoEqual(&lhs[0], &rhs[0]);
		}

		friend bool	operator !=(VectorT const& lhs, VectorT const& rhs)
		{
			return lhs.values_ != rhs.values_;
		}

		VectorT Normalize() const // float only
		{
			return *this * ReciprocalSqrt(LengthSquared());
		}

		T Length() const // float only
		{
			return T(1) / ReciprocalSqrt(LengthSquared());
		}

		T LengthSquared() const
		{
			return Dot(*this, *this);
		}

		friend T Dot(VectorT const& lhs, VectorT const& rhs)
		{
			return MathHelper::VectorHelper<T, N>::DoDot(&lhs.values_[0], &rhs.values_[0]);
		}

		T const* GetArray() const
		{
			return &values_[0];
		}

	private:
		std::array<T, N> values_;
	};

	template <typename T, uint32 N>
	VectorT<T, N> const VectorT<T, N>::Zero = VectorT(T(0));



	template <typename T>
	VectorT<T, 3> Cross(VectorT<T, 3> const& lhs, VectorT<T, 3> const& rhs)
	{
		return VectorT<T, 3>(
			lhs.Y() * rhs.Z() - lhs.Z() * rhs.Y(),
			lhs.Z() * rhs.X() - lhs.X() * rhs.Z(),
			lhs.X() * rhs.Y() - lhs.Y() * rhs.X());
	}


	typedef VectorT<float, 1> floatV1;
	typedef VectorT<float, 2> floatV2;
	typedef VectorT<float, 3> floatV3;
	typedef VectorT<float, 4> floatV4;

	typedef VectorT<double, 1> doubleV1;
	typedef VectorT<double, 2> doubleV2;
	typedef VectorT<double, 3> doubleV3;
	typedef VectorT<double, 4> doubleV4;

	typedef VectorT<int32, 1> intV1;
	typedef VectorT<int32, 2> intV2;
	typedef VectorT<int32, 3> intV3;
	typedef VectorT<int32, 4> intV4;

	typedef VectorT<uint32, 1> uintV1;
	typedef VectorT<uint32, 2> uintV2;
	typedef VectorT<uint32, 3> uintV3;
	typedef VectorT<uint32, 4> uintV4;

}
