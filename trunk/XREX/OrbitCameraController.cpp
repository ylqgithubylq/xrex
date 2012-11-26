#include "XREX.hpp"

#include "OrbitCameraController.hpp"

#include "SceneObject.hpp"
#include "Camera.hpp"

using std::function;

namespace XREX
{
	namespace
	{
		InputHandler::ActionMap GenerateActionMap()
		{
			InputHandler::ActionMap map;
			map.Set(InputCenter::InputSemantic::M_Move, static_cast<uint32>(OrbitCameraController::OrbitSemantic::Orbit));
			map.Set(InputCenter::InputSemantic::M_Button0, static_cast<uint32>(OrbitCameraController::OrbitSemantic::TriggerOrbit));
			map.Set(InputCenter::InputSemantic::M_Wheel, static_cast<uint32>(OrbitCameraController::OrbitSemantic::ScaleDistance));
			return map;
		}
	}

	OrbitCameraController::OrbitCameraController(SceneObjectSP const& target, float orbitScaler, float distanceScalingScaler)
		: InputHandler(GenerateActionMap()), target_(target), orbitScaler_(orbitScaler), distanceScalingScaler_(distanceScalingScaler),
		orbitTriggered_(false), previousPointerPosition_(0, 0), azimuthAngle_(0), elevationAngle_(0), distance_(0)
	{
		assert(target != nullptr);
	}


	OrbitCameraController::~OrbitCameraController()
	{
	}

	void OrbitCameraController::AttachToCamera(SceneObjectSP const& cameraObject)
	{
		assert(cameraObject->GetComponent<Camera>() != nullptr);
		cameraObject_ = cameraObject;
		TransformationSP const& cameraTransformation = cameraObject_->GetComponent<Transformation>();
		TransformationSP const& targetTransformation = target_->GetComponent<Transformation>();

		// make camera position in world space the same.
		floatV3 displacement = cameraTransformation->GetWorldPosition() - targetTransformation->GetWorldPosition();
		cameraTransformation->SetParent(targetTransformation);
		cameraTransformation->SetPosition(displacement);
		distance_ = displacement.Length();

		floatV3 front = cameraTransformation->GetModelFrontDirection();
		floatV3 right = Cross(front, cameraTransformation->GetModelUpDirection());
		floatV3 up = Cross(right, front);

		floatV3 actualHeading = -displacement;
		float angleToUp = Angle(actualHeading, up);
		elevationAngle_ = PI / 2 - angleToUp;

		// get the projection vector of the displacement in front & right plane
		floatV3 axisToRotateDown = Cross(actualHeading, up);
		// rotate back to the plane
		floatV3 directionInFrontRightPlane = RotateByQuaternion(RotationQuaternion(-elevationAngle_, axisToRotateDown), actualHeading);

		bool positive = Dot(Cross(directionInFrontRightPlane, front), up) > 0;
		// the angle from directionInFrontRightPlane to front referenced by axis up, so negate it.
		azimuthAngle_ = -(Angle(directionInFrontRightPlane, front) * (positive ? 1 : -1));


		floatQ elevation = RotationQuaternion(elevationAngle_, right);
		floatQ azimuth = RotationQuaternion(azimuthAngle_, up);
		floatQ totalRotation = azimuth * elevation;

		// face to the target object, at the origin of local space.
		cameraTransformation->FaceToPosition(floatV3::Zero, RotateByQuaternion(totalRotation, cameraTransformation->GetModelUpDirection()));


	}

	std::pair<bool, function<void()>> OrbitCameraController::GenerateAction(InputCenter::InputEvent const& inputEvent)
	{
		switch (static_cast<OrbitSemantic>(inputEvent.mappedSemantic))
		{
		case OrbitSemantic::Orbit:
			{
				if (orbitTriggered_)
				{
					intV2 delta = inputEvent.pointerPosition - previousPointerPosition_;
					// negative x because screen coordinate is 180 rotated by Y axis of camera coordinate
					floatV2 deltaTurn(-delta.X() * orbitScaler_, delta.Y() * orbitScaler_);
					
					XREXContext::GetInstance().GetInputCenter().EnqueueAction(GenerateOrbitAction(deltaTurn));
				}
				previousPointerPosition_ = inputEvent.pointerPosition;
			}
			break;
		case OrbitSemantic::TriggerOrbit:
			{
				orbitTriggered_ = !!inputEvent.data;
			}
			break;
		case OrbitSemantic::ScaleDistance:
			{
				float delta = inputEvent.data * distanceScalingScaler_ * 0.01f;
				XREXContext::GetInstance().GetInputCenter().EnqueueAction(GenerateDistanceScalingAction(delta));
			}
			break;
		default:
			assert(false);
			break;
		}
		return std::make_pair(false, function<void()>());
	}

	function<void()> OrbitCameraController::GenerateOrbitAction(floatV2 const& deltaTurn)
	{

		function<void()> action = [this, deltaTurn] ()
		{
			float deltaX = RadianFromDegree(deltaTurn.X()) * orbitScaler_;
			float deltaY = RadianFromDegree(deltaTurn.Y()) * orbitScaler_;
			azimuthAngle_ += deltaX;
			elevationAngle_ += deltaY;
			if (azimuthAngle_ > PI)
			{
				azimuthAngle_ -= 2 * PI;
			}
			else if (azimuthAngle_ < -PI)
			{
				azimuthAngle_ += 2 * PI;
			}
			 // clamp to [-PI / 2 + 5degree, PI / 2 - 5degree]
			elevationAngle_ = std::max(std::min(elevationAngle_, PI / 2 - RadianFromDegree(5)), -PI / 2 + RadianFromDegree(5));

			UpdateTranformation();
		};
		return action;
	}

	function<void()> OrbitCameraController::GenerateDistanceScalingAction(float delta)
	{
		function<void()> action = [this, delta] ()
		{
			float distanceDelta = std::sqrt(distance_ + 1) * delta;
			if (!(distanceDelta < -distance_))
			{
				distance_ = (distance_ + distanceDelta);
			}
			UpdateTranformation();
		};
		return action;
	}

	void OrbitCameraController::UpdateTranformation()
	{
		TransformationSP cameraTransformation = cameraObject_->GetComponent<Transformation>();
		floatV3 up = cameraTransformation->GetModelUpDirection();
		floatV3 right = Cross(cameraTransformation->GetModelFrontDirection(), up);
		up = Cross(right, cameraTransformation->GetModelFrontDirection());

		floatQ elevation = RotationQuaternion(elevationAngle_, right);
		floatQ azimuth = RotationQuaternion(azimuthAngle_, up);
		floatQ totalRotation = azimuth * elevation;

		floatV3 heading = RotateByQuaternion(totalRotation, cameraTransformation->GetModelFrontDirection());
		cameraTransformation->SetPosition(-(heading * distance_)); // move camera backward
		cameraTransformation->FaceToPosition(floatV3::Zero, RotateByQuaternion(totalRotation, cameraTransformation->GetModelUpDirection())); // look at target
	}

}
