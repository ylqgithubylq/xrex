#include <XREXAll.hpp>

#include <CoreGL.hpp>

#include <iostream>
#include <numeric>
#include <random>

#include "../GLTutorialFramework.hpp"

#undef LoadString

using namespace std;


struct GLComputeShader
	: TutorialFramework
{

	GLComputeShader()
		: TutorialFramework(XREX::Size<XREX::uint32, 2>(800, 600))
	{
		XREX::XREXContext::GetInstance().GetResourceManager().AddResourceLocation("GLTutorials/GLComputeShader");
	}

	virtual void Initialize() override
	{
		enum class ShowCase
		{
			MoveObjects,
		};

		ShowCase showCase = ShowCase::MoveObjects; // change this to select a sample

		switch (showCase)
		{
		case ShowCase::MoveObjects:
			MoveObjects();
			break;

		default:
			assert(false);
			break;
		}
	}

	virtual bool Update(double current, double delta) override
	{
		return false; // stop the main loop
	}

	virtual void Render() override
	{
	}

	shared_ptr<string> LoadStringFromFile(string fileName)
	{
		// create your own implementation~
		string fullPath;
		if (XREX::XREXContext::GetInstance().GetResourceManager().LocatePath(fileName, &fullPath))
		{
			return XREX::XREXContext::GetInstance().GetResourceLoader().LoadString(fullPath);
		}
		else
		{
			return nullptr;
		}
	}

	GLuint CreateComputeShaderProgram(string const& code)
	{
		// create a shader object
		GLuint shader = gl::CreateShader(gl::GL_COMPUTE_SHADER);
		char const* sources[] = { code.c_str() };
		// provide the shader source, the third parameter is an array of char arrays
		gl::ShaderSource(shader, 1, sources, nullptr);
		// compile it
		gl::CompileShader(shader);

		// check if compile success
		GLint compiled = 0;
		gl::GetShaderiv(shader, gl::GL_COMPILE_STATUS, &compiled);

		if (compiled != gl::GL_TRUE)
		{
			// if have error in it, get the error message
			GLint length = 0;
			gl::GetShaderiv(shader, gl::GL_INFO_LOG_LENGTH, &length);
			string errorString;
			if (length > 0)
			{
				errorString.resize(length, 0);
				gl::GetShaderInfoLog(shader, length, &length, &errorString[0]);
			}
			else
			{
				errorString = "Unknown compiling error.";
			}
			cout << errorString << endl;
		}

		// create a GPU program
		GLuint program = gl::CreateProgram();
		// attach the compiled shader object
		gl::AttachShader(program, shader);
		// link program
		gl::LinkProgram(program);

		// request OpenGL to delete the shader object, it will be deleted when not used (not now)
		gl::DeleteShader(shader);

		// check if link success
		GLint linked = 0;
		gl::GetProgramiv(program, gl::GL_LINK_STATUS, &linked);

		if (linked != gl::GL_TRUE)
		{
			// if have error in it, get the error message
			GLint length = 0;
			gl::GetProgramiv(program, gl::GL_INFO_LOG_LENGTH, &length);
			string errorString;
			if (length > 0)
			{
				errorString.resize(length, 0);
				gl::GetProgramInfoLog(program, length, &length, &errorString[0]);
			}
			else
			{
				errorString = "Unknown linking error.";
			}
			cout << errorString << endl;
		}
		return program;
	}

	void MoveObjects()
	{
		shared_ptr<string> code = LoadStringFromFile("MoveObjects.glsl");
		GLuint program = CreateComputeShaderProgram(*code);

		// specify all the resource binding channels
		// binding channel index of image is specified using glProgramUniform
		GLint const PositionAndTimeChannelIndex = 0;
		GLint const VelocityAndDeltaChannelIndex = 1;
		GLint const NewPositionAndTimeChannelIndex = 2;

		GLint positionAndTimeLocation = gl::GetUniformLocation(program, "PositionAndTime");
		assert(positionAndTimeLocation != -1);
		gl::ProgramUniform1i(program, positionAndTimeLocation, PositionAndTimeChannelIndex);

		GLint velocityAndDeltaLocation = gl::GetUniformLocation(program, "VelocityAndDelta");
		assert(velocityAndDeltaLocation != -1);
		gl::ProgramUniform1i(program, velocityAndDeltaLocation, VelocityAndDeltaChannelIndex);

		GLint newPositionAndTimeLocation = gl::GetUniformLocation(program, "NewPositionAndTime");
		assert(velocityAndDeltaLocation != -1);
		gl::ProgramUniform1i(program, newPositionAndTimeLocation, NewPositionAndTimeChannelIndex);


		struct Vector4
		{
			float x;
			float y;
			float z;
			float w;
			Vector4()
			{ // do nothing
			}
			Vector4(float x, float y, float z, float w)
				: x(x), y(y), z(z), w(w)
			{
			}
			float Length()
			{
				return sqrt(x * x + y * y + z * z + w * w);
			}
		};

		// we need a random generator
		random_device rd;
		mt19937 generator(rd());
		std::uniform_real_distribution<float> distribution(-1, 1);
		for (int n = 0; n < 10; ++n) {
			std::cout << distribution(generator) << ' ';
		}

		GLuint const Count = 4096;

		// generate resources
		GLuint positionAndTime = 0;
		gl::GenTextures(1, &positionAndTime);
		GLuint velocityAndDelta = 0;
		gl::GenTextures(1, &velocityAndDelta);
		GLuint newPositionAndTime = 0;
		gl::GenTextures(1, &newPositionAndTime);

		// positionAndTime
		gl::BindTexture(gl::GL_TEXTURE_1D, positionAndTime);
		gl::TexImage1D(gl::GL_TEXTURE_1D, 0, gl::GL_RGBA32F, Count, 0, gl::GL_RGBA, gl::GL_FLOAT, nullptr);

		array<Vector4, Count> initialPositionAndTime; // points in unit sphere
		for (Vector4& object : initialPositionAndTime)
		{
			object.w = 0;
			do
			{
				object.x = distribution(generator);
				object.y = distribution(generator);
				object.z = distribution(generator);
			}
			while (object.Length() > 1); // until in the unit sphere
		}
		gl::TexSubImage1D(gl::GL_TEXTURE_1D, 0, 0, Count, gl::GL_RGBA, gl::GL_FLOAT, initialPositionAndTime.data());

		// velocityAndDelta
		gl::BindTexture(gl::GL_TEXTURE_1D, velocityAndDelta);
		gl::TexImage1D(gl::GL_TEXTURE_1D, 0, gl::GL_RGBA32F, Count, 0, gl::GL_RGBA, gl::GL_FLOAT, nullptr);

		array<Vector4, Count> initialVelocityAndDelta; // points in unit sphere
		for (Vector4& velocity : initialVelocityAndDelta)
		{
			velocity.w = 0;
			do
			{
				velocity.x = distribution(generator);
				velocity.y = distribution(generator);
				velocity.z = distribution(generator);
			}
			while (velocity.Length() > 1); // until in the unit sphere
			velocity.w = distribution(generator);
			if (velocity.w < 0) // make it positive
			{
				velocity.w = -velocity.w;
			}
		}
		gl::TexSubImage1D(gl::GL_TEXTURE_1D, 0, 0, Count, gl::GL_RGBA, gl::GL_FLOAT, initialVelocityAndDelta.data());

		// newPositionAndTime
		gl::BindTexture(gl::GL_TEXTURE_1D, newPositionAndTime);
		gl::TexImage1D(gl::GL_TEXTURE_1D, 0, gl::GL_RGBA32F, Count, 0, gl::GL_RGBA, gl::GL_FLOAT, nullptr);


		gl::UseProgram(program);

		// bind all resources the program need to the pipeline
		gl::BindImageTexture(PositionAndTimeChannelIndex, positionAndTime, 0, true, 0, gl::GL_READ_ONLY, gl::GL_RGBA32F);
		gl::BindImageTexture(VelocityAndDeltaChannelIndex, velocityAndDelta, 0, true, 0, gl::GL_READ_ONLY, gl::GL_RGBA32F);
		gl::BindImageTexture(NewPositionAndTimeChannelIndex, newPositionAndTime, 0, true, 0, gl::GL_WRITE_ONLY, gl::GL_RGBA32F);

		// launch compute work
		gl::DispatchCompute(Count, 1, 1);

		// work done, have look at results
		array<Vector4, Count> result;
		gl::BindTexture(gl::GL_TEXTURE_1D, newPositionAndTime);
		gl::GetTexImage(gl::GL_TEXTURE_1D, 0, gl::GL_RGBA, gl::GL_FLOAT, &result);
		for (Vector4& newPosition : result)
		{
			// do whatever you want with it
		}

		// don't forget to delete resources
		gl::DeleteTextures(1, &positionAndTime);
		gl::DeleteTextures(1, &velocityAndDelta);
		gl::DeleteTextures(1, &newPositionAndTime);
		// and the shader program
		gl::DeleteProgram(program);
	}

};

int main()
{
	GLComputeShader shader;

	shader.Start();

	return 0;
}

#include "../MemoryLeakCheck.hpp" // include to enable memory leak checking