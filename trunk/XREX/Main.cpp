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

#include <CoreGL.hpp>
//#include "CoreGL.h"

#include <fstream>
#include <vector>
#include <iostream>
#include <functional>
#include <type_traits>
#include <map>












using namespace std;





struct Scene
{
	StaticMeshSP mesh_;
	SceneObjectSP cube_;

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

		RenderingEffectSP effect;
		effect = MakeSP<RenderingEffect>("test effect");
		RenderingPassSP pass = MakeSP<RenderingPass>(*effect);
		pass->Initialize(program);
		effect->Initialize(vector<RenderingPassSP>(1, pass));

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
		mesh_->SetEffect("cube", effect);


		cube_ = MakeSP<SceneObject>("cube");
		cube_->SetComponent(mesh_);
	}


	void Render(double delta)
	{
		gl::ClearColor(0.4f, 0.6f, 0.9f, 1);
		gl::Clear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT | gl::GL_STENCIL_BUFFER_BIT);


		RenderingEffectSP effect = mesh_->GetEffect("cube");
		EffectParameterSP color = effect->GetParameterByName("color");
		color->SetValue(floatV3(0.5f, 0.5f, 1.0f));
		floatV3 c = color->GetValue();
		EffectParameterSP wMatrix = effect->GetParameterByName("wMatrix");
		EffectParameterSP vMatrix = effect->GetParameterByName("vMatrix");
		EffectParameterSP pMatrix = effect->GetParameterByName("pMatrix");

		TransformationSP transform = cube_->GetComponent<Transformation>();
		CameraSP camera = cube_->GetComponent<Camera>();

		floatV3 position = floatV3(0, 0, -5);
		transform->SetPosition(position);
		transform->SetScaling(2);
		floatM44 orientation = Rotation(PI / 8, 0.0f, 1.0f, 0.0f);
		transform->SetOrientation(orientation);

		//transform->Rotate(PI / 4096, 1.0f, 0.5f, 0.25f);

// 		floatM44 translate = Translation(floatV3(0, 0, -5));
// 		floatM44 rotation = Rotation(0 * PI / 4, floatV3(0.5, 0.5, 0.5));
		floatV3 eye = floatV3(0.5, 1.0, 0.0);
		floatV3 at = position;
		floatV3 up = floatV3(0.0, 1.0, 0.0);
		floatM44 view = LookAt(eye, at, up);
		//view = Translation(eye) * RotationFromTo(floatV3(0, 0, -1), (at - eye)).Inverse() * Translation(-eye);
		floatM44 projection = Frustum(PI / 4, 6.0f / 5.0f, 1.0f, 10.0f);
// 		wMatrix->SetValue(translate * rotation);
		transform->Update();
		floatM44 model = transform->GetModelMatrix();
		floatV3 temp = Transform(orientation, floatV3(0, 0, -1));
		model = FaceTo(position, temp + position, up) * Scaling(transform->GetScaling()); // TODO finish this, seems not correct. how to do FaceTo?
		wMatrix->SetValue(model);
		vMatrix->SetValue(view);
		pMatrix->SetValue(projection);

		effect->GetPass(0)->Bind();
		//gl::UseProgram(1);

		vector<Renderable::LayoutAndEffect> const & layouts = mesh_->GetLayoutsAndEffects();
		layouts[0].layout->BindToProgram(*effect->GetPass(0)->GetProgram());


		int32 count = layouts[0].layout->GetElementCount();
		ElementType elementType = layouts[0].layout->GetIndexElementType();
		uint32 glType = GetGLType(elementType);
		assert(glType == gl::GL_UNSIGNED_SHORT);

		gl::DrawElements(gl::GL_TRIANGLES, count, GetGLType(elementType), reinterpret_cast<void const *>(0));


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

	Scene s;
	s.InitializeScene();

	Context::GetInstance().GetRenderingEngine().SetRenderingFunction(function<void(double)>(s));



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
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

struct DML
{
	~DML()
	{
		if (_CrtDumpMemoryLeaks())
		{
			cout << "memory leaks." << endl;
		}
		else
		{
			cout << "no memeory leaks." << endl;
		}
		cin.get();
	}
} _dml;