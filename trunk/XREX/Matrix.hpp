/*
*	Code partially come from KlayGE, with some simplification and improvement.
*/
#pragma once

#include "Math.hpp"
#include "Vector.hpp"


#include <array>

namespace XREX
{

	/*
	 *	Column major matrix.
	 */
	template <typename T>
	class Matrix4T
	{
		template <typename U>
		friend class Matrix4T;

	public:
		typedef T ValueType;

		typedef ValueType* Pointer;
		typedef ValueType const* ConstPointer;

		typedef ValueType& Reference;
		typedef ValueType const& ConstReference;

		static uint32 const Dimension = 4;
		static uint32 const ElementCount = Dimension * Dimension;

	public:
		static Matrix4T const Zero;
		static Matrix4T const Identity;

	public:
		/*
		 *	Create an uninitialized Matrix.
		 */
		Matrix4T()
		{
		}
		/*
		 *	Create a scalar equivalent matrix, fill principal diagonal with rhs.
		 */
		explicit Matrix4T(T const& rhs)
		{
			T* temp = GetFirstElementNonConstPointer(*this);
			temp[0] = rhs;
			temp[1] = 0;
			temp[2] = 0;
			temp[3] = 0;

			temp[4] = 0;
			temp[5] = rhs;
			temp[6] = 0;
			temp[7] = 0;

			temp[8] = 0;
			temp[9] = 0;
			temp[10] = rhs;
			temp[11] = 0;

			temp[12] = 0;
			temp[13] = 0;
			temp[14] = 0;
			temp[15] = rhs;
		}
		/*
		 *	@rhs: column major array.
		 */
		explicit Matrix4T(T const* rhs)
		{
			for (uint32 i = 0; i < Dimension; ++i)
			{
				vectors_[i] = VectorT<T, Dimension>(rhs);
				rhs += Dimension;
			}
		}
		/*
		 *	@rhs: column major array.
		 */
		explicit Matrix4T(std::array<T, ElementCount> const& rhs) // TODO need test
		{
			for (uint32 i = 0; i < Dimension; ++i)
			{
				vectors_[i] = VectorT<T, Dimension>(&rhs[i * 4]);
			}
		}
		/*
		 *	@rhs: column major vectors.
		 */
		explicit Matrix4T(VectorT<VectorT<T, Dimension>, Dimension> const& rhs)
			: vectors_(rhs)
		{
		}
		Matrix4T(Matrix4T const& rhs)
			: vectors_(rhs.vectors_)
		{
		}
		template <typename U>
		Matrix4T(Matrix4T<U> const& rhs)
			: vectors_(rhs.vectors_)
		{
		}
		/*
		 *	parameters subscript: row, column
		 */
		Matrix4T(T const& m11, T const& m21, T const& m31, T const& m41,
			T const& m12, T const& m22, T const& m32, T const& m42,
			T const& m13, T const& m23, T const& m33, T const& m43,
			T const& m14, T const& m24, T const& m34, T const& m44)
		{

			T* temp = GetFirstElementNonConstPointer(*this);
			temp[0] = m11;
			temp[1] = m21;
			temp[2] = m31;
			temp[3] = m41;

			temp[4] = m12;
			temp[5] = m22;
			temp[6] = m32;
			temp[7] = m42;

			temp[8] = m13;
			temp[9] = m23;
			temp[10] = m33;
			temp[11] = m43;

			temp[12] = m14;
			temp[13] = m24;
			temp[14] = m34;
			temp[15] = m44;
		}


		ConstReference operator ()(uint32 row, uint32 column) const
		{
			return vectors_[column][row];
		}

		/*
		 *	@index: column major index.
		 */
		ConstReference operator [](uint32 index) const
		{
			return *(&vectors_[0][0] + index);
		}


		VectorT<T, Dimension> const Row(uint32 index) const
		{
			VectorT<T, Dimension> result;
			for (uint32 i = 0; i < Dimension; ++ i)
			{
				result.values_[i] = vectors_[i][index];
			}
			return result;
		}
		VectorT<T, Dimension> const& Column(uint32 index) const
		{
			return vectors_[index];
		}

		friend Matrix4T operator +(Matrix4T const& lhs, Matrix4T const& rhs)
		{
			return Matrix4T(lhs.vectors_ + rhs.vectors_);
		}
		friend Matrix4T operator -(Matrix4T const& lhs, Matrix4T const& rhs)
		{
			return Matrix4T(lhs.vectors_ - rhs.vectors_);
		}

		friend Matrix4T operator *(Matrix4T const& lhs, Matrix4T const& rhs)
		{
			Matrix4T temp;
			MathHelper::MatrixHepler<T>::DoMultiply(Matrix4T::GetFirstElementNonConstPointer(temp), Matrix4T::GetFirstElementNonConstPointer(lhs), Matrix4T::GetFirstElementNonConstPointer(rhs));
			return temp;
		}
		friend Matrix4T operator *(Matrix4T const& lhs, T const& rhs)
		{
			return Matrix4T(lhs.vectors_ * VectorT<T, Dimension>(rhs));
		}
		friend Matrix4T operator *(T const& lhs, Matrix4T const& rhs)
		{
			return Matrix4T(VectorT<T, Dimension>(lhs) * rhs.vectors_);
		}

		friend Matrix4T operator /(Matrix4T const& lhs, T const& rhs)
		{
			return Matrix4T(lhs.vectors_ / VectorT<T, Dimension>(rhs));
		}

		Matrix4T& operator =(Matrix4T const& rhs)
		{
			if (this != &rhs)
			{
				vectors_ = rhs.vectors_;
			}
			return *this;
		}
		template <typename U>
		Matrix4T const& operator =(Matrix4T<U> const& rhs)
		{
			if (this != &rhs)
			{
				vectors_ = rhs.vectors_;
			}
			return *this;
		}

		Matrix4T const& operator +() const
		{
			return *this;
		}
		Matrix4T operator -() const
		{
			Matrix4T temp(*this);
			temp.vectors_ = -vectors_;
			return temp;
		}

		friend bool operator ==(Matrix4T const& lhs, Matrix4T const& rhs)
		{
			return lhs.vectors_ == rhs.vectors_;
		}
		friend bool operator !=(Matrix4T const& lhs, Matrix4T const& rhs)
		{
			return lhs.vectors_ != rhs.vectors_;
		}

		Matrix4T Transpose() const
		{
			return Matrix4T(operator [](0), operator [](4), operator [](8), operator [](12),
				operator [](1), operator [](5), operator [](9),operator [](13),
				operator [](2), operator [](6), operator [](10), operator [](14),
				operator [](3), operator [](7), operator [](11), operator [](15));
		}

		Matrix4T Inverse() const
		{
			Matrix4T temp;
			bool invertible = MathHelper::MatrixHepler<T>::DoInverse(GetFirstElementNonConstPointer(temp), GetFirstElementNonConstPointer(*this));
			assert(invertible);
			return temp;
		}

		ValueType Determinant() const
		{
			return MathHelper::MatrixHepler<T>::CalculateDeterminant(&vectors_[0][0]);
		}

		ConstPointer GetArray() const
		{
			return &vectors_[0][0];
		}
	
	private:
		static Pointer GetFirstElementNonConstPointer(Matrix4T const& result)
		{
			return const_cast<Pointer>(&result.vectors_[0][0]);
		}

	private:
		VectorT<VectorT<T, Dimension>, Dimension> vectors_;
	};


	template <typename T>
	Matrix4T<T> const Matrix4T<T>::Zero = Matrix4T(T(0));

	template <typename T>
	Matrix4T<T> const Matrix4T<T>::Identity = Matrix4T(T(1));


	typedef Matrix4T<float> floatM44;

}
