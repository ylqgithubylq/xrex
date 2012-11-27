#include "XREX.hpp"

#include "FirstPersonCameraController.hpp"

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
			map.Set(InputCenter::InputSemantic::K_W, static_cast<uint32>(FirstPersonCameraController::MoveSemantic::MoveForward));
			map.Set(InputCenter::InputSemantic::K_S, static_cast<uint32>(FirstPersonCameraController::MoveSemantic::MoveBack));
			map.Set(InputCenter::InputSemantic::K_A, static_cast<uint32>(FirstPersonCameraController::MoveSemantic::MoveLeft));
			map.Set(InputCenter::InputSemantic::K_D, static_cast<uint32>(FirstPersonCameraController::MoveSemantic::MoveRight));
			map.Set(InputCenter::InputSemantic::K_V, static_cast<uint32>(FirstPersonCameraController::MoveSemantic::MoveUp));
			map.Set(InputCenter::InputSemantic::K_C, static_cast<uint32>(FirstPersonCameraController::MoveSemantic::MoveDown));
			map.Set(InputCenter::InputSemantic::M_Move, static_cast<uint32>(FirstPersonCameraController::MoveSemantic::Turn));
			map.Set(InputCenter::InputSemantic::K_LeftShift, static_cast<uint32>(FirstPersonCameraController::MoveSemantic::SpeedUp));
			map.Set(InputCenter::InputSemantic::K_RightShift, static_cast<uint32>(FirstPersonCameraController::MoveSemantic::SpeedUp));
			map.Set(InputCenter::InputSemantic::M_Button0, static_cast<uint32>(FirstPersonCameraController::MoveSemantic::TriggerTurn));
			return map;
		}

		float const DefaultMoveSpeed = 5.0f;
	}

	FirstPersonCameraController::FirstPersonCameraController(float moveScaler, float rotateScaler, float speedScaler)
		: InputHandler(GenerateActionMap()),
		moveScaler_(moveScaler), rotateScaler_(rotateScaler), speedScaler_(speedScaler), previousFrameTime_(0),
		previousPointerPosition_(0, 0), forward_(0), left_(0), up_(0), azimuthAngle_(0), elevationAngle_(0), turnTriggered_(false), spedUp_(false)
	{
		semanticStates_.fill(0);
	}


	FirstPersonCameraController::~FirstPersonCameraController()
	{
	}

	void FirstPersonCameraController::AttachToCamera(SceneObjectSP const& cameraObject)
	{
		assert(cameraObject->GetComponent<Camera>() != nullptr);
		cameraObject_ = cameraObject;
		TransformationSP const& cameraTransformation = cameraObject->GetComponent<Transformation>();

		floatV3 front = cameraTransformation->GetModelFrontDirection();
		floatV3 right = Cross(front, cameraTransformation->GetModelUpDirection());
		floatV3 up = Cross(right, front);

		floatV3 actualHeading = RotateByQuaternion(cameraTransformation->GetOrientation(), cameraTransformation->GetModelFrontDirection());
		float angleToUp = Angle(actualHeading, up);
		elevationAngle_ = PI / 2 - angleToUp;

		// get the projection vector of the displacement in front & right plane
		floatV3 axisToRotateDown = Cross(actualHeading, up);
		// rotate back to the plane
		floatV3 directionInFrontRightPlane = RotateByQuaternion(RotationQuaternion(-elevationAngle_, axisToRotateDown), actualHeading);

		bool positive = Dot(Cross(directionInFrontRightPlane, front), up) > 0;
		// the angle from directionInFrontRightPlane to front referenced by axis up, so negate it.
		azimuthAngle_ = -(Angle(directionInFrontRightPlane, front) * (positive ? 1 : -1));

		// TODO check if same
		floatQ elevation = RotationQuaternion(elevationAngle_, right);
		floatQ azimuth = RotationQuaternion(azimuthAngle_, up);
		floatQ totalRotation = azimuth * elevation;

		cameraTransformation->SetOrientation(totalRotation);
	}

	std::pair<bool, std::function<void()>> FirstPersonCameraController::DoOnBeforeLogicFrame(double currentTime)
	{
		float delta = static_cast<float>(currentTime - previousFrameTime_);
		previousFrameTime_ = currentTime;

		return std::make_pair(true, GenerateFrameAction(delta));
	}

	std::pair<bool, std::function<void()>> FirstPersonCameraController::GenerateAction(InputCenter::InputEvent const& inputEvent)
	{
		switch (static_cast<MoveSemantic>(inputEvent.mappedSemantic))
		{
		case MoveSemantic::MoveForward:
		case MoveSemantic::MoveBack:
		case MoveSemantic::MoveLeft:
		case MoveSemantic::MoveRight:
		case MoveSemantic::MoveUp:
		case MoveSemantic::MoveDown:
			{
				semanticStates_[static_cast<uint32>(inputEvent.mappedSemantic)] = inputEvent.data;
				forward_ = semanticStates_[static_cast<uint32>(MoveSemantic::MoveForward)] - semanticStates_[static_cast<uint32>(MoveSemantic::MoveBack)];
				left_ = semanticStates_[static_cast<uint32>(MoveSemantic::MoveLeft)] - semanticStates_[static_cast<uint32>(MoveSemantic::MoveRight)];
				up_ = semanticStates_[static_cast<uint32>(MoveSemantic::MoveUp)] - semanticStates_[static_cast<uint32>(MoveSemantic::MoveDown)];
			}
			break;
		case MoveSemantic::Turn:
			{
				if (turnTriggered_)
				{
					intV2 delta = inputEvent.pointerPosition - previousPointerPosition_;
					// negative x because screen coordinate is 180 rotated by Y axis of camera coordinate
					floatV2 deltaTurn(-delta.X() * rotateScaler_, delta.Y() * rotateScaler_);

					XREXContext::GetInstance().GetInputCenter().EnqueueAction(GenerateRotateAction(deltaTurn));
				}
				previousPointerPosition_ = inputEvent.pointerPosition;
			}
			break;
		case MoveSemantic::TriggerTurn:
			{
				turnTriggered_ = !!inputEvent.data;
			}
			break;
		case MoveSemantic::SpeedUp:
			{
				spedUp_ = !!inputEvent.data;
			}
			break;
		default:
			{
				assert(false);
			}
		}
		return std::make_pair(false, std::function<void()>());
	}


	function<void()> FirstPersonCameraController::GenerateFrameAction(float delta)
	{

		float spedUpDelta = DefaultMoveSpeed * moveScaler_ * delta * ((spedUp_ ? 1 : 0) * speedScaler_ + 1);

		return GenerateMoveAction(spedUpDelta * forward_, spedUpDelta * left_, spedUpDelta * up_);

	}

	function<void()> FirstPersonCameraController::GenerateMoveAction(float forward, float left, float up)
	{
		SceneObjectSP const& cameraObject = cameraObject_;

		function<void()> action = [cameraObject, forward, left, up] ()
		{
			TransformationSP cameraTransformation = cameraObject->GetComponent<Transformation>();
			floatQ orientation = cameraTransformation->GetOrientation();
			floatV3 forwardTransformed = RotateByQuaternion(orientation, floatV3(0, 0, 1));
			floatV3 leftTransformed = RotateByQuaternion(orientation, floatV3(1, 0, 0));
			floatV3 upTransformed = RotateByQuaternion(orientation, floatV3(0, 1, 0));
			floatV3 finalVector = forwardTransformed * forward + leftTransformed * left + upTransformed * up;
			cameraTransformation->Translate(finalVector);
		};
		return action;
	}


	function<void()> FirstPersonCameraController::GenerateRotateAction(floatV2 const& deltaTurn)
	{
		function<void()> action = [this, deltaTurn] ()
		{
			float deltaX = RadianFromDegree(deltaTurn.X()) * rotateScaler_;
			float deltaY = RadianFromDegree(deltaTurn.Y()) * rotateScaler_;
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

			TransformationSP cameraTransformation = cameraObject_->GetComponent<Transformation>();
			floatV3 up = cameraTransformation->GetModelUpDirection();
			floatV3 right = Cross(cameraTransformation->GetModelFrontDirection(), up);
			up = Cross(right, cameraTransformation->GetModelFrontDirection());

			floatQ elevation = RotationQuaternion(elevationAngle_, right);
			floatQ azimuth = RotationQuaternion(azimuthAngle_, up);
			floatQ totalRotation = azimuth * elevation;

			cameraTransformation->SetOrientation(totalRotation);
		};
		return action;
	}

}
