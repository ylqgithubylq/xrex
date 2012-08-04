#include "XREX.hpp"

#include "Camera.hpp"


Color const Camera::DefaultBackgroundColor = Color(0.4f, 0.6f, 0.9f, 1.0f);

Camera::Camera(float fieldOfView, float aspectRatio, float near, float far)
	: fieldOfView_(fieldOfView), aspectRatio_(aspectRatio), near_(near), far_(far),
	backgroundColor_(DefaultBackgroundColor), active_(true)
{
	projectionMatrix_ = Frustum(fieldOfView, aspectRatio, near, far);
}


Camera::~Camera()
{
}
