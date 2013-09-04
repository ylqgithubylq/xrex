#include "XREXAll.hpp"
#include "TextureTest.h"
#include "TextureTestShaderCodes.h"
#include <CoreGL.hpp>
#include <assert.h>
#include <iostream>


namespace
{

	MeshSP LoadTeapot()
	{
		MeshSP model;
		model = XREXContext::GetInstance().GetResourceManager().LoadModel("Data/teapot/teapot.obj")->Create();
		return model;
	}

	MeshSP MakeCube()
	{
		vector<floatV3> vertexData;
		vector<uint16> indexData;

		vertexData.push_back(floatV3(0.5, 0.5, 0.5));
		vertexData.push_back(floatV3(1, 1, 0));
		vertexData.push_back(floatV3(0.5, -0.5, 0.5));
		vertexData.push_back(floatV3(1, 0, 0));
		vertexData.push_back(floatV3(-0.5, -0.5, 0.5));
		vertexData.push_back(floatV3(0, 0, 0));
		vertexData.push_back(floatV3(-0.5, 0.5, 0.5));
		vertexData.push_back(floatV3(0, 1, 0));
		vertexData.push_back(floatV3(0.5, 0.5, -0.5));
		vertexData.push_back(floatV3(1, 1, 0));
		vertexData.push_back(floatV3(0.5, -0.5, -0.5));
		vertexData.push_back(floatV3(1, 0, 0));
		vertexData.push_back(floatV3(-0.5, -0.5, -0.5));
		vertexData.push_back(floatV3(0, 0, 0));
		vertexData.push_back(floatV3(-0.5, 0.5, -0.5));
		vertexData.push_back(floatV3(0, 1, 0));

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
		layoutDesc.AddChannelLayout(VertexBuffer::DataLayoutDescription::ElementLayoutDescription(0, 2 * sizeof(floatV3), ElementType::FloatV3, "position"));
		layoutDesc.AddChannelLayout(VertexBuffer::DataLayoutDescription::ElementLayoutDescription(sizeof(floatV3), 2 * sizeof(floatV3), ElementType::FloatV3, "textureCoordinate0"));
		VertexBufferSP vertices = XREXContext::GetInstance().GetRenderingFactory().CreateVertexBuffer(GraphicsBuffer::Usage::StaticDraw, vertexData, move(layoutDesc));
		IndexBufferSP indices = XREXContext::GetInstance().GetRenderingFactory().CreateIndexBuffer(GraphicsBuffer::Usage::StaticDraw, indexData, IndexBuffer::TopologicalType::Triangles);
		RenderingLayoutSP layout = MakeSP<RenderingLayout>(vector<VertexBufferSP>(1, vertices), indices);

		MeshSP cubeMesh = MakeSP<Mesh>("cube mesh");

		SubMeshSP const& subMesh = cubeMesh->CreateSubMesh("cube sub mesh", layout);

		return cubeMesh;
	}

	RenderingEffectSP MakeEffect()
	{
		TextureTestShaderCodes codes;

		RenderingEffectSP commonEffect = MakeSP<RenderingEffect>("common effect");
		commonEffect->AddShaderCode(std::move(codes.GetShaderCodes()[0]));
		RenderingEffectSP cubeEffect = MakeSP<RenderingEffect>("test cube effect");
		cubeEffect->AddShaderCode(std::move(codes.GetShaderCodes()[1]));
		cubeEffect->AddInclude(commonEffect);

		ProgramObjectSP cubeProgram = XREXContext::GetInstance().GetRenderingFactory().CreateProgramObject();

		std::vector<std::string const*> const& shaderStrings = cubeEffect->GetFullShaderCode();
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
		cubeProgram->AttachShader(vs);
		cubeProgram->AttachShader(fs);
		cubeProgram->Link();
		if (!cubeProgram->IsValidate())
		{
			cerr << cubeProgram->GetLinkError() << endl;
			return nullptr;
		}

		RasterizerState resterizerState;
		DepthStencilState depthStencilState;
		BlendState blendState;
		blendState.blendEnable = true;
		blendState.blendOperation = RenderingPipelineState::BlendOperation::Add;
		blendState.blendOperationAlpha = RenderingPipelineState::BlendOperation::Add;
		blendState.sourceBlend = RenderingPipelineState::AlphaBlendFactor::SourceAlpha;
		blendState.sourceBlendAlpha = RenderingPipelineState::AlphaBlendFactor::SourceAlpha;
		blendState.destinationBlend = RenderingPipelineState::AlphaBlendFactor::OneMinusSourceAlpha;
		blendState.destinationBlendAlpha = RenderingPipelineState::AlphaBlendFactor::OneMinusSourceAlpha;
		RasterizerStateObjectSP rso = XREXContext::GetInstance().GetRenderingFactory().CreateRasterizerStateObject(resterizerState);
		DepthStencilStateObjectSP dsso = XREXContext::GetInstance().GetRenderingFactory().CreateDepthStencilStateObject(depthStencilState);
		BlendStateObjectSP bso = XREXContext::GetInstance().GetRenderingFactory().CreateBlendStateObject(blendState);

		RenderingTechniqueSP cubeTechnique = cubeEffect->CreateTechnique("test cube technique");
		RenderingPassSP cubePass = cubeTechnique->CreatePass(cubeProgram, rso, dsso, bso);

		return cubeEffect;
	}


	pair<TextureSP, SamplerSP> MakeBumpTexture()
	{
		array<uint32, 2> dim = {2, 2};
		Texture2D::DataDescription<2> desc(TexelFormat::R32F, dim);
		vector<float> dataLevel0(4);
		dataLevel0[0] = floatV4(1.0f, 0, 0, 1).X();
		dataLevel0[1] = floatV4(0.7f, 0, 0, 1).X();
		dataLevel0[2] = floatV4(0.4f, 0, 0, 1).X();
		dataLevel0[3] = floatV4(0.1f, 0, 0, 1).X();
		vector<vector<float>> data(1);
		data[0] = move(dataLevel0);
		TextureSP texture = XREXContext::GetInstance().GetRenderingFactory().CreateTexture2D(desc, data, false);
		SamplerState ss;
		ss.addressingModeR = SamplerState::TextureAddressingMode::MirroredRepeat;
		ss.addressingModeS = SamplerState::TextureAddressingMode::MirroredRepeat;
		ss.addressingModeT = SamplerState::TextureAddressingMode::MirroredRepeat;
		SamplerSP sampler = XREXContext::GetInstance().GetRenderingFactory().CreateSampler(ss);
		return pair<TextureSP, SamplerSP>(texture, sampler);
	}

	TextureSP MakeTestImageTexture0()
	{
		array<uint32, 2> dim = {2, 2};
		Texture2D::DataDescription<2> desc(TexelFormat::RGBA32F, dim);
		vector<floatV4> dataLevel0(4);
		dataLevel0[0] = floatV4(0, 1.0f, 0, 1);
		dataLevel0[1] = floatV4(0, 0.7f, 0, 1);
		dataLevel0[2] = floatV4(0, 0.4f, 0, 1);
		dataLevel0[3] = floatV4(0, 0.1f, 0, 1);
		vector<vector<floatV4>> data(1);
	// 	vector<int> dataLevel0(4);
	// 	dataLevel0[0] = int(255);
	// 	dataLevel0[1] = int(63);
	// 	dataLevel0[2] = int(127);
	// 	dataLevel0[3] = int(191);
	// 	vector<vector<int>> data(1);

		data[0] = move(dataLevel0);
		TextureSP texture = XREXContext::GetInstance().GetRenderingFactory().CreateTexture2D(desc, data, false);
		return texture;
	}
	TextureSP MakeTestImageTexture1()
	{
		array<uint32, 2> dim = {2, 2};
		Texture2D::DataDescription<2> desc(TexelFormat::RGBA32F, dim);
		vector<floatV4> dataLevel0(4);
		dataLevel0[0] = floatV4(0, 0, 0.1f, 1);
		dataLevel0[1] = floatV4(0, 0, 0.4f, 1);
		dataLevel0[2] = floatV4(0, 0, 0.7f, 1);
		dataLevel0[3] = floatV4(0, 0, 1.0f, 1);
		vector<vector<floatV4>> data(1);
		// 	vector<int> dataLevel0(4);
		// 	dataLevel0[0] = int(255);
		// 	dataLevel0[1] = int(63);
		// 	dataLevel0[2] = int(127);
		// 	dataLevel0[3] = int(191);
		// 	vector<vector<int>> data(1);

		data[0] = move(dataLevel0);
		TextureSP texture = XREXContext::GetInstance().GetRenderingFactory().CreateTexture2D(desc, data, false);
		return texture;
	}

	pair<TextureSP, SamplerSP> Make3DTexture()
	{
		array<uint32, 3> dim = {2, 2, 2};
		Texture2D::DataDescription<3> desc(TexelFormat::RGBA32F, dim);
		vector<floatV4> dataLevel0(8);
		dataLevel0[0] = floatV4(1.0f, 0, 0, 1);
		dataLevel0[1] = floatV4(0.7f, 0, 0, 1);
		dataLevel0[2] = floatV4(0.4f, 0, 0, 1);
		dataLevel0[3] = floatV4(0.1f, 0, 0, 1);
		dataLevel0[4] = floatV4(0, 0, 1.0f, 1);
		dataLevel0[5] = floatV4(0, 0, 0.7f, 1);
		dataLevel0[6] = floatV4(0, 0, 0.4f, 1);
		dataLevel0[7] = floatV4(0, 0, 0.1f, 1);
		vector<vector<floatV4>> data(1);
		data[0] = move(dataLevel0);
		TextureSP texture = XREXContext::GetInstance().GetRenderingFactory().CreateTexture3D(desc, data, true);
		SamplerState ss;
		ss.addressingModeR = SamplerState::TextureAddressingMode::MirroredRepeat;
		ss.addressingModeS = SamplerState::TextureAddressingMode::MirroredRepeat;
		ss.addressingModeT = SamplerState::TextureAddressingMode::MirroredRepeat;
		ss.magFilterOperation = SamplerState::TextureFilterOperation::Linear;
		ss.minFilterOperation = SamplerState::TextureFilterOperation::LinearMipmapLinear;
		SamplerSP sampler = XREXContext::GetInstance().GetRenderingFactory().CreateSampler(ss);
		return pair<TextureSP, SamplerSP>(texture, sampler);
	}

}

SceneObjectSP theCameraObject_;

void InitializeScene()
{
	SceneSP scene;
	scene = XREXContext::GetInstance().GetScene();

	SceneObjectSP cameraObject = MakeSP<SceneObject>("camera");
	Settings const& settings = XREXContext::GetInstance().GetSettings();
	float aspectRatio = static_cast<float>(settings.renderingSettings.width) / settings.renderingSettings.height;
	CameraSP camera = MakeSP<PerspectiveCamera>(PI / 4, aspectRatio, 1.f, 100.0f);
	//CameraSP camera = MakeSP<OrthogonalCamera>(100.f * aspectRatio, 100.f, 100.f);
	cameraObject->SetComponent(camera);
	scene = XREXContext::GetInstance().GetScene();
	scene->AddObject(cameraObject);

	auto cc = MakeSP<FirstPersonCameraController>();
	cc->AttachToCamera(cameraObject);
	XREXContext::GetInstance().GetInputCenter().AddInputHandler(cc);

	SceneObjectSP cubeObject = MakeSP<SceneObject>("cube object");
	//MeshSP cube = MakeCube();
	MeshSP cube = LoadTeapot();
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
	RenderingEffectSP cubeEffect = MakeEffect();
	MaterialSP material = MakeSP<Material>("cube effect parameters");
	auto image0 = MakeTestImageTexture0()->GetImage_TEMP(0, TexelFormat::RGBA32F);
	auto image1 = MakeTestImageTexture1()->GetImage_TEMP(0, TexelFormat::RGBA32F);
	material->SetParameter("testImage0", image0);
	material->SetParameter("testImage1", image1);
	auto bump = MakeBumpTexture();
	material->SetParameter("notUsedTexture0", std::make_pair(/*bump.first*/image0->GetTexture(), bump.second));
	material->SetParameter("notUsedTexture1", std::make_pair(/*bump.first*/image1->GetTexture(), bump.second));
	auto texture3D = Make3DTexture();
	material->SetParameter("test3DTexture", texture3D);


	for (auto& sub : cube->GetAllSubMeshes())
	{
		sub->SetTechnique(cubeEffect->GetTechnique(0));
		sub->SetMaterial(material);
	}
	cubeObject->SetComponent(cube);
	TransformationSP trans = cubeObject->GetComponent<Transformation>();
	trans->SetPosition(0, 0, 50);
	scene->AddObject(cubeObject);

	theCameraObject_ = cameraObject;
}

#include <sstream>

TextureTest::TextureTest(void)
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
		auto& transformation = theCameraObject_->GetComponent<Transformation>();
		floatV3 const& position = transformation->GetWorldPosition();
		floatV3 to = TransformDirection(transformation->GetWorldMatrix(), transformation->GetModelFrontDirection());
		floatV3 up = TransformDirection(transformation->GetWorldMatrix(), transformation->GetModelUpDirection());

		Ray ray = theCameraObject_->GetComponent<Camera>()->GetViewRay(floatV2(0, 0), Camera::ViewportOrigin::ViewportCenter);

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

	InitializeScene();

	struct QuitHandler
		: InputHandler
	{
		static ActionMap GetActionMap()
		{
			ActionMap am;
			am.Set(InputCenter::InputSemantic::K_Escape, 0);
			return am;
		}

		QuitHandler()
			: InputHandler(GetActionMap())
		{
		}
		virtual std::pair<bool, std::function<void()>> GenerateAction(InputCenter::InputEvent const& inputEvent) override
		{
			if (inputEvent.data == 0)
			{
				return std::make_pair(true, [] ()
				{
					XREXContext::GetInstance().GetMainWindow().SetRunning(false);
				});
			}
			return std::make_pair(false, [] () {});
		}
	};
	InputHandlerSP quitHandler = MakeSP<QuitHandler>();
	XREXContext::GetInstance().GetInputCenter().AddInputHandler(quitHandler);

	XREXContext::GetInstance().Start();
}


TextureTest::~TextureTest(void)
{
}

