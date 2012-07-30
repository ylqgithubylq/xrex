#pragma once

#include "Declare.hpp"

#include "Component.hpp"


class Camera
	: public TemplateComponent<Camera>
{
public:
	static Color const DefaultBackgroundColor;
public:
	Camera();
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
		// TODO
		assert(false);
	}

	floatM44 GetProjectionMatrix()
	{
		// TODO
		assert(false);
	}

	void LookAt(floatV3 const & at);
	void LookAt(floatV3 const & at, floatV3 const & up);

	void SetBackgroundColor(Color & color)
	{
		backgroundColor_ = color;
	}
	Color const & GetBackgroundColor() const
	{
		return backgroundColor_;
	}


private:
	bool active_;

	floatV3 at_;
	floatV3 up_;

	Color backgroundColor_;

};

