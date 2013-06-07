#include "XREXAll.hpp"
#include "TextureTest.h"
#include <CoreGL.hpp>
#include <assert.h>
#include <iostream>


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
	VertexBufferSP vertices = XREXContext::GetInstance().GetRenderingFactory().CreateVertexBuffer(GraphicsBuffer::Usage::Static, vertexData, move(layoutDesc));
	IndexBufferSP indices = XREXContext::GetInstance().GetRenderingFactory().CreateIndexBuffer(GraphicsBuffer::Usage::Static, indexData, IndexBuffer::PrimitiveType::Triangles);
	RenderingLayoutSP layout = MakeSP<RenderingLayout>(vector<VertexBufferSP>(1, vertices), indices);

	MeshSP cubeMesh = MakeSP<Mesh>("cube mesh");

	SubMeshSP const& subMesh = cubeMesh->CreateSubMesh("cube sub mesh", layout);

	return cubeMesh;
}

RenderingEffectSP MakeEffect()
{
	string testCommonFunctionString =
	"\n\
	vec3 ReturnSelf(vec3 v)\n\
	{\n\
		return v;\n\
	}\n\
	";
	string shaderString =
	"\n\
	uniform mat4 modelMatrix;\n\
	uniform mat4 normalMatrix;\n\
	uniform mat4 viewMatrix;\n\
	uniform mat4 projectionMatrix;\n\
	uniform vec3 cameraPosition;\n\
	\n\
	uniform sampler2D bumpTexture;\n\
	\n\
	#ifdef VS\n\
	\n\
	in vec3 position;\n\
	in vec3 textureCoordinate0;\n\
	out vec2 pixelTextureCoordinate;\n\
	\n\
	void main()\n\
	{\n\
		position = ReturnSelf(position);\n\
		gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);\n\
		pixelTextureCoordinate = textureCoordinate0.st;\n\
	}\n\
	\n\
	#endif\n\
	\n\
	#ifdef FS\n\
	\n\
	in vec2 pixelTextureCoordinate;\n\
	\n\
	layout(location = 0) out vec4 finalColor;\n\
	\n\
	void main()\n\
	{\n\
		finalColor = vec4(ReturnSelf(textureLod(bumpTexture, pixelTextureCoordinate, 0).rgb), 1);\n\
		//finalColor = vec4(textureLod(bumpTexture, vec2(1, 1), 0).rgb, 1);\n\
	}\n\
	\n\
	#endif\n\
	\n\
	";

	ProgramObjectSP cubeProgram = XREXContext::GetInstance().GetRenderingFactory().CreateProgramObject();

	std::vector<std::string> shaderStrings;
	shaderStrings.emplace_back(std::move(testCommonFunctionString));
	shaderStrings.emplace_back(std::move(shaderString));
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

	RenderingEffectSP cubeEffect = MakeSP<RenderingEffect>("test cube effect");
	RenderingTechniqueSP cubeTechnique = cubeEffect->CreateTechnique();
	RenderingPassSP cubePass = cubeTechnique->CreatePass(cubeProgram, rso, dsso, bso);

	return cubeEffect;
}


pair<TextureSP, SamplerSP> MakeBumpTexture()
{
	array<uint32, 2> dim = {2, 2};
	Texture2D::DataDescription<2> desc(Texture::TexelFormat::R32F, dim);
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
		assert(gl::GetError() == gl::GL_NO_ERROR);
		return true;
	};
	XREXContext::GetInstance().SetLogicFunction(l);

	InitializeScene();

	XREXContext::GetInstance().Start();
}


TextureTest::~TextureTest(void)
{
}

void TextureTest::InitializeScene()
{
	scene_ = XREXContext::GetInstance().GetRenderingEngine().GetScene();

	SceneObjectSP cameraObject = MakeSP<SceneObject>("camera");
	Settings const& settings = XREXContext::GetInstance().GetSettings();
	CameraSP camera = MakeSP<Camera>(PI / 4, static_cast<float>(settings.renderingSettings.width) / settings.renderingSettings.height, 1.f, 10000.0f);
	cameraObject->SetComponent(camera);
	scene_ = XREXContext::GetInstance().GetRenderingEngine().GetScene();
	scene_->AddObject(cameraObject);

	auto cc = MakeSP<FirstPersonCameraController>();
	cc->AttachToCamera(cameraObject);
	XREXContext::GetInstance().GetInputCenter().AddInputHandler(cc);

	SceneObjectSP cubeObject = MakeSP<SceneObject>("cube object");
	MeshSP cube = MakeCube();
	RenderingEffectSP cubeEffect = MakeEffect();
	MaterialSP material = MakeSP<Material>("cube effect parameters");
	material->SetParameter("bumpTexture", MakeBumpTexture());
	for (auto& sub : cube->GetAllSubMeshes())
	{
		sub->SetEffect(cubeEffect);
		sub->SetMaterial(material);
	}
	cubeObject->SetComponent(cube);
	TransformationSP trans = cubeObject->GetComponent<Transformation>();
	trans->SetPosition(0, 0, 10);
	scene_->AddObject(cubeObject);
}
