#pragma once

#include "Declare.hpp"

#include "InputHandler.hpp"

#include "InputCenter.hpp"

#include <functional>
#include <vector>

class FPSCameraController
	: public InputHandler
{
public:
	enum FPSSemantic
	{
		MoveForward,
		MoveBack,
		MoveLeft,
		MoveRight,
		MoveUp,
		MoveDown,
		Turn,

		FPSSemanticCount
	};

public:
	FPSCameraController(float moveScaler = 2.0f, float rotateScaler = 1.0f);
	virtual ~FPSCameraController();

	void AttachToCamera(SceneObjectSP const & cameraObject);

protected:

	virtual bool DoOnBeforeLogicFrame(double currentTime, std::function<void()>* generatedAction) override;

	virtual ActionMap GenerateActionMap() override;

	virtual bool GenerateAction(uint32 mappedSemantic, uint32 data, VectorT<uint32, 2> pointerPosition, VectorT<uint32, 2> previousPointerPosition,
		double currentTime, std::function<void()>* generatedAction) override;

private:
	std::function<void()> GenerateMoveAction(float forward, float left, float up);
	std::function<void()> GenerateRotateAction(float yaw, float pitch, float roll);


private:
	SceneObjectSP cameraObject_;

	float moveScaler_;
	float rotateScaler_;

	std::vector<int32> semanticStates_;
	int32 forward_;
	int32 left_;
	int32 up_;

	double previousFrameTime_;
};

