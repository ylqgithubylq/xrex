#pragma once

#include "Declare.hpp"
#include "inputhandler.hpp"

namespace XREX
{
	class XREX_API OrbitCameraController
		: public InputHandler
	{
	public:
		enum class OrbitSemantic
		{
			Orbit,
			TriggerOrbit,
			ScaleDistance,
			OrbitSemanticCount,
		};
	public:
		OrbitCameraController(SceneObjectSP const& target, float orbitScaler = 1.0f, float distanceScalingScaler = 1.0f);
		virtual ~OrbitCameraController() override;

		/*
		 *	Notice: Will set parent Transformation of camera to Transformation of target.
		 */
		void AttachToCamera(SceneObjectSP const& cameraObject);

	protected:
		virtual std::pair<bool, std::function<void()>> GenerateAction(InputCenter::InputEvent const& inputEvent) override;

	private:	
		std::function<void()> GenerateOrbitAction(floatV2 const& deltaTurn);
		std::function<void()> GenerateDistanceScalingAction(float delta);
		void UpdateTranformation();
	private:
		SceneObjectSP cameraObject_;
		SceneObjectSP target_;

		float orbitScaler_;
		float distanceScalingScaler_;

		bool orbitTriggered_;
		intV2 previousPointerPosition_;
		float azimuthAngle_; // camera to object
		float elevationAngle_; // camera to object
		float distance_;
	};
}


