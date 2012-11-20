#pragma once

#include "Math.hpp"
#include "Geometry.hpp"

namespace XREX
{
	/*
	 *	@outDistance: Intersection distance. Will be modified when not parallel. Can be null if not care.
	 *	@outPoint: Intersection point. Will be modified when intersected. Can be null if not care.
	 */
	template <typename T>
	bool Intersect(RayT<T> const& ray, PlaneT<T> const& plane, typename RayT<T>::Pointer outDistance, VectorT<typename RayT<T>::ValueType, 3>* outPoint);
}


