#include "XREX.hpp"

#include "DefaultRenderingProcess.hpp"

#include "XREXContext.hpp"
#include "Window.hpp"
#include "Scene.hpp"
#include "SceneObject.hpp"
#include "Camera.hpp"
#include "Viewport.hpp"
#include "Renderable.hpp"
#include "RenderingEffect.hpp"
#include "DefinedShaderName.hpp"
#include "Material.hpp"
#include "RenderingLayout.hpp"

#include <CoreGL.hpp>

#include <vector>
#include <map>

namespace XREX
{
	DefaultRenderingProcess::DefaultRenderingProcess()
	{
	}


	DefaultRenderingProcess::~DefaultRenderingProcess()
	{
	}

	void DefaultRenderingProcess::RenderScene(SceneSP const& scene)
	{
		if (scene != nullptr)
		{
			std::vector<SceneObjectSP> cameras_ = scene->GetCameras();
			for (auto& camera : cameras_)
			{
				assert(camera->GetComponent<Camera>() != nullptr);
				assert(camera->GetComponent<Camera>()->IsActive());
			}
			// sort by depth order of viewport, larger first.
			std::sort(cameras_.begin(), cameras_.end(), [] (SceneObjectSP const& left, SceneObjectSP const& right)
			{
				auto depthL = left->GetComponent<Camera>()->GetViewport()->GetDepthOrder();
				auto depthR = right->GetComponent<Camera>()->GetViewport()->GetDepthOrder();
				return depthL > depthR;
			});

			for (auto& camera : cameras_)
			{
				RenderACamera(scene, camera);
			}
		}
	}

	void DefaultRenderingProcess::RenderACamera(SceneSP const& scene, SceneObjectSP const& cameraObject)
	{
		CameraSP camera = cameraObject->GetComponent<Camera>();
		Size<uint32> windowSize = XREXContext::GetInstance().GetMainWindow().GetClientRegionSize();
		camera->GetViewport()->Bind(windowSize.x, windowSize.y);

		Color const& backgroundColor = camera->GetBackgroundColor();
		gl::ClearColor(backgroundColor.R(), backgroundColor.G(), backgroundColor.B(), backgroundColor.A());
		gl::Clear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT | gl::GL_STENCIL_BUFFER_BIT);

		floatM44 const& viewMatrix = camera->GetViewMatrix();
		floatM44 const& projectionMatrix = camera->GetProjectionMatrix();
		floatV3 const& cameraPosition = cameraObject->GetComponent<Transformation>()->GetWorldPosition();

		std::vector<SceneObjectSP> sceneObjects = scene->GetRenderableQueue(cameraObject);

		std::vector<Renderable::RenderablePack> allRenderableNeedToRender;

		for (SceneObjectSP sceneObject : sceneObjects)
		{
			TransformationSP transformation = sceneObject->GetComponent<Transformation>();
			RenderableSP renderable = sceneObject->GetComponent<Renderable>();
			assert(renderable != nullptr);
			assert(renderable->IsVisible());

			std::vector<Renderable::RenderablePack> renderablePacks = renderable->GetRenderablePack(cameraObject);
			for (auto& renderablePack : renderablePacks)
			{
				allRenderableNeedToRender.push_back(std::move(renderablePack));
			}
		}
		// TODO do some sorting works
		std::map<int32, std::vector<Renderable::RenderablePack*>> renderingGroups;
		for (auto& renderablePack : allRenderableNeedToRender)
		{
			renderingGroups[renderablePack.renderingGroup].push_back(&renderablePack);
		}

		for (auto& group : renderingGroups)
		{
			for (auto& renderablePack : group.second)
			{
				Renderable& ownerRenderable = *renderablePack->renderable;
				RenderingTechniqueSP const& technique = renderablePack->technique;
				RenderingLayoutSP const& layout = renderablePack->layout;
				MaterialSP const& material = renderablePack->material;

				floatM44 const& modelMatrix = ownerRenderable.GetOwnerSceneObject()->GetComponent<Transformation>()->GetWorldMatrix();
				floatM44 normalMatrix = modelMatrix; // TODO do inverse transpose to the upper floatV3 of modelMatrix

				RenderingEffectSP effect = technique->GetEffect();

				// are these too hard coded?
				{
					EffectParameterSP const& model = effect->GetParameterByName(GetUniformString(DefinedUniform::ModelMatrix));
					if (model)
					{
						model->As<floatM44>().SetValue(modelMatrix);
					}
					EffectParameterSP const& normal = effect->GetParameterByName(GetUniformString(DefinedUniform::NormalMatrix));
					if (normal)
					{
						normal->As<floatM44>().SetValue(normalMatrix);
					}
					EffectParameterSP const& view = effect->GetParameterByName(GetUniformString(DefinedUniform::ViewMatrix));
					if (view)
					{
						view->As<floatM44>().SetValue(viewMatrix);
					}
					EffectParameterSP const& projection = effect->GetParameterByName(GetUniformString(DefinedUniform::ProjectionMatrix));
					if (projection)
					{
						projection->As<floatM44>().SetValue(projectionMatrix);
					}
					EffectParameterSP const& position = effect->GetParameterByName(GetUniformString(DefinedUniform::CameraPosition));
					if (position)
					{
						position->As<floatV3>().SetValue(cameraPosition);
					}
				}

				if (material)
				{
					material->BindToEffect(renderablePack->technique->GetEffect());
					material->SetAllEffectParameterValues();
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

}

