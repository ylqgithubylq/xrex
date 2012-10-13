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





using std::vector;

RenderingEngine::RenderingEngine()
	: scene_(MakeSP<NaiveManagedScene>())
{
}


RenderingEngine::~RenderingEngine()
{
}


void RenderingEngine::Initialize()
{
	gl::PolygonMode(gl::GL_FRONT_AND_BACK, gl::GL_FILL);
	gl::Enable(gl::GL_CULL_FACE);
	gl::Enable(gl::GL_DEPTH_TEST);
	gl::BlendFunc(gl::GL_SRC_ALPHA, gl::GL_ONE_MINUS_SRC_ALPHA);
	gl::Enable(gl::GL_BLEND);
}

void RenderingEngine::Update()
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
	// TODO scissor rectangle
	gl::ClearColor(backgroundColor.R(), backgroundColor.G(), backgroundColor.B(), backgroundColor.A());
	gl::Clear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT | gl::GL_STENCIL_BUFFER_BIT);

	floatM44 const& viewMatrix = camera->GetViewMatrix();
	floatM44 const& projectionMatrix = camera->GetProjectionMatrix();

	vector<SceneObjectSP> sceneObjects = scene_->GetRenderableQueue(cameraObject);

	vector<Renderable::LayoutAndTechnique> allLayoutAndTechniqueNeedToRender;

	for (SceneObjectSP sceneObject : sceneObjects)
	{
		TransformationSP transformation = sceneObject->GetComponent<Transformation>();
		RenderableSP renderable = sceneObject->GetComponent<Renderable>();
		assert(renderable != nullptr);
		assert(renderable->IsVisible());

		vector<Renderable::LayoutAndTechnique> const& layoutAndTechniques = renderable->GetLayoutsAndTechniques(cameraObject);
		for (auto& layoutAndTechnique : layoutAndTechniques)
		{
			allLayoutAndTechniqueNeedToRender.push_back(layoutAndTechnique);
		}
	}
	// TODO do some sorting works

	for (auto& layoutAndTechnique : allLayoutAndTechniqueNeedToRender)
	{
		Renderable& ownerRenderable = layoutAndTechnique.renderable;
		RenderingTechniqueSP const& technique = layoutAndTechnique.technique;
		RenderingLayoutSP const& layout = layoutAndTechnique.layout;

		floatM44 const& modelMatrix = ownerRenderable.GetOwnerSceneObject()->GetComponent<Transformation>()->GetModelMatrix();

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

		ownerRenderable.OnLayoutBeforeRendered(layoutAndTechnique);

		uint32 passCount = technique->GetPassCount();

		for (uint32 passIndex = 0; passIndex < passCount; ++passIndex)
		{
			RenderingPassSP pass = technique->GetPass(passIndex);
			pass->Bind();
			layout->BindToProgram(pass->GetProgram());
			layout->Draw();
			layout->Unbind();
		}
	}
}
