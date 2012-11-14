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

		uint32 GetGLError();
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

		ViewportSP const& GetDefaultViewport() const
		{
			return defaultViewport_;
		}

		void Start()
		{
			timer_.Restart();
			lastTime_ = timer_.Elapsed();
		}

		void RenderAFrame();

		void RenderScene();


		void OnBeforeRendering(std::function<void(double current, double delta)> const& beforeRenderingFunction)
		{
			beforeRenderingFunction_ = beforeRenderingFunction;
		}
		void OnAfterRendering(std::function<void(double current, double delta)> const& afterRenderingFunction)
		{
			afterRenderingFunction_ = afterRenderingFunction;
		}


	private:
		void RenderACamera(SceneObjectSP const& cameraObject);

	private:
		std::function<void(double current, double delta)> beforeRenderingFunction_;
		std::function<void(double current, double delta)> afterRenderingFunction_;
		SceneSP scene_;
		Timer timer_;
		double lastTime_;
		ViewportSP defaultViewport_;
		RasterizerStateObjectSP defaultRasterizerState_;
		DepthStencilStateObjectSP defaultDepthStencilState_;
		BlendStateObjectSP defaultBlendState_;
		Color defaultBlendColor_;
	};

}
