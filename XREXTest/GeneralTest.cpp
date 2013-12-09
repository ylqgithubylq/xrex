#include "XREXAll.hpp"
#include "GeneralTest.h"

#include <fstream>
#include <vector>
#include <iostream>
#include <functional>
#include <type_traits>
#include <sstream>
#include <iosfwd>
#include <assert.h>


using namespace XREX;
using namespace std;




struct CameraCubeController
	: public InputHandler
{
	static ActionMap GenerateActionMap()
	{
		ActionMap actions;
		actions.Set(InputCenter::InputSemantic::K_LeftArrow, 1);
		actions.Set(InputCenter::InputSemantic::K_RightArrow, -1);
		return actions;
	}
	CameraCubeController(SceneObjectSP& theCameraCube)
		: InputHandler(GenerateActionMap()), cameraCube(theCameraCube)
	{

	}
	virtual std::pair<bool, function<void()>> GenerateAction(InputCenter::InputEvent const& inputEvent) override
	{
		cameraCube->GetComponent<Transformation>()->Translate(floatV3(inputEvent.mappedSemantic * 0.1f, 0, 0));
		return std::make_pair(false, function<void()>());
	}
	SceneObjectSP cameraCube;
};

struct TempScene
{
	SceneSP scene_;
	SceneObjectSP cube_;
	SceneObjectSP camera_;
	vector<SceneObjectSP> objects_;
	floatV3 centerPosition_;

	void InitializeScene()
	{
		vector<floatV3> vertexData;
		vector<uint16> indexData;

		vertexData.push_back(floatV3(0.5, 0.5, 0.5));
		vertexData.push_back(floatV3(0.5, -0.5, 0.5));
		vertexData.push_back(floatV3(-0.5, -0.5, 0.5));
		vertexData.push_back(floatV3(-0.5, 0.5, 0.5));
		vertexData.push_back(floatV3(0.5, 0.5, -0.5));
		vertexData.push_back(floatV3(0.5, -0.5, -0.5));
		vertexData.push_back(floatV3(-0.5, -0.5, -0.5));
		vertexData.push_back(floatV3(-0.5, 0.5, -0.5));

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


		
		string shaderFile = "../../XREXTest/Effects/Test.glsl";
		shared_ptr<string> shaderString = XREXContext::GetInstance().GetResourceLoader().LoadString(shaderFile);
		if (!shaderString)
		{
			cerr << "file not found. file: " << shaderFile << endl;
		}

		TechniqueBuildingInformationSP testTechnique = MakeSP<TechniqueBuildingInformation>("test technique");

		testTechnique->AddCommonCode(shaderString);
		testTechnique->SetStageCode(ShaderObject::ShaderType::VertexShader, MakeSP<string>());
		testTechnique->SetStageCode(ShaderObject::ShaderType::FragmentShader, MakeSP<string>());


		shaderFile = "../../XREXTest/Effects/TestCube.glsl";
		shaderString = XREXContext::GetInstance().GetResourceLoader().LoadString(shaderFile);
		if (!shaderString)
		{
			cerr << "file not found. file: " << shaderFile << endl;
		}

		TechniqueBuildingInformationSP testCubeTechnique = MakeSP<TechniqueBuildingInformation>("test cube technique");
		testCubeTechnique->AddCommonCode(shaderString);
		testCubeTechnique->SetStageCode(ShaderObject::ShaderType::VertexShader, MakeSP<string>());
		testCubeTechnique->SetStageCode(ShaderObject::ShaderType::FragmentShader, MakeSP<string>());


		testTechnique->AddInclude(TransformationTechnique().GetTechniqueToInclude());
		testTechnique->AddInclude(CameraTechnique().GetTechniqueToInclude());

		testCubeTechnique->AddInclude(TransformationTechnique().GetTechniqueToInclude());
		testCubeTechnique->AddInclude(CameraTechnique().GetTechniqueToInclude());

		vector<VariableInformation const> variables;
		testTechnique->AddUniformBufferInformation(BufferInformation("Material", BufferView::BufferType::Uniform, move(variables)));
		testTechnique->AddUniformBufferInformation(BufferInformation("Info", BufferView::BufferType::Uniform, move(variables)));

		testCubeTechnique->AddUniformBufferInformation(BufferInformation("Info", BufferView::BufferType::Uniform, move(variables)));

		string defaultSamplerName = "defaultSampler";
		SamplerState defaultSampler;
		testTechnique->AddSamplerState(defaultSamplerName, defaultSampler);

		testTechnique->AddTextureInformation(TextureInformation("diffuseMap", Texture::TextureType::Texture2D, ElementType::FloatV4, defaultSamplerName));
		testTechnique->AddTextureInformation(TextureInformation("specularMap", Texture::TextureType::Texture2D, ElementType::FloatV4, defaultSamplerName));
		testTechnique->AddTextureInformation(TextureInformation("normalMap", Texture::TextureType::Texture2D, ElementType::FloatV4, defaultSamplerName));
		testTechnique->AddTextureInformation(TextureInformation("shininessMap", Texture::TextureType::Texture2D, ElementType::FloatV4, defaultSamplerName));
		testTechnique->AddTextureInformation(TextureInformation("opacityMap", Texture::TextureType::Texture2D, ElementType::FloatV4, defaultSamplerName));

		testTechnique->AddAttributeInputInformation(AttributeInputInformation("position", ElementType::FloatV3));
		testTechnique->AddAttributeInputInformation(AttributeInputInformation("normal", ElementType::FloatV3));
		testTechnique->AddAttributeInputInformation(AttributeInputInformation("textureCoordinate0", ElementType::FloatV3));

		testCubeTechnique->AddAttributeInputInformation(AttributeInputInformation("position", ElementType::FloatV3));

		testTechnique->SetFrameBufferDescription(XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer()->GetLayoutDescription());

		testCubeTechnique->SetFrameBufferDescription(XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer()->GetLayoutDescription());


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


		testTechnique->SetRasterizerState(resterizerState);
		testTechnique->SetDepthStencilState(depthStencilState);
		testTechnique->SetBlendState(blendState);

		testCubeTechnique->SetRasterizerState(resterizerState);
		testCubeTechnique->SetDepthStencilState(depthStencilState);
		testCubeTechnique->SetBlendState(blendState);


		RenderingTechniqueSP effect = TechniqueBuilder(testTechnique).GetRenderingTechnique();
		RenderingTechniqueSP cubeEffect = TechniqueBuilder(testCubeTechnique).GetRenderingTechnique();
		effect->ConnectFrameBuffer(XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer());
		cubeEffect->ConnectFrameBuffer(XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer());



		VertexBufferSP vertices = XREXContext::GetInstance().GetRenderingFactory().CreateVertexBuffer(GraphicsBuffer::Usage::StaticDraw, vertexData, "position");
		IndexBufferSP indices = XREXContext::GetInstance().GetRenderingFactory().CreateIndexBuffer(GraphicsBuffer::Usage::StaticDraw, indexData, IndexBuffer::TopologicalType::Triangles);
		RenderingLayoutSP layout = MakeSP<RenderingLayout>(vector<VertexBufferSP>(1, vertices), indices);

		MeshSP cubeMesh = MakeSP<Mesh>("cube mesh");


		SubMeshSP const& subMesh = cubeMesh->CreateSubMesh("cube sub mesh", layout, nullptr, cubeEffect);
		centerPosition_ = floatV3(0, 0, 0000);

		TechniqueParameterSP const& centerPosition = effect->GetParameterByName("centerPosition");
		if (centerPosition)
		{
			centerPosition->As<floatV3>().SetValue(centerPosition_);
			centerPosition_ = centerPosition->As<floatV3>().GetValue();
		}
		camera_ = MakeSP<SceneObject>("camera");
		Settings const& settings = XREXContext::GetInstance().GetSettings();
		CameraSP camera = MakeSP<PerspectiveCamera>(PI / 4, static_cast<float>(settings.renderingSettings.width) / settings.renderingSettings.height, 1.f, 10000.0f);
		camera_->SetComponent(camera);
		camera_->SetComponent(cubeMesh->GetShallowClone());
		SceneObjectSP cameraCube = MakeSP<SceneObject>("camera cube");
		cameraCube->GetComponent<Transformation>()->SetParent(camera_->GetComponent<Transformation>());
		cameraCube->GetComponent<Transformation>()->Translate(0, 0, 20);
		cameraCube->SetComponent(cubeMesh->GetShallowClone());
		scene_ = XREXContext::GetInstance().GetScene();
		scene_->AddObject(camera_);
		//scene_->AddObject(cameraCube);

		shared_ptr<CameraCubeController> controller = MakeSP<CameraCubeController>(cameraCube);
		XREXContext::GetInstance().GetInputCenter().AddInputHandler(controller);


		floatV3 eye = floatV3(0.0f, 5.5f, -10.0f);
		floatV3 up = floatV3(0.0f, 1.0f, 0.0f);
		TransformationSP cameraTransformation = camera_->GetComponent<Transformation>();
		cameraTransformation->SetPosition(eye + centerPosition_);
		cameraTransformation->SetModelUpDirection(up);
		cameraTransformation->FaceToDirection(floatV3(0.0, 0.0, -1), floatV3(0, 1, 0));

		bool b;
		int32 edgeCount = 3;
		float intervalSize = 4.0f;
		float center = static_cast<float>(edgeCount - 1) / 2;
		int32 totalCount = edgeCount * edgeCount * edgeCount;
		objects_.resize(totalCount);
		for (int32 i = 1; i < edgeCount - 1; ++i) // set to this because the cube left are not cube form...
		{
			for (int32 j = 0; j < edgeCount; ++j) // TODO try to find what's wrong with the cube left
			{
				for (int32 k = 1; k < edgeCount - 1; ++k) // set to this because cube left are not cube form...
				{
					SceneObjectSP& object = objects_[i];
					stringstream objectNameStream;
					objectNameStream << "cube" << i << j << k;
					object = MakeSP<SceneObject>(objectNameStream.str());
					object->SetComponent(cubeMesh->GetShallowClone());
					b = scene_->AddObject(object);
					assert(b);

					TransformationSP cubeTransform = object->GetComponent<Transformation>();

					floatV3 position = floatV3((k - center) * intervalSize, (j - center) * intervalSize, (i - center) * intervalSize);
					cubeTransform->SetPosition(position + centerPosition_);
					cubeTransform->SetScaling(1);
					cubeTransform->SetModelFrontDirection(floatV3(1, 0, 0));
					cubeTransform->FaceToPosition(floatV3::Zero + centerPosition_, floatV3(0, 1, 0));
					floatQ orientation = cubeTransform->GetOrientation();
				}
			}
		}

		rootObj = MakeSP<SceneObject>("root");
		rootObj->SetComponent(cubeMesh->GetShallowClone());
		b = scene_->AddObject(rootObj);
		assert(b);
		TransformationSP rootTransform = rootObj->GetComponent<Transformation>();
		rootTransform->Translate(0, 50, 0);
		rootTransform->SetScaling(3);

		obj1 = MakeSP<SceneObject>("obj1");
		obj1->SetComponent(cubeMesh->GetShallowClone());
		b = scene_->AddObject(obj1);
		assert(b);
		TransformationSP obj1Trans = obj1->GetComponent<Transformation>();
		obj1Trans->Translate(15, 0, 0);
		obj1Trans->SetScaling(2);
		obj1Trans->SetParent(rootTransform);

		obj2 = MakeSP<SceneObject>("obj2");
		obj2->SetComponent(cubeMesh->GetShallowClone());
		b = scene_->AddObject(obj2);
		assert(b);
		TransformationSP obj2Trans = obj2->GetComponent<Transformation>();
		obj2Trans->Translate(0, 8, 0);
		obj2Trans->SetScaling(2);
		obj2Trans->SetParent(obj1Trans);

		//camera_->GetComponent<Transformation>()->SetParent(rootTransform);

		TechniqueParameterSP const& cubeCenterPosition = cubeEffect->GetParameterByName("centerPosition");
		if (cubeCenterPosition)
		{
			cubeCenterPosition->As<floatV3>().SetValue(floatV3(0, 50, 0));
		}

		//		FreeRoamCameraControllerSP cameraController = MakeSP<FreeRoamCameraController>();
		FirstPersonCameraControllerSP cameraController = MakeSP<FirstPersonCameraController>();
		//OrbitCameraControllerSP cameraController = MakeSP<OrbitCameraController>(rootObj);
		cameraController->AttachToCamera(camera_);
		XREXContext::GetInstance().GetInputCenter().AddInputHandler(cameraController);
		// 		wMatrix->SetValue(translate * rotation);
		XREXContext::GetInstance().GetResourceManager().AddResourceLocation("Data/");
		MeshSP model;
		
		//model = XREXContext::GetInstance().GetResourceManager().LoadModel("Data/dabrovic-sponza/sponza.obj")->Create();
		model = XREXContext::GetInstance().GetResourceManager().LoadModel("crytek-sponza/sponza.obj")->Create();
		//model = XREXContext::GetInstance().GetResourceManager().LoadModel("yfl/yfl.obj")->Create();

		//model = XREXContext::GetInstance().GetResourceManager().LoadModel("vcc/vcc.dae")->Create();
		//model = XREXContext::GetInstance().GetResourceManager().GetModel("sibenik/sibenik.obj");
		//model = XREXContext::GetInstance().GetResourceManager().GetModel("rungholt/rungholt.obj");
		if (model)
		{
			for (auto& subMesh : model->GetAllSubMeshes())
			{
				subMesh->SetTechnique(effect);
			}
			SceneObjectSP sceneObject = MakeSP<SceneObject>("model");
			sceneObject->SetComponent(model);
			sceneObject->GetComponent<Transformation>()->SetPosition(centerPosition_);
			sceneObject->GetComponent<Transformation>()->Scale(1.f);
			b = scene_->AddObject(sceneObject);
			assert(b);
		}


	}
	SceneObjectSP rootObj;
	SceneObjectSP obj1;
	SceneObjectSP obj2;

	void Logic(double currentTime, double deltaTime)
	{
		auto& transformation = camera_->GetComponent<Transformation>();
		floatV3 const& position = transformation->GetWorldPosition();
		floatV3 to = TransformDirection(transformation->GetWorldMatrix(), transformation->GetModelFrontDirection());
		floatV3 up = TransformDirection(transformation->GetWorldMatrix(), transformation->GetModelUpDirection());

		Ray ray = camera_->GetComponent<Camera>()->GetViewRay(floatV2(0, 0), Camera::ViewportOrigin::ViewportCenter);

		wstringstream wss;
		wss << "position: (" << position.X() << ", " << position.Y() << ", " << position.Z() << "), ";
		wss << "direction: (" << to.X() << ", " << to.Y() << ", " << to.Z() << "), ";
		wss << "ray: (" << ray.GetDirection().X() << ", " << ray.GetDirection().Y() << ", " << ray.GetDirection().Z() << "), ";
		//wss << "up: (" << up.X() << ", " << up.Y() << ", " << up.Z() << "), ";

		XREXContext::GetInstance().GetMainWindow().SetTitleText(wss.str());
	}

	void Render(double current, double delta)
	{
		// 		cout << "Timer.Elapsed: " << timer_.Elapsed() << endl;
		// 		cout << "Timer.CurrentTime: " << timer_.CurrentTime() << endl;
		if (rootObj)
		{
			float deltaf = static_cast<float>(delta);
			//rootObj->GetComponent<Transformation>()->Rotate(deltaf * 0.2f, 0, 0, 1);
			obj1->GetComponent<Transformation>()->Rotate(deltaf * 0.5f, 1, 0, 0);
			obj2->GetComponent<Transformation>()->Rotate(deltaf * 1, 0, 1, 0);
		}
	}
	void operator ()(double current, double delta)
	{
		Render(current, delta);
	}
	Timer timer_;

};

GeneralTest::GeneralTest()
{
	Settings settings("../../");
	settings.windowTitle = L"GL4 window";

	settings.renderingSettings.sampleCount = 4;

	settings.renderingSettings.left = 300;
	settings.renderingSettings.top = 200;
	settings.renderingSettings.width = 1024;
	settings.renderingSettings.height = 800;

	XREXContext::GetInstance().Initialize(settings);
	TempScene s;
	s.InitializeScene();


	function<void(double current, double delta)> f = [&s] (double current, double delta)
	{
		assert(XREXContext::GetInstance().GetRenderingEngine().GetGLError() == 0);
		s(current, delta);
	};
	XREXContext::GetInstance().GetRenderingEngine().OnBeforeRendering(f);
	function<bool(double current, double delta)> l = [&s] (double current, double delta)
	{
		assert(XREXContext::GetInstance().GetRenderingEngine().GetGLError() == 0);
		s.Logic(current, delta);
		return true;
	};
	XREXContext::GetInstance().SetLogicFunction(l);

	//assert(testmain(0, nullptr) == 0);
	//XREXContext::GetInstance().GetRenderingEngine().OnBeforeRendering(function<void(double,double)>(DrawHelper));


	XREXContext::GetInstance().Start();
}


GeneralTest::~GeneralTest()
{
}






// GL4 sample code
//#include "../CPPTest/DemoCode_HelloTriangle.h"

void Main() 
{



}
