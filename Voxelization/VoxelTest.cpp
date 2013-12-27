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
		RenderingTechniqueSP volumeBuild;
	};

	UsedTechniques MakeVoxelizationTechnique()
	{

		RenderingTechniqueSP listTechnique = []
		{
			string shaderFile = "../../Voxelization/Shaders/ListGeneration.glsl";
			shared_ptr<string> shaderString = XREXContext::GetInstance().GetResourceLoader().LoadString(shaderFile);
			if (!shaderString)
			{
				XREXContext::GetInstance().GetLogger().LogLine("file not found. file: " + shaderFile);
			}

			TechniqueBuildingInformationSP technique = MakeSP<TechniqueBuildingInformation>("voxelization list technique");
			technique->AddCommonCode(shaderString);
			technique->AddStageCode(ShaderObject::ShaderType::VertexShader, MakeSP<string>());
			technique->AddStageCode(ShaderObject::ShaderType::FragmentShader, MakeSP<string>());

			technique->AddInclude(XREXContext::GetInstance().GetRenderingEngine().GetSystemTechniqueFactory("Transformation")->GetTechniqueInformationToInclude());
			technique->AddInclude(XREXContext::GetInstance().GetRenderingEngine().GetSystemTechniqueFactory("Camera")->GetTechniqueInformationToInclude());

			technique->AddUniformBufferInformation(BufferInformation("PerObject", "", BufferView::BufferType::Uniform, std::vector<VariableInformation const>()));
			technique->AddUniformBufferInformation(BufferInformation("PerAxis", "", BufferView::BufferType::Uniform, std::vector<VariableInformation const>()));
			technique->AddUniformBufferInformation(BufferInformation("NeverChanged", "", BufferView::BufferType::Uniform, std::vector<VariableInformation const>()));

			technique->AddImageInformation(ImageInformation("heads", TextureImage::ImageType::Image2D, TexelFormat::R32UI, AccessType::ReadWrite));
			technique->AddImageInformation(ImageInformation("nodePool", TextureImage::ImageType::ImageBuffer, TexelFormat::RGBA32UI, AccessType::WriteOnly));

			technique->AddAtomicCounterBufferInformation(BufferInformation("AtomicBuffer0", "", BufferView::BufferType::AtomicCounter, std::vector<VariableInformation const>()));

			technique->AddAttributeInputInformation(AttributeInputInformation("position", ElementType::FloatV3));

			technique->SetFrameBufferDescription(XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer()->GetLayoutDescription());

			RasterizerState resterizerState;
			resterizerState.cullMode = RenderingPipelineState::CullMode::None;
			DepthStencilState depthStencilState;
			depthStencilState.depthTestEnable = false;
			depthStencilState.depthWriteMask = false;
			BlendState blendState;
			blendState.blendEnable = false;
			blendState.redMask = false;
			blendState.greenMask = false;
			blendState.blueMask = false;
			blendState.alphaMask = false;
			technique->SetRasterizerState(resterizerState);
			technique->SetDepthStencilState(depthStencilState);
			technique->SetBlendState(blendState);


			return TechniqueBuilder(technique).GetRenderingTechnique();
		} ();
		listTechnique->ConnectFrameBuffer(XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer());


		RenderingTechniqueSP generationTechnique = []
		{
			string shaderFile = "../../Voxelization/Shaders/VoxelGeneration.glsl";
			shared_ptr<string> shaderString = XREXContext::GetInstance().GetResourceLoader().LoadString(shaderFile);
			if (!shaderString)
			{
				XREXContext::GetInstance().GetLogger().LogLine("file not found. file: " + shaderFile);
			}
			TechniqueBuildingInformationSP technique = MakeSP<TechniqueBuildingInformation>("voxelization generation technique");
			technique->AddCommonCode(shaderString);
			technique->AddStageCode(ShaderObject::ShaderType::VertexShader, MakeSP<string>());
			technique->AddStageCode(ShaderObject::ShaderType::FragmentShader, MakeSP<string>());

			technique->AddUniformBufferInformation(BufferInformation("PerAxis", "", BufferView::BufferType::Uniform, std::vector<VariableInformation const>()));

			technique->AddImageInformation(ImageInformation("heads", TextureImage::ImageType::Image2D, TexelFormat::R32UI, AccessType::ReadOnly));
			technique->AddImageInformation(ImageInformation("nodePool", TextureImage::ImageType::ImageBuffer, TexelFormat::RGBA32UI, AccessType::ReadOnly));
			technique->AddImageInformation(ImageInformation("volume", TextureImage::ImageType::Image3D, TexelFormat::RGBA8, AccessType::WriteOnly));

			technique->AddAttributeInputInformation(AttributeInputInformation("position", ElementType::FloatV2));

			technique->SetFrameBufferDescription(XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer()->GetLayoutDescription());

			RasterizerState resterizerState;
			resterizerState.cullMode = RenderingPipelineState::CullMode::None;
			DepthStencilState depthStencilState;
			depthStencilState.depthTestEnable = false;
			depthStencilState.depthWriteMask = false;
			BlendState blendState;
			blendState.blendEnable = false;
			blendState.redMask = false;
			blendState.greenMask = false;
			blendState.blueMask = false;
			blendState.alphaMask = false;
			technique->SetRasterizerState(resterizerState);
			technique->SetDepthStencilState(depthStencilState);
			technique->SetBlendState(blendState);


			return TechniqueBuilder(technique).GetRenderingTechnique();
		} ();
		generationTechnique->ConnectFrameBuffer(XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer());


		UsedTechniques effects = {listTechnique, generationTechnique, };
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
		TechniqueBuildingInformationSP technique = MakeSP<TechniqueBuildingInformation>("cone tracing technique");
		technique->AddCommonCode(shaderString);
		technique->AddStageCode(ShaderObject::ShaderType::VertexShader, MakeSP<string>());
		technique->AddStageCode(ShaderObject::ShaderType::FragmentShader, MakeSP<string>());

		technique->AddInclude(XREXContext::GetInstance().GetRenderingEngine().GetSystemTechniqueFactory("Transformation")->GetTechniqueInformationToInclude());
		technique->AddInclude(XREXContext::GetInstance().GetRenderingEngine().GetSystemTechniqueFactory("Camera")->GetTechniqueInformationToInclude());

		technique->AddUniformBufferInformation(BufferInformation("NeverChanged", "", BufferView::BufferType::Uniform, std::vector<VariableInformation const>()));
		technique->AddUniformBufferInformation(BufferInformation("TracingParameter", "", BufferView::BufferType::Uniform, std::vector<VariableInformation const>()));

		SamplerState samplerState;
		samplerState.borderColor = Color(0, 0, 0, 0);
		samplerState.addressingModeR = SamplerState::TextureAddressingMode::ClampToBorder;
		samplerState.addressingModeS = SamplerState::TextureAddressingMode::ClampToBorder;
		samplerState.addressingModeT = SamplerState::TextureAddressingMode::ClampToBorder;
		samplerState.magFilterMode = SamplerState::TextureFilterMode::Linear;
		samplerState.minFilterMode = SamplerState::TextureFilterMode::LinearMipmapLinear;
		technique->AddSamplerState("cone tracing sampler", samplerState);

		technique->AddTextureInformation(TextureInformation("voxels", Texture::TextureType::Texture3D, Texture::TexelType::FloatV4, "cone tracing sampler"));

		technique->AddAttributeInputInformation(AttributeInputInformation("position", ElementType::FloatV3));

		technique->SetFrameBufferDescription(XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer()->GetLayoutDescription());

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
		technique->SetRasterizerState(resterizerState);
		technique->SetDepthStencilState(depthStencilState);
		technique->SetBlendState(blendState);



		return TechniqueBuilder(technique).GetRenderingTechnique();
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

		std::shared_ptr<TransformationSetter> voxelizationTechniqueTransformationSetter;
		std::shared_ptr<CameraSetter> voxelizationTechniqueCameraSetter;

		std::shared_ptr<TransformationSetter> coneTracingTechniqueTransformationSetter;
		std::shared_ptr<CameraSetter> coneTracingTechniqueCameraSetter;

		MaterialSP voxelizationMaterial;
		ViewportSP listBuildingViewport;

		std::array<SceneObjectSP, 3> voxelizationCameras;

		std::array<TextureSP, 3> headPointers;
		std::array<TextureSP, 3> nodePools;
		std::array<GraphicsBufferSP, 3> atomicCounterBuffers;
		GraphicsBufferSP clearPointer;

		floatV3 sceneCenter;
		float sceneHalfSize;

		uint32 voxelVolumeResolution;

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
		

		RenderToTextureProcess(floatV3 const& sceneCenter, float sceneHalfSize, uint32 voxelVolumeResolution, float cameraSpeedScaler)
			: frame(0)
		{
			this->sceneCenter = sceneCenter;
			this->sceneHalfSize = sceneHalfSize;

			this->voxelVolumeResolution = voxelVolumeResolution;

			CreateVoxelizationObjects();

			viewCameraObject = MakeCamera(cameraSpeedScaler);
			coneTracingProxyCube = MakeConeTracingProxyCube(CheckedSPCast<PerspectiveCamera>(viewCameraObject->GetComponent<Camera>()), sceneCenter, sceneHalfSize);

			// texture3DToTest = MakeTest3DTexture();

			screenQuad = MakeScreenQuad();
		}

		void CreateVoxelizationObjects()
		{
			listBuildingViewport = XREXContext::GetInstance().GetRenderingFactory().CreateViewport(0, 0, 0, voxelVolumeResolution, voxelVolumeResolution);
			voxelizationTechnique = MakeVoxelizationTechnique();

			RenderingTechniqueSP listTechnique = voxelizationTechnique.listBuild;
			TechniqueParameterSP neverChanged = listTechnique->GetParameterByName("NeverChanged");
			ShaderResourceBufferSP neverChangedBuffer = neverChanged->As<ShaderResourceBufferSP>().GetValue();
			ShaderResourceBuffer::BufferMapper mapper = neverChangedBuffer->GetMapper();
			auto voxelVolumeHalfSizeSetter = neverChangedBuffer->GetSetter("voxelVolumeHalfSize");
			assert(voxelVolumeHalfSizeSetter.first);
			voxelVolumeHalfSizeSetter.second.SetValue(mapper, sceneHalfSize);
			auto voxelVolumeCenterSetter = neverChangedBuffer->GetSetter("voxelVolumeCenter");
			assert(voxelVolumeCenterSetter.first);
			voxelVolumeCenterSetter.second.SetValue(mapper, sceneCenter);
			mapper.Finish();

			voxelizationTechniqueTransformationSetter = MakeSP<TransformationSetter>(voxelizationTechnique.listBuild);
			voxelizationTechniqueCameraSetter = MakeSP<CameraSetter>(voxelizationTechnique.listBuild);

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
				GraphicsBufferSP headPointer = XREXContext::GetInstance().GetRenderingFactory().CreateGraphicsBuffer(GraphicsBuffer::Usage::StaticDraw, voxelVolumeResolution * voxelVolumeResolution * sizeof(uint32));
				headPointer->Clear(0u);
				clearPointer = headPointer;
			}

			for (uint32 i = 0; i < 3; ++i)
			{
				Size<uint32, 2> dim(voxelVolumeResolution, voxelVolumeResolution);
				Texture::DataDescription<2> desc(TexelFormat::R32UI, dim);

				TextureSP headPointer = XREXContext::GetInstance().GetRenderingFactory().CreateTexture2D(desc, false);
				headPointers[i] = headPointer;

				TexelFormat linkedListNodeFormat = TexelFormat::RGBA32UI;
				uint32 poolSize = voxelVolumeResolution * voxelVolumeResolution * GetTexelSizeInBytes(linkedListNodeFormat) * 8;
				GraphicsBufferSP buffer = XREXContext::GetInstance().GetRenderingFactory().CreateGraphicsBuffer(GraphicsBuffer::Usage::StreamCopy, poolSize, BufferView::BufferType::Texture);
				TextureSP linkedListNodePool = XREXContext::GetInstance().GetRenderingFactory().CreateTextureBuffer(buffer, linkedListNodeFormat);
				nodePools[i] = linkedListNodePool;
				GraphicsBufferSP atomicBuffer = XREXContext::GetInstance().GetRenderingFactory().CreateGraphicsBuffer(GraphicsBuffer::Usage::StreamCopy, 4, BufferView::BufferType::AtomicCounter);
				atomicCounterBuffers[i] = atomicBuffer;
			}

			//voxelVolume = MakeTest3DTexture();
			voxelVolume = MakeVoxelVolume(voxelVolumeResolution);

			clearVoxelVolume = MakeClearVoxelVolume(voxelVolumeResolution);

		}

		SceneObjectSP MakeConeTracingProxyCube(PerspectiveCameraSP const& camera, floatV3 const& cubePosition, float cubeHalfSize)
		{

			SceneObjectSP cubeObject = MakeSP<SceneObject>("cube object");
			MeshSP cube = MakeCube(cubeHalfSize);

			RenderingTechniqueSP coneTracingTechnique = MakeConeTracingTechnique();
			coneTracingTechnique->ConnectFrameBuffer(XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer());

			TechniqueParameterSP neverChanged = coneTracingTechnique->GetParameterByName("NeverChanged");
			ShaderResourceBufferSP neverChangedBuffer = neverChanged->As<ShaderResourceBufferSP>().GetValue();
			ShaderResourceBuffer::BufferMapper mapper = neverChangedBuffer->GetMapper();
			auto voxelVolumeHalfSizeSetter = neverChangedBuffer->GetSetter("voxelVolumeHalfSize");
			assert(voxelVolumeHalfSizeSetter.first);
			voxelVolumeHalfSizeSetter.second.SetValue(mapper, sceneHalfSize);
			auto voxelVolumeCenterSetter = neverChangedBuffer->GetSetter("voxelVolumeCenter");
			assert(voxelVolumeCenterSetter.first);
			voxelVolumeCenterSetter.second.SetValue(mapper, sceneCenter);
			mapper.Finish();

			coneTracingTechniqueTransformationSetter = MakeSP<TransformationSetter>(coneTracingTechnique);
			coneTracingTechniqueCameraSetter = MakeSP<CameraSetter>(coneTracingTechnique);

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

		virtual void RenderScene(SceneSP const& scene) override
		{
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
 
 			BuildVoxelVolume();

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
					Texture2DSP headPointerAs2D = CheckedSPCast<Texture2D>(headPointers[i]);
					headPointerAs2D->Bind(0);
					GLTextureFormat glFormat = GLTextureFormatFromTexelFormat(headPointerAs2D->GetDescription().GetFormat());

					gl::BindBuffer(gl::GL_PIXEL_UNPACK_BUFFER, glClearPointer);
					gl::TexSubImage2D(gl::GL_TEXTURE_2D, 0, 0, 0, headPointerAs2D->GetDescription().GetSize()[0], headPointerAs2D->GetDescription().GetSize()[1],
						glFormat.glSourceFormat, glFormat.glTextureElementType, nullptr);
					gl::BindBuffer(gl::GL_PIXEL_UNPACK_BUFFER, 0);
				}


				CameraSP const& camera = voxelizationCameras[i]->GetComponent<Camera>();
				camera->GetViewport()->Bind(0, 0);

				voxelizationTechniqueCameraSetter->SetParameter(camera);
				voxelizationTechniqueTransformationSetter->Connect(camera);

				TechniqueParameterSP const& linkedListNodePool = listTechnique->GetParameterByName("nodePool");
				linkedListNodePool->As<TextureImageSP>().SetValue(CheckedSPCast<TextureBuffer>(nodePools[i])->GetImage());
				TechniqueParameterSP const& headPointer = listTechnique->GetParameterByName("heads");
				headPointer->As<TextureImageSP>().SetValue(CheckedSPCast<Texture2D>(headPointers[i])->GetImage(0));
				TechniqueParameterSP const& atomicCounter = listTechnique->GetParameterByName("AtomicBuffer0");
				ShaderResourceBufferSP atomicBuffer = atomicCounter->As<ShaderResourceBufferSP>().GetValue();
				atomicBuffer->SetBuffer(atomicCounterBuffers[i]);
				atomicCounterBuffers[i]->Clear(1u);
// 				{
// 					ShaderResourceBuffer::BufferVariableSetter setter = atomicBuffer->GetVariableSetter();
// 					std::pair<bool, ShaderResourceBuffer::BufferVariableSetter::Setter> nodeCounterSetter = setter.GetSetter("nodeCounter");
// 					assert(nodeCounterSetter.first);
// 					nodeCounterSetter.second.SetValue(setter, 1u);
// 				}

				ShaderResourceBufferSP perAxisBuffer = listTechnique->GetParameterByName("PerAxis")->As<ShaderResourceBufferSP>().GetValue();
				ShaderResourceBuffer::BufferMapper mapper = perAxisBuffer->GetMapper();
				auto axisSetter = perAxisBuffer->GetSetter("axis");
				assert(axisSetter.first);
				axisSetter.second.SetValue(mapper, static_cast<int32>(i));
				mapper.Finish();


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

					voxelizationTechniqueTransformationSetter->SetParameter(ownerRenderable.GetOwnerSceneObject()->GetComponent<Transformation>());

					ShaderResourceBufferSP perObjectBuffer = listTechnique->GetParameterByName("PerObject")->As<ShaderResourceBufferSP>().GetValue();
					ShaderResourceBuffer::BufferMapper mapper = perObjectBuffer->GetMapper();
					auto objectSetter = perObjectBuffer->GetSetter("objectID");
					assert(objectSetter.first);
					objectSetter.second.SetValue(mapper, objectID);
					mapper.Finish();



					listTechnique->SetupAllResources();
					LayoutAndProgramConnectorSP connector = XREXContext::GetInstance().GetRenderingFactory().GetConnector(layout, listTechnique);
					drawer.SetRenderingLayout(layout);
					connector->Bind();
					drawer.CoreLaunch(); // core launch
					connector->Unbind();
				}
			}

		}

		void BuildVoxelVolume()
		{
			listBuildingViewport->Bind(0, 0);

			RenderingTechniqueSP voxelizeTechnique = voxelizationTechnique.volumeBuild;

			{ // clear voxelVolume texture
				uint32 glClearVoxelVolume = clearVoxelVolume->GetID();
				Texture3DSP voxelVolumeAs3D = CheckedSPCast<Texture3D>(voxelVolume);
				voxelVolumeAs3D->Bind(0);
				GLTextureFormat glFormat = GLTextureFormatFromTexelFormat(voxelVolumeAs3D->GetDescription().GetFormat());

				gl::BindBuffer(gl::GL_PIXEL_UNPACK_BUFFER, glClearVoxelVolume);
				gl::TexSubImage3D(gl::GL_TEXTURE_3D, 0, 0, 0, 0, voxelVolumeAs3D->GetDescription().GetSize()[0], voxelVolumeAs3D->GetDescription().GetSize()[1], voxelVolumeAs3D->GetDescription().GetSize()[2],
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
				volume->As<TextureImageSP>().SetValue(CheckedSPCast<Texture3D>(voxelVolume)->GetImage(0));
				ShaderResourceBufferSP perAxisBuffer = voxelizeTechnique->GetParameterByName("PerAxis")->As<ShaderResourceBufferSP>().GetValue();
				ShaderResourceBuffer::BufferMapper mapper = perAxisBuffer->GetMapper();
				auto axisSetter = perAxisBuffer->GetSetter("axis");
				assert(axisSetter.first);
				axisSetter.second.SetValue(mapper, static_cast<int32>(i));
				mapper.Finish();


				RenderingLayoutSP const& layout = screenQuad;

				LayoutAndProgramConnectorSP connector = XREXContext::GetInstance().GetRenderingFactory().GetConnector(layout, voxelizeTechnique);
				drawer.SetTechnique(voxelizeTechnique);
				drawer.SetLayoutAndProgramConnector(connector);
				drawer.SetRenderingLayout(layout);
				drawer.Launch();
			}
		}

		void ConeTracing()
		{
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
			XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer()->Clear(FrameBuffer::ClearMask::All, backgroundColor, 1, 0);

			coneTracingTechniqueCameraSetter->SetParameter(camera);
			coneTracingTechniqueTransformationSetter->Connect(camera);

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

				coneTracingTechniqueTransformationSetter->SetParameter(ownerRenderable.GetOwnerSceneObject()->GetComponent<Transformation>());

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
			SponzaWithTeapots,
		} target = Scene::Teapot;

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
		case Scene::SponzaWithTeapots:
			center = floatV3(0, 32.f, 0);
			halfSize = 128;
			break;
		default:
			assert(false);
			break;
		}

		std::shared_ptr<RenderToTextureProcess> renderingProcess = MakeSP<RenderToTextureProcess>(center, halfSize, 256, halfSize / 50);
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
			filePath = "Data/dabrovic-sponza/sponza.obj";
			//filePath = "Data/teapot/teapot.obj";
			
			sceneObject = LoadModel(filePath);
			sceneObject->GetComponent<Transformation>()->SetScaling(16);
			assert(sceneObject);
			XREXContext::GetInstance().GetScene()->AddObject(sceneObject);

			filePath = "Data/teapot/teapot.obj";
			SceneObjectSP innerSceneObject = LoadModel(filePath);
			//SceneObjectSP innerSceneObject = MakeSP<SceneObject>("teapot");
			//innerSceneObject->SetComponent(sceneObject->GetComponent<Renderable>()->ShallowClone());

			MeshSP mesh = CheckedSPCast<Mesh>(innerSceneObject->GetComponent<Renderable>());
			for (int32 i = 0; i < 8; ++i)
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
