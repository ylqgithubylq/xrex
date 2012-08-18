#pragma once

#include "Declare.hpp"

#include "Component.hpp"
#include "SceneObject.hpp"
#include "Transformation.hpp"

/*
 *	+z is the front direction.
 */
class Camera
	: public TemplateComponent<Camera>
{
public:
	static Color const DefaultBackgroundColor;
public:
	Camera(float fieldOfView, float aspectRatio, float near, float far);
	virtual ~Camera();

	bool IsActive() const
	{
		return active_;
	}
	void SetActive(bool active)
	{
		active_ = active;
	}

	floatM44 GetViewMatrix()
	{
		return viewMatrix_;
	}

	floatM44 GetProjectionMatrix()
	{
		return projectionMatrix_;
	}



	void SetBackgroundColor(Color & color)
	{
		backgroundColor_ = color;
	}
	Color const & GetBackgroundColor() const
	{
		return backgroundColor_;
	}

	virtual void Update() override
	{
		SceneObjectSP sceneObject = GetOwnerSceneObject();
		TransformationSP transformation = sceneObject->GetComponent<Transformation>();
		viewMatrix_ = RotationMatrixY(PI) * MatrixFromQuaternion(transformation->GetOrientation().Conjugate()) * TranslationMatrix(-transformation->GetPosition());
	}


private:
	float fieldOfView_;
	float aspectRatio_;
	float near_;
	float far_;

	floatM44 viewMatrix_;
	floatM44 projectionMatrix_;


	Color backgroundColor_;

	bool active_;

};

