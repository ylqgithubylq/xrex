#pragma once

#include "Declare.hpp"

#include "Component.hpp"

/*
 *	The result of this representation is scaling first, then rotation, translation last.
 *
 */
class Transformation
	: public TemplateComponent<Transformation>
{
public:
	Transformation();
	virtual ~Transformation();

	virtual void Update() override
	{
		modelMatrix_ = Translation(position_) * orientation_ * Scaling(scaling_);
	}

	floatM44 const & GetModelMatrix() const
	{
		return modelMatrix_;
	}

	void SetPosition(float x, float y, float z)
	{
		SetPosition(floatV3(x, y, z));
	}
	void SetPosition(floatV3 const & position)
	{
		position_ = position;
	}
	floatV3 const & GetPosition() const
	{
		return position_;
	}

	void SetOrientation(float angle, float x, float y, float z)
	{
		orientation_ = Rotation(angle, x, y, z);
	}
	void SetOrientation(float angle, floatV3 const & axis)
	{
		orientation_ = Rotation(angle, axis);
	}
	void SetOrientation(floatM44 const & orientation)
	{
		orientation_ = orientation;
	}
	floatM44 const & GetOrientation() const
	{
		return orientation_;
	}

	void SetScaling(float s)
	{
		SetScaling(floatV3(s, s, s));
	}
	void SetScaling(float sx, float sy, float sz)
	{
		SetScaling(floatV3(sx, sy, sz));
	}
	void SetScaling(floatV3 const & scaling)
	{
		scaling_ = scaling;
	}
	floatV3 GetScaling() const
	{
		return scaling_;
	}

	bool IsUniformlyScaled() const
	{
		return scaling_.X() == scaling_.Y() && scaling_.X() == scaling_.Z();
	}

	void Translate(float x, float y, float z)
	{
		Translate(floatV3(x, y, z));
	}
	void Translate(floatV3 const & displacement)
	{
		position_ = position_ + displacement;
	}

	void Scale(float s)
	{
		Scale(floatV3(s, s, s));
	}
	void Scale(float sx, float sy, float sz)
	{
		Scale(floatV3(sx, sy, sz));
	}
	void Scale(floatV3 const & s)
	{
		scaling_ = scaling_ * s;
	}

	void RotateX(float angleX)
	{
		orientation_ = RotationX(angleX) * orientation_;
	}
	void RotateY(float angleY)
	{
		orientation_ = RotationY(angleY) * orientation_;
	}
	void RotateZ(float angleZ)
	{
		orientation_ = RotationZ(angleZ) * orientation_;
	}
	void Rotate(float angle, float x, float y, float z)
	{
		orientation_ = Rotation(angle, x, y, z) * orientation_;
	}
	void Rotate(float angle, floatV3 const & axis)
	{
		orientation_ = Rotation(angle, axis) * orientation_;
	}

	/*
	 *	Used by FaceTo.
	 */
	void SetFrontDirection(floatV3 const & front)
	{
		front_ = front;
	}
	/*
	 *	Used by FaceTo.
	 *	Default value is +y.
	 */
	void SetUpDirection(floatV3 const & up)
	{
		up_ = up;
	}
	/*
	 *	Call this will set orientation. Make sure front and up are set by SetFrontDirection and SetUpDirection.
	 */
	void FaceTo(floatV3 to)
	{
		orientation_ = ::FaceTo(front_, position_, to, up_);
	}

private:
	floatV3 position_;
	floatM44 orientation_; // TODO change to quaternion
	floatV3 scaling_;

	floatV3 front_;
	floatV3 up_;

	floatM44 modelMatrix_;

};

