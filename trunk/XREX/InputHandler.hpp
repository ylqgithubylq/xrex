#pragma once

#include "Declare.hpp"

#include "InputCenter.hpp"
#include "XREXContext.hpp"

#include <functional>
#include <unordered_map>


namespace XREX
{

	class XREX_API InputHandler
		: Noncopyable
	{
	public:
		class ActionMap
			: Noncopyable
		{
		public:
			ActionMap()
			{
			}
			ActionMap(ActionMap&& rhs)
				: actionMap_(std::move(rhs.actionMap_))
			{
			}
			ActionMap& operator =(ActionMap&& rhs)
			{
				actionMap_ = std::move(rhs.actionMap_);
				return *this;
			}
			void Set(InputCenter::InputSemantic inputSemantic, uint32 mappedSemantic)
			{
				actionMap_[inputSemantic] = mappedSemantic;
			}
			bool Contains(InputCenter::InputSemantic inputSemantic) const
			{
				return actionMap_.find(inputSemantic) != actionMap_.end();
			}
			uint32 Get(InputCenter::InputSemantic inputSemantic) const
			{
				assert(Contains(inputSemantic));
				return actionMap_.at(inputSemantic);
			}
			void Remove(InputCenter::InputSemantic inputSemantic)
			{
				assert(Contains(inputSemantic));
				actionMap_.erase(inputSemantic);
			}

			std::unordered_map<InputCenter::InputSemantic, uint32> const& GetAllActions() const
			{
				return actionMap_;
			}

		private:
			std::unordered_map<InputCenter::InputSemantic, uint32> actionMap_;
		};



	public:
		virtual ~InputHandler();

		ActionMap const& GetActionMap() const
		{
			return actionMap_;
		}

		void OnAction(InputCenter::InputEvent const& inputEvent);

		void OnBeforeLogicFrame(double currentTime);


	protected:

		InputHandler(ActionMap&& actionMap);

		/*
		 *	Override this if something need to do before a logic frame begin.
		 */
		virtual bool DoOnBeforeLogicFrame(double currentTime, std::function<void()>* generatedAction)
		{
			return false;
		}
		/*
		 *	@data: if event is Key/Mouse Down, data is 1, Key/Mouse Up, data is 0. else if event is mouse wheel, data is the wheel delta.
		 *	@return: true indicates action generated.
		 */
		virtual bool GenerateAction(InputCenter::InputEvent const& inputEvent, std::function<void()>* generatedAction) = 0;

		ActionMap& GetMutableActionMap()
		{
			return actionMap_;
		}

	private:
		ActionMap actionMap_;
	};

}
