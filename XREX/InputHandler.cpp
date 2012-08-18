#include "XREX.hpp"

#include "InputHandler.hpp"


InputHandler::InputHandler()
	: initialized_(false)
{
}


InputHandler::~InputHandler()
{
}

void InputHandler::OnAction(uint32 mappedSemantic, uint32 data, VectorT<uint32, 2> pointerPosition, VectorT<uint32, 2> previousPointerPosition, double currentTime)
{
	std::function<void()> action;
	if (GenerateAction(mappedSemantic, data, pointerPosition, previousPointerPosition, currentTime, &action))
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

