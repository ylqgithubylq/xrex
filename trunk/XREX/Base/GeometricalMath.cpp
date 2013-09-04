#include "XREX.hpp"

#include "GeometricalMath.hpp"

namespace XREX
{
	template <typename T>
	bool Intersect(RayT<T> const& ray, PlaneT<T> const& plane, typename RayT<T>::Pointer outDistance, VectorT<typename RayT<T>::ValueType, 3>* outPoint)
	{
		T pnDotRd = Dot(plane.GetNormal(), ray.GetDirection());
		if (Equal<T>(pnDotRd, 0)) // parallel
		{
			return false;
		}

		T t = -((Dot(plane.GetNormal(), ray.GetOrigin()) + plane.GetDistance()) / pnDotRd);
		if (outDistance)
		{
			*outDistance = t;
		}
		if (t > 0)
		{
			if (outPoint)
			{
				*outPoint = ray.GetOrigin() + ray.GetDirection() * t;
			}
			return true;
		}
		return false;
	}
	template XREX_API bool Intersect(Ray const& ray, Plane const& plane, float* outDistance, floatV3* outPoint);
}

