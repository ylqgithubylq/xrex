#include "XREX.hpp"

#include "Camera.hpp"


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
	TransformationSP transformation = GetOwnerSceneObject()->GetComponent<Transformation>();
	dirty_ = transformation->NeedUpdate();
	if (dirty_)
	{
		// RotationMatrixY(PI) make +z as the view direction
		viewMatrix_ = RotationMatrixY(PI) * MatrixFromQuaternion(transformation->GetOrientation().Conjugate()) * TranslationMatrix(-transformation->GetPosition());
		dirty_ = false;
	}
}
