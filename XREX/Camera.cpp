#include "XREX.hpp"

#include "Camera.hpp"

namespace XREX
{

	Color const Camera::DefaultBackgroundColor = Color(0.4f, 0.6f, 0.9f, 1.0f);

	Camera::Camera(float fieldOfView, float aspectRatio, float near, float far)
		: fieldOfView_(fieldOfView), aspectRatio_(aspectRatio), near_(near), far_(far),
		backgroundColor_(DefaultBackgroundColor), active_(true), dirty_(true)
	{
		projectionMatrix_ = FrustumMatrix(fieldOfView, aspectRatio, near, far);
	}


	Camera::~Camera()
	{
	}

	void Camera::Update() const
	{
		static floatV3 const LocalTo = floatV3(0, 0, 1);
		static floatV3 const LocalUp = floatV3(0, 1, 0);
		TransformationSP transformation = GetOwnerSceneObject()->GetComponent<Transformation>();
		floatV3 to = TransformNormal(transformation->GetWorldMatrix(), LocalTo);
		floatV3 up = TransformNormal(transformation->GetWorldMatrix(), LocalUp);
		viewMatrix_ = LookToViewMatrix(transformation->GetGlobalPosition(), to, up);
		dirty_ = false;
	}

}