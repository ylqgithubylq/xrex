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
		// TODO replace all map by unordered_map
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

		void InitializeActionMap()
		{
			actionMap_ = GenerateActionMap(); // better using polymorphism constructor trick?
			initialized_ = true;
		}

		ActionMap const& GetActionMap() const
		{
			assert(initialized_);
			return actionMap_;
		}

		void OnAction(uint32 mappedSemantic, int32 data, VectorT<int32, 2> pointerPosition, double currentTime);

		void OnBeforeLogicFrame(double currentTime);


	protected:

		InputHandler();

		virtual ActionMap GenerateActionMap() = 0;

		virtual bool DoOnBeforeLogicFrame(double currentTime, std::function<void()>* generatedAction)
		{
			return false;
		}
		/*
		 *	@data: if event is Key/Mouse Down, data is 1, Key/Mouse Up, data is 0. else if event is mouse wheel, data is the wheel delta.
		 *	@return: true indicates action generated.
		 */
		virtual bool GenerateAction(uint32 mappedSemantic, int32 data, VectorT<int32, 2> pointerPosition, double currentTime, std::function<void()>* generatedAction) = 0;

		ActionMap& GetMutableActionMap()
		{
			return actionMap_;
		}

	private:
		ActionMap actionMap_;
		bool initialized_;
	};

}
