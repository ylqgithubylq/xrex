#include "XREX.hpp"

#include "InputHandler.hpp"


InputHandler::InputHandler()
	: initialized_(false)
{
}


InputHandler::~InputHandler()
{
}

void InputHandler::OnAction(uint32 mappedSemantic, int32 data, VectorT<int32, 2> pointerPosition, double currentTime)
{
	std::function<void()> action;
	if (GenerateAction(mappedSemantic, data, pointerPosition, currentTime, &action))
	{
		Application::GetInstance().GetInputCenter().EnqueueAction(std::move(action));
	}
}

void InputHandler::OnBeforeLogicFrame(double currentTime)
{
	std::function<void()> action;
	if (DoOnBeforeLogicFrame(currentTime, &action))
	{
		Application::GetInstance().GetInputCenter().EnqueueAction(std::move(action));
	}
}
