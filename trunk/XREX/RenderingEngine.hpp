#pragma once

#include "Declare.hpp"
#include "Timer.hpp"

#include <functional>
#include <vector>



class RenderingEngine
	: Noncopyable
{

public:
	RenderingEngine();
	~RenderingEngine();

	void Initialize();

	/*
	 *	@scene: set to nullptr to make no scene to render.
	 */
	void SetScene(SceneSP const& scene)
	{
		scene_ = scene;
	}
	SceneSP const& GetCurrentScene() const
	{
		return scene_;
	}

	void Start()
	{
		timer_.Restart();
		lastTime_ = timer_.Elapsed();
	}

	void Update();

	void RenderScene();


	// Temp function for convenience
	void SetRenderingFunction(std::function<void(double current, double delta)>& renderingFunction)
	{
		renderingFunction_ = renderingFunction;
	}


private:
	void RenderACamera(SceneObjectSP const& cameraObject);

private:
	std::function<void(double current, double delta)> renderingFunction_;
	SceneSP scene_;
	Timer timer_;
	double lastTime_;
};

