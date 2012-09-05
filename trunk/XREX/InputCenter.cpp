#include "XREX.hpp"

#include "InputCenter.hpp"
#include "InputHandler.hpp"

#include <algorithm>

using std::unordered_map;

InputCenter::InputCenter()
	: semanticStates_(static_cast<uint32>(InputSemantic::InputSemanticInvalid) + 1, false), previousPointerPosition_(0, 0), pointerPosition_(0, 0)
{
}


InputCenter::~InputCenter()
{
}


bool InputCenter::AddInputHandler(InputHandlerSP const& inputHandler)
{
	auto result = inputHandlers_.insert(inputHandler);
	return result.second;
}

bool InputCenter::RemoveInputHandler(InputHandlerSP const& inputHandler)
{
	auto found = inputHandlers_.find(inputHandler);
	if (found == inputHandlers_.end())
	{
		return false;
	}
	inputHandlers_.erase(found);

	return true;
}

void InputCenter::ExecuteAllQueuedActions()
{
	double currentTime = Application::GetInstance().GetElapsedTime();
	for (auto i = inputHandlers_.begin(); i != inputHandlers_.end(); ++i)
	{
		(*i)->OnBeforeLogicFrame(currentTime);
	}

	while (!actionQueue_.empty())
	{
		Action& action = actionQueue_.front();
		action.inputCommand();
		actionQueue_.pop();
	}
}



void InputCenter::GenerateKeyDown(InputSemantic semantic)
{
	semanticStates_[static_cast<uint32>(semantic)] = true;
	DispatchInputEvent(semantic, static_cast<uint32>(true));
}

void InputCenter::GenerateKeyUp(InputSemantic semantic)
{
	semanticStates_[static_cast<uint32>(semantic)] = false;
	DispatchInputEvent(semantic, static_cast<uint32>(false));
}

void InputCenter::GenerateMouseDown(InputSemantic semantic, int32 x, int32 y)
{
	semanticStates_[static_cast<uint32>(semantic)] = true;
	previousPointerPosition_ = pointerPosition_;
	pointerPosition_ = VectorT<int32, 2>(x, y);
	DispatchInputEvent(semantic, static_cast<uint32>(true));
}

void InputCenter::GenerateMouseUp(InputSemantic semantic, int32 x, int32 y)
{
	semanticStates_[static_cast<uint32>(semantic)] = false;
	previousPointerPosition_ = pointerPosition_;
	pointerPosition_ = VectorT<int32, 2>(x, y);
	DispatchInputEvent(semantic, static_cast<uint32>(false));
}

void InputCenter::GenerateMouseWheel(InputSemantic semantic, int32 x, int32 y, int32 wheelDelta)
{
	previousPointerPosition_ = pointerPosition_;
	pointerPosition_ = VectorT<int32, 2>(x, y);
	DispatchInputEvent(semantic, wheelDelta);
}

void InputCenter::GenerateMouseMove(InputSemantic semantic, int32 x, int32 y)
{
	previousPointerPosition_ = pointerPosition_;
	pointerPosition_ = VectorT<int32, 2>(x, y);
	DispatchInputEvent(semantic, 0);
}


void InputCenter::DispatchInputEvent(InputSemantic semantic, int32 data)
{
	double currentTime = Application::GetInstance().GetElapsedTime();
	for (auto i = inputHandlers_.begin(); i != inputHandlers_.end(); ++i)
	{
		InputHandler::ActionMap const& actionMap = (*i)->GetActionMap();
		unordered_map<InputSemantic, uint32> const& mapData = actionMap.GetAllActions();
		auto found = mapData.find(semantic);
		if (found != mapData.end())
		{
			(*i)->OnAction(found->second, data, pointerPosition_, currentTime);
		}
	}
}
