#include <XREXAll.hpp>

#include <CoreGL.hpp>

#include <iostream>
#include <numeric>

#include "../GLTutorialFramework.hpp"

#undef LoadString

using namespace std;


struct GLShader
	: TutorialFramework
{

	GLShader()
		: TutorialFramework(XREX::Size<XREX::uint32, 2>(800, 600))
	{
		XREX::XREXContext::GetInstance().GetResourceManager().AddResourceLocation("GLTutorials/GLShader");
	}

	virtual void Initialize() override
	{
		enum class ShowCase
		{
			CreateAndCompileAndUse,
		};

		ShowCase showCase = ShowCase::CreateAndCompileAndUse; // change this to select a sample

		switch (showCase)
		{
		case ShowCase::CreateAndCompileAndUse:
			CreateAndCompileAndUse();
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

	void CreateAndCompileAndUse()
	{
		string code = 
			"\n"
			"#version 430 core\n"
			"// input layout qualifier declaring a 1 x 1 (x 1) local work group size\n"
			"layout (local_size_x = 1, local_size_y = 1) in;\n"
			"\n"
			"// define the input and output resources\n"
			"// input resource is an image2D, with format r32f, and can only be read from\n"
			"layout (r32f) uniform readonly image2D inputData;\n"
			"// output resource is an image2D, with format r32f, and can only be write to\n"
			"layout (r32f) uniform writeonly image2D outputData;\n"
			"\n"
			"void main()\n"
			"{\n"
			"	// load from the input image\n"
			"	vec4 value = imageLoad(inputData, ivec2(gl_GlobalInvocationID.xy));\n"
			"	// store the value to output image\n"
			"	imageStore(outputData, ivec2(gl_GlobalInvocationID.xy), value);\n"
			"}\n"
			"\n";
		GLuint program = CreateComputeShaderProgram(code);

		// specify all the resource binding channels
		// binding channel index of image is specified using glProgramUniform
		GLint const inputDataChannelIndex = 0;
		GLint const outputDataChannelIndex = 1;
		GLint inputDataLocation = gl::GetUniformLocation(program, "inputData");
		if (inputDataLocation != -1) // note: not GL_INVALID_INDEX
		{
			// specify the binding channel index
			gl::ProgramUniform1i(program, inputDataLocation, inputDataChannelIndex);
		}
		else
		{
			assert(false); // something wrong
		}
		GLint outputDataLocation = gl::GetUniformLocation(program, "outputData");
		if (outputDataLocation != -1) // note: not GL_INVALID_INDEX
		{
			// specify the binding channel index
			gl::ProgramUniform1i(program, outputDataLocation, outputDataChannelIndex);
		}
		else
		{
			assert(false); // something wrong
		}

		GLuint const width = 2;
		GLuint const height = 2;

		// generate the input and output texture
		GLuint inputTexture = 0;
		gl::GenTextures(1, &inputTexture);
		GLuint outputTexture = 0;
		gl::GenTextures(1, &outputTexture);

		// generate and fill data into the input texture
		gl::BindTexture(gl::GL_TEXTURE_2D, inputTexture);
		gl::TexImage2D(gl::GL_TEXTURE_2D, 0, gl::GL_R32F, width, height, 0, gl::GL_RED, gl::GL_FLOAT, nullptr);
		array<float, 4> input = { 1.0f, 2.0f, 3.0f, 4.0f };
		gl::TexSubImage2D(gl::GL_TEXTURE_2D, 0, 0, 0, width, height, gl::GL_RED, gl::GL_FLOAT, &input);

		// generate the output texture
		gl::BindTexture(gl::GL_TEXTURE_2D, outputTexture);
		gl::TexImage2D(gl::GL_TEXTURE_2D, 0, gl::GL_R32F, width, height, 0, gl::GL_RED, gl::GL_FLOAT, nullptr);


		// let the OpenGL pipeline use this program
		gl::UseProgram(program);

		// bind all resources the program need to the pipeline
		// inputData is an image2D, using the level 0 of inputTexture as the input image
		gl::BindImageTexture(inputDataChannelIndex, inputTexture, 0, true, 0, gl::GL_READ_ONLY, gl::GL_R32F);
		// outputData is an image2D, using the level 0 of outputTexture as the output image
		gl::BindImageTexture(outputDataChannelIndex, outputTexture, 0, true, 0, gl::GL_WRITE_ONLY, gl::GL_R32F);

		// launch compute work
		// parameters are group counts in each dimension, so this will launch (width * 1, height * 1, 1 * 1) GPU threads
		gl::DispatchCompute(width, height, 1);

		// work done, check whether the compute shader fills the outputData
		array<float, 4> result;
		gl::BindTexture(gl::GL_TEXTURE_2D, outputTexture);
		gl::GetTexImage(gl::GL_TEXTURE_2D, 0, gl::GL_RED, gl::GL_FLOAT, &result);
		if (result != input)
		{
			assert(false); // something wrong
		}

		// don't forget to delete resources
		gl::DeleteTextures(1, &inputTexture);
		gl::DeleteTextures(1, &outputTexture);
		// and the shader program
		gl::DeleteProgram(program);
	}


};

int main()
{
	GLShader shader;

	shader.Start();

	return 0;
}

#include "../MemoryLeakCheck.hpp" // include to enable memory leak checking