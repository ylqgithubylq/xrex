#include "XREX.hpp"

#include "RenderingEngine.hpp"

#include "XREXContext.hpp"
#include "RenderingFactory.hpp"
#include "NaiveManagedScene.hpp"
#include "Transformation.hpp"
#include "Renderable.hpp"
#include "Camera.hpp"
#include "RenderingEffect.hpp"
#include "RenderingLayout.hpp"
#include "Material.hpp"
#include "RenderingPipelineState.hpp"

#include "GLUtil.hpp"
#include "DefinedShaderName.hpp"

#include <CoreGL.hpp>

using std::vector;

namespace XREX
{

	RenderingEngine::RenderingEngine()
		: scene_(MakeSP<NaiveManagedScene>()), defaultBlendColor_(0, 0, 0, 1)
	{
	}


	RenderingEngine::~RenderingEngine()
	{
		beforeRenderingFunction_.swap(std::function<void(double, double)>());
		afterRenderingFunction_.swap(std::function<void(double, double)>());
		scene_.reset(); // release scene
	}

	XREX::uint32 RenderingEngine::GetGLError()
	{
		return gl::GetError();
	}


	void RenderingEngine::Initialize()
	{
		// move gl context creation here?
		//gl::Enable(gl::GL_DEBUG_OUTPUT); // ogl 4.3

		gl::Enable(gl::GL_POLYGON_OFFSET_FILL);
		gl::Enable(gl::GL_POLYGON_OFFSET_POINT);
		gl::Enable(gl::GL_POLYGON_OFFSET_LINE);
		RasterizerState rasterizerState;
		defaultRasterizerState_ = XREXContext::GetInstance().GetRenderingFactory().CreateRasterizerStateObject(rasterizerState);
		DepthStencilState depthStencilState;
		defaultDepthStencilState_ = XREXContext::GetInstance().GetRenderingFactory().CreateDepthStencilStateObject(depthStencilState);
		BlendState blendState;
		defaultBlendState_ = XREXContext::GetInstance().GetRenderingFactory().CreateBlendStateObject(blendState);
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
		// TODO temp hack for CEGUI. it seems CEGUI do not disable depth test itself
		gl::Enable(gl::GL_POLYGON_OFFSET_FILL);
		gl::Enable(gl::GL_POLYGON_OFFSET_POINT);
		gl::Enable(gl::GL_POLYGON_OFFSET_LINE);
#endif

		RenderScene();

#ifdef USE_OPENGL_COMPATIBILITY_PROFILE
		gl::UseProgram(0);
		gl::ActiveTexture(gl::GL_TEXTURE0);
		gl::BindBuffer(gl::GL_ARRAY_BUFFER, 0);
		gl::BindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, 0);
		// TODO temp hack for CEGUI. it seems CEGUI do not disable depth test itself
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

	void RenderingEngine::RenderScene()
	{
		if (scene_ != nullptr)
		{
			vector<SceneObjectSP> cameras_ = scene_->GetCameras();

			for (uint32 i = 0; i < cameras_.size(); ++ i)
			{
				if (cameras_[i]->GetComponent<Camera>()->IsActive())
				{
					RenderACamera(cameras_[i]);
				}
			}
		}
#ifdef USE_OPENGL_COMPATIBILITY_PROFILE
		gl::UseProgram(0);
#endif
	}

	void RenderingEngine::RenderACamera(SceneObjectSP const& cameraObject)
	{
		CameraSP camera = cameraObject->GetComponent<Camera>();
		assert(camera != nullptr);

		Color const& backgroundColor = camera->GetBackgroundColor();
		// TODO viewport and scissor rectangle
		gl::ClearColor(backgroundColor.R(), backgroundColor.G(), backgroundColor.B(), backgroundColor.A());
		gl::Clear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT | gl::GL_STENCIL_BUFFER_BIT);

		floatM44 const& viewMatrix = camera->GetViewMatrix();
		floatM44 const& projectionMatrix = camera->GetProjectionMatrix();

		vector<SceneObjectSP> sceneObjects = scene_->GetRenderableQueue(cameraObject);

		vector<Renderable::RenderablePack> allRenderableNeedToRender;

		for (SceneObjectSP sceneObject : sceneObjects)
		{
			TransformationSP transformation = sceneObject->GetComponent<Transformation>();
			RenderableSP renderable = sceneObject->GetComponent<Renderable>();
			assert(renderable != nullptr);
			assert(renderable->IsVisible());

			vector<Renderable::RenderablePack> renderablePacks = renderable->GetRenderablePack(cameraObject);
			for (auto& renderablePack : renderablePacks)
			{
				allRenderableNeedToRender.push_back(std::move(renderablePack));
			}
		}
		// TODO do some sorting works

		for (auto& renderablePack : allRenderableNeedToRender)
		{
			Renderable& ownerRenderable = renderablePack.renderable;
			RenderingTechniqueSP const& technique = renderablePack.technique;
			RenderingLayoutSP const& layout = renderablePack.layout;

			floatM44 const& modelMatrix = ownerRenderable.GetOwnerSceneObject()->GetComponent<Transformation>()->GetWorldMatrix();

			RenderingEffect const& effect = technique->GetEffect();

			// are these too hard coded?
			EffectParameterSP const& model = effect.GetParameterByName(GetUniformString(DefinedUniform::ModelMatrix));
			if (model)
			{
				model->SetValue(modelMatrix);
			}
			EffectParameterSP const& view = effect.GetParameterByName(GetUniformString(DefinedUniform::ViewMatrix));
			if (view)
			{
				view->SetValue(viewMatrix);
			}
			EffectParameterSP const& projection = effect.GetParameterByName(GetUniformString(DefinedUniform::ProjectionMatrix));
			if (projection)
			{
				projection->SetValue(projectionMatrix);
			}

			if (renderablePack.material)
			{
				renderablePack.material->BindToEffect(renderablePack.technique->GetEffect().shared_from_this());
				renderablePack.material->SetAllEffectParameterValues();
			}

			uint32 passCount = technique->GetPassCount();

			for (uint32 passIndex = 0; passIndex < passCount; ++passIndex)
			{
				RenderingPassSP pass = technique->GetPass(passIndex);
				pass->Use();
				layout->BindToProgram(pass->GetProgram());
				layout->Draw();
				layout->Unbind();
			}
		}
	}

}
