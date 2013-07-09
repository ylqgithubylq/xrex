#include "XREXAll.hpp"
#include "VoxelTest.h"

#include <CoreGL.hpp>

#include <iostream>

#undef LoadString

using namespace XREX;
using namespace std;

namespace
{
	void SetData(vector<floatV4>& voxelVolume, uint32 size, uint32 x, uint32 y, uint32 z, floatV4 data)
	{
		voxelVolume[size *size * z + size * y + x] = data;
	}
	TextureSP MakeTest3DTexture()
	{
		uint32 size = 64;
		array<uint32, 3> dim = {size, size, size};
		Texture::DataDescription<3> desc(TexelFormat::RGBA32F, dim);
		vector<floatV4> dataLevel0(size * size * size);
		for (uint32 i = 0; i < size; ++i)
		{
			for (uint32 j = 0; j < size; ++j)
			{
				for (uint32 k = 0; k < size; ++k)
				{
					SetData(dataLevel0, size, k, j, i, floatV4(1.f / 64 * k, 1.f / 64 * j, 1.f / 64 * i, 0.99f));
				}
			}
		}
		// 		for (uint32 i = 0; i < size; ++i)
		// 		{
		// 			for (uint32 j = 0; j < size; ++j)
		// 			{
		// 				for (uint32 k = 0; k < size; ++k)
		// 				{
		// 					SetData(dataLevel0, size, k, j, i, floatV4(fmod(k * 2.0f, size) / size / 2, fmod(j * 3.0f, size) / size / 2, fmod(i * 4.0f, size) / size / 2, fmod((i + j + k) * 1.0f, size) / size / 2));
		// 				}
		// 			}
		// 		}
		vector<vector<floatV4>> data(1);
		data[0] = move(dataLevel0);
		TextureSP texture = XREXContext::GetInstance().GetRenderingFactory().CreateTexture3D(desc, data, true);

		return texture;
	}

	TextureSP MakeVoxelVolume(uint32 size)
	{
		array<uint32, 3> dim = {size, size, size};
		Texture2D::DataDescription<3> desc(TexelFormat::R32I, dim);

		TextureSP voxelVolume = XREXContext::GetInstance().GetRenderingFactory().CreateTexture3D(desc);
		return voxelVolume;
	}

	RenderingEffectSP MakeVoxelizationEffect()
	{
		string shaderString0;
		string shaderFile0 = "../../Effects/ListGeneration.glsl";
		if (!XREXContext::GetInstance().GetResourceLoader().LoadString(shaderFile0, &shaderString0))
		{
			XREXContext::GetInstance().GetLogger().LogLine("file not found. file: " + shaderFile0);
		}

		string shaderString1;
		string shaderFile1 = "../../Effects/VoxelGeneration.glsl";
		if (!XREXContext::GetInstance().GetResourceLoader().LoadString(shaderFile1, &shaderString1))
		{
			XREXContext::GetInstance().GetLogger().LogLine("file not found. file: " + shaderFile1);
		}

		RenderingEffectSP effect = MakeSP<RenderingEffect>("voxelization effect");
		effect->AddShaderCode(std::move(shaderString0));
		effect->AddShaderCode(std::move(shaderString1));


		bool listGenerationResult = [&effect] ()
		{
			ProgramObjectSP listGenerationProgram = XREXContext::GetInstance().GetRenderingFactory().CreateProgramObject();

			std::vector<std::string const*> listGenerationShaderStrings = effect->GetFullShaderCode();
			listGenerationShaderStrings.erase(listGenerationShaderStrings.end() - 1); // remove last one
			ShaderObjectSP listGenerationVS = XREXContext::GetInstance().GetRenderingFactory().CreateShaderObject(ShaderObject::ShaderType::VertexShader);
			ShaderObjectSP listGenerationFS = XREXContext::GetInstance().GetRenderingFactory().CreateShaderObject(ShaderObject::ShaderType::FragmentShader);
			listGenerationVS->Compile(listGenerationShaderStrings);
			listGenerationFS->Compile(listGenerationShaderStrings);

			listGenerationProgram->AttachShader(listGenerationVS);
			listGenerationProgram->AttachShader(listGenerationFS);
			listGenerationProgram->Link();
			if (!listGenerationProgram->IsValidate())
			{
				return false;
			}

			RasterizerState resterizerState;
			resterizerState.cullMode = RenderingPipelineState::CullMode::None;
			DepthStencilState depthStencilState;
			depthStencilState.depthEnable = false;
			depthStencilState.depthWriteMask = false;
			BlendState blendState;
			blendState.blendEnable = false;
			blendState.redMask = false;
			blendState.greenMask = false;
			blendState.blueMask = false;
			blendState.alphaMask = false;
			RasterizerStateObjectSP rso = XREXContext::GetInstance().GetRenderingFactory().CreateRasterizerStateObject(resterizerState);
			DepthStencilStateObjectSP dsso = XREXContext::GetInstance().GetRenderingFactory().CreateDepthStencilStateObject(depthStencilState);
			BlendStateObjectSP bso = XREXContext::GetInstance().GetRenderingFactory().CreateBlendStateObject(blendState);

			RenderingTechniqueSP listGenerationTechnique = effect->CreateTechnique("list generation technique");
			RenderingPassSP listGenerationCubePass = listGenerationTechnique->CreatePass(listGenerationProgram, rso, dsso, bso);
			return true;
		} ();

		bool voxelGenerationResult = [&effect] ()
		{
			ProgramObjectSP voxelGenerationProgram = XREXContext::GetInstance().GetRenderingFactory().CreateProgramObject();

			std::vector<std::string const*> voxelGenerationShaderStrings = effect->GetFullShaderCode();
			voxelGenerationShaderStrings.erase(voxelGenerationShaderStrings.end() - 2);
			ShaderObjectSP voxelGenerationVS = XREXContext::GetInstance().GetRenderingFactory().CreateShaderObject(ShaderObject::ShaderType::VertexShader);
			ShaderObjectSP voxelGenerationFS = XREXContext::GetInstance().GetRenderingFactory().CreateShaderObject(ShaderObject::ShaderType::FragmentShader);
			voxelGenerationVS->Compile(voxelGenerationShaderStrings);
			voxelGenerationFS->Compile(voxelGenerationShaderStrings);

			voxelGenerationProgram->AttachShader(voxelGenerationVS);
			voxelGenerationProgram->AttachShader(voxelGenerationFS);
			voxelGenerationProgram->Link();
			if (!voxelGenerationProgram->IsValidate())
			{
				return false;
			}

			RasterizerState resterizerState;
			resterizerState.cullMode = RenderingPipelineState::CullMode::Front;
			DepthStencilState depthStencilState;
			BlendState blendState;
			blendState.blendEnable = true;
			blendState.blendOperation = RenderingPipelineState::BlendOperation::Add;
			blendState.blendOperationAlpha = RenderingPipelineState::BlendOperation::Add;
			blendState.sourceBlend = RenderingPipelineState::AlphaBlendFactor::One;
			blendState.sourceBlendAlpha = RenderingPipelineState::AlphaBlendFactor::SourceAlpha;
			blendState.destinationBlend = RenderingPipelineState::AlphaBlendFactor::OneMinusSourceAlpha;
			blendState.destinationBlendAlpha = RenderingPipelineState::AlphaBlendFactor::OneMinusSourceAlpha;
			RasterizerStateObjectSP rso = XREXContext::GetInstance().GetRenderingFactory().CreateRasterizerStateObject(resterizerState);
			DepthStencilStateObjectSP dsso = XREXContext::GetInstance().GetRenderingFactory().CreateDepthStencilStateObject(depthStencilState);
			BlendStateObjectSP bso = XREXContext::GetInstance().GetRenderingFactory().CreateBlendStateObject(blendState);

			RenderingTechniqueSP voxelGenerationTechnique = effect->CreateTechnique("voxel generation technique");
			RenderingPassSP voxelGenerationCubePass = voxelGenerationTechnique->CreatePass(voxelGenerationProgram, rso, dsso, bso);
			return true;
		} ();
		
		assert(listGenerationResult && voxelGenerationResult);

		return effect;
	}


	MeshSP MakeCube(float cubeSize)
	{
		vector<floatV3> vertexData;
		vector<uint16> indexData;

		vertexData.push_back(floatV3(cubeSize, cubeSize, cubeSize));
		vertexData.push_back(floatV3(cubeSize, -cubeSize, cubeSize));
		vertexData.push_back(floatV3(-cubeSize, -cubeSize, cubeSize));
		vertexData.push_back(floatV3(-cubeSize, cubeSize, cubeSize));
		vertexData.push_back(floatV3(cubeSize, cubeSize, -cubeSize));
		vertexData.push_back(floatV3(cubeSize, -cubeSize, -cubeSize));
		vertexData.push_back(floatV3(-cubeSize, -cubeSize, -cubeSize));
		vertexData.push_back(floatV3(-cubeSize, cubeSize, -cubeSize));

		indexData.push_back(0);
		indexData.push_back(3);
		indexData.push_back(2);
		indexData.push_back(0);
		indexData.push_back(2);
		indexData.push_back(1);

		indexData.push_back(4);
		indexData.push_back(0);
		indexData.push_back(1);
		indexData.push_back(4);
		indexData.push_back(1);
		indexData.push_back(5);

		indexData.push_back(7);
		indexData.push_back(4);
		indexData.push_back(5);
		indexData.push_back(7);
		indexData.push_back(5);
		indexData.push_back(6);

		indexData.push_back(3);
		indexData.push_back(7);
		indexData.push_back(6);
		indexData.push_back(3);
		indexData.push_back(6);
		indexData.push_back(2);

		indexData.push_back(0);
		indexData.push_back(4);
		indexData.push_back(7);
		indexData.push_back(0);
		indexData.push_back(7);
		indexData.push_back(3);

		indexData.push_back(6);
		indexData.push_back(5);
		indexData.push_back(1);
		indexData.push_back(6);
		indexData.push_back(1);
		indexData.push_back(2);

		assert(indexData.size() == 36);

		VertexBuffer::DataLayoutDescription layoutDesc(8);
		layoutDesc.AddChannelLayout(VertexBuffer::DataLayoutDescription::ElementLayoutDescription(0, sizeof(floatV3), ElementType::FloatV3, "position"));
		VertexBufferSP vertices = XREXContext::GetInstance().GetRenderingFactory().CreateVertexBuffer(GraphicsBuffer::Usage::StaticDraw, vertexData, move(layoutDesc));
		IndexBufferSP indices = XREXContext::GetInstance().GetRenderingFactory().CreateIndexBuffer(GraphicsBuffer::Usage::StaticDraw, indexData, IndexBuffer::TopologicalType::Triangles);
		RenderingLayoutSP layout = MakeSP<RenderingLayout>(vector<VertexBufferSP>(1, vertices), indices);

		MeshSP cubeMesh = MakeSP<Mesh>("cube mesh");

		SubMeshSP const& subMesh = cubeMesh->CreateSubMesh("cube sub mesh", layout);

		return cubeMesh;
	}

	RenderingEffectSP MakeConeTracingEffect()
	{
		string shaderString;
		string shaderFile = "../../Effects/ConeTracing.glsl";
		if (!XREXContext::GetInstance().GetResourceLoader().LoadString(shaderFile, &shaderString))
		{
			XREXContext::GetInstance().GetLogger().LogLine("file not found. file: " + shaderFile);
		}

		RenderingEffectSP coneTracingEffect = MakeSP<RenderingEffect>("cone tracing effect");
		coneTracingEffect->AddShaderCode(std::move(shaderString));

		ProgramObjectSP coneTracingProgram = XREXContext::GetInstance().GetRenderingFactory().CreateProgramObject();

		std::vector<std::string const*> const& shaderStrings = coneTracingEffect->GetFullShaderCode();
		ShaderObjectSP vs = XREXContext::GetInstance().GetRenderingFactory().CreateShaderObject(ShaderObject::ShaderType::VertexShader);
		ShaderObjectSP fs = XREXContext::GetInstance().GetRenderingFactory().CreateShaderObject(ShaderObject::ShaderType::FragmentShader);
		vs->Compile(shaderStrings);
		fs->Compile(shaderStrings);

		coneTracingProgram->AttachShader(vs);
		coneTracingProgram->AttachShader(fs);
		coneTracingProgram->Link();
		if (!coneTracingProgram->IsValidate())
		{
			return nullptr;
		}

		RasterizerState resterizerState;
		resterizerState.cullMode = RenderingPipelineState::CullMode::Front;
		DepthStencilState depthStencilState;
		BlendState blendState;
		blendState.blendEnable = true;
		blendState.blendOperation = RenderingPipelineState::BlendOperation::Add;
		blendState.blendOperationAlpha = RenderingPipelineState::BlendOperation::Add;
		blendState.sourceBlend = RenderingPipelineState::AlphaBlendFactor::One;
		blendState.sourceBlendAlpha = RenderingPipelineState::AlphaBlendFactor::SourceAlpha;
		blendState.destinationBlend = RenderingPipelineState::AlphaBlendFactor::OneMinusSourceAlpha;
		blendState.destinationBlendAlpha = RenderingPipelineState::AlphaBlendFactor::OneMinusSourceAlpha;
		RasterizerStateObjectSP rso = XREXContext::GetInstance().GetRenderingFactory().CreateRasterizerStateObject(resterizerState);
		DepthStencilStateObjectSP dsso = XREXContext::GetInstance().GetRenderingFactory().CreateDepthStencilStateObject(depthStencilState);
		BlendStateObjectSP bso = XREXContext::GetInstance().GetRenderingFactory().CreateBlendStateObject(blendState);

		RenderingTechniqueSP cubeTechnique = coneTracingEffect->CreateTechnique("cone tracing technique");
		RenderingPassSP cubePass = cubeTechnique->CreatePass(coneTracingProgram, rso, dsso, bso);

		return coneTracingEffect;
	}


	SamplerSP CreateConeTracingSampler()
	{
		SamplerState ss;
		ss.borderColor = Color(0, 0, 0, 0);
		ss.addressingModeR = SamplerState::TextureAddressingMode::ClampToBorder;
		ss.addressingModeS = SamplerState::TextureAddressingMode::ClampToBorder;
		ss.addressingModeT = SamplerState::TextureAddressingMode::ClampToBorder;
		ss.magFilterOperation = SamplerState::TextureFilterOperation::Linear;
		ss.minFilterOperation = SamplerState::TextureFilterOperation::LinearMipmapLinear;
		SamplerSP sampler = XREXContext::GetInstance().GetRenderingFactory().CreateSampler(ss);
		return sampler;
	}

	SceneObjectSP MakeConeTracingProxyCube(TextureSP const& voxelVolume, PerspectiveCameraSP const& camera, floatV3 const& cubePosition, float cubeHalfSize)
	{

		SceneObjectSP cubeObject = MakeSP<SceneObject>("cube object");
		MeshSP cube = MakeCube(cubeHalfSize);

		RenderingEffectSP coneTracingEffect = MakeConeTracingEffect();
		MaterialSP material = MakeSP<Material>("tracing effect parameters");
		std::pair<TextureSP, SamplerSP> texture3D = std::make_pair(voxelVolume, CreateConeTracingSampler());
		material->SetParameter("voxels", texture3D);
		material->SetParameter("voxelVolumeCenter", cubePosition);
		material->SetParameter("voxelVolumeHalfSize", cubeHalfSize);
		material->SetParameter("aperture", camera->GetFieldOfView() / XREXContext::GetInstance().GetMainWindow().GetClientRegionSize().y);



		for (auto& sub : cube->GetAllSubMeshes())
		{
			sub->SetTechnique(coneTracingEffect->GetTechnique(0));
			sub->SetMaterial(material);
		}
		cubeObject->SetComponent(cube);
		TransformationSP trans = cubeObject->GetComponent<Transformation>();
		trans->SetPosition(cubePosition);

		return cubeObject;
	}

	SceneObjectSP MakeCamera()
	{
		SceneObjectSP cameraObject = MakeSP<SceneObject>("camera");
		Settings const& settings = XREXContext::GetInstance().GetSettings();
		CameraSP camera = MakeSP<PerspectiveCamera>(PI / 4, static_cast<float>(settings.renderingSettings.width) / settings.renderingSettings.height, 0.01f, 1000.0f);
		cameraObject->SetComponent(camera);

		auto cc = MakeSP<FirstPersonCameraController>();
		cc->AttachToCamera(cameraObject);
		XREXContext::GetInstance().GetInputCenter().AddInputHandler(cc);

		return cameraObject;
	}


	struct VoxelizationAndRenderProcess
		: RenderingProcess
	{

		RenderingEffectSP voxelizationEffect;
		MaterialSP voxelizationMaterial;
		ViewportSP voxelizationViewport;

		std::array<SceneObjectSP, 3> voxelizationCameras;

		std::array<TextureSP, 3> headPointers;
		std::array<TextureSP, 3> nodePools;
		std::array<GraphicsBufferSP, 3> atomicCounterBuffers;

		SceneObjectSP coneTracingProxyCube;
		SceneObjectSP viewCameraObject;

		floatV3 sceneCenter;
		float sceneHalfSize;

		uint32 voxelVolumeResolution;
		TextureSP voxelVolume;

		uint32 frame;
		

		VoxelizationAndRenderProcess(floatV3 const& sceneCenter, float sceneHalfSize, uint32 voxelVolumeResolution)
			: frame(0)
		{
			this->sceneCenter = sceneCenter;
			this->sceneHalfSize = sceneHalfSize;

			this->voxelVolumeResolution = voxelVolumeResolution;

			CreateVoxelizationObjects();


			viewCameraObject = MakeCamera();
			coneTracingProxyCube = MakeConeTracingProxyCube(voxelVolume, CheckedSPCast<PerspectiveCamera>(viewCameraObject->GetComponent<Camera>()), sceneCenter, sceneHalfSize);
		}

		void CreateVoxelizationObjects()
		{
			voxelizationViewport = XREXContext::GetInstance().GetRenderingFactory().CreateViewport(0, 0, 0, voxelVolumeResolution, voxelVolumeResolution);
			voxelizationEffect = MakeVoxelizationEffect();
			voxelizationMaterial = MakeSP<Material>("voxelization material");

			for (uint32 i = 0; i < 3; ++i)
			{
				CameraSP camera = MakeSP<OrthogonalCamera>(sceneHalfSize * 2, sceneHalfSize * 2, sceneHalfSize * 2);
				camera->AttachToViewport(voxelizationViewport);
				std::string cameraName = "voxelization camera 0";
				cameraName[cameraName.find('0')] = '0' + i;
				SceneObjectSP cameraObject = MakeSP<SceneObject>(std::move(cameraName));
				cameraObject->SetComponent(camera);
				voxelizationCameras[i] = cameraObject;
			}
			TransformationSP const& cameraTransformation0 = voxelizationCameras[0]->GetComponent<Transformation>();
			cameraTransformation0->Translate(sceneCenter + floatV3(sceneHalfSize, 0, 0));
			cameraTransformation0->FaceToDirection(floatV3(-1, 0, 0), floatV3(0, 1, 0));
			TransformationSP const& cameraTransformation1 = voxelizationCameras[1]->GetComponent<Transformation>();
			cameraTransformation1->Translate(sceneCenter + floatV3(0, sceneHalfSize, 0));
			cameraTransformation1->FaceToDirection(floatV3(0, -1, 0), floatV3(0, 0, 1));
			TransformationSP const& cameraTransformation2 = voxelizationCameras[2]->GetComponent<Transformation>();
			cameraTransformation2->Translate(sceneCenter + floatV3(0, 0, sceneHalfSize));
			cameraTransformation2->FaceToDirection(floatV3(0, 0, -1), floatV3(1, 0, 0));

			for (uint32 i = 0; i < 3; ++i)
			{
				std::array<uint32, 2> dim = {voxelVolumeResolution, voxelVolumeResolution};
				Texture::DataDescription<2> desc(TexelFormat::R32UI, dim);

				TextureSP headPointer = XREXContext::GetInstance().GetRenderingFactory().CreateTexture2D(desc);
				headPointers[i] = headPointer;

				TexelFormat linkedListNodeFormat = TexelFormat::RGBA32UI;
				uint32 poolSize = GetTexelSizeInBytes(linkedListNodeFormat) * voxelVolumeResolution * voxelVolumeResolution;
				GraphicsBufferSP buffer = XREXContext::GetInstance().GetRenderingFactory().CreateGraphicsBuffer(GraphicsBuffer::Usage::StreamCopy, poolSize, BufferView::BufferType::Texture);
				TextureSP linkedListNodePool = XREXContext::GetInstance().GetRenderingFactory().CreateTextureBuffer(buffer, linkedListNodeFormat);
				nodePools[i] = linkedListNodePool;
				GraphicsBufferSP atomicBuffer = XREXContext::GetInstance().GetRenderingFactory().CreateGraphicsBuffer(GraphicsBuffer::Usage::StreamCopy, 4, BufferView::BufferType::AtomicCounter);
				atomicCounterBuffers[i] = atomicBuffer;
			}

			voxelVolume = MakeVoxelVolume(voxelVolumeResolution);
		}

		virtual void RenderScene(SceneSP const& scene) override
		{
			XREXContext::GetInstance().GetLogger().LogLine("frame: " + std::to_string(frame++));
			std::vector<SceneObjectSP> sceneObjects = scene->GetRenderableQueue(nullptr);

			std::vector<Renderable::RenderablePack> allRenderableNeedToRender;

			for (SceneObjectSP sceneObject : sceneObjects)
			{
				TransformationSP transformation = sceneObject->GetComponent<Transformation>();
				RenderableSP renderable = sceneObject->GetComponent<Renderable>();
				assert(renderable != nullptr);
				assert(renderable->IsVisible());

				std::vector<Renderable::RenderablePack> renderablePacks = renderable->GetRenderablePack(viewCameraObject);
				for (auto& renderablePack : renderablePacks)
				{
					allRenderableNeedToRender.emplace_back(std::move(renderablePack));
				}
			}

			BuildFragmentLists(allRenderableNeedToRender);
			for (uint32 i = 0; i < 3; ++i)
			{
				GraphicsBuffer::BufferMapper mapper = atomicCounterBuffers[i]->GetMapper(AccessType::ReadOnly);
				uint32 count = mapper.GetPointer<uint32>()[0];
				XREXContext::GetInstance().GetLogger().LogLine("count: " + std::to_string(count));
			}
			BuildVoxelVolume();

// EffectParameterSP const& atomicCounter = voxelizationEffect->GetParameterByName("0");
// ShaderResourceBufferSP atomicBuffer = atomicCounter->As<ShaderResourceBufferSP>().GetValue();
// ShaderResourceBuffer::BufferVariableSetter setter = atomicBuffer->GetVariableSetter();
// bool settingResult = setter.SetValue("nodeCounter", 0u);
		}

		void BuildFragmentLists(std::vector<Renderable::RenderablePack> const& allRenderableNeedToRender)
		{
			for (uint32 i = 0; i < 3; ++i)
			{
				//atomicCounterBuffers[i]->Clear(0);
				// TODO clear linked list heads texture
				// 
				CameraSP const& camera = voxelizationCameras[i]->GetComponent<Camera>();
				floatM44 viewMatrix = camera->GetViewMatrix();
				floatM44 projectionMatrix = camera->GetProjectionMatrix();
				EffectParameterSP const& view = voxelizationEffect->GetParameterByName(GetUniformString(DefinedUniform::ViewMatrix));
				view->As<floatM44>().SetValue(viewMatrix);
				EffectParameterSP const& projection = voxelizationEffect->GetParameterByName(GetUniformString(DefinedUniform::ProjectionMatrix));
				projection->As<floatM44>().SetValue(projectionMatrix);

				EffectParameterSP const& linkedListNodePool = voxelizationEffect->GetParameterByName("nodePool");
				linkedListNodePool->As<TextureImageSP>().SetValue(nodePools[i]->GetImage_TEMP(0, nodePools[i]->GetFormat()));
				EffectParameterSP const& headPointer = voxelizationEffect->GetParameterByName("header");
				headPointer->As<TextureImageSP>().SetValue(headPointers[i]->GetImage_TEMP(0, headPointers[i]->GetFormat()));
				EffectParameterSP const& atomicCounter = voxelizationEffect->GetParameterByName("0");
				ShaderResourceBufferSP atomicBuffer = atomicCounter->As<ShaderResourceBufferSP>().GetValue();
				atomicBuffer->SetBuffer(atomicCounterBuffers[i]);
				{
					ShaderResourceBuffer::BufferVariableSetter setter = atomicBuffer->GetVariableSetter();
					bool settingResult = setter.SetValue("nodeCounter", 0u);
					assert(settingResult);
				}


				RenderingTechniqueSP const& rasterizeTechnique = voxelizationEffect->GetTechnique(0);
				RenderingPassSP rasterizePass = rasterizeTechnique->GetPass(0);
				for (auto& renderablePack : allRenderableNeedToRender)
				{
					Renderable& ownerRenderable = *renderablePack.renderable;
					RenderingLayoutSP const& layout = renderablePack.layout;

					floatM44 const& modelMatrix = ownerRenderable.GetOwnerSceneObject()->GetComponent<Transformation>()->GetWorldMatrix();
					EffectParameterSP const& model = voxelizationEffect->GetParameterByName(GetUniformString(DefinedUniform::ModelMatrix));
					model->As<floatM44>().SetValue(modelMatrix);

					rasterizePass->Use();
					layout->BindToProgram(rasterizePass->GetProgram());
					layout->Draw();
					layout->Unbind();
				}
			}


		}

		void BuildVoxelVolume()
		{
			// use linked list heads texture, linked list texture
		}
	};

	SceneObjectSP LoadTeapot()
	{
		MeshSP model = XREXContext::GetInstance().GetResourceManager().LoadModel("Data/teapot/teapot.obj")->Create();
		SceneObjectSP sceneObject = MakeSP<SceneObject>("scene object");
		sceneObject->SetComponent(model);
		return sceneObject;
	}

	void InitializeScene()
	{
		auto texture3D = MakeTest3DTexture();
		std::shared_ptr<VoxelizationAndRenderProcess> renderingProcess = MakeSP<VoxelizationAndRenderProcess>(floatV3(0, 0, 0), 16.f, 256);
		XREXContext::GetInstance().GetRenderingEngine().SetRenderingProcess(renderingProcess);
		SceneObjectSP sceneObject = LoadTeapot();
		assert(sceneObject);
		XREXContext::GetInstance().GetScene()->AddObject(sceneObject);
	}
}


VoxelTest::VoxelTest()
{
	Settings settings("../../");
	settings.windowTitle = L"GL4 window";

	settings.renderingSettings.colorBits = 32;
	settings.renderingSettings.depthBits = 24;
	settings.renderingSettings.stencilBits = 8;
	settings.renderingSettings.sampleCount = 4;

	settings.renderingSettings.left = 300;
	settings.renderingSettings.top = 200;
	settings.renderingSettings.width = 800;
	settings.renderingSettings.height = 600;

	XREXContext::GetInstance().Initialize(settings);

	function<bool(double current, double delta)> l = [] (double current, double delta)
	{
		assert(gl::GetError() == gl::GL_NO_ERROR);
		return true;
	};
	XREXContext::GetInstance().SetLogicFunction(l);

	InitializeScene();

	XREXContext::GetInstance().Start();
}


VoxelTest::~VoxelTest()
{
}
