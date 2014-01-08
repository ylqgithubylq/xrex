#include "XREXAll.hpp"
#include "RenderToTextureTest.h"

#include <CoreGL.hpp>

#include <iostream>
#include <sstream>
#include <cassert>

#undef LoadString

using namespace std;
using namespace XREX;

namespace
{
	struct SpotLight
	{
		SceneObjectSP lightObject_;
		FirstPersonCameraControllerSP controller_;

		float coneAperture_;
		float lightConeAppearanceSize;
		float lightMaxDistance;
		SpotLight()
		{
			lightObject_ = MakeSP<SceneObject>("SpotLight");
			lightObject_->SetComponent(MakeConeMesh(1, 0.5f, 15));

			lightConeAppearanceSize = 100;

			lightMaxDistance = 1000;

			coneAperture_ = PI / 4;
			auto camera = MakeSP<PerspectiveCamera>(coneAperture_, 1.f, 1.f, lightMaxDistance);
			lightObject_->SetComponent(camera);
			controller_ = MakeSP<FirstPersonCameraController>();
			controller_->AttachToCamera(lightObject_);

			lightObject_->GetComponent<Transformation>()->Translate(0, 20, 0);
			SwitchToLightingVolume();

			XREXContext::GetInstance().GetScene()->AddObject(lightObject_);
		}
		~SpotLight()
		{
			XREXContext::GetInstance().GetScene()->RemoveObject(lightObject_);
			RemoveController();
		}

		void SwitchToLightingVolume()
		{
			SwitchSize(lightMaxDistance);
		}
		void SwitchToVisualizationVolume()
		{
			SwitchSize(lightConeAppearanceSize);
		}

		void SwitchSize(float size)
		{
			TransformationSP transformation = lightObject_->GetComponent<Transformation>();
			float radius = std::tan(coneAperture_ / 2) * size;
			transformation->SetScaling(radius, radius, size);
		}

		void AddController()
		{
			XREXContext::GetInstance().GetInputCenter().AddInputHandler(controller_);
		}
		void RemoveController()
		{
			XREXContext::GetInstance().GetInputCenter().RemoveInputHandler(controller_);
		}
		/*
		 *	Cone facing +z, apex at (0, 0, 0).
		 */
		MeshSP MakeConeMesh(float height, float radius, uint32 tessellationCount)
		{
			std::vector<floatV3> vertices;
			vertices.reserve(tessellationCount + 2);
			for (uint32 i = 0; i < tessellationCount; ++i)
			{
				float percentage = static_cast<float>(i) / tessellationCount;
				float radian = percentage * 2 * PI;
				vertices.push_back(floatV3(std::cos(radian) * radius, -std::sin(radian) * radius, height));
			}
			vertices.push_back(floatV3(0, 0, 0));
			vertices.push_back(floatV3(0, 0, height));
			assert(vertices.size() == tessellationCount + 2);

			std::vector<uint16> indices;
			indices.reserve(tessellationCount * 6);
			for (uint32 i = 0; i < tessellationCount; ++i)
			{
				indices.push_back(i);
				indices.push_back((i + 1) % tessellationCount);
				indices.push_back(tessellationCount);
			}
			for (uint32 i = 0; i < tessellationCount; ++i)
			{
				indices.push_back((i + 1) % tessellationCount);
				indices.push_back(i);
				indices.push_back(tessellationCount + 1);
			}
			assert(indices.size() == tessellationCount * 6);

			VertexBuffer::DataLayoutDescription layoutDesc(vertices.size());
			layoutDesc.AddChannelLayout(VertexBuffer::DataLayoutDescription::ElementLayoutDescription(0, sizeof(floatV3), ElementType::FloatV3, "position"));
			VertexBufferSP vertexBuffer = XREXContext::GetInstance().GetRenderingFactory().CreateVertexBuffer(GraphicsBuffer::Usage::StaticDraw, vertices, move(layoutDesc));
			IndexBufferSP indexBuffer = XREXContext::GetInstance().GetRenderingFactory().CreateIndexBuffer(GraphicsBuffer::Usage::StaticDraw, indices, IndexBuffer::TopologicalType::Triangles);
			RenderingLayoutSP layout = XREXContext::GetInstance().GetRenderingFactory().CreateRenderingLayout(vector<VertexBufferSP>(1, vertexBuffer), indexBuffer);

			MeshSP mesh = MakeSP<Mesh>("Cone");
			mesh->CreateSubMesh("ConeSubMesh", layout, nullptr, nullptr);
			return mesh;
		}
	};

	struct ViewCamera
	{
		SceneObjectSP theCameraObject_;
		FirstPersonCameraControllerSP controller_;
		ViewCamera()
		{
			theCameraObject_ = MakeSP<SceneObject>("camera");
			Size<uint32, 2> windowSize = XREXContext::GetInstance().GetMainWindow().GetClientRegionSize();
			float aspectRatio = static_cast<float>(windowSize.X()) / windowSize.Y();
			CameraSP camera = MakeSP<PerspectiveCamera>(PI / 4, aspectRatio, 1.f, 10000.0f);
			theCameraObject_->SetComponent(camera);

			controller_ = MakeSP<FirstPersonCameraController>();
			controller_->AttachToCamera(theCameraObject_);
		}
		~ViewCamera()
		{
			RemoveFromScene();
		}

		void AddToScene()
		{
			XREXContext::GetInstance().GetScene()->AddObject(theCameraObject_);
			XREXContext::GetInstance().GetInputCenter().AddInputHandler(controller_);
		}
		void RemoveFromScene()
		{
			XREXContext::GetInstance().GetScene()->RemoveObject(theCameraObject_);
			XREXContext::GetInstance().GetInputCenter().RemoveInputHandler(controller_);
		}

	};


	struct RenderToTextureProcess
		: RenderingProcess
	{
		struct LightingTechnique_CameraVolumeSetter
			: ComponentParameterSetter<PerspectiveCamera>, NUpdatePerFrame
		{
			explicit LightingTechnique_CameraVolumeSetter(RenderingTechniqueSP technique)
				: ComponentParameterSetter(std::move(technique))
			{
				cameraParameter_ = GetTechnique()->GetParameterByName("CameraVolume");
				if (cameraParameter_ != nullptr)
				{
					parameterBuffer_ = cameraParameter_->As<ShaderResourceBufferSP>().GetValue();

					auto projectionAResult = parameterBuffer_->GetSetter("ProjectionA");
					assert(projectionAResult.first);
					projectionA_ = projectionAResult.second;
					auto projectionBResult = parameterBuffer_->GetSetter("ProjectionB");
					assert(projectionBResult.first);
					projectionB_ = projectionBResult.second;
				}
			}


			virtual void SetParameter(PerspectiveCameraSP const& component) override
			{
				if (parameterBuffer_ != nullptr)
				{
					float farPlane = component->GetFar();
					float nearPlane = component->GetNear();
					float projectionA = farPlane / (farPlane - nearPlane);
					float projectionB = (-farPlane * nearPlane) / (farPlane - nearPlane);
					ShaderResourceBuffer::BufferMapper mapper = parameterBuffer_->GetMapper();

					projectionA_.SetValue(mapper, projectionA);
					projectionB_.SetValue(mapper, projectionB);
				}
			}

		private:
			TechniqueParameterSP cameraParameter_;
			ShaderResourceBufferSP parameterBuffer_;

			ShaderResourceBuffer::VariableSetter projectionA_;
			ShaderResourceBuffer::VariableSetter projectionB_;
		};

		struct LightingTechnique_LightTransformationSetter
			: ComponentParameterSetter<Camera>, IParameterSetterDepend<Camera>
		{
			explicit LightingTechnique_LightTransformationSetter(RenderingTechniqueSP technique)
				: ComponentParameterSetter(std::move(technique))
			{
				bias_ = floatM44(
					0.5f, 0.0f, 0.0f, 0.0f,
					0.0f, 0.5f, 0.0f, 0.0f,
					0.0f, 0.0f, 0.5f, 0.0f,
					0.5f, 0.5f, 0.5f, 1.0f);

				cameraParameter_ = GetTechnique()->GetParameterByName("LightTransformation");
				if (cameraParameter_ != nullptr)
				{
					parameterBuffer_ = cameraParameter_->As<ShaderResourceBufferSP>().GetValue();

					auto lightTextureClipFromViewResult = parameterBuffer_->GetSetter("LightTextureClipFromView");
					assert(lightTextureClipFromViewResult.first);
					lightTextureClipFromView_ = lightTextureClipFromViewResult.second;
				}
			}

			virtual void Connect(CameraSP const& viewCamera) override
			{
				viewCamera_ = viewCamera;
			}

			virtual void SetParameter(CameraSP const& lightCamera) override
			{
				if (parameterBuffer_ != nullptr)
				{
					floatM44 lightProjection = lightCamera->GetProjectionMatrix();
					floatM44 lightView = lightCamera->GetViewMatrix();
					floatM44 cameraView = viewCamera_->GetViewMatrix();

					floatM44 lightTextureClipFromView = bias_ * lightProjection * lightView * cameraView.Inverse();

					ShaderResourceBuffer::BufferMapper mapper = parameterBuffer_->GetMapper();
					lightTextureClipFromView_.SetValue(mapper, lightTextureClipFromView);
				}
			}


		private:
			CameraSP viewCamera_;
			TechniqueParameterSP cameraParameter_;
			ShaderResourceBufferSP parameterBuffer_;

			ShaderResourceBuffer::VariableSetter lightTextureClipFromView_;

			floatM44 bias_;
		};

		SpotLight spotLight_;
		ViewCamera viewCamera_;

		FrameBufferSP shadowMapFrameBuffer_;
		Texture2DSP shadowMap_;
		Texture2DSP shadowMapViewInColor_;

		FrameBufferSP gBuffer_;
		Texture2DSP depth_;
		Texture2DSP color_;
		Texture2DSP normal_;
		Texture2DSP depthInColor_;
		Texture2DSP viewPosition_;

		FrameBufferSP tempBuffer_;
		Texture2DSP lightingOutput_;
		Texture2DSP lightingOutput2_;

		RenderingTechniqueSP shadowMapTechnique_;
		std::shared_ptr<TransformationSetter> shadowMapTechniqueTransformationSetter_;
		std::shared_ptr<CameraSetter> shadowMapTechniqueCameraSetter_;

		RenderingTechniqueSP gBufferTechnique_;
		std::shared_ptr<TransformationSetter> gBufferTechniqueTransformationSetter_;
		std::shared_ptr<CameraSetter> gBufferTechniqueCameraSetter_;
		
		RenderingTechniqueSP lightingTechnique_;
		std::shared_ptr<TransformationSetter> lightingTechniqueTransformationSetter_;
		std::shared_ptr<CameraSetter> lightingTechniqueCameraSetter_;
		std::shared_ptr<LightingTechnique_CameraVolumeSetter> lightingTechniqueCameraVolumeSetter_;
		std::shared_ptr<LightingTechnique_LightTransformationSetter> lightingTechniqueLightTransformationSetter_;
		RenderingTechniqueSP copyTechnique_;

		RenderingLayoutSP quad_;
		LayoutAndProgramConnectorSP quadConnector_;

		uint32 glQuery;

		void InitializeShadowTextureAndFrameBuffer()
		{
			string framebufferFile = "XREXTest/Effects/ShadowMapBuffer.framebuffer";
			FrameBufferLoadingResultSP loadResult = XREXContext::GetInstance().GetResourceManager().LoadFrameBuffer(framebufferFile);

			shadowMapFrameBuffer_ = loadResult->Create();

			shadowMap_ = shadowMapFrameBuffer_->GetDepthAttachment()->GetTexture();
			shadowMapViewInColor_ = shadowMapFrameBuffer_->GetColorAttachment("colorOutput")->GetTexture();
		}

		void InitializeTextureAndGBuffer()
		{
			string framebufferFile = "XREXTest/Effects/GBuffer.framebuffer";
			FrameBufferLoadingResultSP loadResult = XREXContext::GetInstance().GetResourceManager().LoadFrameBuffer(framebufferFile);

			gBuffer_ = loadResult->Create();

			color_ = gBuffer_->GetColorAttachment("colorOutput")->GetTexture();
			normal_ = gBuffer_->GetColorAttachment("normalOutput")->GetTexture();
			depthInColor_ = gBuffer_->GetColorAttachment("depthInColorOutput")->GetTexture();
			viewPosition_ = gBuffer_->GetColorAttachment("viewPositionOutput")->GetTexture();
			depth_ = gBuffer_->GetDepthAttachment()->GetTexture();
		}

		void InitializeTextureAndTempBuffer()
		{
			string framebufferFile = "XREXTest/Effects/TempBuffer.framebuffer";
			FrameBufferLoadingResultSP loadResult = XREXContext::GetInstance().GetResourceManager().LoadFrameBuffer(framebufferFile);

			tempBuffer_ = loadResult->Create();

			lightingOutput_ = tempBuffer_->GetColorAttachment("lightingOutput")->GetTexture();
			lightingOutput2_ = tempBuffer_->GetColorAttachment("lightingOutput2")->GetTexture();
		}

		void InitializeShadowMapTechnique()
		{
			string techniqueFile = "XREXTest/Effects/ShadowMapGenerate.technique";
			TechniqueLoadingResultSP loadResult = XREXContext::GetInstance().GetResourceManager().LoadTechnique(techniqueFile, vector<pair<string, string>>());

			shadowMapTechnique_ = loadResult->Create();

			shadowMapTechnique_->ConnectFrameBuffer(shadowMapFrameBuffer_);

			shadowMapTechniqueTransformationSetter_ = MakeSP<TransformationSetter>(shadowMapTechnique_);
			shadowMapTechniqueCameraSetter_ = MakeSP<CameraSetter>(shadowMapTechnique_);
		}

		void InitializeGBufferGenerateTechnique()
		{
			string techniqueFile = "XREXTest/Effects/GBufferGenerate.technique";
			TechniqueLoadingResultSP loadResult = XREXContext::GetInstance().GetResourceManager().LoadTechnique(techniqueFile, vector<pair<string, string>>());

			gBufferTechnique_ = loadResult->Create();

			gBufferTechnique_->ConnectFrameBuffer(gBuffer_);

			gBufferTechniqueTransformationSetter_ = MakeSP<TransformationSetter>(gBufferTechnique_);
			gBufferTechniqueCameraSetter_ = MakeSP<CameraSetter>(gBufferTechnique_);
		}


		void InitializeLightingTechnique()
		{
			string techniqueFile = "XREXTest/Effects/DeferredLighting.technique";
			TechniqueLoadingResultSP loadResult = XREXContext::GetInstance().GetResourceManager().LoadTechnique(techniqueFile, vector<pair<string, string>>());

			lightingTechnique_ = loadResult->Create();

			//lightingTechnique_->ConnectFrameBuffer(XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer());
			lightingTechnique_->ConnectFrameBuffer(tempBuffer_);

			lightingTechniqueTransformationSetter_ = MakeSP<TransformationSetter>(lightingTechnique_);
			lightingTechniqueCameraSetter_ = MakeSP<CameraSetter>(lightingTechnique_);
			lightingTechniqueCameraVolumeSetter_ = MakeSP<LightingTechnique_CameraVolumeSetter>(lightingTechnique_);
			lightingTechniqueLightTransformationSetter_ = MakeSP<LightingTechnique_LightTransformationSetter>(lightingTechnique_);

			TechniqueParameterSP diffuse = lightingTechnique_->GetParameterByName("diffuse");
			TechniqueParameterSP normal = lightingTechnique_->GetParameterByName("normal");
			TechniqueParameterSP depth = lightingTechnique_->GetParameterByName("depth");
			TechniqueParameterSP shadowMap = lightingTechnique_->GetParameterByName("shadowMap");
			TechniqueParameterSP depthInColor = lightingTechnique_->GetParameterByName("depthInColor");
			if (diffuse)
			{
				diffuse->As<TextureSP>().SetValue(color_);
			}
			if (normal)
			{
				normal->As<TextureSP>().SetValue(normal_);
			}
			if (depth)
			{
				depth->As<TextureSP>().SetValue(depth_);
			}
			if (shadowMap)
			{
				shadowMap->As<TextureSP>().SetValue(shadowMap_);
			}
			if (depthInColor)
			{
				depthInColor->As<TextureSP>().SetValue(shadowMapViewInColor_);
			}
		}

		void InitializeTextureShowTechnique()
		{
			string techniqueFile = "XREXTest/Effects/TestCopyTextures.technique";
			TechniqueLoadingResultSP loadResult = XREXContext::GetInstance().GetResourceManager().LoadTechnique(techniqueFile, vector<pair<string, string>>());

			RenderingTechniqueSP technique = loadResult->Create();
			copyTechnique_ = technique;

			copyTechnique_->ConnectFrameBuffer(XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer());

			TechniqueParameterSP color = copyTechnique_->GetParameterByName("color");
			TechniqueParameterSP normal = copyTechnique_->GetParameterByName("normal");
			TechniqueParameterSP depthInColor = copyTechnique_->GetParameterByName("depthInColor");
			TechniqueParameterSP depth = copyTechnique_->GetParameterByName("depth");
			TechniqueParameterSP viewPosition = copyTechnique_->GetParameterByName("viewPosition");
			TechniqueParameterSP lightingOutput = copyTechnique_->GetParameterByName("lightingOutput");
			TechniqueParameterSP lightingOutput2 = copyTechnique_->GetParameterByName("lightingOutput2");
			TechniqueParameterSP shadowMap = copyTechnique_->GetParameterByName("shadowMap");
			TechniqueParameterSP shadowMapViewInColor = copyTechnique_->GetParameterByName("shadowMapViewInColor");
			
			if (color)
			{
				color->As<TextureSP>().SetValue(color_);
			}
			if (normal)
			{
				normal->As<TextureSP>().SetValue(normal_);
			}
			if (depthInColor)
			{
				depthInColor->As<TextureSP>().SetValue(depthInColor_);
			}
			if (depth)
			{
				depth->As<TextureSP>().SetValue(depth_);
			}
			if (viewPosition)
			{
				viewPosition->As<TextureSP>().SetValue(viewPosition_);
			}
			if (lightingOutput)
			{
				lightingOutput->As<TextureSP>().SetValue(lightingOutput_);
			}
			if (lightingOutput2)
			{
				lightingOutput2->As<TextureSP>().SetValue(lightingOutput2_);
			}
			if (shadowMap)
			{
				shadowMap->As<TextureSP>().SetValue(shadowMap_);
			}
			if (shadowMapViewInColor)
			{
				shadowMapViewInColor->As<TextureSP>().SetValue(shadowMapViewInColor_);
			}
		}

		void InitializeScreenQuad()
		{
			vector<floatV2> vertexData;
			vector<uint16> indexData;

			vertexData.push_back(floatV2(-1, -1));
			vertexData.push_back(floatV2(1, -1));
			vertexData.push_back(floatV2(1, 1));
			vertexData.push_back(floatV2(-1, 1));


			indexData.push_back(3);
			indexData.push_back(0);
			indexData.push_back(2);
			indexData.push_back(2);
			indexData.push_back(0);
			indexData.push_back(1);

			VertexBuffer::DataLayoutDescription layoutDesc(4);
			layoutDesc.AddChannelLayout(VertexBuffer::DataLayoutDescription::ElementLayoutDescription(0, sizeof(floatV2), ElementType::FloatV2, "position"));
			VertexBufferSP vertices = XREXContext::GetInstance().GetRenderingFactory().CreateVertexBuffer(GraphicsBuffer::Usage::StaticDraw, vertexData, move(layoutDesc));
			IndexBufferSP indices = XREXContext::GetInstance().GetRenderingFactory().CreateIndexBuffer(GraphicsBuffer::Usage::StaticDraw, indexData, IndexBuffer::TopologicalType::Triangles);
			RenderingLayoutSP quad = XREXContext::GetInstance().GetRenderingFactory().CreateRenderingLayout(vector<VertexBufferSP>(1, vertices), indices);

			quad_ = quad;

			quadConnector_ = XREXContext::GetInstance().GetRenderingFactory().GetConnector(quad_, copyTechnique_);
		}

		RenderToTextureProcess()
		{
			InitializeShadowTextureAndFrameBuffer();
			InitializeShadowMapTechnique();
			InitializeTextureAndGBuffer();
			InitializeGBufferGenerateTechnique();
			InitializeTextureAndTempBuffer();
			InitializeLightingTechnique();
			InitializeTextureShowTechnique();
			InitializeScreenQuad();

			glQuery = 0;
			gl::GenQueries(1, &glQuery);
		}

		~RenderToTextureProcess()
		{
			gl::DeleteQueries(1, &glQuery);
		}

		virtual void RenderScene(SceneSP const& scene) override
		{
			RenderShadowMap(scene);
			RenderSceneToGBuffer(scene);
			Lighting();
			RenderTextureToWindow();
		}

		void RenderShadowMap(SceneSP const& scene)
		{
			ShadowMapALight(scene, spotLight_);
		}

		void ShadowMapALight(SceneSP const& scene, SpotLight& light)
		{
			CameraSP camera = light.lightObject_->GetComponent<Camera>();
			camera->GetViewport()->Bind(shadowMapFrameBuffer_->GetLayoutDescription()->GetSize());

			shadowMapFrameBuffer_->Clear(FrameBuffer::ClearMask::All, Color(0, 0, 0.9f, 1), 1, 0);

			std::vector<SceneObjectSP> sceneObjects = scene->GetRenderableQueue(nullptr);


			RenderablePackCollector collector;
			for (SceneObjectSP sceneObject : sceneObjects)
			{
				RenderableSP renderable = sceneObject->GetComponent<Renderable>();
				assert(renderable != nullptr);
				assert(renderable->IsVisible());

				renderable->GetSmallRenderablePack(collector, nullptr);
			}
			std::vector<Renderable::SmallRenderablePack> allRenderableNeedToRender = collector.ExtractSmallRenderablePack();


			shadowMapTechniqueCameraSetter_->SetParameter(camera);
			shadowMapTechniqueTransformationSetter_->Connect(camera);
			shadowMapTechnique_->GetPipelineParameters().polygonOffsetFactor = 1.5f;
			shadowMapTechnique_->GetPipelineParameters().polygonOffsetUnits = 4.0f;
			IndexedDrawer drawer;

			for (auto& renderablePack : allRenderableNeedToRender)
			{
				Renderable& ownerRenderable = *renderablePack.renderable;
				RenderingLayoutSP const& layout = renderablePack.layout;
				MaterialSP const& material = renderablePack.material;

				if (material)
				{
					material->BindToTechnique(shadowMapTechnique_);
					material->SetAllTechniqueParameterValues();
				}

				shadowMapTechniqueTransformationSetter_->SetParameter(ownerRenderable.GetOwnerSceneObject()->GetComponent<Transformation>());

				LayoutAndProgramConnectorSP connector = XREXContext::GetInstance().GetRenderingFactory().GetConnector(layout, shadowMapTechnique_);
				drawer.SetTechnique(shadowMapTechnique_);
				drawer.SetLayoutAndProgramConnector(connector);
				drawer.SetRenderingLayout(layout);
				drawer.Launch();
			}


		}

		void RenderSceneToGBuffer(SceneSP const& scene)
		{
			spotLight_.SwitchToVisualizationVolume();
			RenderACamera(scene, viewCamera_.theCameraObject_);
		}

		void RenderACamera(SceneSP const& scene, SceneObjectSP const& cameraObject)
		{
			CameraSP camera = cameraObject->GetComponent<Camera>();
			Size<uint32, 2> windowSize = XREXContext::GetInstance().GetMainWindow().GetClientRegionSize();
			camera->GetViewport()->Bind(windowSize);

			Color const& backgroundColor = camera->GetBackgroundColor();
			gBuffer_->Clear(FrameBuffer::ClearMask::All, backgroundColor, 1, 0);

			std::vector<SceneObjectSP> sceneObjects = scene->GetRenderableQueue(nullptr);


			RenderablePackCollector collector;
			for (SceneObjectSP sceneObject : sceneObjects)
			{
				RenderableSP renderable = sceneObject->GetComponent<Renderable>();
				assert(renderable != nullptr);
				assert(renderable->IsVisible());

				renderable->GetSmallRenderablePack(collector, nullptr);
			}
			std::vector<Renderable::SmallRenderablePack> allRenderableNeedToRender = collector.ExtractSmallRenderablePack();

			gBufferTechniqueCameraSetter_->SetParameter(camera);
			gBufferTechniqueTransformationSetter_->Connect(camera);

			IndexedDrawer drawer;

			for (auto& renderablePack : allRenderableNeedToRender)
			{
				Renderable& ownerRenderable = *renderablePack.renderable;
				RenderingLayoutSP const& layout = renderablePack.layout;
				MaterialSP const& material = renderablePack.material;

				if (material)
				{
					material->BindToTechnique(gBufferTechnique_);
					material->SetAllTechniqueParameterValues();
				}

				gBufferTechniqueTransformationSetter_->SetParameter(ownerRenderable.GetOwnerSceneObject()->GetComponent<Transformation>());

				LayoutAndProgramConnectorSP connector = XREXContext::GetInstance().GetRenderingFactory().GetConnector(layout, gBufferTechnique_);
				drawer.SetTechnique(gBufferTechnique_);
				drawer.SetLayoutAndProgramConnector(connector);
				drawer.SetRenderingLayout(layout);
				drawer.Launch();
			}

		}

		void Lighting()
		{
			spotLight_.SwitchToLightingVolume();
			RenderALight(viewCamera_.theCameraObject_, spotLight_);
		}

		void RenderALight(SceneObjectSP const& cameraObject, SpotLight& light)
		{
			CameraSP camera = cameraObject->GetComponent<Camera>();
			camera->GetViewport()->Bind(XREXContext::GetInstance().GetMainWindow().GetClientRegionSize());
			//XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer()->Clear(FrameBuffer::ClearMask::All, Color(0, 0, 0, 1), 1, 0);
			tempBuffer_->Clear(FrameBuffer::ClearMask::All, Color(0, 0, 0, 1), 1, 0);

			RenderablePackCollector collector;

			RenderableSP renderable = light.lightObject_->GetComponent<Renderable>();
			assert(renderable != nullptr);
			assert(renderable->IsVisible());

			renderable->GetSmallRenderablePack(collector, nullptr);
			std::vector<Renderable::SmallRenderablePack> allRenderableNeedToRender = collector.ExtractSmallRenderablePack();


			lightingTechniqueCameraSetter_->SetParameter(camera);
			lightingTechniqueTransformationSetter_->Connect(camera);
			lightingTechniqueCameraVolumeSetter_->SetParameter(CheckedSPCast<PerspectiveCamera>(camera));
			lightingTechniqueLightTransformationSetter_->Connect(camera);
			lightingTechniqueLightTransformationSetter_->SetParameter(light.lightObject_->GetComponent<Camera>());

			IndexedDrawer drawer;

			gl::BeginQuery(gl::GL_SAMPLES_PASSED, glQuery);
			auto scopeExit = [this]
			{
				gl::EndQuery(gl::GL_SAMPLES_PASSED);

				uint32 fragmentCount = 0;
				gl::GetQueryObjectuiv(glQuery, gl::GL_QUERY_RESULT, &fragmentCount);
				if (fragmentCount == 0)
				{
					std::cout << XREXContext::GetInstance().GetElapsedTime() << ", no fragment generated." << std::endl;
				}
			};
			ScopeGuard<decltype(scopeExit)> guard(scopeExit);

			for (auto& renderablePack : allRenderableNeedToRender)
			{
				Renderable& ownerRenderable = *renderablePack.renderable;
				RenderingLayoutSP const& layout = renderablePack.layout;
				MaterialSP const& material = renderablePack.material;

				if (material)
				{
					material->BindToTechnique(lightingTechnique_);
					material->SetAllTechniqueParameterValues();
				}

				lightingTechniqueTransformationSetter_->SetParameter(ownerRenderable.GetOwnerSceneObject()->GetComponent<Transformation>());

				LayoutAndProgramConnectorSP connector = XREXContext::GetInstance().GetRenderingFactory().GetConnector(layout, lightingTechnique_);
				drawer.SetTechnique(lightingTechnique_);
				drawer.SetLayoutAndProgramConnector(connector);
				drawer.SetRenderingLayout(layout);
				drawer.Launch();
			}

		}

		void RenderTextureToWindow()
		{
			XREXContext::GetInstance().GetRenderingFactory().GetDefaultViewport()->Bind(XREXContext::GetInstance().GetMainWindow().GetClientRegionSize());
			IndexedDrawer drawer;
			drawer.SetTechnique(copyTechnique_);
			drawer.SetLayoutAndProgramConnector(quadConnector_);
			drawer.SetRenderingLayout(quad_);
			drawer.Launch();
		}
	};

}

namespace
{


	MeshSP LoadModel()
	{
		MeshSP model = XREXContext::GetInstance().GetResourceManager().LoadModel("Data/crytek-sponza/sponza.obj")->Create();
		return model;
	}


	void InitializeScene()
	{

		SceneSP scene = XREXContext::GetInstance().GetScene();
		SceneObjectSP modelObject = MakeSP<SceneObject>("mesh object");
		MeshSP model = LoadModel();
		modelObject->SetComponent(model);
		TransformationSP trans = modelObject->GetComponent<Transformation>();
		trans->SetPosition(0, 0, 50);
		scene->AddObject(modelObject);

	}


	struct GlobalInputHandler
		: public InputHandler
	{
		enum class ControlType
		{
			MainView,
			SpotLight,
		};
		static XREX::InputHandler::ActionMap GenerateActionMap()
		{
			XREX::InputHandler::ActionMap map;
			map.Set(XREX::InputCenter::InputSemantic::K_F1, static_cast<uint32>(ControlType::MainView));
			map.Set(XREX::InputCenter::InputSemantic::K_F2, static_cast<uint32>(ControlType::SpotLight));
			return map;
		}

		GlobalInputHandler(std::shared_ptr<RenderToTextureProcess> process)
			: InputHandler(GenerateActionMap()), process_(process)
		{
		}
		virtual std::pair<bool, std::function<void()>> GenerateAction(InputCenter::InputEvent const& inputEvent) override
		{
			ControlType action = static_cast<ControlType>(inputEvent.mappedSemantic);
			if (inputEvent.data == 1) // key down
			{
				return std::make_pair(true, [this, action] ()
				{
					switch (action)
					{ // TODO change rendering process
					case ControlType::MainView:
						process_->viewCamera_.AddToScene();
						process_->spotLight_.RemoveController();
						break;
					case ControlType::SpotLight:
						process_->spotLight_.AddController();
						process_->viewCamera_.RemoveFromScene();
						break;
					default:
						break;
					}

				});
			}
			return std::make_pair(false, std::function<void()>());

		}
	private:
		std::shared_ptr<RenderToTextureProcess> process_;
	};

}




RenderToTextureTest::RenderToTextureTest()
{
	Settings settings("../../");
	settings.windowTitle = L"GL4 window";

	settings.renderingSettings.sampleCount = 1;

	settings.renderingSettings.left = 100;
	settings.renderingSettings.top = 100;
	settings.renderingSettings.width = 1000;
	settings.renderingSettings.height = 600;

	XREXContext::GetInstance().Initialize(settings);

	{ // test code
		auto camera = MakeSP<PerspectiveCamera>(PI / 2, 2.f, 1.f, 100.f);
		auto obj = MakeSP<SceneObject>("Camera Container");
		obj->SetComponent(camera);
		floatM44 inverseProjection = camera->GetProjectionMatrix().Inverse();
		floatV3 original = floatV3(2, 2, 3);
		floatV4 clip = Transform(camera->GetProjectionMatrix(), floatV4(original.X(), original.Y(), original.Z(), 1));
		floatV3 ndc = Transform(camera->GetProjectionMatrix(), original);

		floatV4 reverseClip = Transform(inverseProjection, clip);
		floatV3 normalizedReverseClip = floatV3(reverseClip) / reverseClip.W();

		floatV4 reverseNDC = Transform(inverseProjection, floatV4(ndc.X(), ndc.Y(), ndc.Z(), 1));
		floatV3 normailizedReverseNDC = floatV3(reverseNDC) / reverseNDC.W();

		floatV4 reverseP = Transform(inverseProjection, floatV4(2, 2, 3, 1));
		floatV3 normalizedReverseP = floatV3(reverseP) / reverseP.W();

		original = original;
	}


	function<void(double current, double delta)> f = [] (double current, double delta)
	{
		assert(gl::GetError() == gl::GL_NO_ERROR);
	};
	XREXContext::GetInstance().GetRenderingEngine().OnBeforeRendering(f);
	XREXContext::GetInstance().GetRenderingEngine().OnAfterRendering(f);


	auto theProcess = MakeSP<RenderToTextureProcess>();
	XREXContext::GetInstance().GetRenderingEngine().SetRenderingProcess(theProcess);
	theProcess->viewCamera_.AddToScene();
	function<bool(double current, double delta)> l = [&theProcess] (double current, double delta)
	{
		auto process = CheckedSPCast<RenderToTextureProcess>(theProcess);
		auto& transformation = process->viewCamera_.theCameraObject_->GetComponent<Transformation>();
		floatV3 const& position = transformation->GetWorldPosition();
		floatV3 to = TransformDirection(transformation->GetWorldMatrix(), transformation->GetModelFrontDirection());
		floatV3 up = TransformDirection(transformation->GetWorldMatrix(), transformation->GetModelUpDirection());

		Ray ray = process->viewCamera_.theCameraObject_->GetComponent<Camera>()->GetViewRay(floatV2(0, 0), Camera::ViewportOrigin::ViewportCenter);

		wstringstream wss;
		wss << "p: (" << position.X() << ", " << position.Y() << ", " << position.Z() << "), ";
		wss << "d: (" << to.X() << ", " << to.Y() << ", " << to.Z() << "), ";
		wss << "ray: (" << ray.GetDirection().X() << ", " << ray.GetDirection().Y() << ", " << ray.GetDirection().Z() << "), ";
		//wss << "up: (" << up.X() << ", " << up.Y() << ", " << up.Z() << "), ";

		XREXContext::GetInstance().GetMainWindow().SetTitleText(wss.str());

		assert(gl::GetError() == gl::GL_NO_ERROR);
		return true;
	};
	XREXContext::GetInstance().SetLogicFunction(l);

	auto globalInputHandler = MakeSP<GlobalInputHandler>(theProcess);
	XREXContext::GetInstance().GetInputCenter().AddInputHandler(globalInputHandler);

	InitializeScene();


	XREXContext::GetInstance().Start();

	XREXContext::GetInstance().GetRenderingEngine().SetRenderingProcess(nullptr);
	XREXContext::GetInstance().GetInputCenter().RemoveInputHandler(globalInputHandler);
}


RenderToTextureTest::~RenderToTextureTest()
{
}
