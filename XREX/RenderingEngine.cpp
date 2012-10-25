#include "XREX.hpp"

#include "RenderingEngine.hpp"

#include "NaiveManagedScene.hpp"
#include "Transformation.hpp"
#include "Renderable.hpp"
#include "Camera.hpp"
#include "RenderingEffect.hpp"
#include "RenderingLayout.hpp"
#include "Material.hpp"

#include "GLUtil.hpp"
#include "DefinedShaderName.hpp"





using std::vector;

RenderingEngine::RenderingEngine()
	: scene_(MakeSP<NaiveManagedScene>())
{
}


RenderingEngine::~RenderingEngine()
{
	scene_.reset(); // release scene first
}


void RenderingEngine::Initialize()
{
}

void RenderingEngine::RenderAFrame()
{
	double currentTime = timer_.Elapsed();
	double delta = currentTime - lastTime_;
	if (renderingFunction_ != nullptr)
	{
		renderingFunction_(currentTime, delta);
	}
	RenderScene();
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

		vector<Renderable::RenderablePack> const& renderablePacks = renderable->GetRenderablePack(cameraObject);
		for (auto& renderablePack : renderablePacks)
		{
			allRenderableNeedToRender.push_back(renderablePack);
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
