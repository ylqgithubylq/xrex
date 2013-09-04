#pragma once

#include "Declare.hpp"

#include "Input/InputCenter.hpp"
#include "Base/XREXContext.hpp"

#include <functional>
#include <unordered_map>


namespace XREX
{

	class XREX_API InputHandler
		: Noncopyable
	{
	public:
		class XREX_API ActionMap
			: Noncopyable
		{
		public:
			ActionMap()
			{
			}
			ActionMap(ActionMap&& right)
				: actionMap_(std::move(right.actionMap_))
			{
			}
			ActionMap& operator =(ActionMap&& right)
			{
				actionMap_ = std::move(right.actionMap_);
				return *this;
			}
			void Set(InputCenter::InputSemantic inputSemantic, int32 mappedSemantic)
			{
				actionMap_[inputSemantic] = mappedSemantic;
			}
			bool Contains(InputCenter::InputSemantic inputSemantic) const
			{
				return actionMap_.find(inputSemantic) != actionMap_.end();
			}
			int32 Get(InputCenter::InputSemantic inputSemantic) const
			{
				assert(Contains(inputSemantic));
				return actionMap_.at(inputSemantic);
			}
			void Remove(InputCenter::InputSemantic inputSemantic)
			{
				assert(Contains(inputSemantic));
				actionMap_.erase(inputSemantic);
			}

			std::unordered_map<InputCenter::InputSemantic, int32> const& GetAllActions() const
			{
				return actionMap_;
			}

		private:
			std::unordered_map<InputCenter::InputSemantic, int32> actionMap_;
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
		 *	@return: first component is true indicates action generated. second component will be enqueued into InputCenter if first is true.
		 */
		virtual std::pair<bool, std::function<void()>> DoOnBeforeLogicFrame(double currentTime)
		{
			return std::make_pair(false, std::function<void()>());
		}
		/*
		 *	@return: first component is true indicates action generated. second component will be enqueued into InputCenter if first is true.
		 */
		virtual std::pair<bool, std::function<void()>> GenerateAction(InputCenter::InputEvent const& inputEvent) = 0;

		ActionMap& GetMutableActionMap()
		{
			return actionMap_;
		}

	private:
		ActionMap actionMap_;
	};

}
