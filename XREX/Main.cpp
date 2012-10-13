#include "XREX.hpp"


#include "Settings.hpp"
#include "GLWindow.hpp"
#include "RenderingFactory.hpp"
#include "RenderingEngine.hpp"
#include "LocalResourceLoader.hpp"
#include "Shader.hpp"
#include "RenderingEffect.hpp"
#include "GraphicsBuffer.hpp"
#include "RenderingLayout.hpp"
#include "Mesh.hpp"
#include "GLUtil.hpp"
#include "SceneObject.hpp"
#include "Transformation.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
#include "NaiveManagedScene.hpp"
#include "Timer.hpp"
#include "FreeRoamCameraController.hpp"
#include "InputCenter.hpp"
#include "Material.hpp"
#include "ResourceManager.hpp"

#include <CoreGL.hpp>

//#include <CoreGL.h>

#include <fstream>
#include <vector>
#include <iostream>
#include <functional>
#include <type_traits>
#include <sstream>
#include <iosfwd>




using namespace std;





struct TempScene
{
	SceneSP scene_;
	SceneObjectSP cube_;
	SceneObjectSP camera_;
	vector<SceneObjectSP> objects_;
	floatV3 centerPosition_;

	void InitializeScene()
	{
		gl::Enable(gl::GL_DEPTH_TEST);
		//gl::Enable(gl::GL_BLEND);


		ProgramObjectSP program = Application::GetInstance().GetRenderingFactory().CreateProgramObject();
		string shaderString;
		string shaderFile = "../../Effects/Test.glsl";
		if (!Application::GetInstance().GetResourceLoader().LoadString(shaderFile, &shaderString))
		{
			cerr << "file not found. file: " << shaderFile << endl;
		}

		ShaderObjectSP vs = Application::GetInstance().GetRenderingFactory().CreateShaderObject(ShaderObject::ShaderType::VertexShader, shaderString);
		ShaderObjectSP fs = Application::GetInstance().GetRenderingFactory().CreateShaderObject(ShaderObject::ShaderType::FragmentShader, shaderString);
		

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

		GraphicsBufferSP vertices = Application::GetInstance().GetRenderingFactory().CreateGraphicsVertexBuffer(GraphicsBuffer::Usage::Static, vertexData, "position");
		GraphicsBufferSP indices = Application::GetInstance().GetRenderingFactory().CreateGraphicsIndexBuffer(GraphicsBuffer::Usage::Static, indexData);
		RenderingLayoutSP layout = MakeSP<RenderingLayout>(vector<GraphicsBufferSP>(1, vertices), indices, RenderingLayout::DrawingMode::Triangles);

		MeshSP cubeMesh = MakeSP<Mesh>("cube");

		centerPosition_ = floatV3(0, 0, 60000);


		RenderingEffectSP effect = MakeSP<RenderingEffect>("test effect");
		RenderingTechniqueSP technique = effect->CreateTechnique();
		RenderingPassSP pass = technique->CreatePass();
		pass->Initialize(program);

		SubMeshSP const& subMesh = cubeMesh->CreateSubMesh("cube", nullptr, layout, effect);

		EffectParameterSP const& centerPosition = effect->GetParameterByName("centerPosition");
		if (centerPosition)
		{
			centerPosition->SetValue(centerPosition_);
			centerPosition_ = centerPosition->GetValue<floatV3>();
		}

		camera_ = MakeSP<SceneObject>("camera");
		Settings const& settings = Application::GetInstance().GetSettings();
		CameraSP camera = MakeSP<Camera>(PI / 4, static_cast<float>(settings.renderingSettings.width) / settings.renderingSettings.height, 1.f, 10000.0f);
		camera_->SetComponent(camera);

		scene_ = Application::GetInstance().GetRenderingEngine().GetCurrentScene();
		scene_->AddObject(camera_);


		floatV3 eye = floatV3(0.0f, 5.5f, -10.0f);
		floatV3 up = floatV3(0.3f, 1.0f, 0.0f);
		TransformationSP cameraTransformation = camera_->GetComponent<Transformation>();
		cameraTransformation->SetPosition(eye + centerPosition_);
		cameraTransformation->SetUpDirection(up);
		cameraTransformation->SetFrontDirection(floatV3(0, 0, -1));
		cameraTransformation->FaceToDirection(floatV3(0.0, 0.0, -1), floatV3(0, 1, 0));

		int32 edgeCount = 3;
		float intervalSize = 4.0f;
		float center = static_cast<float>(edgeCount - 1) / 2;
		int32 totalCount = edgeCount * edgeCount * edgeCount;
// 		objects_.resize(totalCount);
// 		for (int32 i = 1; i < edgeCount - 1; ++i) // set to this because the cube left are not cube form...
// 		{
// 			for (int32 j = 0; j < edgeCount; ++j) // TODO try to find what's wrong with the cube left
// 			{
// 				for (int32 k = 1; k < edgeCount - 1; ++k) // set to this because cube left are not cube form...
// 				{
// 					SceneObjectSP& object = objects_[i];
// 					stringstream objectNameStream;
// 					objectNameStream << "cube" << i << j << k;
// 					object = MakeSP<SceneObject>(objectNameStream.str());
// 					object->SetComponent(cubeMesh);
// 					scene_->AddObject(object);
// 
// 					TransformationSP cubeTransform = object->GetComponent<Transformation>();
// 
// 					floatV3 position = floatV3((k - center) * intervalSize, (j - center) * intervalSize, (i - center) * intervalSize);
// 					cubeTransform->SetPosition(position + centerPosition_);
// 					cubeTransform->SetScaling(2);
// 					cubeTransform->SetFrontDirection(floatV3(1, 0, 0));
// 					cubeTransform->FaceToPosition(floatV3::Zero + centerPosition_, floatV3(0, 1, 0));
// 					floatQ orientation = cubeTransform->GetOrientation();
// 				}
// 			}
// 		}


		FreeRoamCameraControllerSP cameraController = MakeSP<FreeRoamCameraController>();
		cameraController->InitializeActionMap();
		cameraController->AttachToCamera(camera_);
		Application::GetInstance().GetInputCenter().AddInputHandler(cameraController);
		// 		wMatrix->SetValue(translate * rotation);

		Application::GetInstance().GetResourceManager().AddResourceLocation("Data/");
		MeshSP model = Application::GetInstance().GetResourceManager().GetModel("crytek-sponza/sponza.obj");
		//MeshSP model = Application::GetInstance().GetResourceManager().GetModel("sibenik/sibenik.obj");
		//MeshSP model = Application::GetInstance().GetResourceManager().GetModel("rungholt/rungholt.obj");
		for (auto& subMesh : model->GetAllSubMeshes())
		{
			subMesh->SetEffect(effect);
		}
		SceneObjectSP bannerObject = MakeSP<SceneObject>("model");
		bannerObject->SetComponent(model);
		bannerObject->GetComponent<Transformation>()->SetPosition(centerPosition_);
		scene_->AddObject(bannerObject);
		
	}


	void Render(double current, double delta)
	{
// 		cout << "Timer.Elapsed: " << timer_.Elapsed() << endl;
// 		cout << "Timer.CurrentTime: " << timer_.CurrentTime() << endl;
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
	Settings settings;
	settings.windowTitle = L"GL4 window";

	settings.renderingSettings.colorBits = 32;
	settings.renderingSettings.depthBits = 24;
	settings.renderingSettings.stencilBits = 8;
	settings.renderingSettings.sampleCount = 4;

	settings.renderingSettings.left = 300;
	settings.renderingSettings.top = 200;
	settings.renderingSettings.width = 800;
	settings.renderingSettings.height = 600;


	Application::GetInstance().Initialize(settings);

	TempScene s;
	s.InitializeScene();

	function<void(double current, double delta)> f = [&s] (double current, double delta)
	{
		s(current, delta);
	};
	//Application::GetInstance().GetRenderingEngine().SetRenderingFunction(f);

	//assert(testmain(0, nullptr) == 0);
	//Application::GetInstance().GetRenderingEngine().SetRenderingFunction(function<void(double,double)>(DrawHelper));


	Application::GetInstance().Start();


}


#include "TestFile.hpp"


int main()
{
	//TestMath();
	//SQRTSpeedTest();
	//return 0;
	Main();

	

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