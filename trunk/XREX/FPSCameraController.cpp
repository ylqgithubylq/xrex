#include "XREX.hpp"

#include "FPSCameraController.hpp"

#include "SceneObject.hpp"
#include "Camera.hpp"


using std::function;


FPSCameraController::FPSCameraController(float moveScaler, float rotateScaler)
	: moveScaler_(moveScaler), rotateScaler_(rotateScaler), forward_(0), left_(0), up_(0), semanticStates_(static_cast<uint32>(FPSSemanticCount)), previousFrameTime_(0)
{
}


FPSCameraController::~FPSCameraController()
{
}


bool FPSCameraController::DoOnBeforeLogicFrame(double currentTime, std::function<void()>* generatedAction)
{
	float delta = currentTime - previousFrameTime_;
	previousFrameTime_ = currentTime;
	delta *= moveScaler_;
	*generatedAction = GenerateMoveAction(forward_ * delta, left_ * delta, up_ * delta);
	return true;
}

bool FPSCameraController::GenerateAction(uint32 mappedSemantic, uint32 data, VectorT<uint32, 2> pointerPosition, VectorT<uint32, 2> previousPointerPosition,
	double currentTime, function<void()>* generatedAction)
{
	switch (mappedSemantic)
	{
	case FPSSemantic::MoveForward:
	case FPSSemantic::MoveBack:
	case FPSSemantic::MoveLeft:
	case FPSSemantic::MoveRight:
	case FPSSemantic::MoveUp:
	case FPSSemantic::MoveDown:
		{
			semanticStates_[static_cast<uint32>(mappedSemantic)] = data;
			forward_ = semanticStates_[static_cast<uint32>(FPSSemantic::MoveForward)] - semanticStates_[static_cast<uint32>(FPSSemantic::MoveBack)];
			left_ = semanticStates_[static_cast<uint32>(FPSSemantic::MoveLeft)] - semanticStates_[static_cast<uint32>(FPSSemantic::MoveRight)];
			up_ = semanticStates_[static_cast<uint32>(FPSSemantic::MoveUp)] - semanticStates_[static_cast<uint32>(FPSSemantic::MoveDown)];
		}
		break;
	case FPSSemantic::Turn:
		{
			VectorT<uint32, 2> deltaTurn = (previousPointerPosition - pointerPosition) * rotateScaler_;
			GenerateRotateAction(-deltaTurn.X(), -deltaTurn.Y(), 0);
		}
		break;
	default:
		{
			assert(false);
		}
	}
	return false;
}

auto FPSCameraController::GenerateActionMap() -> ActionMap
{
	ActionMap map;
	map.Set(InputCenter::InputSemantic::K_W, FPSSemantic::MoveForward);
	map.Set(InputCenter::InputSemantic::K_S, FPSSemantic::MoveBack);
	map.Set(InputCenter::InputSemantic::K_A, FPSSemantic::MoveLeft);
	map.Set(InputCenter::InputSemantic::K_D, FPSSemantic::MoveRight);
	map.Set(InputCenter::InputSemantic::K_Q, FPSSemantic::MoveUp);
	map.Set(InputCenter::InputSemantic::K_E, FPSSemantic::MoveDown);
	map.Set(InputCenter::InputSemantic::M_Move, FPSSemantic::Turn);
	return std::move(map);
}

void FPSCameraController::AttachToCamera(SceneObjectSP const & cameraObject)
{
	assert(cameraObject->GetComponent<Camera>() != nullptr);
	cameraObject_ = cameraObject;
}

function<void()> FPSCameraController::GenerateMoveAction(float forward, float left, float up)
{
	TransformationSP cameraTransformation = cameraObject_->GetComponent<Transformation>();
	function<void()> action = [cameraTransformation, forward, left, up] ()
	{
		// TODO
		cameraTransformation->Translate(left, up, forward);
	};
	return std::move(action);
}

function<void()> FPSCameraController::GenerateRotateAction(float yaw, float pitch, float roll)
{
	// TODO
	function<void()> action = [] ()
	{

	};
	return std::move(action);
}
