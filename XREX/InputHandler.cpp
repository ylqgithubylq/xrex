#include "XREX.hpp"

#include "InputHandler.hpp"

namespace XREX
{

	InputHandler::InputHandler(ActionMap&& actionMap)
		: actionMap_(std::move(actionMap))
	{
	}


	InputHandler::~InputHandler()
	{
	}

	void InputHandler::OnAction(InputCenter::InputEvent const& inputEvent)
	{
		std::function<void()> action;
		if (GenerateAction(inputEvent, &action))
		{
			XREXContext::GetInstance().GetInputCenter().EnqueueAction(std::move(action));
		}
	}

	void InputHandler::OnBeforeLogicFrame(double currentTime)
	{
		std::function<void()> action;
		if (DoOnBeforeLogicFrame(currentTime, &action))
		{
			XREXContext::GetInstance().GetInputCenter().EnqueueAction(std::move(action));
		}
	}

}
