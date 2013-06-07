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
		RenderingEngine(Window& window, Settings const& settings);
		~RenderingEngine();

		uint32 GetGLError();

		GraphicsContext& GetGraphicsContext() const
		{
			return *graphicsContext_;
		}

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
		
		void SwapBuffers();

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
		std::unique_ptr<GraphicsContext> graphicsContext_;

		std::function<void(double current, double delta)> beforeRenderingFunction_;
		std::function<void(double current, double delta)> afterRenderingFunction_;
		SceneSP scene_;
		Timer timer_;
		double lastTime_;
		RasterizerStateObjectSP defaultRasterizerState_;
		DepthStencilStateObjectSP defaultDepthStencilState_;
		BlendStateObjectSP defaultBlendState_;
		Color defaultBlendColor_;
	};

}
