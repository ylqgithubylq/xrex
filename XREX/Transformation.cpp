#include "XREX.hpp"

#include "Transformation.hpp"


Transformation::Transformation()
	: position_(floatV3::Zero), orientation_(floatQ::Identity), scaling_(floatV3(1.0f, 1.0f, 1.0f)), front_(0.0f, 0.0f, 1.0f), up_(0.0f, 1.0f, 0.0f)
{
}


Transformation::~Transformation()
{
}

void Transformation::Update() const
{
	if (dirty_)
	{
		// TODO add a GetMatrixFromTQS(v3, q, v3) to math.hpp
		modelMatrix_ = TranslationMatrix(position_) * MatrixFromQuaternion(orientation_) * ScalingMatrix(scaling_);
		dirty_ = false;
		if (parent_.expired()) // no parent
		{
			worldMatrix_ = modelMatrix_;
		}
	}
	// TODO how to know parent is dirty or not? dirty flag of parent will be cleared when updated.
	if (!parent_.expired())
	{
		worldMatrix_ = parent_.lock()->GetWorldMatrix() * modelMatrix_;
	}
}

