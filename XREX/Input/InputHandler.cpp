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
		std::pair<bool, std::function<void()>> result = GenerateAction(inputEvent);
		if (result.first)
		{
			XREXContext::GetInstance().GetInputCenter().EnqueueAction(std::move(result.second));
		}
	}

	void InputHandler::OnBeforeLogicFrame(double currentTime)
	{
		std::pair<bool, std::function<void()>> result = DoOnBeforeLogicFrame(currentTime);
		if (result.first)
		{
			XREXContext::GetInstance().GetInputCenter().EnqueueAction(std::move(result.second));
		}
	}

}
