/*
*	Code partially come from KlayGE, with some simplification and improvement.
*/
#pragma once

#include "Math.hpp"
#include "Vector.hpp"


#include <array>


/*
 *	Column major matrix.
 */
template <typename T>
class Matrix4
{
	template <typename U>
	friend class Matrix4;

public:
	typedef T ValueType;

	typedef ValueType* Pointer;
	typedef ValueType const * ConstPointer;

	typedef ValueType& Reference;
	typedef ValueType const & ConstReference;

	static uint32 const Dimension = 4;
	static uint32 const ElementCount = Dimension * Dimension;

public:
	/*
	 *	Create an uninitialized Matrix.
	 */
	Matrix4()
	{
	}
	/*
	 *	Create a scalar equivalent matrix, fill principal diagonal with rhs.
	 */
	explicit Matrix4(T const & rhs)
	{
		vectors_.values_[0].values_[0] = rhs;
		vectors_.values_[0].values_[1] = 0;
		vectors_.values_[0].values_[2] = 0;
		vectors_.values_[0].values_[3] = 0;

		vectors_.values_[1].values_[0] = 0;
		vectors_.values_[1].values_[1] = rhs;
		vectors_.values_[1].values_[2] = 0;
		vectors_.values_[1].values_[3] = 0;

		vectors_.values_[2].values_[0] = 0;
		vectors_.values_[2].values_[1] = 0;
		vectors_.values_[2].values_[2] = rhs;
		vectors_.values_[2].values_[3] = 0;

		vectors_.values_[3].values_[0] = 0;
		vectors_.values_[3].values_[1] = 0;
		vectors_.values_[3].values_[2] = 0;
		vectors_.values_[3].values_[3] = rhs;
	}
	/*
	 *	@rhs: column major array.
	 */
	explicit Matrix4(T const * rhs)
	{
		for (uint32 i = 0; i < Dimension; ++i)
		{
			vectors_[i] = Vector<T, Dimension>(rhs);
			rhs += Dimension;
		}
	}
	/*
	 *	@rhs: column major array.
	 */
	explicit Matrix4(std::array<T, ElementCount> const & rhs) // TODO need test
	{
		for (uint32 i = 0; i < Dimension; ++i)
		{
			vectors_[i] = Vector<T, Dimension>(&rhs[i << 2]);
		}
	}
	/*
	 *	@rhs: column major vectors.
	 */
	explicit Matrix4(Vector<Vector<T, Dimension>, Dimension> const & rhs)
		: vectors_(rhs)
	{
	}
	Matrix4(Matrix4 const & rhs)
		: vectors_(rhs.vectors_)
	{
	}
	template <typename U>
	Matrix4(Matrix4<U> const & rhs)
		: vectors_(rhs.vectors_)
	{
	}
	/*
	 *	parameters subscript: row, column
	 */
	Matrix4(T const & m11, T const & m21, T const & m31, T const & m41,
		T const & m12, T const & m22, T const & m32, T const & m42,
		T const & m13, T const & m23, T const & m33, T const & m43,
		T const & m14, T const & m24, T const & m34, T const & m44)
	{
		vectors_.values_[0].values_[0] = m11;
		vectors_.values_[0].values_[1] = m21;
		vectors_.values_[0].values_[2] = m31;
		vectors_.values_[0].values_[3] = m41;

		vectors_.values_[1].values_[0] = m12;
		vectors_.values_[1].values_[1] = m22;
		vectors_.values_[1].values_[2] = m32;
		vectors_.values_[1].values_[3] = m42;

		vectors_.values_[2].values_[0] = m13;
		vectors_.values_[2].values_[1] = m23;
		vectors_.values_[2].values_[2] = m33;
		vectors_.values_[2].values_[3] = m43;

		vectors_.values_[3].values_[0] = m14;
		vectors_.values_[3].values_[1] = m24;
		vectors_.values_[3].values_[2] = m34;
		vectors_.values_[3].values_[3] = m44;
	}

	static Matrix4 const & Zero()
	{
		static Matrix4 zero(
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0);
		return zero;
	}

	static Matrix4 const & Identity()
	{
		static Matrix4 identity(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
		return identity;
	}

	ConstReference operator()(uint32 row, uint32 column) const
	{
		return vectors_[column][row];
	}

	/*
	 *	@index: column major index.
	 */
	ConstReference operator[](uint32 index) const
	{
		return *(&vectors_[0][0] + index);
	}


	Vector<T, Dimension> const Row(uint32 index) const
	{
		Vector<T, Dimension> result;
		for (uint32 i = 0; i < Dimension; ++ i)
		{
			result.values_[i] = vectors_[i][index];
		}
		return result;
	}
	Vector<T, Dimension> const & Column(uint32 index) const
	{
		return vectors_[index];
	}

	friend Matrix4 operator+(Matrix4 const & lhs, Matrix4 const & rhs)
	{
		return Matrix4(lhs.vectors_ + rhs.vectors_);
	}
	friend Matrix4 operator-(Matrix4 const & lhs, Matrix4 const & rhs)
	{
		return Matrix4(lhs.vectors_ - rhs.vectors_);
	}

	friend Matrix4 operator*(Matrix4 const & lhs, Matrix4 const & rhs)
	{
		Matrix4 temp;
		MathHelper::MatrixHepler<T>::DoMultiply(Matrix4::MultiplyHelper(temp), &lhs[0], &rhs[0]);
		return temp;
	}
	friend Matrix4 operator*(Matrix4 const & lhs, T const & rhs)
	{
		return Matrix4(lhs.vectors_ * Vector<T, Dimension>(rhs));
	}
	friend Matrix4 operator*(T const & lhs, Matrix4 const & rhs)
	{
		return Matrix4(Vector<T, Dimension>(lhs) * rhs.vectors_);
	}

	friend Matrix4 operator/(Matrix4 const & lhs, T const & rhs)
	{
		return Matrix4(lhs.vectors_ / Vector<T, Dimension>(rhs));
	}

	Matrix4& operator=(Matrix4 const & rhs)
	{
		if (this != &rhs)
		{
			vectors_ = rhs.vectors_;
		}
		return *this;
	}
	template <typename U>
	Matrix4 const & operator=(Matrix4<U> const & rhs)
	{
		if (this != &rhs)
		{
			vectors_ = rhs.vectors_;
		}
		return *this;
	}

	Matrix4 const & operator+() const
	{
		return *this;
	}
	Matrix4 operator-() const
	{
		Matrix4 temp(*this);
		temp.vectors_ = -vectors_;
		return temp;
	}

	friend bool operator==(Matrix4 const & lhs, Matrix4 const & rhs)
	{
		return lhs.vectors_ == rhs.vectors_;
	}
	friend bool operator!=(Matrix4 const & lhs, Matrix4 const & rhs)
	{
		return lhs.vectors_ != rhs.vectors_;
	}

	Matrix4 Transpose() const
	{
		return Matrix4(operator[](0), operator[](4), operator[](8), operator[](12),
			operator[](1), operator[](5), operator[](9),operator[](13),
			operator[](2), operator[](6), operator[](10), operator[](14),
			operator[](3), operator[](7), operator[](11), operator[](15));
	}

	Matrix4 Inverse() const
	{
		Matrix4 temp;
		bool invertible = MathHelper::MatrixHepler<T>::DoInverse(&temp.vectors_.values_[0].values_[0], &vectors_[0][0]);
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
	static Pointer MultiplyHelper(Matrix4& result)
	{
		return &result.vectors_.values_[0].values_[0];
	}

private:
	Vector<Vector<T, Dimension>, Dimension> vectors_;
};

typedef Matrix4<float> floatM44;
