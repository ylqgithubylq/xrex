#include <XREXAll.hpp>

#include <CoreGL.hpp>

#include <iostream>
#include <numeric>

#include "../GLTutorialFramework.hpp"

#undef LoadString

using namespace std;


struct GLBufferResource
	: TutorialFramework
{

	GLBufferResource()
		: TutorialFramework(XREX::Size<XREX::uint32, 2>(800, 600))
	{
		XREX::XREXContext::GetInstance().GetResourceManager().AddResourceLocation("GLTutorials/GLBufferResource");
	}

	virtual void Initialize() override
	{
		enum class ShowCase
		{
			BufferDefinitionAndBinding,
		};

		ShowCase showCase = ShowCase::BufferDefinitionAndBinding; // change this to select a sample

		switch (showCase)
		{
		case ShowCase::BufferDefinitionAndBinding:
			BufferDefinitionAndBinding();
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
		array<char const*, 1> sources = { code.c_str() };
		// provide the shader source, the third parameter is an array of char arrays
		gl::ShaderSource(shader, 1, sources.data(), nullptr);
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

	void BufferDefinitionAndBinding()
	{
		shared_ptr<string> code = LoadStringFromFile("Buffer.glsl");
		GLuint program = CreateComputeShaderProgram(*code);

		// specify the resource binding channels for this program first

		// uniform buffers, link will set all of these binding to 0 so these need to specified after linking.
		GLuint const UniformBufferChannelIndex = 0;
		GLint uniformBufferCount;
		gl::GetProgramInterfaceiv(program, gl::GL_UNIFORM_BLOCK, gl::GL_ACTIVE_RESOURCES, &uniformBufferCount);
		assert(uniformBufferCount == 1);
		// location means nth uniform block in this shader
		GLint uniformBufferLocation = gl::GetProgramResourceIndex(program, gl::GL_UNIFORM_BLOCK, "MyUniformBuffer");
		//GLint uniformBufferLocation = gl::GetUniformBlockIndex(program, "MyUniformBuffer");
		if (uniformBufferLocation != gl::GL_INVALID_INDEX)
		{
			// unspecified binding buffer will be all 0
			gl::UniformBlockBinding(program, uniformBufferLocation, UniformBufferChannelIndex);
		}
		else
		{
			assert(false);
		}

		// shader storage buffers, link will set all of these binding to 0 so these need to specified after linking.
		GLuint const ShaderStorageBufferChannelIndex = 0;
		GLint shaderStorageBufferCount;
		gl::GetProgramInterfaceiv(program, gl::GL_SHADER_STORAGE_BLOCK, gl::GL_ACTIVE_RESOURCES, &shaderStorageBufferCount);
		assert(shaderStorageBufferCount == 1);
		// location means nth shader storage block in this shader
		GLint shaderStorageBufferLocation = gl::GetProgramResourceIndex(program, gl::GL_SHADER_STORAGE_BLOCK, "MyShaderStorageBuffer");
		if (shaderStorageBufferLocation != gl::GL_INVALID_INDEX)
		{
			// unspecified binding buffer will be all 0
			gl::ShaderStorageBlockBinding(program, shaderStorageBufferLocation, ShaderStorageBufferChannelIndex);
		}
		else
		{
			assert(false);
		}

		// retrieve the metadata of these buffers
		// define a struct to hold the metadata
		struct BufferVariableMetadata
		{
			string name;
			GLenum type;
			GLint offset;
			GLsizei elementCount;
			GLsizei elementStride; // bytes from the beginning of one element to the beginning of the next element
			BufferVariableMetadata(string name, GLenum type, GLint offset)
				: name(move(name)), type(type), offset(offset), elementCount(1), elementStride(0)
			{
			}
			BufferVariableMetadata(string name, GLenum type, GLint offset, GLsizei elementCount, GLsizei elementStride)
				: name(move(name)), type(type), offset(offset), elementCount(elementCount), elementStride(elementStride)
			{
			}
		};


		// the uniform buffer is defined as:
		//	uniform MyUniformBuffer
		//	{
		//		float someFloat;
		//		vec4 someVector4;
		//	}
		
		// get uniform variable count in the uniform buffer
		array<GLint, 2> uniformBufferProperties;
		GLint const& uniformBufferSizeInBytes = uniformBufferProperties[0];
		GLint const& uniformBufferVariableCount = uniformBufferProperties[1];
		array<GLenum, 2> const UniformBufferProperties = { gl::GL_BUFFER_DATA_SIZE, gl::GL_NUM_ACTIVE_VARIABLES };
		gl::GetProgramResourceiv(program, gl::GL_UNIFORM_BLOCK, uniformBufferLocation, UniformBufferProperties.size(), UniformBufferProperties.data(),
			uniformBufferProperties.size(), nullptr, &uniformBufferProperties[0]);
		assert(uniformBufferProperties[1] == 2);

		// get uniform variable indices in the uniform buffer
		vector<GLint> uniformVariableIndices(uniformBufferVariableCount);
		array<GLenum, 1> const UniformBufferVariableList = { gl::GL_ACTIVE_VARIABLES };
		gl::GetProgramResourceiv(program, gl::GL_UNIFORM_BLOCK, uniformBufferLocation, UniformBufferVariableList.size(), UniformBufferVariableList.data(),
			uniformVariableIndices.size(), nullptr, &uniformVariableIndices[0]);

		// get uniform variable metadata values
		unordered_map<string, BufferVariableMetadata> uniformBufferVariables;

		array<GLint, 3> uniformVariableMetadata;
		array<GLenum, 3> const UniformVariableProperties = { gl::GL_NAME_LENGTH, gl::GL_TYPE, gl::GL_OFFSET };
		for (GLint index = 0; index < uniformBufferVariableCount; ++index)
		{
			gl::GetProgramResourceiv(program, gl::GL_UNIFORM, uniformVariableIndices[index], UniformVariableProperties.size(), UniformVariableProperties.data(),
				uniformVariableMetadata.size(), nullptr, &uniformVariableMetadata[0]);
			string nameBuffer;
			nameBuffer.resize(uniformVariableMetadata[0] + 1);
			gl::GetProgramResourceName(program, gl::GL_UNIFORM, uniformVariableIndices[index], nameBuffer.size(), nullptr, &nameBuffer[0]);
			// bug in NVidia GetProgramResourceiv that it's return value of GL_NAME_LENGTH is 1 smaller
			string name(nameBuffer.begin(), *(nameBuffer.end() - 2) == '\0' ? nameBuffer.end() - 2 : nameBuffer.end() - 1);
			uniformBufferVariables.insert(make_pair(name,
				BufferVariableMetadata(name, uniformVariableMetadata[1], uniformVariableMetadata[2])));
		}
		
		// the shader storage buffer is defined as:
		//	struct MyStruct
		//	{
		//		float structFloat;
		//	};
		//	buffer MyShaderStorageBuffer
		//	{
		//		ivec2[2] someIntVector2Array;
		//		int[2] someIntArray;
		//		MyStruct[] someFloatArray;
		//	};

		// get shader storage variable count in the shader storage buffer
		array<GLint, 2> shaderStorageBufferProperties;
		GLint const& shaderStorageBufferSizeInBytes = shaderStorageBufferProperties[0];
		GLint const& shaderStorageBufferVariableCount = shaderStorageBufferProperties[1];
		array<GLenum, 2> const ShaderStorageBufferProperties = { gl::GL_BUFFER_DATA_SIZE, gl::GL_NUM_ACTIVE_VARIABLES };
		gl::GetProgramResourceiv(program, gl::GL_SHADER_STORAGE_BLOCK, shaderStorageBufferLocation, ShaderStorageBufferProperties.size(), ShaderStorageBufferProperties.data(),
			shaderStorageBufferProperties.size(), nullptr, &shaderStorageBufferProperties[0]);
		assert(shaderStorageBufferProperties[1] == 3);

		// get shader storage variable indices in the shader storage buffer
		vector<GLint> shaderStorageVariableIndices(shaderStorageBufferSizeInBytes);
		array<GLenum, 1> const ShaderStorageBufferVariableList = { gl::GL_ACTIVE_VARIABLES };
		gl::GetProgramResourceiv(program, gl::GL_SHADER_STORAGE_BLOCK, shaderStorageBufferLocation, ShaderStorageBufferVariableList.size(), ShaderStorageBufferVariableList.data(),
			shaderStorageVariableIndices.size(), nullptr, &shaderStorageVariableIndices[0]);

		// get shader storage variable metadata values
		unordered_map<string, BufferVariableMetadata> shaderStorageBufferVariables;

		array<GLint, 5> shaderStorageVariableMetadata;
		array<GLenum, 5> ShaderStorageVariableProperties = { gl::GL_NAME_LENGTH, gl::GL_TYPE, gl::GL_OFFSET, gl::GL_ARRAY_SIZE, gl::GL_ARRAY_STRIDE };
		for (GLint index = 0; index < shaderStorageBufferSizeInBytes; ++index)
		{
			gl::GetProgramResourceiv(program, gl::GL_BUFFER_VARIABLE, shaderStorageVariableIndices[index], ShaderStorageVariableProperties.size(), ShaderStorageVariableProperties.data(),
				shaderStorageVariableMetadata.size(), nullptr, &shaderStorageVariableMetadata[0]);
			string nameBuffer;
			nameBuffer.resize(shaderStorageVariableMetadata[0] + 1);
			gl::GetProgramResourceName(program, gl::GL_BUFFER_VARIABLE, shaderStorageVariableIndices[index], nameBuffer.size(), nullptr, &nameBuffer[0]);
			// bug in NVidia GetProgramResourceiv that it's return value of GL_NAME_LENGTH is 1 smaller
			string name(nameBuffer.begin(), *(nameBuffer.end() - 2) == '\0' ? nameBuffer.end() - 2 : nameBuffer.end() - 1);
			shaderStorageBufferVariables.insert(make_pair(name,
				BufferVariableMetadata(name, shaderStorageVariableMetadata[1], shaderStorageVariableMetadata[2], shaderStorageVariableMetadata[3], shaderStorageVariableMetadata[4])));
		}
		// notice: the array size of MyShaderStorageBuffer.someFloatArray (the unsized array) will be 1 and its array stride will be 0


		// create resources for shader
		// uniform buffer
		GLuint uniformBuffer;
		gl::GenBuffers(1, &uniformBuffer);
		gl::BindBuffer(gl::GL_COPY_WRITE_BUFFER, uniformBuffer);
		gl::BufferData(gl::GL_COPY_WRITE_BUFFER, uniformBufferSizeInBytes, nullptr, gl::GL_STATIC_DRAW);

		float const FilledData = 100;

		vector<unsigned char> uniformBufferData(uniformBufferSizeInBytes);
		// assign value to uniformBufferData at offset of MyUniformBuffer.someFloat
		reinterpret_cast<float&>(uniformBufferData[uniformBufferVariables.at("MyUniformBuffer.someFloat").offset]) = FilledData;
		gl::BufferSubData(gl::GL_COPY_WRITE_BUFFER, 0, uniformBufferSizeInBytes, uniformBufferData.data());

		// shader storage buffer
		GLuint const ElementCountForLastArrayElement = 16;
		GLuint const MAGIC = 4; // don't ask why, it works for this example on NVidia, not work on AMD
		GLuint const ShaderStorageBufferSize = shaderStorageBufferSizeInBytes + ElementCountForLastArrayElement * sizeof(float) - MAGIC;
		GLuint shaderStorageBuffer;
		gl::GenBuffers(1, &shaderStorageBuffer);
		gl::BindBuffer(gl::GL_COPY_WRITE_BUFFER, shaderStorageBuffer);
		// last element in MyShaderStorageBuffer is a float array
		gl::BufferData(gl::GL_COPY_WRITE_BUFFER, ShaderStorageBufferSize, nullptr, gl::GL_DYNAMIC_COPY);

		
		// let the OpenGL pipeline use this program
		gl::UseProgram(program);

		// bind all resources the program need to the pipeline
		gl::BindBufferBase(gl::GL_UNIFORM_BUFFER, UniformBufferChannelIndex, uniformBuffer);
		gl::BindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, ShaderStorageBufferChannelIndex, shaderStorageBuffer);

		// launch compute work
		// parameters are group counts in each dimension, so this will launch (width * 1, 1 * 1, 1 * 1) GPU threads
		gl::DispatchCompute(ElementCountForLastArrayElement, 1, 1);

		// work done, check out what are in the shader storage buffer
		gl::BindBuffer(gl::GL_COPY_READ_BUFFER, shaderStorageBuffer);
		vector<unsigned char> retrievedData(ShaderStorageBufferSize);
		gl::GetBufferSubData(gl::GL_COPY_READ_BUFFER, 0, ShaderStorageBufferSize, &retrievedData[0]);

		BufferVariableMetadata const& someIntArrayMetadata = shaderStorageBufferVariables.at("MyShaderStorageBuffer.someIntArray[0]");
		BufferVariableMetadata const& someMyStructArrayMetadata = shaderStorageBufferVariables.at("MyShaderStorageBuffer.someMyStructArray[0].structUint");

		// make the result data easier to inspect
		array<int, 2> someIntArray = reinterpret_cast<array<int, 2>&>(retrievedData[someIntArrayMetadata.offset]);
		assert(someIntArray[0] = ElementCountForLastArrayElement);

		unsigned int* start = &reinterpret_cast<unsigned int&>(retrievedData[someMyStructArrayMetadata.offset]);

		assert(static_cast<unsigned int>(FilledData) == *start);

		// don't forget to delete resources
		gl::DeleteBuffers(1, &uniformBuffer);
		gl::DeleteBuffers(1, &shaderStorageBuffer);
		// and the shader program
		gl::DeleteProgram(program);
	}


};

int main()
{
	GLBufferResource bufferResource;

	bufferResource.Start();

	return 0;
}

#include "../MemoryLeakCheck.hpp" // include to enable memory leak checking