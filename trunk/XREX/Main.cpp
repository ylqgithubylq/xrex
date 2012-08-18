#include "XREX.hpp"


#include "Settings.hpp"
#include "GLWindow.hpp"
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
#include "FPSCameraController.hpp"
#include "InputCenter.hpp"

#include <CoreGL.hpp>

//#include <CoreGL.h>

#include <fstream>
#include <vector>
#include <iostream>
#include <functional>
#include <type_traits>
#include <map>
#include <sstream>
#include <iosfwd>













using namespace std;





struct TempScene
{
	SceneSP scene_;
	StaticMeshSP mesh_;
	SceneObjectSP cube_;
	SceneObjectSP camera_;
	vector<SceneObjectSP> objects_;

	void InitializeScene()
	{
		//gl::Enable(gl::GL_DEPTH_TEST);
		//gl::Enable(gl::GL_BLEND);


		ProgramObjectSP program = MakeSP<ProgramObject>();
		string shaderString;
		string shaderFile = "Effects/Test.glsl";
		if (!LocalResourceLoader::GetInstance().LoadString(shaderFile, &shaderString))
		{
			cerr << "file not found. file: " << shaderFile << endl;
		}

		ShaderObjectSP vs = MakeSP<ShaderObject>(ShaderObject::ShaderType::VertexShader, shaderString);
		ShaderObjectSP fs = MakeSP<ShaderObject>(ShaderObject::ShaderType::FragmentShader, shaderString);
		

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

		camera_ = MakeSP<SceneObject>("camera");
		Settings const & settings = Application::GetInstance().GetSettings();
		CameraSP camera = MakeSP<Camera>(PI / 4, static_cast<float>(settings.renderingSettings.width) / settings.renderingSettings.height, 0.1, 100.0);
		camera_->SetComponent(camera);

		scene_ = Application::GetInstance().GetRenderingEngine().GetCurrentScene();
		scene_->AddObject(camera_);


		floatV3 eye = floatV3(0.0, 0.5, -60.0);
		floatV3 up = floatV3(0.3, 1.0, 0.0);
		TransformationSP cameraTransformation = camera_->GetComponent<Transformation>();
		cameraTransformation->SetPosition(eye);
		cameraTransformation->SetUpDirection(up);
		cameraTransformation->SetFrontDirection(floatV3(0, 0, -1));
		//cameraTransformation->FaceTo(floatV3(0, 0, 0));

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

		GraphicsBufferSP vertices = MakeSP<GraphicsBuffer>(GraphicsBuffer::BufferType::Vertex, GraphicsBuffer::Usage::Static, vertexData, "mPosition");
		GraphicsBufferSP indices = MakeSP<GraphicsBuffer>(GraphicsBuffer::BufferType::Index, GraphicsBuffer::Usage::Static, indexData);
		RenderingLayoutSP layout = MakeSP<RenderingLayout>(vector<GraphicsBufferSP>(1, vertices), indices, RenderingLayout::DrawingMode::Triangles);

		map<string, RenderingLayoutSP> meshLayout;
		meshLayout["cube"] = layout;


		mesh_ = MakeSP<StaticMesh>(move(meshLayout));

		RenderingEffectSP effect = MakeSP<RenderingEffect>("test effect");
		RenderingPassSP pass = MakeSP<RenderingPass>(*effect);
		pass->Initialize(program);
		effect->Initialize(vector<RenderingPassSP>(1, pass));
		mesh_->SetEffect("cube", effect);

		effect = mesh_->GetEffect("cube");
		EffectParameterSP color = effect->GetParameterByName("color"); // this is something that should be in material
		color->SetValue(floatV3(0.5f, 0.5f, 1.0f));
		floatV3 c = color->GetValue();

		int32 edgeCount = 5;
		float intervalSize = 4.0f;
		float center = static_cast<float>(edgeCount - 1) / 2;
		int32 totalCount = edgeCount * edgeCount * edgeCount;
		objects_.resize(totalCount);
		for (int32 i = 0; i < edgeCount; ++i)
		{
			for (int32 j = 0; j < edgeCount; ++j)
			{
				for (int32 k = 0; k < edgeCount; ++k)
				{
					SceneObjectSP& object = objects_[i];
					stringstream objectNameStream;
					objectNameStream << "cube" << i << j << k;
					object = MakeSP<SceneObject>(objectNameStream.str());
					object->SetComponent(mesh_);
					scene_->AddObject(object);

					TransformationSP cubeTransform = object->GetComponent<Transformation>();

					floatV3 position = floatV3((k - center) * intervalSize, (j - center) * intervalSize, (i - center) * intervalSize);
					cubeTransform->SetPosition(position);
					cubeTransform->SetScaling(2);
					cubeTransform->SetFrontDirection(floatV3(1, 0, 0));
					cubeTransform->FaceToPosition(floatV3::Zero, floatV3(0, 1, 0));
					floatQ orientation = cubeTransform->GetOrientation();
					floatV3 faceTo = RotateByQuaternion(orientation, floatV3(1, 0, 0));
					floatV3 direction = floatV3::Zero - position;
				}
			}
		}


		FPSCameraControllerSP cameraController = MakeSP<FPSCameraController>();
		cameraController->InitializeActionMap();
		cameraController->AttachToCamera(camera_);
		Application::GetInstance().GetInputCenter().AddInputHandler(cameraController);
		// 		wMatrix->SetValue(translate * rotation);
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
	settings.windowTitle = L"TestMath GL4 window";

	settings.renderingSettings.colorBits = 32;
	settings.renderingSettings.depthBits = 24;
	settings.renderingSettings.stencilBits = 8;
	settings.renderingSettings.sampleCount = 4;

	settings.renderingSettings.left = 400;
	settings.renderingSettings.top = 300;
	settings.renderingSettings.width = 600;
	settings.renderingSettings.height = 500;


	Application::GetInstance().Initialize(settings);

	TempScene s;
	s.InitializeScene();

	function<void(double current, double delta)> f = [&s] (double current, double delta)
	{
		s(current, delta);
	};
	Application::GetInstance().GetRenderingEngine().SetRenderingFunction(f);

	//assert(testmain(0, nullptr) == 0);
	//Application::GetInstance().GetRenderingEngine().SetRenderingFunction(function<void(double,double)>(DrawHelper));



	Application::GetInstance().Start();


}


void TestMath()
{
	floatV4 f40(1, 2, 3, 4);
	floatV4	f41(4, 3, 2, 1);
	VectorT<uint8, 4> ui40(1, 2, 3, 4);
	floatV4 f42(1, 2, 3, 4);
	auto resultA = static_cast<floatV4>(ui40) + f40;
	auto resultM = static_cast<floatV4>(ui40) * f40;
	auto resultS = static_cast<floatV4>(ui40) - f40;
	auto resultD = static_cast<floatV4>(ui40) / f40;
	float length = f41.Length();
	auto normalized = f41.Normalize();
	floatV4 newV0 = normalized * length;
	floatV4 newV1 = length * normalized;
	floatV4 newV2 = newV0 / length;
	if (f40 != f41)
	{
		f41 = -f41;
	}
	if (f42 == f40)
	{
		f42 = f40;
	}
	newV0 = newV1;

	floatM44 m0;
	floatM44 m1(0.5, 0.5, 0, 0,
		-0.5, 0.5, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);
	m0 = floatM44::Identity;
	floatM44 m2 = m1.Transpose();
	floatM44 m3 = m1.Inverse();
	floatM44 m4 = m1 * m1;
	if (m2 == m3)
	{
		m0 = -m2;
	}
	m0 = 2 * m0 * 2;
	m0 = m0 / 4;

	floatV4 start(1, 0, 0, 1);
	floatV4 resultTrans = Transform(m1, start);
	resultTrans = Transform(m1, resultTrans);
	resultTrans = Transform(m1, resultTrans);
	resultTrans = Transform(m1, resultTrans);
	resultTrans = Transform(m1, resultTrans);
	resultTrans = Transform(m1, resultTrans);
	resultTrans = Transform(m1, resultTrans);
	resultTrans = Transform(m1, resultTrans);

	floatV3 normal(1, 0, 0);
	floatV3 resultNormal = TransformNormal(m1, normal);
	resultNormal = TransformNormal(m1, resultNormal);
	resultNormal = TransformNormal(m1, resultNormal);

	floatM44 trans = TranslationMatrix(floatV3(1, 2, 3));
	floatM44 rot = RotationMatrix(PI / 4, floatV3(0, 1, 0));
	rot = RotationMatrixX(PI / 4);
	rot = RotationMatrixY(PI / 4);
	rot = RotationMatrixZ(PI / 4);
	floatM44 sc = ScalingMatrix(floatV3(1, 2, 1));
	floatM44 rotft = RotationMatrixFromTo(floatV3(0, 1, 0), floatV3(1, 0, 1));

	floatM44 frustum = FrustumMatrix(PI / 4, 8.0f / 6.0f, 1.0f, 10.0f);

	floatQ quat0(sin(PI / 2 / 2) * floatV3(1, 1, 0).Normalize(), cos(PI / 2 / 2));
	floatQ quat1 = quat0.Normalize();
	floatQ quat2 = quat0.Conjugate();
	float angle = PI * 0.15f;
	floatQ quat3 = RotationQuaternion(angle, floatV3(-1, -1, 1));
	floatM44 mat3 = RotationMatrix(angle, floatV3(-1, -1, 1));
	floatV3 axisQ = quat0.V();
	if (quat3.V() != quat0.V())
	{
		axisQ = quat3.V();
	}
	floatV3 vecToRotate = floatV3(1, 1, 0);
	floatM44 tempMat = MatrixFromQuaternion(quat3);
	floatQ resQ = QuaternionFromMatrix(tempMat);
	floatV3 rotRes0 = RotateByQuaternion(quat3, vecToRotate);
	floatM44 fromQuat3 = MatrixFromQuaternion(quat3);
	floatV3 rotRes1 = Transform(fromQuat3, vecToRotate);
	floatV3 rotRes2 = Transform(mat3, vecToRotate);
	floatQ quatFromMat3 = QuaternionFromMatrix(mat3);
	floatV3 rotRes3 = RotateByQuaternion(quatFromMat3, vecToRotate);
	

	floatV3 rfrom = floatV3(1, 0, 0);
	floatV3 rto = floatV3(0, 1, 0);
	floatV3 toRotate0 = floatV3(0, 0, -1);
	floatM44 rotateftm = RotationMatrixFromTo(rfrom, rto);
	floatQ rotateftq = RotationQuaternionFromTo(rfrom, rto);
	floatV3 rftmr0 = Transform(rotateftm, toRotate0);
	floatV3 rftqr0 = RotateByQuaternion(rotateftq, toRotate0);
	floatV3 rftmr1 = Transform(rotateftm, rfrom);
	floatV3 rftqr1 = RotateByQuaternion(rotateftq, rfrom);

	floatV3 front = floatV3(1, 0, 0);
	floatV3 up = floatV3(0, 1, 0);
	floatV3 to0 = floatV3(0, 0, 1);
	floatV3 to1 = floatV3(-1, -1, -1);
	floatQ ftq0 = FaceToQuaternion(to0, up, front, up);
	floatM44 ftm0 = FaceToMatrix(to0, up, front, up);
	floatQ ftq1 = FaceToQuaternion(to1, up, front, up);
	floatM44 ftm1 = FaceToMatrix(to1, up, front, up);
	floatV3 vecToRotateFaceTo0 = floatV3(1, 0, 0);
	floatV3 vecToRotateFaceTo1 = floatV3(0, 1, 0);

	floatV3 ftrrq00 = RotateByQuaternion(ftq0, vecToRotateFaceTo0);
	floatV3 ftrrm00 = Transform(ftm0, vecToRotateFaceTo0);

	floatV3 ftrrq01 = RotateByQuaternion(ftq0, vecToRotateFaceTo1);
	floatV3 ftrrm01 = Transform(ftm0, vecToRotateFaceTo1);

	floatV3 ftrrq10 = RotateByQuaternion(ftq1, vecToRotateFaceTo0);
	floatV3 ftrrm10 = Transform(ftm1, vecToRotateFaceTo0);

	floatV3 ftrrq11 = RotateByQuaternion(ftq1, vecToRotateFaceTo1);
	floatV3 ftrrm11 = Transform(ftm1, vecToRotateFaceTo1);

}

void SQRTSpeedTest()
{
	int N = 1000000;
	vector<float> results(N, 0);
	vector<float> todo(N);
	for (int i = 0; i < N; ++i)
	{
		todo[i] = float(i);
	}

	Timer t;
	cout << ReciprocalSqrt(0.0f) << endl;
	
	for (int i = 0; i < N; ++i)
	{
		results[i] = 1 / sqrt(todo[i]);
	}
	for (int i = 0; i < N; ++i)
	{
		results[i] = ReciprocalSqrt(todo[i]);
	}

	t.Restart();
	float start1 = t.Elapsed();
	for (int i = 0; i < N; ++i)
	{
		results[i] = 1 / sqrt(todo[i]);
		//results[i] = sqrt(todo[i]);
	}
	float end1 = t.Elapsed();
	float result1 = end1 - start1;
	cout << result1 << endl;
	t.Restart();
	float start2 = t.Elapsed();
	for (int i = 0; i < N; ++i)
	{
		results[i] = ReciprocalSqrt(todo[i]);
		//results[i] = 1 / ReciprocalSqrt(todo[i]);
	}
	float end2 = t.Elapsed();
	float result2 = end2 - start2;
	cout << result2 << endl;
	float delta = result1 - result2;
	cout << delta << endl;
}

int main()
{
	TestMath();
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