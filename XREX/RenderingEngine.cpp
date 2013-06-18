#include "XREX.hpp"

#include "RenderingEngine.hpp"

#include "XREXContext.hpp"
#include "RenderingFactory.hpp"
#include "RenderingPipelineState.hpp"
#include "GLUtil.hpp"
#include "GraphicsContext.hpp"
#include "RenderingProcess.hpp"

#include <CoreGL.hpp>

#include <map>

using std::vector;

namespace XREX
{

	RenderingEngine::RenderingEngine(Window& window, Settings const& settings)
		: defaultBlendColor_(0, 0, 0, 1)
	{
		// initialize the graphics context first
		graphicsContext_ = MakeUP<GraphicsContext>(window, settings);

		// move gl context creation here?
		//gl::Enable(gl::GL_DEBUG_OUTPUT); // ogl 4.3

		gl::PixelStorei(gl::GL_UNPACK_ALIGNMENT, 1); // default 4
		gl::PixelStorei(gl::GL_PACK_ALIGNMENT, 1); // default 4


		gl::Enable(gl::GL_SCISSOR_TEST);

		gl::Enable(gl::GL_POLYGON_OFFSET_FILL);
		gl::Enable(gl::GL_POLYGON_OFFSET_POINT);
		gl::Enable(gl::GL_POLYGON_OFFSET_LINE);
		defaultRasterizerState_ = XREXContext::GetInstance().GetRenderingFactory().CreateRasterizerStateObject(RasterizerState());
		defaultDepthStencilState_ = XREXContext::GetInstance().GetRenderingFactory().CreateDepthStencilStateObject(DepthStencilState());
		defaultBlendState_ = XREXContext::GetInstance().GetRenderingFactory().CreateBlendStateObject(BlendState());
	}


	RenderingEngine::~RenderingEngine()
	{
		beforeRenderingFunction_.swap(decltype(beforeRenderingFunction_)());
		afterRenderingFunction_.swap(decltype(afterRenderingFunction_)());
	}

	uint32 RenderingEngine::GetGLError()
	{
		return gl::GetError();
	}

	void RenderingEngine::SwapBuffers()
	{
		graphicsContext_->SwapBuffers();
	}

	void RenderingEngine::RenderAFrame()
	{

		// clear frame buffer globally first.
		gl::ClearColor(0, 0, 0, 0);
		gl::Clear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT | gl::GL_STENCIL_BUFFER_BIT);
		double currentTime = timer_.Elapsed();
		double delta = currentTime - lastTime_;
		if (beforeRenderingFunction_ != nullptr)
		{
			beforeRenderingFunction_(currentTime, delta);
		}

#ifdef USE_OPENGL_COMPATIBILITY_PROFILE
		gl::Enable(gl::GL_POLYGON_OFFSET_FILL);
		gl::Enable(gl::GL_POLYGON_OFFSET_POINT);
		gl::Enable(gl::GL_POLYGON_OFFSET_LINE);
#endif

		SceneSP const& scene = XREXContext::GetInstance().GetScene();
		assert(process_);
		process_->RenderScene(scene);


#ifdef USE_OPENGL_COMPATIBILITY_PROFILE
		gl::UseProgram(0);
		gl::ActiveTexture(gl::GL_TEXTURE0);
		gl::BindSampler(0, 0);
		gl::BindBuffer(gl::GL_ARRAY_BUFFER, 0);
		gl::BindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, 0);
		// TODO temp hack for CEGUI.
		defaultRasterizerState_->Bind(0, 0);
		defaultDepthStencilState_->Bind(0, 0);
		defaultBlendState_->Bind(defaultBlendColor_);
		gl::Disable(gl::GL_POLYGON_OFFSET_FILL);
		gl::Disable(gl::GL_POLYGON_OFFSET_POINT);
		gl::Disable(gl::GL_POLYGON_OFFSET_LINE);
		gl::Disable(gl::GL_DEPTH_TEST);
		gl::Disable(gl::GL_CULL_FACE);
#endif

		if (afterRenderingFunction_ != nullptr)
		{
			afterRenderingFunction_(currentTime, delta);
		}

		lastTime_ = currentTime;
	}

}
