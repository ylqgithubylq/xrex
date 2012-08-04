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

#include <CoreGL.hpp>
//#include "CoreGL.h"

#include <fstream>
#include <vector>
#include <iostream>
#include <functional>
#include <type_traits>
#include <map>












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
		gl::PolygonMode(gl::GL_FRONT_AND_BACK, gl::GL_FILL);
		gl::Enable(gl::GL_CULL_FACE);

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
		Settings const & settings = Context::GetInstance().GetSettings();
		CameraSP camera = MakeSP<Camera>(PI / 4, static_cast<float>(settings.renderingSettings.width) / settings.renderingSettings.height, 0.1, 100.0);
		camera_->SetComponent(camera);

		scene_ = Context::GetInstance().GetRenderingEngine().GetCurrentScene();
		scene_->AddObject(camera_);


		floatV3 eye = floatV3(0.5, 1.5, 15.0);
		floatV3 up = floatV3(0.0, 1.0, 0.0);
		TransformationSP cameraTransformation = camera_->GetComponent<Transformation>();
		cameraTransformation->SetPosition(eye);
		cameraTransformation->SetUpDirection(up);
		cameraTransformation->SetFrontDirection(floatV3(0, 0, -1));
		cameraTransformation->FaceTo(floatV3(0, 0, 0));

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

		int32 count = 9;
		objects_.resize(count);
		for (int32 i = 0; i < count; ++i)
		{
			SceneObjectSP& object = objects_[i];
			object = MakeSP<SceneObject>("cube");
			object->SetComponent(mesh_);
			scene_->AddObject(object);

			TransformationSP cubeTransform = object->GetComponent<Transformation>();

			floatV3 position = floatV3(i % 3 * 4 - 4, i / 3 * 4 - 4, 0);
			cubeTransform->SetPosition(position);
			cubeTransform->SetScaling(2);
			floatM44 orientation = Rotation(PI / 8 * i, 0.0f, 1.0f, 0.0f);
			cubeTransform->SetOrientation(orientation);
			cubeTransform->SetFrontDirection(floatV3(1, 0, 0));
			floatV3 temp = Transform(orientation, floatV3(1, 0, 0));
			cubeTransform->FaceTo(temp + position);
		}



		// 		wMatrix->SetValue(translate * rotation);

	}


	void Render(double delta)
	{

	}
	void operator()(double delta)
	{
		Render(delta);
	}
	

};

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

	floatM44 trans = Translation(floatV3(1, 2, 3));
	floatM44 rot = Rotation(PI / 4, floatV3(0, 1, 0));
	rot = RotationX(PI / 4);
	rot = RotationY(PI / 4);
	rot = RotationZ(PI / 4);
	floatM44 sc = Scaling(floatV3(1, 2, 1));
	floatM44 rotft = RotationFromTo(floatV3(0, 1, 0), floatV3(1, 0, 1));

	floatM44 frustum = Frustum(PI / 4, 8.0f / 6.0f, 1.0f, 10.0f);

	//cin.get();
}

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


	Context::GetInstance().Initialize(settings);

	TempScene s;
	s.InitializeScene();

	//Context::GetInstance().GetRenderingEngine().SetRenderingFunction(function<void(double)>(s));



	Context::GetInstance().Start();
}




int main()
{
	//TestMath();
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