#include "XREX.hpp"

#include "Transformation.hpp"

namespace XREX
{

	Transformation::Transformation()
		: position_(floatV3::Zero), orientation_(floatQ::Identity), scaling_(floatV3(1.0f, 1.0f, 1.0f)),
		front_(0.0f, 0.0f, 1.0f), up_(0.0f, 1.0f, 0.0f), parentWorldMatrix_(floatM44::Identity)
	{
	}


	Transformation::~Transformation()
	{
	}

	void Transformation::SetParent(TransformationSP const& parent)
	{
		assert(parent.get() != this);
		parent_ = parent;
		if (parent)
		{
			parentWorldMatrix_ = parent->GetWorldMatrix();
		}
		else
		{
			parentWorldMatrix_ = floatM44::Identity;
		}
	}

	void Transformation::Update() const
	{
		if (dirty_)
		{
			// TODO add a GetMatrixFromTQS(v3, q, v3) to math.hpp
			modelMatrix_ = TranslationMatrix(position_) * MatrixFromQuaternion(orientation_) * ScalingMatrix(scaling_);
		}
		if (!parent_.expired()) // parent exist
		{
			floatM44 const& parentMatrix = parent_.lock()->GetWorldMatrix();
			if (parentWorldMatrix_ != parentMatrix)
			{ // need update
				worldMatrix_ = parentMatrix * modelMatrix_;
				parentWorldMatrix_ = parentMatrix;
			}
			else if (dirty_)
			{
				worldMatrix_ = parentMatrix * modelMatrix_;
			}
		}
		else if (dirty_)
		{
			worldMatrix_ = modelMatrix_;
		}
		dirty_ = false;
	}

}
