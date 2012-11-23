#include "XREX.hpp"

#include "FreeRoamCameraController.hpp"

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
			map.Set(InputCenter::InputSemantic::K_W, static_cast<uint32>(FreeRoamCameraController::RoamSemantic::MoveForward));
			map.Set(InputCenter::InputSemantic::K_S, static_cast<uint32>(FreeRoamCameraController::RoamSemantic::MoveBack));
			map.Set(InputCenter::InputSemantic::K_A, static_cast<uint32>(FreeRoamCameraController::RoamSemantic::MoveLeft));
			map.Set(InputCenter::InputSemantic::K_D, static_cast<uint32>(FreeRoamCameraController::RoamSemantic::MoveRight));
			map.Set(InputCenter::InputSemantic::K_Q, static_cast<uint32>(FreeRoamCameraController::RoamSemantic::RollLeft));
			map.Set(InputCenter::InputSemantic::K_E, static_cast<uint32>(FreeRoamCameraController::RoamSemantic::RollRight));
			map.Set(InputCenter::InputSemantic::K_V, static_cast<uint32>(FreeRoamCameraController::RoamSemantic::MoveUp));
			map.Set(InputCenter::InputSemantic::K_C, static_cast<uint32>(FreeRoamCameraController::RoamSemantic::MoveDown));
			map.Set(InputCenter::InputSemantic::M_Move, static_cast<uint32>(FreeRoamCameraController::RoamSemantic::Turn));
			map.Set(InputCenter::InputSemantic::K_LeftShift, static_cast<uint32>(FreeRoamCameraController::RoamSemantic::SpeedUp));
			map.Set(InputCenter::InputSemantic::K_RightShift, static_cast<uint32>(FreeRoamCameraController::RoamSemantic::SpeedUp));
			map.Set(InputCenter::InputSemantic::M_Button0, static_cast<uint32>(FreeRoamCameraController::RoamSemantic::TriggerTurn));
			return map;
		}
	}
	FreeRoamCameraController::FreeRoamCameraController(float moveScaler, float rotateScaler, float speedScaler)
		: InputHandler(GenerateActionMap()),
		moveScaler_(moveScaler), rotateScaler_(rotateScaler), speedScaler_(speedScaler), previousFrameTime_(0),
		previousPointerPosition_(0, 0), forward_(0), left_(0), up_(0), roll_(0), turnTriggered_(false), spedUp_(false)
	{
		semanticStates_.fill(0);
	}


	FreeRoamCameraController::~FreeRoamCameraController()
	{
	}


	void FreeRoamCameraController::AttachToCamera(SceneObjectSP const& cameraObject)
	{
		assert(cameraObject->GetComponent<Camera>() != nullptr);
		cameraObject_ = cameraObject;
	}

	std::pair<bool, std::function<void()>> FreeRoamCameraController::DoOnBeforeLogicFrame(double currentTime)
	{
		float delta = static_cast<float>(currentTime - previousFrameTime_);
		previousFrameTime_ = currentTime;
		
		return std::make_pair(true, GenerateFrameAction(delta));
	}

	std::pair<bool, std::function<void()>> FreeRoamCameraController::GenerateAction(InputCenter::InputEvent const& inputEvent)
	{
		switch (static_cast<RoamSemantic>(inputEvent.mappedSemantic))
		{
		case RoamSemantic::MoveForward:
		case RoamSemantic::MoveBack:
		case RoamSemantic::MoveLeft:
		case RoamSemantic::MoveRight:
		case RoamSemantic::MoveUp:
		case RoamSemantic::MoveDown:
			{
				semanticStates_[static_cast<uint32>(inputEvent.mappedSemantic)] = inputEvent.data;
				forward_ = semanticStates_[static_cast<uint32>(RoamSemantic::MoveForward)] - semanticStates_[static_cast<uint32>(RoamSemantic::MoveBack)];
				left_ = semanticStates_[static_cast<uint32>(RoamSemantic::MoveLeft)] - semanticStates_[static_cast<uint32>(RoamSemantic::MoveRight)];
				up_ = semanticStates_[static_cast<uint32>(RoamSemantic::MoveUp)] - semanticStates_[static_cast<uint32>(RoamSemantic::MoveDown)];
			}
			break;
		case RoamSemantic::RollLeft:
		case RoamSemantic::RollRight:
			{
				semanticStates_[static_cast<uint32>(inputEvent.mappedSemantic)] = inputEvent.data;
				roll_ = semanticStates_[static_cast<uint32>(RoamSemantic::RollLeft)] - semanticStates_[static_cast<uint32>(RoamSemantic::RollRight)];
			}
			break;
		case RoamSemantic::Turn:
			{
				if (turnTriggered_)
				{
					// negative x because screen coordinate is 180 rotated by Y axis of camera coordinate
					floatV2 deltaTurn(-(inputEvent.pointerPosition - previousPointerPosition_).X() * rotateScaler_, (inputEvent.pointerPosition - previousPointerPosition_).Y() * rotateScaler_);
					previousPointerPosition_ = inputEvent.pointerPosition;

					XREXContext::GetInstance().GetInputCenter().EnqueueAction(GenerateRotateAction(deltaTurn));
				}
			}
			break;
		case RoamSemantic::TriggerTurn:
			{
				if (!turnTriggered_)
				{
					previousPointerPosition_ = inputEvent.pointerPosition;
				}
				turnTriggered_ = !!inputEvent.data;
			}
			break;
		case RoamSemantic::SpeedUp:
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


	function<void()> FreeRoamCameraController::GenerateFrameAction(float delta)
	{
		struct FrameAction
		{
			function<void()> move_;
			function<void()> roll_;
			FrameAction(function<void()>&& move, function<void()>&& roll)
				: move_(std::move(move)), roll_(std::move(roll))
			{
			}
			void operator ()()
			{
				move_();
				roll_();
			}
		};
		float spedUpDelta = moveScaler_ * delta * ((spedUp_ ? 1 : 0) * speedScaler_ + 1);
		FrameAction action(GenerateMoveAction(spedUpDelta * forward_, spedUpDelta * left_, spedUpDelta * up_), GenerateRollAction(delta * roll_));

		return action;
	}

	function<void()> FreeRoamCameraController::GenerateMoveAction(float forward, float left, float up)
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

	function<void()> FreeRoamCameraController::GenerateRollAction(float roll)
	{
		SceneObjectSP const& cameraObject = cameraObject_;

		function<void()> action = [cameraObject, roll] ()
		{
			TransformationSP cameraTransformation = cameraObject->GetComponent<Transformation>();
			floatQ orientation = cameraTransformation->GetOrientation();
			floatV3 rollAxis = RotateByQuaternion(orientation, floatV3(0, 0, -1));
			floatQ rollRotation = RotationQuaternion(roll, rollAxis);
			orientation = rollRotation * orientation;
			cameraTransformation->SetOrientation(orientation);
		};
		return action;
	}

	function<void()> FreeRoamCameraController::GenerateRotateAction(floatV2 const& deltaTurn)
	{
		SceneObjectSP const& cameraObject = cameraObject_;

		function<void()> action = [cameraObject, deltaTurn] ()
		{
			TransformationSP cameraTransformation = cameraObject->GetComponent<Transformation>();
			floatV3 rotationAxis = floatV3(-deltaTurn.Y(), deltaTurn.X(), 0); // the one perpendicular to the deltaTurn axis
			floatQ orientation = cameraTransformation->GetOrientation();
			floatV3 rotationAxisRotated = RotateByQuaternion(orientation, rotationAxis.Normalize());
			floatQ rotation = RotationQuaternion(RadianFromDegree(rotationAxis.Length()), rotationAxisRotated); // think the rotationAxis.Length is the rotation in degree
			orientation = rotation * orientation;
			cameraTransformation->SetOrientation(orientation);
		};
		return action;
	}

}
