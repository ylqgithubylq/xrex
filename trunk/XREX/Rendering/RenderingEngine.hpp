#pragma once

#include "Declare.hpp"
#include "Base/Timer.hpp"

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

		void SetRenderingProcess(RenderingProcessSP const& process)
		{
			process_ = process;
		}
		RenderingProcessSP const& GetRenderingProcess() const
		{
			return process_;
		}

		void Start()
		{
			timer_.Restart();
			lastTime_ = timer_.Elapsed();
		}
		
		void SwapBuffers();

		void RenderAFrame();


		void OnBeforeRendering(std::function<void(double current, double delta)> const& beforeRenderingFunction)
		{
			beforeRenderingFunction_ = beforeRenderingFunction;
		}
		void OnAfterRendering(std::function<void(double current, double delta)> const& afterRenderingFunction)
		{
			afterRenderingFunction_ = afterRenderingFunction;
		}

	private:
		std::unique_ptr<GraphicsContext> graphicsContext_;

		RenderingProcessSP process_;

		std::function<void(double current, double delta)> beforeRenderingFunction_;
		std::function<void(double current, double delta)> afterRenderingFunction_;
		Timer timer_;
		double lastTime_;
		RasterizerStateObjectSP defaultRasterizerState_;
		DepthStencilStateObjectSP defaultDepthStencilState_;
		BlendStateObjectSP defaultBlendState_;
		Color defaultBlendColor_;
	};

}
