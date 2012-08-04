#pragma once

#include "Declare.hpp"

#include <functional>
#include <vector>


class RenderingEngine
	: Noncopyable
{

public:
	RenderingEngine();
	~RenderingEngine();

	void SetScene(SceneSP const & scene)
	{
		scene_ = scene;
	}
	SceneSP const & GetCurrentScene() const
	{
		return scene_;
	}

	void Update();

	void RenderScene();


	// Temp function for convenience
	void SetRenderingFunction(std::function<void(double delta)>& renderingFunction)
	{
		renderingFunction_ = renderingFunction;
	}


private:
	void RenderACamera(SceneObjectSP const & cameraObject);


	std::function<void(double delta)> renderingFunction_;
	SceneSP scene_;

};

