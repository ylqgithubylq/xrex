#include "XREXAll.hpp"
#include "VoxelTest.h"
#include "VoxelTestShaderCodes.h"

#include <CoreGL.hpp>

#include <iostream>

using namespace XREX;
using namespace std;

namespace
{
	MeshSP MakeCube()
	{
		vector<floatV3> vertexData;
		vector<uint16> indexData;

		float cubeSize = 4;

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
		VertexBufferSP vertices = XREXContext::GetInstance().GetRenderingFactory().CreateVertexBuffer(GraphicsBuffer::Usage::Static, vertexData, move(layoutDesc));
		IndexBufferSP indices = XREXContext::GetInstance().GetRenderingFactory().CreateIndexBuffer(GraphicsBuffer::Usage::Static, indexData, IndexBuffer::TopologicalType::Triangles);
		RenderingLayoutSP layout = MakeSP<RenderingLayout>(vector<VertexBufferSP>(1, vertices), indices);

		MeshSP cubeMesh = MakeSP<Mesh>("cube mesh");

		SubMeshSP const& subMesh = cubeMesh->CreateSubMesh("cube sub mesh", layout);

		return cubeMesh;
	}

	RenderingEffectSP MakeConeTracingEffect()
	{
		VoxelTestShaderCodes codes;

		RenderingEffectSP coneTracingEffect = MakeSP<RenderingEffect>("cone tracing effect");
		coneTracingEffect->AddShaderCode(std::move(codes.GetShaderCodes()[0]));

		ProgramObjectSP coneTracingProgram = XREXContext::GetInstance().GetRenderingFactory().CreateProgramObject();

		std::vector<std::string const*> const& shaderStrings = coneTracingEffect->GetFullShaderCode();
		ShaderObjectSP vs = XREXContext::GetInstance().GetRenderingFactory().CreateShaderObject(ShaderObject::ShaderType::VertexShader);
		ShaderObjectSP fs = XREXContext::GetInstance().GetRenderingFactory().CreateShaderObject(ShaderObject::ShaderType::FragmentShader);
		vs->Compile(shaderStrings);
		fs->Compile(shaderStrings);

		if (!vs->IsValidate())
		{
			cerr << vs->GetCompileError() << endl;
		}
		if (!fs->IsValidate())
		{
			cerr << fs->GetCompileError() << endl;
		}
		coneTracingProgram->AttachShader(vs);
		coneTracingProgram->AttachShader(fs);
		coneTracingProgram->Link();
		if (!coneTracingProgram->IsValidate())
		{
			cerr << coneTracingProgram->GetLinkError() << endl;
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

		RenderingTechniqueSP cubeTechnique = coneTracingEffect->CreateTechnique();
		RenderingPassSP cubePass = cubeTechnique->CreatePass(coneTracingProgram, rso, dsso, bso);

		return coneTracingEffect;
	}

	void SetData(vector<floatV4>& voxelVolume, uint32 size, uint32 x, uint32 y, uint32 z, floatV4 data)
	{
		voxelVolume[size *size * z + size * y + x] = data;
	}
	pair<TextureSP, SamplerSP> Make3DTexture()
	{
		uint32 size = 16;
		array<uint32, 3> dim = {size, size, size};
		Texture2D::DataDescription<3> desc(TexelFormat::RGBA32F, dim);
		vector<floatV4> dataLevel0(size * size * size);
		for (uint32 i = 0; i < size; ++i)
		{
			for (uint32 j = 0; j < size; ++j)
			{
				for (uint32 k = 0; k < size; ++k)
				{
					SetData(dataLevel0, size, k, j, i, floatV4(fmod(k * 2.0f, size) / size / 2, fmod(j * 3.0f, size) / size / 2, fmod(i * 4.0f, size) / size / 2, fmod((i + j + k) * 1.0f, size) / size / 2));
				}
			}
		}
		vector<vector<floatV4>> data(1);
		data[0] = move(dataLevel0);
		TextureSP texture = XREXContext::GetInstance().GetRenderingFactory().CreateTexture3D(desc, data, true);
		SamplerState ss;
		ss.borderColor = Color(0, 0, 0, 0);
		ss.addressingModeR = SamplerState::TextureAddressingMode::ClampToBorder;
		ss.addressingModeS = SamplerState::TextureAddressingMode::ClampToBorder;
		ss.addressingModeT = SamplerState::TextureAddressingMode::ClampToBorder;
		ss.magFilterOperation = SamplerState::TextureFilterOperation::Linear;
		ss.minFilterOperation = SamplerState::TextureFilterOperation::LinearMipmapLinear;
		SamplerSP sampler = XREXContext::GetInstance().GetRenderingFactory().CreateSampler(ss);
		return pair<TextureSP, SamplerSP>(texture, sampler);
	}

	void InitializeScene()
	{
		SceneSP scene = XREXContext::GetInstance().GetScene();

		SceneObjectSP cameraObject = MakeSP<SceneObject>("camera");
		Settings const& settings = XREXContext::GetInstance().GetSettings();
		CameraSP camera = MakeSP<Camera>(PI / 4, static_cast<float>(settings.renderingSettings.width) / settings.renderingSettings.height, 0.01f, 1000.0f);
		cameraObject->SetComponent(camera);
		scene->AddObject(cameraObject);

		auto cc = MakeSP<FirstPersonCameraController>();
		cc->AttachToCamera(cameraObject);
		XREXContext::GetInstance().GetInputCenter().AddInputHandler(cc);

		SceneObjectSP cubeObject = MakeSP<SceneObject>("cube object");
		MeshSP cube = MakeCube();
		std::vector<std::string> allChannels;
		for (auto& sub : cube->GetAllSubMeshes())
		{
			for (auto& vb : sub->GetLayout()->GetVertexBuffers())
			{
				auto& desc = vb->GetDataLayoutDescription();
				for (auto& layouts : desc.GetAllLayouts())
				{
					allChannels.push_back(layouts.channel);
				}
			}
		}
		floatV3 cubePosition = floatV3(0, 0, 0);
		RenderingEffectSP coneTracingEffect = MakeConeTracingEffect();
		MaterialSP material = MakeSP<Material>("tracing effect parameters");
		auto texture3D = Make3DTexture();
		material->SetParameter("voxels", texture3D);
		material->SetParameter("voxelVolumeCenter", cubePosition);
		material->SetParameter("voxelVolumeHalfSize", 8.f);
		material->SetParameter("aperture", camera->GetFieldOfView() / XREXContext::GetInstance().GetMainWindow().GetClientRegionSize().y);



		for (auto& sub : cube->GetAllSubMeshes())
		{
			sub->SetEffect(coneTracingEffect);
			sub->SetMaterial(material);
		}
		cubeObject->SetComponent(cube);
		TransformationSP trans = cubeObject->GetComponent<Transformation>();
		trans->SetPosition(cubePosition);
		scene->AddObject(cubeObject);
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


	function<void(double current, double delta)> f = [] (double current, double delta)
	{
		assert(gl::GetError() == gl::GL_NO_ERROR);
	};
	XREXContext::GetInstance().GetRenderingEngine().OnBeforeRendering(f);
	XREXContext::GetInstance().GetRenderingEngine().OnAfterRendering(f);
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
