#include "XREX.hpp"

#include "InputCenter.hpp"


InputCenter::InputCenter()
	: SemanticStates_(static_cast<uint32>(InputSemantic::InputSemanticInvalid) + 1, false), previousPointerPosition_(0, 0), pointerPosition_(0, 0)
{
}


InputCenter::~InputCenter()
{
}

void InputCenter::KeyDown(InputSemantic semantic)
{
	SemanticStates_[static_cast<uint32>(semantic)] = true;
}

void InputCenter::KeyUp(InputSemantic semantic)
{
	SemanticStates_[static_cast<uint32>(semantic)] = false;
}

void InputCenter::MouseDown(InputSemantic semantic, uint32 x, uint32 y)
{
	SemanticStates_[static_cast<uint32>(semantic)] = true;
	previousPointerPosition_ = pointerPosition_;
	pointerPosition_ = VectorT<uint32, 2>(x, y);
}

void InputCenter::MouseUp(InputSemantic semantic, uint32 x, uint32 y)
{
	SemanticStates_[static_cast<uint32>(semantic)] = false;
	previousPointerPosition_ = pointerPosition_;
	pointerPosition_ = VectorT<uint32, 2>(x, y);
}

void InputCenter::MouseWheel(InputSemantic semantic, uint32 x, uint32 y, int32 wheelDelta)
{
	// semantic should not be relied on
	previousPointerPosition_ = pointerPosition_;
	pointerPosition_ = VectorT<uint32, 2>(x, y);
}

void InputCenter::MouseMove(InputSemantic semantic, uint32 x, uint32 y)
{
	// semantic should not be relied on
	previousPointerPosition_ = pointerPosition_;
	pointerPosition_ = VectorT<uint32, 2>(x, y);
}
