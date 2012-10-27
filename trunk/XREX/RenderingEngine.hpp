#pragma once

#include "Declare.hpp"
#include "Timer.hpp"

#include <functional>
#include <vector>

namespace XREX
{

	class XREX_API RenderingEngine
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
		SceneSP const& GetScene() const
		{
			return scene_;
		}

		void Start()
		{
			timer_.Restart();
			lastTime_ = timer_.Elapsed();
		}

		void RenderAFrame();

		void RenderScene();


		// Temp function for convenience
		void SetRenderingFunction(std::function<void(double current, double delta)> const& renderingFunction)
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

}
