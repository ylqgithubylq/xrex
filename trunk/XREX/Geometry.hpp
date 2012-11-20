#pragma once

#include "BasicType.hpp"
#include "Vector.hpp"

// TODO
namespace XREX
{

	template <typename T>
	class RayT
	{
		template <typename U>
		friend class RayT;

	public:
		typedef T ValueType;

		typedef ValueType* Pointer;
		typedef ValueType const* ConstPointer;

		typedef ValueType& Reference;
		typedef ValueType const& ConstReference;

	public:
		RayT(VectorT<T, 3> const& origin, VectorT<T, 3> const& direction)
			: origin_(origin), direction_(direction.Normalize())
		{
			auto& zero = VectorT<T, 3>::Zero;
			assert(direction != zero);
		}
		RayT(RayT const& rhs)
			: origin_(rhs.origin_), direction_(rhs.direction_)
		{
		}
		template <typename U>
		RayT(RayT<U> const& rhs)
			: origin_(rhs.origin_), direction_(rhs.direction_)
		{
		}

		RayT& operator =(RayT const& rhs)
		{
			if (this != &rhs)
			{
				origin_ = rhs.origin_;
				direction_ = rhs.direction_;
			}
		}
		template <typename U>
		RayT& operator =(RayT<U> const& rhs)
		{
			if (this != &rhs)
			{
				origin_ = rhs.origin_;
				direction_ = rhs.direction_;
			}
		}

		VectorT<T, 3> const& GetOrigin() const
		{
			return origin_;
		}
		VectorT<T, 3> const& GetDirection() const
		{
			return direction_;
		}
		VectorT<T, 3> const& GetPoint(T const& t) const
		{
			return origin_ + t * direction_;
		}
	private:
		VectorT<T, 3> origin_;
		VectorT<T, 3> direction_;
	};
	typedef RayT<float> Ray;


	template <typename T>
	class SegmentT
	{
	public:
		SegmentT();

	private:

	};

	/*
	 *	Plane: Ax + By + Cz + D, represented as normal * p + distance, where p is a point on the plane.
	 */
	template <typename T>
	class PlaneT
	{
		template <typename U>
		friend class PlaneT;

	public:
		typedef T ValueType;

		typedef ValueType* Pointer;
		typedef ValueType const* ConstPointer;

		typedef ValueType& Reference;
		typedef ValueType const& ConstReference;

	public:
		PlaneT(VectorT<T, 3> const& normal, T const& distance)
			: normal_(normal), distance_(distance)
		{
		}

		PlaneT(PlaneT const& rhs)
			: normal_(rhs.normal_), distance_(rhs.distance_)
		{
		}
		template <typename U>
		PlaneT(PlaneT<U> const& rhs)
			: normal_(rhs.normal_), distance_(rhs.distance_)
		{
		}

		PlaneT& operator =(PlaneT const& rhs)
		{
			if (this != &rhs)
			{
				normal_ = rhs.normal_;
				distance_ = rhs.distance_;
			}
		}
		template <typename U>
		PlaneT& operator =(PlaneT<U> const& rhs)
		{
			if (this != &rhs)
			{
				normal_ = rhs.normal_;
				distance_ = rhs.distance_;
			}
		}

		VectorT<T, 3> const& GetNormal() const
		{
			return normal_;
		}
		T const& GetDistance() const
		{
			return distance_;
		}

	private:
		VectorT<T, 3> normal_;
		T distance_;
	};
	typedef PlaneT<float> Plane;

	template <typename T>
	class FrustumT
	{
	public:
		FrustumT();

	private:

	};

}