#include "XREXAll.hpp"
#include "VoxelTest.h"

#include "Rendering/WorkLauncher.hpp"
#include "Rendering/GL/GLUtil.hpp"
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
		Size<uint32, 3> dim = Size<uint32, 3>(size, size, size);
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
		Size<uint32, 3> dim(size, size, size);
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
		Size<uint32, 3> dim(size, size, size);
		Texture2D::DataDescription<3> desc(TexelFormat::RGBA8, dim);

		TextureSP voxelVolume = XREXContext::GetInstance().GetRenderingFactory().CreateTexture3D(desc, true);
		return voxelVolume;
	}

	GraphicsBufferSP MakeClearVoxelVolume(uint32 size)
	{
		GraphicsBufferSP voxelVolume = XREXContext::GetInstance().GetRenderingFactory().CreateGraphicsBuffer(GraphicsBuffer::Usage::StaticDraw, size * size * size * GetTexelSizeInBytes(TexelFormat::RGBA8));
		voxelVolume->Clear(0u);
		return voxelVolume;
	}

	struct UsedTechniques
	{
		RenderingTechniqueSP listBuild;
		RenderingTechniqueSP listSort;
		RenderingTechniqueSP volumeBuild;
		RenderingTechniqueSP anisotropicVolumeBuild;
		RenderingTechniqueSP volumeMerge;
	};

	UsedTechniques MakeVoxelizationTechnique()
	{

		RenderingTechniqueSP listTechnique = [] ()
		{
			string shaderFile = "../../Voxelization/Shaders/ListGeneration.glsl";
			shared_ptr<string> shaderString = XREXContext::GetInstance().GetResourceLoader().LoadString(shaderFile);
			if (!shaderString)
			{
				XREXContext::GetInstance().GetLogger().LogLine("file not found. file: " + shaderFile);
			}

			TechniqueBuilderSP builder = MakeSP<TechniqueBuilder>("voxelization list technique");
			builder->AddCommonCode(shaderString);
			builder->SetStageCode(ShaderObject::ShaderType::VertexShader, MakeSP<string>());
			builder->SetStageCode(ShaderObject::ShaderType::FragmentShader, MakeSP<string>());

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
			builder->SetRasterizerState(resterizerState);
			builder->SetDepthStencilState(depthStencilState);
			builder->SetBlendState(blendState);

			builder->SpecifyFragmentOutput(XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer()->GetLayoutDescription());

			builder->SpecifyImageFormat("heads", TexelFormat::R32UI, AccessType::ReadWrite);
			builder->SpecifyImageFormat("nodePool", TexelFormat::RGBA32UI, AccessType::WriteOnly);

			return builder->GetRenderingTechnique();
		} ();
		listTechnique->SetFrameBuffer(XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer());

		RenderingTechniqueSP sortTechnique = [] ()
		{

			string shaderFile = "../../Voxelization/Shaders/ListSort.glsl";
			shared_ptr<string> shaderString = XREXContext::GetInstance().GetResourceLoader().LoadString(shaderFile);
			if (!shaderString)
			{
				XREXContext::GetInstance().GetLogger().LogLine("file not found. file: " + shaderFile);
			}
			TechniqueBuilderSP builder = MakeSP<TechniqueBuilder>("list sort technique");
			builder->AddCommonCode(shaderString);
			builder->SetStageCode(ShaderObject::ShaderType::VertexShader, MakeSP<string>());
			builder->SetStageCode(ShaderObject::ShaderType::FragmentShader, MakeSP<string>());

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
			builder->SetRasterizerState(resterizerState);
			builder->SetDepthStencilState(depthStencilState);
			builder->SetBlendState(blendState);

			builder->SpecifyFragmentOutput(XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer()->GetLayoutDescription());
			return builder->GetRenderingTechnique();
		} ();
		sortTechnique->SetFrameBuffer(XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer());


		RenderingTechniqueSP generationTechnique = [] ()
		{
			string shaderFile = "../../Voxelization/Shaders/VoxelGeneration.glsl";
			shared_ptr<string> shaderString = XREXContext::GetInstance().GetResourceLoader().LoadString(shaderFile);
			if (!shaderString)
			{
				XREXContext::GetInstance().GetLogger().LogLine("file not found. file: " + shaderFile);
			}
			TechniqueBuilderSP builder = MakeSP<TechniqueBuilder>("voxelization generation technique");
			builder->AddCommonCode(shaderString);
			builder->SetStageCode(ShaderObject::ShaderType::VertexShader, MakeSP<string>());
			builder->SetStageCode(ShaderObject::ShaderType::FragmentShader, MakeSP<string>());

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
			builder->SetRasterizerState(resterizerState);
			builder->SetDepthStencilState(depthStencilState);
			builder->SetBlendState(blendState);

			builder->SpecifyFragmentOutput(XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer()->GetLayoutDescription());

			builder->SpecifyImageFormat("heads", TexelFormat::R32UI, AccessType::ReadOnly);
			builder->SpecifyImageFormat("nodePool", TexelFormat::RGBA32UI, AccessType::ReadOnly);
			builder->SpecifyImageFormat("volume", TexelFormat::RGBA8, AccessType::WriteOnly);

			return builder->GetRenderingTechnique();
		} ();
		generationTechnique->SetFrameBuffer(XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer());

		RenderingTechniqueSP anisotropicGenerationTechnique = [] ()
		{
			string shaderFile = "../../Voxelization/Shaders/AnisotropicVoxelGeneration.glsl";
			shared_ptr<string> shaderString = XREXContext::GetInstance().GetResourceLoader().LoadString(shaderFile);
			if (!shaderString)
			{
				XREXContext::GetInstance().GetLogger().LogLine("file not found. file: " + shaderFile);
			}
			TechniqueBuilderSP builder = MakeSP<TechniqueBuilder>("voxelization anisotropic generation technique");
			builder->AddCommonCode(shaderString);
			builder->SetStageCode(ShaderObject::ShaderType::VertexShader, MakeSP<string>());
			builder->SetStageCode(ShaderObject::ShaderType::FragmentShader, MakeSP<string>());

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
			builder->SetRasterizerState(resterizerState);
			builder->SetDepthStencilState(depthStencilState);
			builder->SetBlendState(blendState);

			builder->SpecifyFragmentOutput(XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer()->GetLayoutDescription());

			builder->SpecifyImageFormat("heads", TexelFormat::R32UI, AccessType::ReadOnly);
			builder->SpecifyImageFormat("nodePool", TexelFormat::RGBA32UI, AccessType::ReadOnly);
			builder->SpecifyImageFormat("volume", TexelFormat::RGBA8, AccessType::WriteOnly);

			return builder->GetRenderingTechnique();

		} ();
		anisotropicGenerationTechnique->SetFrameBuffer(XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer());


		RenderingTechniqueSP mergeTechnique = [] ()
		{
			string shaderFile = "../../Voxelization/Shaders/VoxelMerge.glsl";
			shared_ptr<string> shaderString = XREXContext::GetInstance().GetResourceLoader().LoadString(shaderFile);
			if (!shaderString)
			{
				XREXContext::GetInstance().GetLogger().LogLine("file not found. file: " + shaderFile);
			}
			TechniqueBuilderSP builder = MakeSP<TechniqueBuilder>("voxelization merge technique");
			builder->AddCommonCode(shaderString);
			builder->SetStageCode(ShaderObject::ShaderType::VertexShader, MakeSP<string>());
			builder->SetStageCode(ShaderObject::ShaderType::FragmentShader, MakeSP<string>());

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
			builder->SetRasterizerState(resterizerState);
			builder->SetDepthStencilState(depthStencilState);
			builder->SetBlendState(blendState);

			builder->SpecifyFragmentOutput(XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer()->GetLayoutDescription());

			builder->SpecifyImageFormat("intermediateVolume", TexelFormat::R32UI, AccessType::ReadOnly);
			builder->SpecifyImageFormat("volume", TexelFormat::RGBA32F, AccessType::WriteOnly);

			return builder->GetRenderingTechnique();

		} ();
		mergeTechnique->SetFrameBuffer(XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer());
		
		UsedTechniques effects = {listTechnique, sortTechnique, generationTechnique, anisotropicGenerationTechnique, mergeTechnique};
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

	RenderingTechniqueSP MakeConeTracingTechnique()
	{
		string shaderFile = "../../Voxelization/Shaders/ConeTracing.glsl";
		shared_ptr<string> shaderString = XREXContext::GetInstance().GetResourceLoader().LoadString(shaderFile);
		if (!shaderString)
		{
			XREXContext::GetInstance().GetLogger().LogLine("file not found. file: " + shaderFile);
		}
		TechniqueBuilderSP builder = MakeSP<TechniqueBuilder>("cone tracing technique");
		builder->AddCommonCode(shaderString);
		builder->SetStageCode(ShaderObject::ShaderType::VertexShader, MakeSP<string>());
		builder->SetStageCode(ShaderObject::ShaderType::FragmentShader, MakeSP<string>());

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
		builder->SetRasterizerState(resterizerState);
		builder->SetDepthStencilState(depthStencilState);
		builder->SetBlendState(blendState);
		SamplerState samplerState;
		samplerState.borderColor = Color(0, 0, 0, 0);
		samplerState.addressingModeR = SamplerState::TextureAddressingMode::ClampToBorder;
		samplerState.addressingModeS = SamplerState::TextureAddressingMode::ClampToBorder;
		samplerState.addressingModeT = SamplerState::TextureAddressingMode::ClampToBorder;
		samplerState.magFilterOperation = SamplerState::TextureFilterOperation::Linear;
		samplerState.minFilterOperation = SamplerState::TextureFilterOperation::LinearMipmapLinear;
		builder->SetSamplerState("cone tracing sampler", samplerState);
		builder->SetSamplerChannelToSamplerStateMapping("voxels", "cone tracing sampler");

		builder->SpecifyFragmentOutput(XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer()->GetLayoutDescription());

		return builder->GetRenderingTechnique();
	}

	SceneObjectSP MakeConeTracingProxyCube(PerspectiveCameraSP const& camera, floatV3 const& cubePosition, float cubeHalfSize)
	{

		SceneObjectSP cubeObject = MakeSP<SceneObject>("cube object");
		MeshSP cube = MakeCube(cubeHalfSize);

		RenderingTechniqueSP coneTracingTechnique = MakeConeTracingTechnique();
		coneTracingTechnique->SetFrameBuffer(XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer());
		MaterialSP material = MakeSP<Material>("tracing technique parameters");
		material->SetParameter("voxelVolumeCenter", cubePosition);
		material->SetParameter("voxelVolumeHalfSize", cubeHalfSize);
		material->SetParameter("aperture", camera->GetFieldOfView() / XREXContext::GetInstance().GetMainWindow().GetClientRegionSize().Y());



		for (auto& sub : cube->GetAllSubMeshes())
		{
			sub->SetTechnique(coneTracingTechnique);
			sub->SetMaterial(material);
		}
		cubeObject->SetComponent(cube);
		TransformationSP trans = cubeObject->GetComponent<Transformation>();
		trans->SetPosition(cubePosition);

		return cubeObject;
	}

	SceneObjectSP MakeCamera(float cameraSpeedScaler)
	{
		SceneObjectSP cameraObject = MakeSP<SceneObject>("camera");
		Settings const& settings = XREXContext::GetInstance().GetSettings();
		CameraSP camera = MakeSP<PerspectiveCamera>(PI / 4, static_cast<float>(settings.renderingSettings.width) / settings.renderingSettings.height, 0.01f, 100000.0f);
		cameraObject->SetComponent(camera);

		auto cc = MakeSP<FirstPersonCameraController>(cameraSpeedScaler);
		cc->AttachToCamera(cameraObject);
		XREXContext::GetInstance().GetInputCenter().AddInputHandler(cc);

		return cameraObject;
	}


	struct RenderToTextureProcess
		: RenderingProcess
	{

		UsedTechniques voxelizationTechnique;
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
		//GraphicsBufferSP intermediateClearVoxelVolume;
		GraphicsBufferSP clearVoxelVolume;

		RenderingLayoutSP screenQuad;

		SceneObjectSP coneTracingProxyCube;
		SceneObjectSP viewCameraObject;

		TextureSP voxelVolumeToTrace;

		TextureSP texture3DToTest;

		uint32 frame;

		bool anisotropic;
		

		RenderToTextureProcess(floatV3 const& sceneCenter, float sceneHalfSize, uint32 voxelVolumeResolution, bool anisotropic, float cameraSpeedScaler)
			: frame(0), anisotropic(anisotropic)
		{
			this->sceneCenter = sceneCenter;
			this->sceneHalfSize = sceneHalfSize;

			this->voxelVolumeResolution = voxelVolumeResolution;
			this->intermediateVoxelVolumeResolution = voxelVolumeResolution * (anisotropic ? 2 : 1);

			CreateVoxelizationObjects();

			viewCameraObject = MakeCamera(cameraSpeedScaler);
			coneTracingProxyCube = MakeConeTracingProxyCube(CheckedSPCast<PerspectiveCamera>(viewCameraObject->GetComponent<Camera>()), sceneCenter, sceneHalfSize);

			// texture3DToTest = MakeTest3DTexture();

			screenQuad = MakeScreenQuad();
		}

		void CreateVoxelizationObjects()
		{
			listBuildingViewport = XREXContext::GetInstance().GetRenderingFactory().CreateViewport(0, 0, 0, intermediateVoxelVolumeResolution, intermediateVoxelVolumeResolution);
			voxelMergeViewport = XREXContext::GetInstance().GetRenderingFactory().CreateViewport(0, 0, 0, voxelVolumeResolution, voxelVolumeResolution);
			voxelizationTechnique = MakeVoxelizationTechnique();
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
			// from +x, up is +z
			TransformationSP const& cameraTransformation0 = voxelizationCameras[0]->GetComponent<Transformation>();
			cameraTransformation0->Translate(sceneCenter + floatV3(sceneHalfSize, 0, 0));
			cameraTransformation0->FaceToDirection(floatV3(-1, 0, 0), floatV3(0, 0, 1));
			// from +y, up is +x
			TransformationSP const& cameraTransformation1 = voxelizationCameras[1]->GetComponent<Transformation>();
			cameraTransformation1->Translate(sceneCenter + floatV3(0, sceneHalfSize, 0));
			cameraTransformation1->FaceToDirection(floatV3(0, -1, 0), floatV3(1, 0, 0));
			// from +z, up is +y
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
				Size<uint32, 2> dim(intermediateVoxelVolumeResolution, intermediateVoxelVolumeResolution);
				Texture::DataDescription<2> desc(TexelFormat::R32UI, dim);

				TextureSP headPointer = XREXContext::GetInstance().GetRenderingFactory().CreateTexture2D(desc, false);
				headPointers[i] = headPointer;

				TexelFormat linkedListNodeFormat = TexelFormat::RGBA32UI;
				uint32 poolSize = intermediateVoxelVolumeResolution * intermediateVoxelVolumeResolution * GetTexelSizeInBytes(linkedListNodeFormat) * 8;
				GraphicsBufferSP buffer = XREXContext::GetInstance().GetRenderingFactory().CreateGraphicsBuffer(GraphicsBuffer::Usage::StreamCopy, poolSize, BufferView::BufferType::Texture);
				TextureSP linkedListNodePool = XREXContext::GetInstance().GetRenderingFactory().CreateTextureBuffer(buffer, linkedListNodeFormat);
				nodePools[i] = linkedListNodePool;
				GraphicsBufferSP atomicBuffer = XREXContext::GetInstance().GetRenderingFactory().CreateGraphicsBuffer(GraphicsBuffer::Usage::StreamCopy, 4, BufferView::BufferType::AtomicCounter);
				atomicCounterBuffers[i] = atomicBuffer;
			}

			intermediateVoxelVolume = MakeIntermediateVoxelVolume(voxelVolumeResolution);
			voxelVolume = MakeVoxelVolume(voxelVolumeResolution);

			//intermediateClearVoxelVolume = MakeIntermediateClearVoxelVolume(voxelVolumeResolution);
			clearVoxelVolume = MakeClearVoxelVolume(voxelVolumeResolution);

		}

		virtual void RenderScene(SceneSP const& scene) override
		{
			//XREXContext::GetInstance().GetLogger().LogLine("frame: " + std::to_string(frame++));
			std::vector<SceneObjectSP> sceneObjects = scene->GetRenderableQueue(nullptr);


			RenderablePackCollector collector;
			for (SceneObjectSP sceneObject : sceneObjects)
			{
				TransformationSP transformation = sceneObject->GetComponent<Transformation>();
				RenderableSP renderable = sceneObject->GetComponent<Renderable>();
				assert(renderable != nullptr);
				assert(renderable->IsVisible());

				renderable->GetSmallRenderablePack(collector, nullptr);
			}
			std::vector<Renderable::SmallRenderablePack> allRenderableNeedToRender = collector.ExtractSmallRenderablePack();

			BuildFragmentLists(allRenderableNeedToRender);
// 			for (uint32 i = 0; i < 3; ++i)
// 			{
// 				GraphicsBuffer::BufferMapper mapper = atomicCounterBuffers[i]->GetMapper(AccessType::ReadOnly);
// 				uint32 count = mapper.GetPointer<uint32>()[0];
// 				XREXContext::GetInstance().GetLogger().LogLine("count: " + std::to_string(count));
// 			}

			//SortFragmentLists();

			BuildVoxelVolume();
			//MergeVoxelVolume();

			ConeTracing();
		}

		void BuildFragmentLists(std::vector<Renderable::SmallRenderablePack> const& allRenderableNeedToRender)
		{
			RenderingTechniqueSP listTechnique = voxelizationTechnique.listBuild;
			IndexedDrawer drawer;

			for (uint32 i = 0; i < 3; ++i)
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
				TechniqueParameterSP const& view = listTechnique->GetParameterByName(GetUniformString(DefinedUniform::ViewMatrix));
				view->As<floatM44>().SetValue(viewMatrix);
				TechniqueParameterSP const& projection = listTechnique->GetParameterByName(GetUniformString(DefinedUniform::ProjectionMatrix));
				projection->As<floatM44>().SetValue(projectionMatrix);

				TechniqueParameterSP const& linkedListNodePool = listTechnique->GetParameterByName("nodePool");
				linkedListNodePool->As<TextureImageSP>().SetValue(CheckedSPCast<TextureBuffer>(nodePools[i])->GetImage());
				TechniqueParameterSP const& headPointer = listTechnique->GetParameterByName("heads");
				headPointer->As<TextureImageSP>().SetValue(CheckedSPCast<Texture2D>(headPointers[i])->GetImage(0));
				TechniqueParameterSP const& atomicCounter = listTechnique->GetParameterByName("0");
				ShaderResourceBufferSP atomicBuffer = atomicCounter->As<ShaderResourceBufferSP>().GetValue();
				atomicBuffer->SetBuffer(atomicCounterBuffers[i]);
				atomicCounterBuffers[i]->Clear(1u);
// 				{
// 					ShaderResourceBuffer::BufferVariableSetter setter = atomicBuffer->GetVariableSetter();
// 					std::pair<bool, ShaderResourceBuffer::BufferVariableSetter::Setter> nodeCounterSetter = setter.GetSetter("nodeCounter");
// 					assert(nodeCounterSetter.first);
// 					nodeCounterSetter.second.SetValue(setter, 1u);
// 				}
				TechniqueParameterSP const& halfSize = listTechnique->GetParameterByName("voxelVolumeHalfSize");
				halfSize->As<float>().SetValue(sceneHalfSize);
				TechniqueParameterSP const& voxelVolumeCenter = listTechnique->GetParameterByName("voxelVolumeCenter");
				voxelVolumeCenter->As<floatV3>().SetValue(sceneCenter);
				TechniqueParameterSP const& axis = listTechnique->GetParameterByName("axis");
				if (axis)
				{
					axis->As<int32>().SetValue(i);
				}

				int objectID = 0;

				listTechnique->Use();

				for (auto& renderablePack : allRenderableNeedToRender)
				{
					Renderable& ownerRenderable = *renderablePack.renderable;
					RenderingLayoutSP const& layout = renderablePack.layout;

					floatM44 const& modelMatrix = ownerRenderable.GetOwnerSceneObject()->GetComponent<Transformation>()->GetWorldMatrix();
					TechniqueParameterSP const& model = listTechnique->GetParameterByName(GetUniformString(DefinedUniform::ModelMatrix));
					model->As<floatM44>().SetValue(modelMatrix);
					TechniqueParameterSP const& object = listTechnique->GetParameterByName("objectID");
					object->As<int32>().SetValue(objectID);

					listTechnique->SetupAllResources();
					LayoutAndProgramConnectorSP connector = XREXContext::GetInstance().GetRenderingFactory().GetConnector(layout, listTechnique);
					connector->Bind();
					drawer.SetRenderingLayout(layout);
					drawer.CoreLaunch();
					connector->Unbind();

				}
			}

		}

		void SortFragmentLists()
		{
			listBuildingViewport->Bind(0, 0);

			RenderingTechniqueSP sortTechnique = voxelizationTechnique.listSort;

			gl::MemoryBarrier(gl::GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			IndexedDrawer drawer;
			for (uint32 i = 0; i < 3; ++i)
			{
				TechniqueParameterSP const& linkedListNodePool = sortTechnique->GetParameterByName("nodePool");
				if (linkedListNodePool)
				{
					linkedListNodePool->As<TextureImageSP>().SetValue(CheckedSPCast<TextureBuffer>(nodePools[i])->GetImage());
				}
				TechniqueParameterSP const& headPointer = sortTechnique->GetParameterByName("heads");
				if (headPointer)
				{
					headPointer->As<TextureImageSP>().SetValue(CheckedSPCast<Texture2D>(headPointers[i])->GetImage(0));
				}

				RenderingLayoutSP const& layout = screenQuad;

				LayoutAndProgramConnectorSP connector = XREXContext::GetInstance().GetRenderingFactory().GetConnector(layout, sortTechnique);
				drawer.SetTechnique(sortTechnique);
				drawer.SetLayoutAndProgramConnector(connector);
				drawer.SetRenderingLayout(layout);
				drawer.Launch();
			}
		}

		void BuildVoxelVolume()
		{
			//listBuildingViewport->Bind(0, 0);
			voxelMergeViewport->Bind(0, 0);


			RenderingTechniqueSP voxelizeTechnique = voxelizationTechnique.volumeBuild;

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

			gl::MemoryBarrier(gl::GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			IndexedDrawer drawer;
			for (uint32 i = 0; i < 3; ++i)
			{
// 				if (i == 1)
// 				{
// 					continue;
// 				}
				{
					BlendState blendState;
					DepthStencilState depthStencilState;
					BlendStateObjectSP bso = XREXContext::GetInstance().GetRenderingFactory().CreateBlendStateObject(blendState);
					DepthStencilStateObjectSP dsso = XREXContext::GetInstance().GetRenderingFactory().CreateDepthStencilStateObject(depthStencilState);
					bso->Bind(Color(0, 0, 0, 1));
					dsso->Bind(0, 0);
				}

				TechniqueParameterSP const& linkedListNodePool = voxelizeTechnique->GetParameterByName("nodePool");
				linkedListNodePool->As<TextureImageSP>().SetValue(CheckedSPCast<TextureBuffer>(nodePools[i])->GetImage());
				TechniqueParameterSP const& headPointer = voxelizeTechnique->GetParameterByName("heads");
				headPointer->As<TextureImageSP>().SetValue(CheckedSPCast<Texture2D>(headPointers[i])->GetImage(0));
				TechniqueParameterSP const& volume = voxelizeTechnique->GetParameterByName("volume");
				if (voxelVolume)
				{
					volume->As<TextureImageSP>().SetValue(CheckedSPCast<Texture3D>(voxelVolume)->GetImage(0));
				}
				TechniqueParameterSP const& axis = voxelizeTechnique->GetParameterByName("axis");
				axis->As<int32>().SetValue(i);


				RenderingLayoutSP const& layout = screenQuad;

				LayoutAndProgramConnectorSP connector = XREXContext::GetInstance().GetRenderingFactory().GetConnector(layout, voxelizeTechnique);
				drawer.SetTechnique(voxelizeTechnique);
				drawer.SetLayoutAndProgramConnector(connector);
				drawer.SetRenderingLayout(layout);
				drawer.Launch();
			}
		}

		void BuildAnisotropicVoxelVolume()
		{
			//listBuildingViewport->Bind(0, 0);
			voxelMergeViewport->Bind(0, 0);


			RenderingTechniqueSP voxelizeTechnique = voxelizationTechnique.anisotropicVolumeBuild;

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

			gl::MemoryBarrier(gl::GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			IndexedDrawer drawer;
			for (uint32 i = 0; i < 3; ++i)
			{
				{
					BlendState blendState;
					DepthStencilState depthStencilState;
					BlendStateObjectSP bso = XREXContext::GetInstance().GetRenderingFactory().CreateBlendStateObject(blendState);
					DepthStencilStateObjectSP dsso = XREXContext::GetInstance().GetRenderingFactory().CreateDepthStencilStateObject(depthStencilState);
					bso->Bind(Color(0, 0, 0, 1));
					dsso->Bind(0, 0);
				}

				TechniqueParameterSP const& linkedListNodePool = voxelizeTechnique->GetParameterByName("nodePool");
				linkedListNodePool->As<TextureImageSP>().SetValue(CheckedSPCast<TextureBuffer>(nodePools[i])->GetImage());
				TechniqueParameterSP const& headPointer = voxelizeTechnique->GetParameterByName("heads");
				headPointer->As<TextureImageSP>().SetValue(CheckedSPCast<Texture2D>(headPointers[i])->GetImage(0));

				TechniqueParameterSP const& volume = voxelizeTechnique->GetParameterByName("volume");
				if (voxelVolume)
				{
					volume->As<TextureImageSP>().SetValue(CheckedSPCast<Texture3D>(voxelVolume)->GetImage(0));
				}
				TechniqueParameterSP const& axis = voxelizeTechnique->GetParameterByName("axis");
				axis->As<int32>().SetValue(i);

				RenderingLayoutSP const& layout = screenQuad;

				LayoutAndProgramConnectorSP connector = XREXContext::GetInstance().GetRenderingFactory().GetConnector(layout, voxelizeTechnique);
				drawer.SetTechnique(voxelizeTechnique);
				drawer.SetLayoutAndProgramConnector(connector);
				drawer.SetRenderingLayout(layout);
				drawer.Launch();
			}
		}

		void MergeVoxelVolume()
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
			RenderingTechniqueSP voxelMergeTechnique = voxelizationTechnique.volumeMerge;

			gl::MemoryBarrier(gl::GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			TechniqueParameterSP const& intermediateVolume = voxelMergeTechnique->GetParameterByName("intermediateVolume");
			intermediateVolume->As<TextureImageSP>().SetValue(CheckedSPCast<Texture3D>(intermediateVoxelVolume)->GetImage(0));
			TechniqueParameterSP const& volume = voxelMergeTechnique->GetParameterByName("volume");
			volume->As<TextureImageSP>().SetValue(CheckedSPCast<Texture3D>(voxelVolume)->GetImage(0));

			RenderingLayoutSP const& layout = screenQuad;

			voxelMergeTechnique->Use();
			//layout->BindToProgram(pass->GetProgram());
			//layout->Draw();
			gl::DispatchCompute(voxelVolumeResolution, voxelVolumeResolution, 1);
			//layout->Unbind();

			//voxelVolume->RecreateMipmap();
		}

		void ConeTracing()
		{
			//voxelVolumeToTrace.first = texture3DToTest;
			voxelVolumeToTrace = voxelVolume;
			

			CameraSP camera = viewCameraObject->GetComponent<Camera>();
			Size<uint32, 2> windowSize = XREXContext::GetInstance().GetMainWindow().GetClientRegionSize();
			camera->GetViewport()->Bind(windowSize.X(), windowSize.Y());

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

			RenderablePackCollector collector;
			coneTracingProxyCube->GetComponent<Renderable>()->GetRenderablePack(collector, nullptr);
			std::vector<Renderable::RenderablePack> allRenderableNeedToRender = collector.ExtractRenderablePacks();

			gl::MemoryBarrier(gl::GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			IndexedDrawer drawer;
			for (auto& renderable : allRenderableNeedToRender)
			{
				Renderable& ownerRenderable = *renderable.renderable;
				RenderingTechniqueSP const& technique = renderable.technique;
				RenderingLayoutSP const& layout = renderable.layout;
				MaterialSP const& material = renderable.material;

				TechniqueParameterSP const& cameraPosition = technique->GetParameterByName(GetUniformString(DefinedUniform::CameraPosition));
				cameraPosition->As<floatV3>().SetValue(viewCameraObject->GetComponent<Transformation>()->GetWorldPosition());
				TechniqueParameterSP const& view = technique->GetParameterByName(GetUniformString(DefinedUniform::ViewMatrix));
				view->As<floatM44>().SetValue(viewMatrix);
				TechniqueParameterSP const& projection = technique->GetParameterByName(GetUniformString(DefinedUniform::ProjectionMatrix));
				projection->As<floatM44>().SetValue(projectionMatrix);
				floatM44 const& modelMatrix = transformation->GetWorldMatrix();
				TechniqueParameterSP const& model = technique->GetParameterByName(GetUniformString(DefinedUniform::ModelMatrix));
				model->As<floatM44>().SetValue(modelMatrix);

				material->SetParameter("voxels", voxelVolumeToTrace);
				material->BindToTechnique(technique);
				material->SetAllTechniqueParameterValues();

				LayoutAndProgramConnectorSP connector = XREXContext::GetInstance().GetRenderingFactory().GetConnector(layout, technique);
				drawer.SetTechnique(technique);
				drawer.SetLayoutAndProgramConnector(connector);
				drawer.SetRenderingLayout(layout);
				drawer.Launch();
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


	SceneObjectSP LoadModel(std::string const& path)
	{
		MeshSP model = XREXContext::GetInstance().GetResourceManager().LoadModel(path)->Create();
		SceneObjectSP sceneObject = MakeSP<SceneObject>("scene object");
		sceneObject->SetComponent(model);
		return sceneObject;
	}


	void InitializeScene()
	{
		enum Scene
		{
			Teapot,
			Sponza,
			CrytekSponza,
			Conference,
			Dragon, // cannot work
			Buddha, // cannot work
			LostEmpire, // cannot work
			SponzaWithTeapots,
		} target = Scene::Sponza;

		floatV3 center;
		float halfSize;
		std::string filePath;

		switch (target)
		{
		case Scene::Teapot:
			center = floatV3(0, 32.f, 0);
			halfSize = 96;
			filePath = "Data/teapot/teapot.obj";
			break;
		case Scene::Sponza:
			center = floatV3(0, 5.f, 0);
			halfSize = 24;
			filePath = "Data/dabrovic-sponza/sponza.obj";
			break;
		case Scene::CrytekSponza:
			center = floatV3(0, 512.f, 0);
			halfSize = 1024;
			filePath = "Data/crytek-sponza/sponza.obj";
			break;
		case Scene::Conference:
			center = floatV3(0, 0, 0);
			halfSize = 1536;
			filePath = "Data/conference/conference.obj";
			break;
		case Scene::Dragon:
			center = floatV3(0, 0, 0);
			halfSize = 128;
			filePath = "Data/dragon/dragon.obj";
			break;
		case Scene::Buddha:
			center = floatV3(0, 0, 0);
			halfSize = 128;
			filePath = "Data/buddha/buddha.obj";
			break;
		case Scene::LostEmpire:
			center = floatV3(0, 0, 0);
			halfSize = 1024;
			filePath = "Data/lost_empire/lost_empire.obj";
			break;
		case Scene::SponzaWithTeapots:
			center = floatV3(0, 32.f, 0);
			halfSize = 128;
			break;
		default:
			assert(false);
			break;
		}

		std::shared_ptr<RenderToTextureProcess> renderingProcess = MakeSP<RenderToTextureProcess>(center, halfSize, 512, false, halfSize / 50);
		XREXContext::GetInstance().GetRenderingEngine().SetRenderingProcess(renderingProcess);
		function<bool(double current, double delta)> l = [renderingProcess] (double current, double delta)
		{
			//renderingProcess->Logic(current, delta);
			return true;
		};
		XREXContext::GetInstance().SetLogicFunction(l);

		SceneObjectSP sceneObject;

		if (target == Scene::SponzaWithTeapots)
		{
			//filePath = "Data/dabrovic-sponza/sponza.obj";
			filePath = "Data/teapot/teapot.obj";
			
			sceneObject = LoadModel(filePath);
			sceneObject->GetComponent<Transformation>()->SetScaling(2);
			assert(sceneObject);
			XREXContext::GetInstance().GetScene()->AddObject(sceneObject);

			filePath = "Data/teapot/teapot.obj";
			//SceneObjectSP innerSceneObject = LoadModel(filePath);
			SceneObjectSP innerSceneObject = MakeSP<SceneObject>("teapot");
			innerSceneObject->SetComponent(sceneObject->GetComponent<Renderable>()->ShallowClone());

			MeshSP mesh = CheckedSPCast<Mesh>(innerSceneObject->GetComponent<Renderable>());
			for (int32 i = 0; i < 2; ++i)
			{
				MeshSP cloned = mesh->GetShallowClone();
				SceneObjectSP clonedSceneObject = MakeSP<SceneObject>("so " + to_string(i));
				clonedSceneObject->SetComponent(cloned);
				TransformationSP transformation = clonedSceneObject->GetComponent<Transformation>();
				const float positionScaler = 32;
				floatV3 position((i & 1) * 2 * positionScaler - positionScaler, ((i & 2) >> 1) * 2 * positionScaler - positionScaler, ((i & 4) >> 2) * 2 * positionScaler - positionScaler);
				transformation->Translate(position + floatV3(0, positionScaler, 0));
				transformation->Rotate(0.2f * PI, position);
				transformation->SetParent(sceneObject->GetComponent<Transformation>());
				XREXContext::GetInstance().GetScene()->AddObject(clonedSceneObject);
			}

		}
		else
		{
			sceneObject = LoadModel(filePath);
			assert(sceneObject);
			XREXContext::GetInstance().GetScene()->AddObject(sceneObject);
		}


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
			TeapotController(SceneObjectSP& object, float scaler)
				: InputHandler(GenerateActionMap()), object(object), scaler(scaler)
			{

			}
			virtual std::pair<bool, function<void()>> GenerateAction(InputCenter::InputEvent const& inputEvent) override
			{
				float x = static_cast<float>((inputEvent.mappedSemantic == Semantic::NX ? -1 : 0) + (inputEvent.mappedSemantic == Semantic::PX ? 1 : 0));
				float y = static_cast<float>((inputEvent.mappedSemantic == Semantic::NY ? -1 : 0) + (inputEvent.mappedSemantic == Semantic::PY ? 1 : 0));
				float z = static_cast<float>((inputEvent.mappedSemantic == Semantic::NZ ? -1 : 0) + (inputEvent.mappedSemantic == Semantic::PZ ? 1 : 0));
				object->GetComponent<Transformation>()->Translate(floatV3(x * scaler, y * scaler, z * scaler));
				return std::make_pair(false, function<void()>());
			}
			SceneObjectSP object;
			float scaler;
		};
		std::shared_ptr<TeapotController> c = MakeSP<TeapotController>(sceneObject, halfSize / 100);
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
