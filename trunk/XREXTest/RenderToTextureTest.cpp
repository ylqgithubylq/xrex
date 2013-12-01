#include "XREXAll.hpp"
#include "RenderToTextureTest.h"

#include <CoreGL.hpp>

#include <iostream>
#include <sstream>
#include <cassert>

#undef LoadString

using namespace std;
using namespace XREX;

SceneObjectSP theCameraObject_;

struct RenderToTextureProcess
	: RenderingProcess
{
	TextureSP depth_;
	TextureSP color_;
	TextureSP normal_;
	TextureSP depthInColor_;

	FrameBufferSP frameBuffer_;
	
	RenderingTechniqueSP technique_;
	std::shared_ptr<TransformationSetter> techniqueTransformationSetter_;
	std::shared_ptr<CameraSetter> techniqueCameraSetter_;
	RenderingTechniqueSP copyTechnique_;

	RenderingLayoutSP quad_;
	LayoutAndProgramConnectorSP connector_;

	void InitializeTextureAndFrameBuffer()
	{
		Size<uint32, 2> size = XREXContext::GetInstance().GetMainWindow().GetClientRegionSize();
		Texture::DataDescription<2> depthDescription(TexelFormat::Depth32, size);
		depth_ = XREXContext::GetInstance().GetRenderingFactory().CreateTexture2D(depthDescription, false);
		Texture::DataDescription<2> colorDescription(TexelFormat::RGBA8, size);
		color_ = XREXContext::GetInstance().GetRenderingFactory().CreateTexture2D(colorDescription, false);
		Texture::DataDescription<2> normalDescription(TexelFormat::RGBA16F, size);
		normal_ = XREXContext::GetInstance().GetRenderingFactory().CreateTexture2D(normalDescription, false);
		Texture::DataDescription<2> depthColorDescription(TexelFormat::R32F, size);
		depthInColor_ = XREXContext::GetInstance().GetRenderingFactory().CreateTexture2D(depthColorDescription, false);

		FrameBufferLayoutDescription description(size, true, false);
		description.AddColorChannel(FrameBufferLayoutDescription::ColorChannelDescription("colorOutput", TexelFormat::RGBA8));
		description.AddColorChannel(FrameBufferLayoutDescription::ColorChannelDescription("normalOutput", TexelFormat::RGBA16F));
		description.AddColorChannel(FrameBufferLayoutDescription::ColorChannelDescription("depthInColorOutput", TexelFormat::R32F));

		unordered_map<string, TextureImageSP> colorTextures;
		colorTextures[description.GetAllColorChannels()[0].GetChannel()] = CheckedSPCast<Texture2D>(color_)->GetImage(0);
		colorTextures[description.GetAllColorChannels()[1].GetChannel()] = CheckedSPCast<Texture2D>(normal_)->GetImage(0);
		colorTextures[description.GetAllColorChannels()[2].GetChannel()] = CheckedSPCast<Texture2D>(depthInColor_)->GetImage(0);

		FrameBuffer::DepthStencilBinding depthStencilBinding(CheckedSPCast<Texture2D>(depth_)->GetImage(0), nullptr);
		frameBuffer_ = XREXContext::GetInstance().GetRenderingFactory().CreateFrameBuffer(description, std::move(colorTextures), depthStencilBinding);
	}

	void InitializeRenderingTechnique()
	{
		string shaderFile = "../../XREXTest/Effects/TestRenderToTexture.glsl";
		//string shaderFile = "../../XREXTest/Effects/TestRenderToTexture.glsl";
		shared_ptr<string> shaderString = XREXContext::GetInstance().GetResourceLoader().LoadString(shaderFile);
		if (!shaderString)
		{
			cerr << "file not found. file: " << shaderFile << endl;
		}

		TechniqueBuilderSP builder = MakeSP<TechniqueBuilder>("render technique");

		builder->AddInclude(TransformationTechnique().GetTechniqueToInclude());
		builder->AddInclude(CameraTechnique().GetTechniqueToInclude());

		builder->AddCommonCode(shaderString);
		builder->SetStageCode(ShaderObject::ShaderType::VertexShader, MakeSP<string>());
		builder->SetStageCode(ShaderObject::ShaderType::FragmentShader, MakeSP<string>());


		vector<VariableInformation const> variables;
		builder->AddUniformBufferInformation(BufferInformation("Material", BufferView::BufferType::Uniform, move(variables)));

		string defaultSamplerName = "defaultSampler";
		builder->AddTextureInformation(TextureInformation("diffuseMap", Texture::TextureType::Texture2D, ElementType::FloatV4, defaultSamplerName));
		builder->AddTextureInformation(TextureInformation("specularMap", Texture::TextureType::Texture2D, ElementType::FloatV4, defaultSamplerName));
		builder->AddTextureInformation(TextureInformation("normalMap", Texture::TextureType::Texture2D, ElementType::FloatV4, defaultSamplerName));
		builder->AddTextureInformation(TextureInformation("shininessMap", Texture::TextureType::Texture2D, ElementType::FloatV4, defaultSamplerName));
		builder->AddTextureInformation(TextureInformation("opacityMap", Texture::TextureType::Texture2D, ElementType::FloatV4, defaultSamplerName));

		SamplerState defaultSampler;
		builder->AddSamplerState(defaultSamplerName, defaultSampler);

		builder->AddAttributeInputInformation(AttributeInputInformation(VariableInformation("position", ElementType::FloatV3, 0)));
		builder->AddAttributeInputInformation(AttributeInputInformation(VariableInformation("normal", ElementType::FloatV3, 0)));
		builder->AddAttributeInputInformation(AttributeInputInformation(VariableInformation("textureCoordinate0", ElementType::FloatV3, 0)));

		builder->AddFragmentOutputInformation(FragmentOutputInformation(VariableInformation("colorOutput", ElementType::FloatV4, 0)));
		builder->AddFragmentOutputInformation(FragmentOutputInformation(VariableInformation("normalOutput", ElementType::FloatV4, 0)));
		builder->AddFragmentOutputInformation(FragmentOutputInformation(VariableInformation("depthInColorOutput", ElementType::Float, 0)));

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

		builder->SetRasterizerState(resterizerState);
		builder->SetDepthStencilState(depthStencilState);
		builder->SetBlendState(blendState);

		technique_ = builder->GetRenderingTechnique();
		technique_->SetFrameBuffer(frameBuffer_);

		techniqueTransformationSetter_ = MakeSP<TransformationSetter>(technique_);
		techniqueCameraSetter_ = MakeSP<CameraSetter>(technique_);
	}

	void InitializeTextureShowTechnique()
	{
		string shaderFile = "../../XREXTest/Effects/TestCopyTextures.glsl";
		shared_ptr<string> shaderString = XREXContext::GetInstance().GetResourceLoader().LoadString(shaderFile);
		if (!shaderString)
		{
			cerr << "file not found. file: " << shaderFile << endl;
		}

		TechniqueBuilderSP builder = MakeSP<TechniqueBuilder>("texture show technique");

		builder->AddCommonCode(shaderString);
		builder->SetStageCode(ShaderObject::ShaderType::VertexShader, MakeSP<string>());
		builder->SetStageCode(ShaderObject::ShaderType::FragmentShader, MakeSP<string>());


		string defaultSamplerName = "defaultSampler";
		builder->AddTextureInformation(TextureInformation("color", Texture::TextureType::Texture2D, ElementType::FloatV4, defaultSamplerName));
		builder->AddTextureInformation(TextureInformation("normal", Texture::TextureType::Texture2D, ElementType::FloatV4, defaultSamplerName));
		builder->AddTextureInformation(TextureInformation("depthInColor", Texture::TextureType::Texture2D, ElementType::FloatV4, defaultSamplerName));
		builder->AddTextureInformation(TextureInformation("depth", Texture::TextureType::Texture2D, ElementType::FloatV4, defaultSamplerName));

		SamplerState defaultSampler;
		builder->AddSamplerState(defaultSamplerName, defaultSampler);

		builder->AddAttributeInputInformation(AttributeInputInformation(VariableInformation("position", ElementType::FloatV2, 0)));

		builder->AddFragmentOutputInformation(FragmentOutputInformation(VariableInformation("xrex_FinalColor", ElementType::FloatV4, 0)));

		RasterizerState resterizerState;
		DepthStencilState depthStencilState;
		BlendState blendState;

		builder->SetRasterizerState(resterizerState);
		builder->SetDepthStencilState(depthStencilState);
		builder->SetBlendState(blendState);

		copyTechnique_ = builder->GetRenderingTechnique();

		copyTechnique_->SetFrameBuffer(XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer());

		TechniqueParameterSP color = copyTechnique_->GetParameterByName("color");
		TechniqueParameterSP normal = copyTechnique_->GetParameterByName("normal");
		TechniqueParameterSP depthInColor = copyTechnique_->GetParameterByName("depthInColor");
		TechniqueParameterSP depth = copyTechnique_->GetParameterByName("depth");
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

		connector_ = XREXContext::GetInstance().GetRenderingFactory().GetConnector(quad_, copyTechnique_);
	}

	RenderToTextureProcess()
	{
		InitializeTextureAndFrameBuffer();
		InitializeRenderingTechnique();
		InitializeTextureShowTechnique();
		InitializeScreenQuad();
	}

	virtual void RenderScene(SceneSP const& scene) override
	{
		RenderSceneToTexture(scene);
		RenderTextureToWindow();
	}

	void RenderSceneToTexture(SceneSP const& scene)
	{
		RenderACamera(scene, theCameraObject_);
	}

	void RenderACamera(SceneSP const& scene, SceneObjectSP const& cameraObject)
	{
		CameraSP camera = cameraObject->GetComponent<Camera>();
		Size<uint32, 2> windowSize = XREXContext::GetInstance().GetMainWindow().GetClientRegionSize();
		camera->GetViewport()->Bind(windowSize.X(), windowSize.Y());

		Color const& backgroundColor = camera->GetBackgroundColor();
		frameBuffer_->Clear(FrameBuffer::ClearMask::All, backgroundColor, 1, 0);

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

		techniqueCameraSetter_->SetParameter(camera);
		techniqueTransformationSetter_->Connect(camera);

		IndexedDrawer drawer;

		for (auto& renderablePack : allRenderableNeedToRender)
		{
			Renderable& ownerRenderable = *renderablePack.renderable;
			RenderingLayoutSP const& layout = renderablePack.layout;
			MaterialSP const& material = renderablePack.material;

			if (material)
			{
				material->BindToTechnique(technique_);
				material->SetAllTechniqueParameterValues();
			}

			techniqueTransformationSetter_->SetParameter(ownerRenderable.GetOwnerSceneObject()->GetComponent<Transformation>());

			LayoutAndProgramConnectorSP connector = XREXContext::GetInstance().GetRenderingFactory().GetConnector(layout, technique_);
			drawer.SetTechnique(technique_);
			drawer.SetLayoutAndProgramConnector(connector);
			drawer.SetRenderingLayout(layout);
			drawer.Launch();
		}
	}

	void RenderTextureToWindow()
	{
		IndexedDrawer drawer;
		drawer.SetTechnique(copyTechnique_);
		drawer.SetLayoutAndProgramConnector(connector_);
		drawer.SetRenderingLayout(quad_);
		drawer.Launch();
	}
};
namespace
{
	void InitializeRenderingProcess()
	{
		RenderingProcessSP rtrp = MakeSP<RenderToTextureProcess>();
		XREXContext::GetInstance().GetRenderingEngine().SetRenderingProcess(rtrp);
	}

	MeshSP LoadModel()
	{
		MeshSP model = XREXContext::GetInstance().GetResourceManager().LoadModel("Data/crytek-sponza/sponza.obj")->Create();
		return model;
	}

	SceneObjectSP MakeCamera()
	{
		SceneSP scene = XREXContext::GetInstance().GetScene();

		SceneObjectSP cameraObject = MakeSP<SceneObject>("camera");
		Size<uint32, 2> windowSize = XREXContext::GetInstance().GetMainWindow().GetClientRegionSize();
		float aspectRatio = static_cast<float>(windowSize.X()) / windowSize.Y();
		CameraSP camera = MakeSP<PerspectiveCamera>(PI / 4, aspectRatio, 1.f, 10000.0f);
		//CameraSP camera = MakeSP<OrthogonalCamera>(100.f * aspectRatio, 100.f, 100.f);
		cameraObject->SetComponent(camera);
		scene->AddObject(cameraObject);

		auto cc = MakeSP<FirstPersonCameraController>();
		cc->AttachToCamera(cameraObject);
		XREXContext::GetInstance().GetInputCenter().AddInputHandler(cc);
		return cameraObject;
	}

	void InitializeScene()
	{
		theCameraObject_ = MakeCamera();

		SceneSP scene = XREXContext::GetInstance().GetScene();
		SceneObjectSP modelObject = MakeSP<SceneObject>("cube object");
		MeshSP model = LoadModel();
		modelObject->SetComponent(model);
		TransformationSP trans = modelObject->GetComponent<Transformation>();
		trans->SetPosition(0, 0, 50);
		scene->AddObject(modelObject);

	}
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

	InitializeRenderingProcess();
	InitializeScene();

	XREXContext::GetInstance().Start();

}


RenderToTextureTest::~RenderToTextureTest()
{
}
