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
		// TODO add a GetmatrixFromTQS(v3, q, v3)
		modelMatrix_ = TranslationMatrix(position_) * MatrixFromQuaternion(orientation_) * ScalingMatrix(scaling_);
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

	void SetOrientation(floatQ const & orientation)
	{
		orientation_ = orientation;
	}
	floatQ const & GetOrientation() const
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


	void Rotate(float angle, float x, float y, float z)
	{
		orientation_ = RotationQuaternion(angle, x, y, z) * orientation_;
	}
	void Rotate(float angle, floatV3 const & axis)
	{
		orientation_ = RotationQuaternion(angle, axis) * orientation_;
	}
	void Rotate(floatQ const rotation)
	{
		orientation_ = rotation * orientation_;
	}

	/*
	 *	Used by FaceTo.
	 *	Default value is +z.
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
	 *	Face to a direction, using front and up as reference.
	 *	@up: up direction in world space as reference.
	 */
	void FaceToDirection(floatV3 const & to, floatV3 const & up)
	{
		orientation_ = FaceToQuaternion(to, up, front_, up_);
	}
	/*
	 *	Face to a position, using front and up as reference.
	 *	@up: up direction in world space as reference.
	 */
	void FaceToPosition(floatV3 const & to, floatV3 const & up)
	{
		orientation_ = FaceToQuaternion(to - position_, up, front_, up_);
	}

private:
	floatV3 position_;
	floatQ orientation_;
	floatV3 scaling_;

	floatV3 front_;
	floatV3 up_;

	floatM44 modelMatrix_;

};

