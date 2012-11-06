#include "XREX.hpp"

#include "Transformation.hpp"

namespace XREX
{

	Transformation::Transformation()
		: position_(floatV3::Zero), orientation_(floatQ::Identity), scaling_(1, 1, 1), globalPosition_(0, 0, 0),
		front_(0, 0, 1), up_(0, 1, 0), parentWorldMatrix_(floatM44::Identity), dirty_(true)
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
				globalPosition_ = Transform(worldMatrix_, floatV3::Zero);
				parentWorldMatrix_ = parentMatrix;
			}
			else if (dirty_)
			{
				worldMatrix_ = parentMatrix * modelMatrix_;
				globalPosition_ = Transform(worldMatrix_, floatV3::Zero);
			}
		}
		else if (dirty_)
		{
			worldMatrix_ = modelMatrix_;
			globalPosition_ = position_;
		}
		dirty_ = false;
	}

}
