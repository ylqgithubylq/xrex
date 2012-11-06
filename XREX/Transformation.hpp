#pragma once

#include "Declare.hpp"

#include "Component.hpp"

namespace XREX
{

	/*
	 *	The result of this representation is scaling first, then rotation, translation last.
	 *
	 */
	class XREX_API Transformation
		: public TemplateComponent<Transformation>
	{
	public:
		Transformation();
		virtual ~Transformation() override;

		/*
		 *	@return: model matrix contains only this transformation.
		 */
		floatM44 const& GetModelMatrix() const
		{
			Update();
			return modelMatrix_;
		}

		/*
		 *	@return: model matrix includes all parent transformation.
		 */
		floatM44 const& GetWorldMatrix() const
		{
			Update();
			return worldMatrix_;
		}

		void SetParent(TransformationSP const& parent);
		TransformationSP GetParent() const
		{
			return parent_.lock();
		}

		void SetPosition(float x, float y, float z)
		{
			SetPosition(floatV3(x, y, z));
			dirty_ = true;
		}
		void SetPosition(floatV3 const& position)
		{
			position_ = position;
			dirty_ = true;
		}
		floatV3 const& GetPosition() const
		{
			return position_;
		}

		floatV3 const& GetGlobalPosition() const
		{
			Update();
			return globalPosition_;
		}

		void SetOrientation(floatQ const& orientation)
		{
			orientation_ = orientation;
			dirty_ = true;
		}
		floatQ const& GetOrientation() const
		{
			return orientation_;
		}

		void SetScaling(float s)
		{
			assert(s > 0);
			SetScaling(floatV3(s, s, s));
			dirty_ = true;
		}
		void SetScaling(float sx, float sy, float sz)
		{
			assert(sx > 0 && sy > 0 && sz > 0);
			SetScaling(floatV3(sx, sy, sz));
			dirty_ = true;
		}
		void SetScaling(floatV3 const& scaling)
		{
			assert(scaling.X() > 0 && scaling.Y() > 0 && scaling.Z() > 0);
			scaling_ = scaling;
			dirty_ = true;
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
			dirty_ = true;
		}
		void Translate(floatV3 const& displacement)
		{
			position_ = position_ + displacement;
			dirty_ = true;
		}

		void Scale(float s)
		{
			assert(s > 0);
			Scale(floatV3(s, s, s));
			dirty_ = true;
		}
		void Scale(float sx, float sy, float sz)
		{
			assert(sx > 0 && sy > 0 && sz > 0);
			Scale(floatV3(sx, sy, sz));
			dirty_ = true;
		}
		void Scale(floatV3 const& s)
		{
			assert(s.X() > 0 && s.Y() > 0 && s.Z() > 0);
			scaling_ = scaling_ * s;
			dirty_ = true;
		}


		void Rotate(float angle, float x, float y, float z)
		{
			orientation_ = RotationQuaternion(angle, x, y, z) * orientation_;
			dirty_ = true;
		}
		void Rotate(float angle, floatV3 const& axis)
		{
			orientation_ = RotationQuaternion(angle, axis) * orientation_;
			dirty_ = true;
		}
		void Rotate(floatQ const rotation)
		{
			orientation_ = rotation * orientation_;
			dirty_ = true;
		}

		floatV3 const& GetModelFrontDirection() const
		{
			return front_;
		}
		/*
		 *	Used by FaceTo.
		 *	Default value is +z.
		 */
		void SetModelFrontDirection(floatV3 const& front)
		{
			front_ = front;
			dirty_ = true;
		}

		floatV3 const& GetModelUpDirection() const
		{
			return up_;
		}
		/*
		 *	Used by FaceTo.
		 *	Default value is +y.
		 */
		void SetModelUpDirection(floatV3 const& up)
		{
			up_ = up;
			dirty_ = true;
		}
		/*
		 *	Face to a direction, using front and up as reference.
		 *	@up: up direction in world space as reference.
		 */
		void FaceToDirection(floatV3 const& to, floatV3 const& up)
		{
			orientation_ = FaceToQuaternion(to, up, front_, up_);
			dirty_ = true;
		}
		/*
		 *	Face to a position, using front and up as reference.
		 *	@up: up direction in world space as reference.
		 */
		void FaceToPosition(floatV3 const& to, floatV3 const& up)
		{
			orientation_ = FaceToQuaternion(to - position_, up, front_, up_);
			dirty_ = true;
		}

	protected:

	private:
		void Update() const;

	private:
		std::weak_ptr<Transformation> parent_;

		floatV3 position_;
		floatQ orientation_;
		floatV3 scaling_;

		floatV3 mutable globalPosition_;

		floatV3 front_;
		floatV3 up_;

		floatM44 mutable modelMatrix_;
		floatM44 mutable worldMatrix_;

		floatM44 mutable parentWorldMatrix_;

		bool mutable dirty_;
	};

}
