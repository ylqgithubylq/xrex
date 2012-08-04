#include "XREX.hpp"

#include "RenderingEngine.hpp"

#include "NaiveManagedScene.hpp"
#include "Transformation.hpp"
#include "Renderable.hpp"
#include "Camera.hpp"
#include "RenderingEffect.hpp"
#include "RenderingLayout.hpp"

#include "GLUtil.hpp"
#include "DefinedShaderName.hpp"

#include <utility>



using std::vector;

RenderingEngine::RenderingEngine()
	: scene_(MakeSP<NaiveManagedScene>())
{
}


RenderingEngine::~RenderingEngine()
{
}


void RenderingEngine::RenderScene()
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

void RenderingEngine::Update()
{
	if (renderingFunction_ != nullptr)
	{
		renderingFunction_(0.0);
	}
	RenderScene();
}

void RenderingEngine::RenderACamera(SceneObjectSP const & cameraObject)
{
	CameraSP camera = cameraObject->GetComponent<Camera>();
	assert(camera != nullptr);

	Color const & backgroundColor = camera->GetBackgroundColor();
	// TODO scissor rectangle
	gl::ClearColor(backgroundColor.R(), backgroundColor.G(), backgroundColor.B(), backgroundColor.A());
	gl::Clear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT | gl::GL_STENCIL_BUFFER_BIT);

	camera->Update();
	floatM44 const & viewMatrix = camera->GetViewMatrix();
	floatM44 const & projectionMatrix = camera->GetProjectionMatrix();

	vector<SceneObjectSP> sceneObjects = scene_->GetRenderableQueue(cameraObject);

	for (uint32 i = 0; i < sceneObjects.size(); ++i)
	{
		SceneObjectSP sceneObject = sceneObjects[i];
		TransformationSP transformation = sceneObject->GetComponent<Transformation>();
		RenderableSP renderable = sceneObject->GetComponent<Renderable>();
		assert(renderable != nullptr);
		assert(renderable->IsVisible());

		transformation->Update();
		floatM44 const & modelMatrix = transformation->GetModelMatrix();

		vector<Renderable::LayoutAndEffect> const & layoutAndEffects = renderable->GetLayoutsAndEffects();
		for (uint32 k = 0; k < layoutAndEffects.size(); ++k)
		{
			RenderingEffectSP const & effect = layoutAndEffects[k].effect;
			RenderingLayoutSP const & layout = layoutAndEffects[k].layout;

			// are these too hard coded?
			EffectParameterSP const & model = effect->GetParameterByName(DefinedUniform::ModelMatrix);
			if (model)
			{
				model->SetValue(modelMatrix);
			}
			EffectParameterSP const & view = effect->GetParameterByName(DefinedUniform::ViewMatrix);
			if (view)
			{
				view->SetValue(viewMatrix);
			}
			EffectParameterSP const & projection = effect->GetParameterByName(DefinedUniform::ProjectionMatrix);
			if (projection)
			{
				projection->SetValue(projectionMatrix);
			}

			uint32 passCount = effect->GetPassCount();

			for (uint32 passIndex = 0; passIndex < passCount; ++passIndex)
			{
				RenderingPassSP pass = effect->GetPass(passIndex);
				pass->Bind();
				layout->BindToProgram(*effect->GetPass(0)->GetProgram());
				layout->Draw();
				layout->Unbind();
			}
		}
	}
}
