#include "XREX.hpp"

#include "Camera.hpp"

#include "Rendering/Viewport.hpp"
#include "Base/XREXContext.hpp"
#include "Rendering/RenderingFactory.hpp"

namespace XREX
{

	Color const Camera::DefaultBackgroundColor = Color(0.4f, 0.6f, 0.9f, 1.0f);

	Camera::Camera()
		: backgroundColor_(DefaultBackgroundColor), active_(true), dirty_(true)
	{
		viewport_ = XREXContext::GetInstance().GetRenderingFactory().GetDefaultViewport();
	}


	void Camera::Update() const
	{
		static floatV3 const LocalTo = floatV3(0, 0, 1);
		static floatV3 const LocalUp = floatV3(0, 1, 0);
		TransformationSP transformation = GetOwnerSceneObject()->GetComponent<Transformation>();
		floatV3 to = TransformDirection(transformation->GetWorldMatrix(), LocalTo);
		floatV3 up = TransformDirection(transformation->GetWorldMatrix(), LocalUp);
		viewMatrix_ = LookToViewMatrix(transformation->GetWorldPosition(), to, up);
		dirty_ = false;
	}

	Ray Camera::GetViewRay(floatV2 const& position, ViewportOrigin origin)
	{
		TransformationSP transformation = GetOwnerSceneObject()->GetComponent<Transformation>();
		float xFromViewportCenter;
		float yFromViewportCenter;

		switch (origin)
		{
		case ViewportOrigin::ViewportCenter:
			{
				assert(position.X() <= 1 && position.X() >= -1);
				assert(position.Y() <= 1 && position.Y() >= -1);
				xFromViewportCenter = position.X();
				yFromViewportCenter = position.Y();
			}
			break;
		case ViewportOrigin::ViewportLowerLeft:
			{
				assert(position.X() <= 1 && position.X() >= 0);
				assert(position.Y() <= 1 && position.Y() >= 0);
				xFromViewportCenter = position.X() * 2 - 1;
				yFromViewportCenter = position.Y() * 2 - 1;
			}
			break;
		default:
			assert(false);
			break;
		}

		floatM44 inverseViewPrection = (GetProjectionMatrix() * GetViewMatrix()).Inverse();
		floatV3 nearPoint(xFromViewportCenter, yFromViewportCenter, -1);
		floatV3 middlePoint(xFromViewportCenter, yFromViewportCenter, 0);

		// Get ray origin and ray direction on near plane in world space
		floatV3 rayOrigin = Transform(inverseViewPrection, nearPoint);
		floatV3 rayTarget = Transform(inverseViewPrection, middlePoint);
		// using (rayOrigin - cameraPosition) as direction may have visible error
		floatV3 rayDirection = (rayTarget - rayOrigin);

		return Ray(rayOrigin, rayDirection);
	}



	PerspectiveCamera::PerspectiveCamera(float fieldOfView, float aspectRatio, float near, float far)
		: fieldOfView_(fieldOfView), aspectRatio_(aspectRatio), near_(near), far_(far)
	{
		projectionMatrix_ = FrustumProjectionMatrix(fieldOfView, aspectRatio, near, far);
	}


	PerspectiveCamera::~PerspectiveCamera()
	{
	}



	OrthogonalCamera::OrthogonalCamera(float width, float height, float depth)
		: width_(width), height_(height), depth_(depth)
	{
		projectionMatrix_ = OrthogonalProjectionMatrix(width, height, depth);
	}

	OrthogonalCamera::~OrthogonalCamera()
	{
	}
}