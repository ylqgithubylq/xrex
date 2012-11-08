#include <XREXAll.hpp>

//#include <CoreGL.hpp>

//#include <CoreGL.h>

#include <fstream>
#include <vector>
#include <iostream>
#include <functional>
#include <type_traits>
#include <sstream>
#include <iosfwd>



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
	virtual bool GenerateAction(InputCenter::InputEvent const& inputEvent, std::function<void()>* generatedAction) override
	{
		cameraCube->GetComponent<Transformation>()->Translate(floatV3(inputEvent.mappedSemantic * 0.1f, 0, 0));
		return false;
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

		ProgramObjectSP program = XREXContext::GetInstance().GetRenderingFactory().CreateProgramObject();
		string shaderString;
		string shaderFile = "../../Effects/Test.glsl";
		if (!XREXContext::GetInstance().GetResourceLoader().LoadString(shaderFile, &shaderString))
		{
			cerr << "file not found. file: " << shaderFile << endl;
		}

		ShaderObjectSP vs = XREXContext::GetInstance().GetRenderingFactory().CreateShaderObject(ShaderObject::ShaderType::VertexShader, shaderString);
		ShaderObjectSP fs = XREXContext::GetInstance().GetRenderingFactory().CreateShaderObject(ShaderObject::ShaderType::FragmentShader, shaderString);

		if (!vs->IsValidate())
		{
			cerr << vs->GetCompileError() << endl;
		}
		if (!fs->IsValidate())
		{
			cerr << fs->GetCompileError() << endl;
		}
		program->AttachShader(vs);
		program->AttachShader(fs);
		program->Link();
		if (!program->IsValidate())
		{
			cerr << program->GetLinkError() << endl;
			return;
		}
		ProgramObjectSP cubeProgram = XREXContext::GetInstance().GetRenderingFactory().CreateProgramObject();
		shaderFile = "../../Effects/TestCube.glsl";
		if (!XREXContext::GetInstance().GetResourceLoader().LoadString(shaderFile, &shaderString))
		{
			cerr << "file not found. file: " << shaderFile << endl;
		}

		vs = XREXContext::GetInstance().GetRenderingFactory().CreateShaderObject(ShaderObject::ShaderType::VertexShader, shaderString);
		fs = XREXContext::GetInstance().GetRenderingFactory().CreateShaderObject(ShaderObject::ShaderType::FragmentShader, shaderString);


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
			return;
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
		

		GraphicsBufferSP vertices = XREXContext::GetInstance().GetRenderingFactory().CreateGraphicsVertexBuffer(GraphicsBuffer::Usage::Static, vertexData, "position");
		GraphicsBufferSP indices = XREXContext::GetInstance().GetRenderingFactory().CreateGraphicsIndexBuffer(GraphicsBuffer::Usage::Static, indexData);
		RenderingLayoutSP layout = MakeSP<RenderingLayout>(vector<GraphicsBufferSP>(1, vertices), indices, RenderingLayout::DrawingMode::Triangles);

		MeshSP cubeMesh = MakeSP<Mesh>("cube mesh");

		centerPosition_ = floatV3(0, 0, 0000);

		RenderingEffectSP cubeEffect = MakeSP<RenderingEffect>("test cube effect");
		RenderingTechniqueSP cubeTechnique = cubeEffect->CreateTechnique();
		RenderingPassSP cubePass = cubeTechnique->CreatePass(cubeProgram, rso, dsso, bso);

		RenderingEffectSP effect = MakeSP<RenderingEffect>("test effect");
		RenderingTechniqueSP technique = effect->CreateTechnique();
		RenderingPassSP pass = technique->CreatePass(program, rso, dsso, bso);

		SubMeshSP const& subMesh = cubeMesh->CreateSubMesh("cube sub mesh", nullptr, layout, cubeEffect);

		EffectParameterSP const& centerPosition = effect->GetParameterByName("centerPosition");
		if (centerPosition)
		{
			centerPosition->SetValue(centerPosition_);
			centerPosition_ = centerPosition->GetValue<floatV3>();
		}
		camera_ = MakeSP<SceneObject>("camera");
		Settings const& settings = XREXContext::GetInstance().GetSettings();
		CameraSP camera = MakeSP<Camera>(PI / 4, static_cast<float>(settings.renderingSettings.width) / settings.renderingSettings.height, 1.f, 10000.0f);
		camera_->SetComponent(camera);
		camera_->SetComponent(cubeMesh->GetShallowClone());
		SceneObjectSP cameraCube = MakeSP<SceneObject>("camera cube");
		cameraCube->GetComponent<Transformation>()->SetParent(camera_->GetComponent<Transformation>());
		cameraCube->GetComponent<Transformation>()->Translate(0, 0, 20);
		cameraCube->SetComponent(cubeMesh->GetShallowClone());
		scene_ = XREXContext::GetInstance().GetRenderingEngine().GetScene();
		scene_->AddObject(camera_);
		scene_->AddObject(cameraCube);
		
		shared_ptr<CameraCubeController> controller = MakeSP<CameraCubeController>(cameraCube);
		XREXContext::GetInstance().GetInputCenter().AddInputHandler(controller);
		

		floatV3 eye = floatV3(0.0f, 5.5f, -10.0f);
		floatV3 up = floatV3(0.3f, 1.0f, 0.0f);
		TransformationSP cameraTransformation = camera_->GetComponent<Transformation>();
		cameraTransformation->SetPosition(eye + centerPosition_);
		cameraTransformation->SetModelUpDirection(up);
		cameraTransformation->SetModelFrontDirection(floatV3(0, 0, -1));
		cameraTransformation->FaceToDirection(floatV3(0.0, 0.0, -1), floatV3(0, 1, 0));

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
					assert(scene_->AddObject(object));

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
		assert(scene_->AddObject(rootObj));
		TransformationSP rootTransform = rootObj->GetComponent<Transformation>();
		rootTransform->Translate(0, 50, 0);
		rootTransform->SetScaling(3);

		obj1 = MakeSP<SceneObject>("obj1");
		obj1->SetComponent(cubeMesh->GetShallowClone());
		assert(scene_->AddObject(obj1));
		TransformationSP obj1Trans = obj1->GetComponent<Transformation>();
		obj1Trans->Translate(15, 0, 0);
		obj1Trans->SetScaling(2);
		obj1Trans->SetParent(rootTransform);

		obj2 = MakeSP<SceneObject>("obj2");
		obj2->SetComponent(cubeMesh->GetShallowClone());
		assert(scene_->AddObject(obj2));
		TransformationSP obj2Trans = obj2->GetComponent<Transformation>();
		obj2Trans->Translate(0, 8, 0);
		obj2Trans->SetScaling(2);
		obj2Trans->SetParent(obj1Trans);

		camera_->GetComponent<Transformation>()->SetParent(rootTransform);

		EffectParameterSP const& cubeCenterPosition = cubeEffect->GetParameterByName("centerPosition");
		if (cubeCenterPosition)
		{
			cubeCenterPosition->SetValue(floatV3(0, 50, 0));
		}

		FreeRoamCameraControllerSP cameraController = MakeSP<FreeRoamCameraController>();
		cameraController->AttachToCamera(camera_);
		XREXContext::GetInstance().GetInputCenter().AddInputHandler(cameraController);
		// 		wMatrix->SetValue(translate * rotation);
		XREXContext::GetInstance().GetResourceManager().AddResourceLocation("Data/");
		MeshSP model;
		model = XREXContext::GetInstance().GetResourceManager().GetModel("crytek-sponza/sponza.obj");
		//model = XREXContext::GetInstance().GetResourceManager().GetModel("sibenik/sibenik.obj");
		//model = XREXContext::GetInstance().GetResourceManager().GetModel("rungholt/rungholt.obj");
		if (model)
		{
			for (auto& subMesh : model->GetAllSubMeshes())
			{
				subMesh->SetEffect(effect);
			}
			SceneObjectSP sceneObject = MakeSP<SceneObject>("model");
			sceneObject->SetComponent(model);
			sceneObject->GetComponent<Transformation>()->SetPosition(centerPosition_);
			sceneObject->GetComponent<Transformation>()->Scale(1.f);
			assert(scene_->AddObject(sceneObject));
		}

		
	}
	SceneObjectSP rootObj;
	SceneObjectSP obj1;
	SceneObjectSP obj2;

	void Render(double current, double delta)
	{
// 		cout << "Timer.Elapsed: " << timer_.Elapsed() << endl;
// 		cout << "Timer.CurrentTime: " << timer_.CurrentTime() << endl;
		if (rootObj)
		{
			rootObj->GetComponent<Transformation>()->Rotate(delta * 0.2, 0, 0, 1);
			obj1->GetComponent<Transformation>()->Rotate(delta * 0.5, 1, 0, 0);
			obj2->GetComponent<Transformation>()->Rotate(delta * 1, 0, 1, 0);
		}
	}
	void operator ()(double current, double delta)
	{
		Render(current, delta);
	}
	Timer timer_;

};



// GL4 sample code
//#include "../CPPTest/DemoCode_HelloTriangle.h"

void Main() 
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
	TempScene s;
	s.InitializeScene();


	function<void(double current, double delta)> f = [&s] (double current, double delta)
	{
		//assert(gl::GetError() == gl::GL_NO_ERROR);
		s(current, delta);
	};
	XREXContext::GetInstance().GetRenderingEngine().OnBeforeRendering(f);

	//assert(testmain(0, nullptr) == 0);
	//XREXContext::GetInstance().GetRenderingEngine().OnBeforeRendering(function<void(double,double)>(DrawHelper));


	XREXContext::GetInstance().Start();


}


#include "TestFile.hpp"


int main()
{
	//TestMath();
	//SQRTSpeedTest();
	//return 0;
	TestFile t;
	t.TestTransformation();
	Main();

	shared_ptr<int> isp = MakeSP<int>(1);
	unique_ptr<int> iup = MakeUP<int>(1);
	
// 	struct LeakTest
// 	{
// 		shared_ptr<LeakTest> p;
// 	};
// 	shared_ptr<LeakTest> ltp0 = MakeSP<LeakTest>();
// 	shared_ptr<LeakTest> ltp1 = MakeSP<LeakTest>();
// 	ltp0->p = ltp1;
// 	ltp1->p = ltp0;
	return 0;
}

//memory leak check

// #define _CRTDBG_MAP_ALLOC
// #include <crtdbg.h>
// 
// struct DML
// {
// 	~DML()
// 	{
// 		if (_CrtDumpMemoryLeaks())
// 		{
// 			cout << "memory leaks." << endl;
// 		}
// 		else
// 		{
// 			cout << "no memeory leaks." << endl;
// 		}
// 		cin.get();
// 	}
// } _dml;