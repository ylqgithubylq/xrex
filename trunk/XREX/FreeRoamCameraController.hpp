#pragma once

#include "Declare.hpp"

#include "InputHandler.hpp"

#include "InputCenter.hpp"

#include <functional>
#include <vector>

namespace XREX
{

	class XREX_API FreeRoamCameraController
		: public InputHandler
	{
	public:
		enum class RoamSemantic
		{
			MoveForward,
			MoveBack,
			MoveLeft,
			MoveRight,
			MoveUp,
			MoveDown,
			RollLeft,
			RollRight,
			Turn,
			TriggerTurn,
			SpeedUp,

			FPSSemanticCount,
		};

	public:
		FreeRoamCameraController(float moveScaler = 5.0f, float rotateScaler = 1.0f,  float speedScaler = 10.0);
		virtual ~FreeRoamCameraController() override;

		void AttachToCamera(SceneObjectSP const& cameraObject);

	protected:

		virtual bool DoOnBeforeLogicFrame(double currentTime, std::function<void()>* generatedAction) override;

		virtual ActionMap GenerateActionMap() override;

		virtual bool GenerateAction(uint32 mappedSemantic, int32 data, intV2 pointerPosition,
			double currentTime, std::function<void()>* generatedAction) override;

	private:
		std::function<void()> GenerateFrameAction(float delta);
		std::function<void()> GenerateMoveAction(float forward, float left, float up);
		std::function<void()> GenerateRollAction(float roll);
		std::function<void()> GenerateRotateAction(floatV2 const& deltaTurn);


	private:
		SceneObjectSP cameraObject_;

		float moveScaler_;
		float rotateScaler_;
		float speedScaler_;

		double previousFrameTime_;
		intV2 previousPointerPosition_;

		std::vector<int32> semanticStates_;
		int32 forward_;
		int32 left_;
		int32 up_;
		int32 roll_;

		bool turnTriggered_;
		bool spedUp_;
	};

}
