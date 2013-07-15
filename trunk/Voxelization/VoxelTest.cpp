#include "XREXAll.hpp"
#include "VoxelTest.h"

#include "GLUtil.hpp"
#include <CoreGL.hpp>

#include <iostream>
#include <sstream>


#undef LoadString

using namespace XREX;
using namespace std;

namespace
{
	void SetData(vector<floatV4>& voxelVolume, uint32 size, uint32 x, uint32 y, uint32 z, floatV4 data)
	{
		voxelVolume[size *size * z + size * y + x] = data;
		//voxelVolume[size *size * z + size * y + x] = floatV4(0, 0, 0, 0.1f);
	}
	TextureSP MakeTest3DTexture()
	{
		uint32 size = 128;
		array<uint32, 3> dim = {size, size, size};
		Texture::DataDescription<3> desc(TexelFormat::RGBA32F, dim);
		vector<floatV4> dataLevel0(size * size * size);
		for (uint32 i = 0; i < size; ++i)
		{
			for (uint32 j = 0; j < size; ++j)
			{
				for (uint32 k = 0; k < size; ++k)
				{
					float alpha = 0.09f;
					SetData(dataLevel0, size, k, j, i, floatV4(1.f / size * k * alpha, 1.f / size * j * alpha, 1.f / size * i * alpha, alpha));
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

	RenderingLayoutSP MakeScreenQuad()
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

		return quad;
	}

	TextureSP MakeIntermediateVoxelVolume(uint32 size)
	{
		array<uint32, 3> dim = {size, size, size};
		Texture2D::DataDescription<3> desc(TexelFormat::R32UI, dim);

		TextureSP voxelVolume = XREXContext::GetInstance().GetRenderingFactory().CreateTexture3D(desc, false);
		return voxelVolume;
	}
	GraphicsBufferSP MakeIntermediateClearVoxelVolume(uint32 size)
	{
		GraphicsBufferSP voxelVolume = XREXContext::GetInstance().GetRenderingFactory().CreateGraphicsBuffer(GraphicsBuffer::Usage::StaticDraw, size * size * size * sizeof(uint32));
		voxelVolume->Clear(0u);
		return voxelVolume;
	}

	TextureSP MakeVoxelVolume(uint32 size)
	{
		array<uint32, 3> dim = {size, size, size};
		Texture2D::DataDescription<3> desc(TexelFormat::RGBA32F, dim);

		TextureSP voxelVolume = XREXContext::GetInstance().GetRenderingFactory().CreateTexture3D(desc, true);
		return voxelVolume;
	}

	GraphicsBufferSP MakeClearVoxelVolume(uint32 size)
	{
		GraphicsBufferSP voxelVolume = XREXContext::GetInstance().GetRenderingFactory().CreateGraphicsBuffer(GraphicsBuffer::Usage::StaticDraw, size * size * size * sizeof(floatV4));
		voxelVolume->Clear(0u);
		return voxelVolume;
	}
	std::array<RenderingEffectSP, 3> MakeVoxelizationEffect()
	{
		string shaderString0;
		string shaderFile0 = "../../Voxelization/ListGeneration.glsl";
		if (!XREXContext::GetInstance().GetResourceLoader().LoadString(shaderFile0, &shaderString0))
		{
			XREXContext::GetInstance().GetLogger().LogLine("file not found. file: " + shaderFile0);
		}


		RenderingEffectSP listEffect = MakeSP<RenderingEffect>("voxelization list effect");
		listEffect->AddShaderCode(std::move(shaderString0));

		bool listGenerationResult = [&listEffect] ()
		{
			ProgramObjectSP listGenerationProgram = XREXContext::GetInstance().GetRenderingFactory().CreateProgramObject();

			std::vector<std::string const*> listGenerationShaderStrings = listEffect->GetFullShaderCode();
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

			RenderingTechniqueSP listGenerationTechnique = listEffect->CreateTechnique("list generation technique");
			RenderingPassSP listGenerationCubePass = listGenerationTechnique->CreatePass(listGenerationProgram, rso, dsso, bso);
			return true;
		} ();

		string shaderString1;
		string shaderFile1 = "../../Voxelization/VoxelGeneration.glsl";
		if (!XREXContext::GetInstance().GetResourceLoader().LoadString(shaderFile1, &shaderString1))
		{
			XREXContext::GetInstance().GetLogger().LogLine("file not found. file: " + shaderFile1);
		}
		RenderingEffectSP generationEffect = MakeSP<RenderingEffect>("voxelization generation effect");
		generationEffect->AddShaderCode(std::move(shaderString1));

		bool voxelGenerationResult = [&generationEffect] ()
		{
			ProgramObjectSP voxelGenerationProgram = XREXContext::GetInstance().GetRenderingFactory().CreateProgramObject();

			std::vector<std::string const*> voxelGenerationShaderStrings = generationEffect->GetFullShaderCode();
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

			RenderingTechniqueSP voxelGenerationTechnique = generationEffect->CreateTechnique("voxel generation technique");
			RenderingPassSP voxelGenerationCubePass = voxelGenerationTechnique->CreatePass(voxelGenerationProgram, rso, dsso, bso);
			return true;
		} ();

		string shaderString2;
		string shaderFile2 = "../../Voxelization/VoxelMerge.glsl";
		if (!XREXContext::GetInstance().GetResourceLoader().LoadString(shaderFile2, &shaderString2))
		{
			XREXContext::GetInstance().GetLogger().LogLine("file not found. file: " + shaderFile2);
		}
		RenderingEffectSP mergeEffect = MakeSP<RenderingEffect>("voxelization merge effect");
		mergeEffect->AddShaderCode(std::move(shaderString2));

		bool voxelMergeResult = [&mergeEffect] ()
		{
			ProgramObjectSP voxelMergeProgram = XREXContext::GetInstance().GetRenderingFactory().CreateProgramObject();

			std::vector<std::string const*> voxelGenerationShaderStrings = mergeEffect->GetFullShaderCode();
			ShaderObjectSP voxelMergeVS = XREXContext::GetInstance().GetRenderingFactory().CreateShaderObject(ShaderObject::ShaderType::VertexShader);
			ShaderObjectSP voxelMergeFS = XREXContext::GetInstance().GetRenderingFactory().CreateShaderObject(ShaderObject::ShaderType::FragmentShader);
			voxelMergeVS->Compile(voxelGenerationShaderStrings);
			voxelMergeFS->Compile(voxelGenerationShaderStrings);

			voxelMergeProgram->AttachShader(voxelMergeVS);
			voxelMergeProgram->AttachShader(voxelMergeFS);
			voxelMergeProgram->Link();
			if (!voxelMergeProgram->IsValidate())
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

			RenderingTechniqueSP voxelMergeTechnique = mergeEffect->CreateTechnique("voxel merge technique");
			RenderingPassSP voxelMergeCubePass = voxelMergeTechnique->CreatePass(voxelMergeProgram, rso, dsso, bso);
			return true;
		} ();
		
		assert(listGenerationResult && voxelGenerationResult);
		std::array<RenderingEffectSP, 3> effects = {listEffect, generationEffect, mergeEffect};
		return effects;
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
		RenderingLayoutSP layout = XREXContext::GetInstance().GetRenderingFactory().CreateRenderingLayout(vector<VertexBufferSP>(1, vertices), indices);

		MeshSP cubeMesh = MakeSP<Mesh>("cube mesh");

		SubMeshSP const& subMesh = cubeMesh->CreateSubMesh("cube sub mesh", layout);

		return cubeMesh;
	}

	RenderingEffectSP MakeConeTracingEffect()
	{
		string shaderString;
		string shaderFile = "../../Voxelization/ConeTracing.glsl";
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
		resterizerState.cullMode = RenderingPipelineState::CullMode::None;
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

	SceneObjectSP MakeConeTracingProxyCube(PerspectiveCameraSP const& camera, floatV3 const& cubePosition, float cubeHalfSize)
	{

		SceneObjectSP cubeObject = MakeSP<SceneObject>("cube object");
		MeshSP cube = MakeCube(cubeHalfSize);

		RenderingEffectSP coneTracingEffect = MakeConeTracingEffect();
		MaterialSP material = MakeSP<Material>("tracing effect parameters");
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

		std::array<RenderingEffectSP, 3> voxelizationEffect;
		MaterialSP voxelizationMaterial;
		ViewportSP listBuildingViewport;
		ViewportSP voxelMergeViewport;

		std::array<SceneObjectSP, 3> voxelizationCameras;

		std::array<TextureSP, 3> headPointers;
		std::array<TextureSP, 3> nodePools;
		std::array<GraphicsBufferSP, 3> atomicCounterBuffers;
		GraphicsBufferSP clearPointer;

		floatV3 sceneCenter;
		float sceneHalfSize;

		uint32 voxelVolumeResolution;
		uint32 intermediateVoxelVolumeResolution;

		TextureSP intermediateVoxelVolume;
		TextureSP voxelVolume;
		GraphicsBufferSP intermediateClearVoxelVolume;
		GraphicsBufferSP clearVoxelVolume;

		RenderingLayoutSP screenQuad;

		SceneObjectSP coneTracingProxyCube;
		SceneObjectSP viewCameraObject;

		std::pair<TextureSP, SamplerSP> voxelVolumeToTrace;

		TextureSP texture3DToTest;

		uint32 frame;
		

		VoxelizationAndRenderProcess(floatV3 const& sceneCenter, float sceneHalfSize, uint32 voxelVolumeResolution)
			: frame(0)
		{
			this->sceneCenter = sceneCenter;
			this->sceneHalfSize = sceneHalfSize;

			this->voxelVolumeResolution = voxelVolumeResolution;
			this->intermediateVoxelVolumeResolution = voxelVolumeResolution * 2;

			CreateVoxelizationObjects();

			viewCameraObject = MakeCamera();
			coneTracingProxyCube = MakeConeTracingProxyCube(CheckedSPCast<PerspectiveCamera>(viewCameraObject->GetComponent<Camera>()), sceneCenter, sceneHalfSize);
			voxelVolumeToTrace = std::make_pair(nullptr, CreateConeTracingSampler());

			texture3DToTest = MakeTest3DTexture();

			screenQuad = MakeScreenQuad();
		}

		void CreateVoxelizationObjects()
		{
			listBuildingViewport = XREXContext::GetInstance().GetRenderingFactory().CreateViewport(0, 0, 0, intermediateVoxelVolumeResolution, intermediateVoxelVolumeResolution);
			voxelMergeViewport = XREXContext::GetInstance().GetRenderingFactory().CreateViewport(0, 0, 0, voxelVolumeResolution, voxelVolumeResolution);
			voxelizationEffect = MakeVoxelizationEffect();
			voxelizationMaterial = MakeSP<Material>("voxelization material");

			for (uint32 i = 0; i < 3; ++i)
			{
				CameraSP camera = MakeSP<OrthogonalCamera>(sceneHalfSize * 2, sceneHalfSize * 2, sceneHalfSize * 2);
				camera->AttachToViewport(listBuildingViewport);
				std::string cameraName = "voxelization camera " + std::to_string(i);
				SceneObjectSP cameraObject = MakeSP<SceneObject>(std::move(cameraName));
				cameraObject->SetComponent(camera);
				voxelizationCameras[i] = cameraObject;
			}
			// from +x, up is +y
			TransformationSP const& cameraTransformation0 = voxelizationCameras[0]->GetComponent<Transformation>();
			cameraTransformation0->Translate(sceneCenter + floatV3(sceneHalfSize, 0, 0));
			cameraTransformation0->FaceToDirection(floatV3(-1, 0, 0), floatV3(0, 0, 1));
			// from +y, up is +z
			TransformationSP const& cameraTransformation1 = voxelizationCameras[1]->GetComponent<Transformation>();
			cameraTransformation1->Translate(sceneCenter + floatV3(0, sceneHalfSize, 0));
			cameraTransformation1->FaceToDirection(floatV3(0, -1, 0), floatV3(1, 0, 0));
			// from +z, up is +x
			TransformationSP const& cameraTransformation2 = voxelizationCameras[2]->GetComponent<Transformation>();
			cameraTransformation2->Translate(sceneCenter + floatV3(0, 0, sceneHalfSize));
			cameraTransformation2->FaceToDirection(floatV3(0, 0, -1), floatV3(0, 1, 0));
			
			{
				GraphicsBufferSP headPointer = XREXContext::GetInstance().GetRenderingFactory().CreateGraphicsBuffer(GraphicsBuffer::Usage::StaticDraw, intermediateVoxelVolumeResolution * intermediateVoxelVolumeResolution * sizeof(uint32));
				headPointer->Clear(0u);
				clearPointer = headPointer;
			}

			for (uint32 i = 0; i < 3; ++i)
			{
				std::array<uint32, 2> dim = {intermediateVoxelVolumeResolution, intermediateVoxelVolumeResolution};
				Texture::DataDescription<2> desc(TexelFormat::R32UI, dim);

				TextureSP headPointer = XREXContext::GetInstance().GetRenderingFactory().CreateTexture2D(desc, false);
				headPointers[i] = headPointer;

				TexelFormat linkedListNodeFormat = TexelFormat::RGBA32UI;
				uint32 poolSize = GetTexelSizeInBytes(linkedListNodeFormat) * intermediateVoxelVolumeResolution * intermediateVoxelVolumeResolution * 8;
				GraphicsBufferSP buffer = XREXContext::GetInstance().GetRenderingFactory().CreateGraphicsBuffer(GraphicsBuffer::Usage::StreamCopy, poolSize, BufferView::BufferType::Texture);
				TextureSP linkedListNodePool = XREXContext::GetInstance().GetRenderingFactory().CreateTextureBuffer(buffer, linkedListNodeFormat);
				nodePools[i] = linkedListNodePool;
				GraphicsBufferSP atomicBuffer = XREXContext::GetInstance().GetRenderingFactory().CreateGraphicsBuffer(GraphicsBuffer::Usage::StreamCopy, 4, BufferView::BufferType::AtomicCounter);
				atomicCounterBuffers[i] = atomicBuffer;
			}

			intermediateVoxelVolume = MakeIntermediateVoxelVolume(voxelVolumeResolution);
			voxelVolume = MakeVoxelVolume(voxelVolumeResolution);

			intermediateClearVoxelVolume = MakeIntermediateClearVoxelVolume(voxelVolumeResolution);
			clearVoxelVolume = MakeClearVoxelVolume(voxelVolumeResolution);

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

				std::vector<Renderable::RenderablePack> renderablePacks = renderable->GetRenderablePack(nullptr);
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

			BuildIntermediateVoxelVolume();
			BuildVoxelVolume();

			ConeTracing();
		}

		void BuildFragmentLists(std::vector<Renderable::RenderablePack> const& allRenderableNeedToRender)
		{
			RenderingEffectSP listEffect = voxelizationEffect[0];

			for (uint32 i = 0; i < 3; ++i) // TODO
			{
				{ // clear headPointer texture
					uint32 glClearPointer = clearPointer->GetID();
					std::shared_ptr<Texture2D> headPointerAs2D = CheckedSPCast<Texture2D>(headPointers[i]);
					headPointerAs2D->Bind(0);
					GLTextureFormat glFormat = GLTextureFormatFromTexelFormat(headPointerAs2D->GetDescription().GetFormat());

					gl::BindBuffer(gl::GL_PIXEL_UNPACK_BUFFER, glClearPointer);
					gl::TexSubImage2D(gl::GL_TEXTURE_2D, 0, 0, 0, headPointerAs2D->GetDescription().GetSizes()[0], headPointerAs2D->GetDescription().GetSizes()[1],
						glFormat.glSourceFormat, glFormat.glTextureElementType, nullptr);
					gl::BindBuffer(gl::GL_PIXEL_UNPACK_BUFFER, 0);
				}


				CameraSP const& camera = voxelizationCameras[i]->GetComponent<Camera>();
				camera->GetViewport()->Bind(0, 0);
				floatM44 viewMatrix = camera->GetViewMatrix();
				floatM44 projectionMatrix = camera->GetProjectionMatrix();
				EffectParameterSP const& view = listEffect->GetParameterByName(GetUniformString(DefinedUniform::ViewMatrix));
				view->As<floatM44>().SetValue(viewMatrix);
				EffectParameterSP const& projection = listEffect->GetParameterByName(GetUniformString(DefinedUniform::ProjectionMatrix));
				projection->As<floatM44>().SetValue(projectionMatrix);

				EffectParameterSP const& linkedListNodePool = listEffect->GetParameterByName("nodePool");
				linkedListNodePool->As<TextureImageSP>().SetValue(nodePools[i]->GetImage_TEMP(0, nodePools[i]->GetFormat()));
				EffectParameterSP const& headPointer = listEffect->GetParameterByName("header");
				headPointer->As<TextureImageSP>().SetValue(headPointers[i]->GetImage_TEMP(0, headPointers[i]->GetFormat()));
				EffectParameterSP const& atomicCounter = listEffect->GetParameterByName("0");
				ShaderResourceBufferSP atomicBuffer = atomicCounter->As<ShaderResourceBufferSP>().GetValue();
				atomicBuffer->SetBuffer(atomicCounterBuffers[i]);
				{
					ShaderResourceBuffer::BufferVariableSetter setter = atomicBuffer->GetVariableSetter();
					std::pair<bool, ShaderResourceBuffer::BufferVariableSetter::Setter> nodeCounterSetter = setter.GetSetter("nodeCounter");
					assert(nodeCounterSetter.first);
					nodeCounterSetter.second.SetValue(setter, 1u);
				}


				RenderingTechniqueSP const& rasterizeTechnique = listEffect->GetTechnique(0);
				RenderingPassSP rasterizePass = rasterizeTechnique->GetPass(0);
				for (auto& renderablePack : allRenderableNeedToRender)
				{
					Renderable& ownerRenderable = *renderablePack.renderable;
					RenderingLayoutSP const& layout = renderablePack.layout;

					floatM44 const& modelMatrix = ownerRenderable.GetOwnerSceneObject()->GetComponent<Transformation>()->GetWorldMatrix();
					EffectParameterSP const& model = listEffect->GetParameterByName(GetUniformString(DefinedUniform::ModelMatrix));
					model->As<floatM44>().SetValue(modelMatrix);

					rasterizePass->Use();
					layout->BindToProgram(rasterizePass->GetProgram());
					layout->Draw();
					layout->Unbind();
				}
			}


		}

		void BuildIntermediateVoxelVolume()
		{
			listBuildingViewport->Bind(0, 0);

			{ // clear intermediateVoxelVolume texture
				uint32 glIntermediateClearVoxelVolume = intermediateClearVoxelVolume->GetID();
				std::shared_ptr<Texture3D> intermediateVoxelVolumeAs3D = CheckedSPCast<Texture3D>(intermediateVoxelVolume);
				intermediateVoxelVolumeAs3D->Bind(0);
				GLTextureFormat glFormat = GLTextureFormatFromTexelFormat(intermediateVoxelVolumeAs3D->GetDescription().GetFormat());
				gl::BindBuffer(gl::GL_PIXEL_UNPACK_BUFFER, glIntermediateClearVoxelVolume);
				gl::TexSubImage3D(gl::GL_TEXTURE_3D, 0, 0, 0, 0, intermediateVoxelVolumeAs3D->GetDescription().GetSizes()[0], intermediateVoxelVolumeAs3D->GetDescription().GetSizes()[1], intermediateVoxelVolumeAs3D->GetDescription().GetSizes()[2],
					glFormat.glSourceFormat, glFormat.glTextureElementType, nullptr);
				gl::BindBuffer(gl::GL_PIXEL_UNPACK_BUFFER, 0);
			}
			RenderingEffectSP voxelizeEffect = voxelizationEffect[1];

			gl::MemoryBarrier(gl::GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			for (uint32 i = 0; i < 3; ++i) // TODO
			{
				EffectParameterSP const& linkedListNodePool = voxelizeEffect->GetParameterByName("nodePool");
				linkedListNodePool->As<TextureImageSP>().SetValue(nodePools[i]->GetImage_TEMP(0, nodePools[i]->GetFormat()));
				EffectParameterSP const& headPointer = voxelizeEffect->GetParameterByName("header");
				headPointer->As<TextureImageSP>().SetValue(headPointers[i]->GetImage_TEMP(0, headPointers[i]->GetFormat()));
				EffectParameterSP const& volume = voxelizeEffect->GetParameterByName("intermediateVolume");
				volume->As<TextureImageSP>().SetValue(intermediateVoxelVolume->GetImage_TEMP(0, intermediateVoxelVolume->GetFormat()));
				EffectParameterSP const& axis = voxelizeEffect->GetParameterByName("axis");
				axis->As<int32>().SetValue(i);

				RenderingTechniqueSP const& technique = voxelizeEffect->GetTechnique(0);
				RenderingPassSP pass = technique->GetPass(0);

				RenderingLayoutSP const& layout = screenQuad;

				pass->Use();
				layout->BindToProgram(pass->GetProgram());
				layout->Draw();
				layout->Unbind();

			}
		}

		void BuildVoxelVolume()
		{
			voxelMergeViewport->Bind(0, 0);

			{ // clear voxelVolume texture
				uint32 glClearVoxelVolume = clearVoxelVolume->GetID();
				std::shared_ptr<Texture3D> voxelVolumeAs3D = CheckedSPCast<Texture3D>(voxelVolume);
				voxelVolumeAs3D->Bind(0);
				GLTextureFormat glFormat = GLTextureFormatFromTexelFormat(voxelVolumeAs3D->GetDescription().GetFormat());

				gl::BindBuffer(gl::GL_PIXEL_UNPACK_BUFFER, glClearVoxelVolume);
				gl::TexSubImage3D(gl::GL_TEXTURE_3D, 0, 0, 0, 0, voxelVolumeAs3D->GetDescription().GetSizes()[0], voxelVolumeAs3D->GetDescription().GetSizes()[1], voxelVolumeAs3D->GetDescription().GetSizes()[2],
					glFormat.glSourceFormat, glFormat.glTextureElementType, nullptr);
				gl::BindBuffer(gl::GL_PIXEL_UNPACK_BUFFER, 0);
			}
			RenderingEffectSP voxelMergeEffect = voxelizationEffect[2];

			gl::MemoryBarrier(gl::GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			EffectParameterSP const& intermediateVolume = voxelMergeEffect->GetParameterByName("intermediateVolume");
			intermediateVolume->As<TextureImageSP>().SetValue(intermediateVoxelVolume->GetImage_TEMP(0, intermediateVoxelVolume->GetFormat()));
			EffectParameterSP const& volume = voxelMergeEffect->GetParameterByName("volume");
			volume->As<TextureImageSP>().SetValue(voxelVolume->GetImage_TEMP(0, voxelVolume->GetFormat()));

			RenderingTechniqueSP const& technique = voxelMergeEffect->GetTechnique(0);
			RenderingPassSP pass = technique->GetPass(0);

			RenderingLayoutSP const& layout = screenQuad;


			pass->Use();
			layout->BindToProgram(pass->GetProgram());
			layout->Draw();
			layout->Unbind();

			voxelVolume->RecreateMipmap();
		}

		void ConeTracing()
		{
			//voxelVolumeToTrace.first = texture3DToTest;
			voxelVolumeToTrace.first = voxelVolume;
			

			CameraSP camera = viewCameraObject->GetComponent<Camera>();
			Size<uint32> windowSize = XREXContext::GetInstance().GetMainWindow().GetClientRegionSize();
			camera->GetViewport()->Bind(windowSize.x, windowSize.y);

			// reset all masks, TODO ClearState
			BlendState blendState;
			DepthStencilState depthStencilState;
			BlendStateObjectSP bso = XREXContext::GetInstance().GetRenderingFactory().CreateBlendStateObject(blendState);
			DepthStencilStateObjectSP dsso = XREXContext::GetInstance().GetRenderingFactory().CreateDepthStencilStateObject(depthStencilState);
			bso->Bind(Color(0, 0, 0, 1));
			dsso->Bind(0, 0);

			Color const& backgroundColor = camera->GetBackgroundColor();
			gl::ClearColor(backgroundColor.R(), backgroundColor.G(), backgroundColor.B(), backgroundColor.A());
			gl::Clear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT | gl::GL_STENCIL_BUFFER_BIT);

			floatM44 viewMatrix = camera->GetViewMatrix();
			floatM44 projectionMatrix = camera->GetProjectionMatrix();

			TransformationSP transformation = coneTracingProxyCube->GetComponent<Transformation>();

			std::vector<Renderable::RenderablePack> allRenderableNeedToRender = coneTracingProxyCube->GetComponent<Renderable>()->GetRenderablePack(nullptr);

			gl::MemoryBarrier(gl::GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			for (auto& renderable : allRenderableNeedToRender)
			{
				Renderable& ownerRenderable = *renderable.renderable;
				RenderingTechniqueSP const& technique = renderable.technique;
				RenderingLayoutSP const& layout = renderable.layout;
				MaterialSP const& material = renderable.material;

				RenderingEffectSP coneTracingEffect = technique->GetEffect();

				EffectParameterSP const& cameraPosition = coneTracingEffect->GetParameterByName(GetUniformString(DefinedUniform::CameraPosition));
				cameraPosition->As<floatV3>().SetValue(viewCameraObject->GetComponent<Transformation>()->GetWorldPosition());
				EffectParameterSP const& view = coneTracingEffect->GetParameterByName(GetUniformString(DefinedUniform::ViewMatrix));
				view->As<floatM44>().SetValue(viewMatrix);
				EffectParameterSP const& projection = coneTracingEffect->GetParameterByName(GetUniformString(DefinedUniform::ProjectionMatrix));
				projection->As<floatM44>().SetValue(projectionMatrix);
				floatM44 const& modelMatrix = transformation->GetWorldMatrix();
				EffectParameterSP const& model = coneTracingEffect->GetParameterByName(GetUniformString(DefinedUniform::ModelMatrix));
				model->As<floatM44>().SetValue(modelMatrix);

				material->SetParameter("voxels", voxelVolumeToTrace);
				material->BindToEffect(coneTracingEffect);
				material->SetAllEffectParameterValues();

				RenderingPassSP pass = technique->GetPass(0);
				
				pass->Use();
				layout->BindToProgram(pass->GetProgram());
				layout->Draw();
				layout->Unbind();
			}

		}

		void Logic(double currentTime, double deltaTime)
		{
			auto& transformation = viewCameraObject->GetComponent<Transformation>();
			floatV3 const& position = transformation->GetWorldPosition();
			floatV3 to = TransformDirection(transformation->GetWorldMatrix(), transformation->GetModelFrontDirection());
			floatV3 up = TransformDirection(transformation->GetWorldMatrix(), transformation->GetModelUpDirection());

			Ray ray = viewCameraObject->GetComponent<Camera>()->GetViewRay(floatV2(0, 0), Camera::ViewportOrigin::ViewportCenter);

			std::wstringstream wss;
			wss << "position: (" << position.X() << ", " << position.Y() << ", " << position.Z() << "), ";
			wss << "direction: (" << to.X() << ", " << to.Y() << ", " << to.Z() << "), ";
			wss << "ray: (" << ray.GetDirection().X() << ", " << ray.GetDirection().Y() << ", " << ray.GetDirection().Z() << "), ";
			//wss << "up: (" << up.X() << ", " << up.Y() << ", " << up.Z() << "), ";

			XREXContext::GetInstance().GetMainWindow().SetTitleText(wss.str());
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
		std::shared_ptr<VoxelizationAndRenderProcess> renderingProcess = MakeSP<VoxelizationAndRenderProcess>(floatV3(0, 32.f, 0), 64.f, 128);
		XREXContext::GetInstance().GetRenderingEngine().SetRenderingProcess(renderingProcess);
		function<bool(double current, double delta)> l = [renderingProcess] (double current, double delta)
		{
			//renderingProcess->Logic(current, delta);
			return true;
		};
		XREXContext::GetInstance().SetLogicFunction(l);
		SceneObjectSP sceneObject = LoadTeapot();
		assert(sceneObject);
		XREXContext::GetInstance().GetScene()->AddObject(sceneObject);

		struct TeapotController
			: public InputHandler
		{
			enum Semantic
			{
				NZ,
				PZ,
				NY,
				PY,
				NX,
				PX,
			};
			static ActionMap GenerateActionMap()
			{
				ActionMap actions;
				actions.Set(InputCenter::InputSemantic::K_LeftArrow, Semantic::PX);
				actions.Set(InputCenter::InputSemantic::K_RightArrow, Semantic::NX);
				actions.Set(InputCenter::InputSemantic::K_UpArrow, Semantic::PZ);
				actions.Set(InputCenter::InputSemantic::K_DownArrow, Semantic::NZ);
				actions.Set(InputCenter::InputSemantic::K_PageUp, Semantic::PY);
				actions.Set(InputCenter::InputSemantic::K_PageDown, Semantic::NY);
				return actions;
			}
			TeapotController(SceneObjectSP& teapot)
				: InputHandler(GenerateActionMap()), teapot(teapot)
			{

			}
			virtual std::pair<bool, function<void()>> GenerateAction(InputCenter::InputEvent const& inputEvent) override
			{
				float x = static_cast<float>((inputEvent.mappedSemantic == Semantic::NX ? -1 : 0) + (inputEvent.mappedSemantic == Semantic::PX ? 1 : 0));
				float y = static_cast<float>((inputEvent.mappedSemantic == Semantic::NY ? -1 : 0) + (inputEvent.mappedSemantic == Semantic::PY ? 1 : 0));
				float z = static_cast<float>((inputEvent.mappedSemantic == Semantic::NZ ? -1 : 0) + (inputEvent.mappedSemantic == Semantic::PZ ? 1 : 0));
				teapot->GetComponent<Transformation>()->Translate(floatV3(x, y, z));
				return std::make_pair(false, function<void()>());
			}
			SceneObjectSP teapot;
		};
		std::shared_ptr<TeapotController> c = MakeSP<TeapotController>(sceneObject);
		XREXContext::GetInstance().GetInputCenter().AddInputHandler(c);


// 		SceneObjectSP viewCameraObject = MakeCamera();
// 		SceneObjectSP coneTracingProxyCube = MakeConeTracingProxyCube(CheckedSPCast<PerspectiveCamera>(viewCameraObject->GetComponent<Camera>()), floatV3::Zero, 2.f);
// 
// 		auto voxelVolumeToTrace = std::make_pair(MakeTest3DTexture(), CreateConeTracingSampler());
// 		MeshSP cube = CheckedSPCast<Mesh>(coneTracingProxyCube->GetComponent<Renderable>());
// 		for (auto& sub : cube->GetAllSubMeshes())
// 		{
// 			sub->GetMaterial()->SetParameter("voxels", voxelVolumeToTrace);
// 		}
// 		XREXContext::GetInstance().GetScene()->AddObject(coneTracingProxyCube);
// 		XREXContext::GetInstance().GetScene()->AddObject(viewCameraObject);
	}
}



VoxelTest::VoxelTest()
{
	Settings settings("../../");
	settings.windowTitle = L"GL4 voxelization";

	settings.renderingSettings.colorBits = 32;
	settings.renderingSettings.depthBits = 24;
	settings.renderingSettings.stencilBits = 8;
	settings.renderingSettings.sampleCount = 4;

	settings.renderingSettings.left = 300;
	settings.renderingSettings.top = 200;
	settings.renderingSettings.width = 1024;
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
