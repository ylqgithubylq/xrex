#include "XREX.hpp"

#include "Camera.hpp"


Color const Camera::DefaultBackgroundColor = Color(0.4f, 0.6f, 0.9f, 1.0f);

Camera::Camera()
	: backgroundColor_(DefaultBackgroundColor), active_(true)
{
}


Camera::~Camera()
{
}
