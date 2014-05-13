#include <XREXAll.hpp>

#include <CoreGL.hpp>

#include <iostream>
#include <numeric>
#include <random>

#include "../GLTutorialFramework.hpp"

#undef LoadString

using namespace std;

namespace
{
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

	GLuint CreateShaderObject(GLenum stage, string const& code)
	{
		// create a shader object
		GLuint shader = gl::CreateShader(stage);
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
		return shader;
	}

	GLuint CreateProgramObject(vector<GLuint> shaderObjects)
	{
		// create a GPU program
		GLuint program = gl::CreateProgram();
		// attach the compiled shader object
		for (GLuint shader : shaderObjects)
		{
			gl::AttachShader(program, shader);
		}
		// link program
		gl::LinkProgram(program);

		// request OpenGL to delete the shader object, it will be deleted when not used (not now)
		for (GLuint shader : shaderObjects)
		{
			gl::DeleteShader(shader);
		}

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

	GLuint CreateComputeShaderProgram(string const& code)
	{
		GLuint shader = CreateShaderObject(gl::GL_COMPUTE_SHADER, code);
		vector<GLuint> shaders;
		shaders.push_back(shader);
		GLuint program = CreateProgramObject(shaders);
		return program;
	}
}

struct GLComputeShader
	: TutorialFramework
{

	static GLuint const TimeBufferChannelIndex = 0;

	static GLuint const PositionBufferChannelIndex = 0;
	static GLuint const VelocityBufferChannelIndex = 1;
	static GLuint const NewPositionBufferChannelIndex = 2;
	static GLuint const NewVelocityBufferChannelIndex = 3;

	struct SimulationProgramMetadata
	{
		static GLuint const ObjectCount = 1024;
		//static GLuint const ObjectCount = 512;
		GLint timeBufferSizeInBytes;
		GLint positionBufferSizeInBytes;
		GLint velocityBufferSizeInBytes;
		GLint vector3Stride;
	};

	struct SimulationProgramResource
	{
		GLuint timeBuffer;
		GLuint positionBuffer;
		GLuint velocityBuffer;
		GLuint newPositionBuffer;
		GLuint newVelocityBuffer;
	};

	struct DisplayProgramMetadata
	{
		GLint positionInputIndex;
		GLint finalColorOutputIndex;
	};

	struct DisplayProgramResource
	{
		GLuint vertexAttributeBinding0;
		GLuint vertexAttributeBinding1;
	};

	GLComputeShader()
		: TutorialFramework(XREX::Size<XREX::uint32, 2>(800, 600))
	{
		XREX::XREXContext::GetInstance().GetResourceManager().AddResourceLocation("GLTutorials/GLNBodySimulation");
	}

	virtual void Initialize() override
	{
		shared_ptr<string> simulationCode = LoadStringFromFile("NBodySimulation.glsl");
		simulationProgramID = CreateComputeShaderProgram(*simulationCode);
		simulationProgramMetadata = GetSimulationProgramResourceMetadata(simulationProgramID);
		simulationProgramResource = CreateSimulationProgramResource(simulationProgramMetadata);
		GenerateInitialData(simulationProgramMetadata, simulationProgramResource);

		shared_ptr<string> displayVSCode = LoadStringFromFile("NBodyDisplay.vs.glsl");
		shared_ptr<string> displayFSCode = LoadStringFromFile("NBodyDisplay.fs.glsl");
		GLuint vsShader = CreateShaderObject(gl::GL_VERTEX_SHADER, *displayVSCode);
		GLuint fsShader = CreateShaderObject(gl::GL_FRAGMENT_SHADER, *displayFSCode);
		vector<GLuint> shaders;
		shaders.push_back(vsShader);
		shaders.push_back(fsShader);
		displayProgramID = CreateProgramObject(shaders);
		displayProgramMetadata = GetDisplayProgramResourceMetadata(displayProgramID);
		displayProgramResource = CreateDisplayProgramResource(simulationProgramMetadata, displayProgramMetadata, simulationProgramResource);
	}

	virtual bool Update(double current, double delta) override
	{
		this->delta = float(delta);
		return true;
	}

	virtual void Render() override
	{

		SimulationStep();
		DisplayStep();

		// ping-pong
		swap(simulationProgramResource.positionBuffer, simulationProgramResource.newPositionBuffer);
		swap(simulationProgramResource.velocityBuffer, simulationProgramResource.newVelocityBuffer);

		swap(displayProgramResource.vertexAttributeBinding0, displayProgramResource.vertexAttributeBinding1);
	}

	void SimulationStep()
	{
		gl::UseProgram(simulationProgramID);

		// update delta in TimeBuffer
		gl::BindBuffer(gl::GL_COPY_WRITE_BUFFER, simulationProgramResource.timeBuffer);
		float* mappedMemory = static_cast<float*>(gl::MapBuffer(gl::GL_COPY_WRITE_BUFFER, gl::GL_WRITE_ONLY));
		*mappedMemory = delta;
		GLboolean succeed = gl::UnmapBuffer(gl::GL_COPY_WRITE_BUFFER);
		assert(succeed == gl::GL_TRUE);

		// bind all resources the program need to the pipeline
		gl::BindBufferBase(gl::GL_UNIFORM_BUFFER, TimeBufferChannelIndex, simulationProgramResource.timeBuffer);
		gl::BindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, PositionBufferChannelIndex, simulationProgramResource.positionBuffer);
		gl::BindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, VelocityBufferChannelIndex, simulationProgramResource.velocityBuffer);
		gl::BindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, NewPositionBufferChannelIndex, simulationProgramResource.newPositionBuffer);
		gl::BindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, NewVelocityBufferChannelIndex, simulationProgramResource.newVelocityBuffer);

		gl::DispatchCompute(SimulationProgramMetadata::ObjectCount, 1, 1);

		gl::MemoryBarrier(gl::GL_SHADER_STORAGE_BARRIER_BIT);
	}

	void DisplayStep()
	{

		gl::ColorMask(true, true, true, true);
		array<float, 4> const ClearColor = {0.01f, 0.01f, 0.01f, 1.f};
		gl::ClearBufferfv(gl::GL_COLOR, 0, ClearColor.data());

		gl::UseProgram(displayProgramID);

		gl::ValidateProgram(displayProgramID);

		GLint validated;
		gl::GetProgramiv(displayProgramID, gl::GL_VALIDATE_STATUS, &validated);
		if (validated == gl::GL_FALSE)
		{
			GLint length = 0;
			gl::GetProgramiv(displayProgramID, gl::GL_INFO_LOG_LENGTH, &length);
			if (length > 0)
			{
				string errorString;
				errorString.resize(length, 0);
				gl::GetProgramInfoLog(displayProgramID, length, &length, &errorString[0]);
				cout << errorString << endl;
			}
		}

		gl::Disable(gl::GL_DEPTH_TEST);

		// use additive blending
		gl::Enable(gl::GL_BLEND);
		gl::BlendEquation(gl::GL_FUNC_ADD);
		gl::BlendFunc(gl::GL_ONE, gl::GL_ONE);

		// bind input attribute
		gl::BindVertexArray(displayProgramResource.vertexAttributeBinding0);

		gl::DrawArrays(gl::GL_POINTS, 0, SimulationProgramMetadata::ObjectCount);

		gl::BindVertexArray(0);
	}

	SimulationProgramMetadata GetSimulationProgramResourceMetadata(GLuint program)
	{
		SimulationProgramMetadata programMetadata;

		GLint uniformBufferCount;
		gl::GetProgramInterfaceiv(program, gl::GL_UNIFORM_BLOCK, gl::GL_ACTIVE_RESOURCES, &uniformBufferCount);
		assert(uniformBufferCount == 1);
		GLint timeBufferLocation = gl::GetProgramResourceIndex(program, gl::GL_UNIFORM_BLOCK, "TimeBuffer");
		assert(timeBufferLocation != gl::GL_INVALID_INDEX);
		gl::UniformBlockBinding(program, timeBufferLocation, TimeBufferChannelIndex);


		GLint shaderStorageBufferCount;
		gl::GetProgramInterfaceiv(program, gl::GL_SHADER_STORAGE_BLOCK, gl::GL_ACTIVE_RESOURCES, &shaderStorageBufferCount);
		assert(shaderStorageBufferCount == 4);
		GLint positionBufferLocation = gl::GetProgramResourceIndex(program, gl::GL_SHADER_STORAGE_BLOCK, "PositionBuffer");
		assert(positionBufferLocation != gl::GL_INVALID_INDEX);
		GLint velocityBufferLocation = gl::GetProgramResourceIndex(program, gl::GL_SHADER_STORAGE_BLOCK, "VelocityBuffer");
		assert(velocityBufferLocation != gl::GL_INVALID_INDEX);
		GLint newPositionBufferLocation = gl::GetProgramResourceIndex(program, gl::GL_SHADER_STORAGE_BLOCK, "NewPositionBuffer");
		assert(newPositionBufferLocation != gl::GL_INVALID_INDEX);
		GLint newVelocityBufferLocation = gl::GetProgramResourceIndex(program, gl::GL_SHADER_STORAGE_BLOCK, "NewVelocityBuffer");
		assert(newVelocityBufferLocation != gl::GL_INVALID_INDEX);

		gl::ShaderStorageBlockBinding(program, positionBufferLocation, PositionBufferChannelIndex);
		gl::ShaderStorageBlockBinding(program, velocityBufferLocation, VelocityBufferChannelIndex);
		gl::ShaderStorageBlockBinding(program, newPositionBufferLocation, NewPositionBufferChannelIndex);
		gl::ShaderStorageBlockBinding(program, newVelocityBufferLocation, NewVelocityBufferChannelIndex);

		array<GLenum, 1> const BufferProperties = { gl::GL_BUFFER_DATA_SIZE };

		GLint bufferSize;

		gl::GetProgramResourceiv(program, gl::GL_UNIFORM_BLOCK, timeBufferLocation, BufferProperties.size(), BufferProperties.data(), 1, nullptr, &bufferSize);
		programMetadata.timeBufferSizeInBytes = bufferSize;
		gl::GetProgramResourceiv(program, gl::GL_SHADER_STORAGE_BLOCK, positionBufferLocation, BufferProperties.size(), BufferProperties.data(), 1, nullptr, &bufferSize);
		programMetadata.positionBufferSizeInBytes = bufferSize;
		gl::GetProgramResourceiv(program, gl::GL_SHADER_STORAGE_BLOCK, velocityBufferLocation, BufferProperties.size(), BufferProperties.data(), 1, nullptr, &bufferSize);
		programMetadata.velocityBufferSizeInBytes = bufferSize;
		gl::GetProgramResourceiv(program, gl::GL_SHADER_STORAGE_BLOCK, newPositionBufferLocation, BufferProperties.size(), BufferProperties.data(), 1, nullptr, &bufferSize);
		assert(programMetadata.positionBufferSizeInBytes == bufferSize); // PositionBuffer and NewPositionBuffer should have same size
		gl::GetProgramResourceiv(program, gl::GL_SHADER_STORAGE_BLOCK, newVelocityBufferLocation, BufferProperties.size(), BufferProperties.data(), 1, nullptr, &bufferSize);
		assert(programMetadata.velocityBufferSizeInBytes == bufferSize); // VelocityBuffer and NewVelocityBuffer should have same size


		array<GLenum, 2> const VariableProperties = { gl::GL_OFFSET, gl::GL_ARRAY_STRIDE };

		GLint deltaLocation = gl::GetProgramResourceIndex(program, gl::GL_UNIFORM, "TimeBuffer.delta");
		GLint timeDeltaOffset;
		gl::GetProgramResourceiv(program, gl::GL_UNIFORM, deltaLocation, VariableProperties.size(), VariableProperties.data(), 1, nullptr, &timeDeltaOffset);
		assert(timeDeltaOffset == 0);

		array<GLint, 2> metadata;

		GLint positionLocation = gl::GetProgramResourceIndex(program, gl::GL_BUFFER_VARIABLE, "PositionBuffer.position[0]");
		gl::GetProgramResourceiv(program, gl::GL_BUFFER_VARIABLE, positionLocation, VariableProperties.size(), VariableProperties.data(), 2, nullptr, &metadata[0]);
		assert(metadata[0] == 0); // the only element should be positioned at the start of the buffer
		programMetadata.vector3Stride = metadata[1]; // save stride value
		GLint velocityLocation = gl::GetProgramResourceIndex(program, gl::GL_BUFFER_VARIABLE, "VelocityBuffer.velocity[0]");
		gl::GetProgramResourceiv(program, gl::GL_BUFFER_VARIABLE, velocityLocation, VariableProperties.size(), VariableProperties.data(), 2, nullptr, &metadata[0]);
		assert(metadata[0] == 0);
		assert(programMetadata.vector3Stride == metadata[1]); // all vec3 should have same element stride
		GLint newPositionLocation = gl::GetProgramResourceIndex(program, gl::GL_BUFFER_VARIABLE, "NewPositionBuffer.position[0]");
		gl::GetProgramResourceiv(program, gl::GL_BUFFER_VARIABLE, newPositionLocation, VariableProperties.size(), VariableProperties.data(), 2, nullptr, &metadata[0]);
		assert(metadata[0] == 0);
		assert(programMetadata.vector3Stride == metadata[1]);
		GLint newVelocityLocation = gl::GetProgramResourceIndex(program, gl::GL_BUFFER_VARIABLE, "VelocityBuffer.velocity[0]");
		gl::GetProgramResourceiv(program, gl::GL_BUFFER_VARIABLE, newVelocityLocation, VariableProperties.size(), VariableProperties.data(), 2, nullptr, &metadata[0]);
		assert(metadata[0] == 0);
		assert(programMetadata.vector3Stride == metadata[1]);

		return programMetadata;
	}

	// create resource buffer only
	SimulationProgramResource CreateSimulationProgramResource(SimulationProgramMetadata const& metadata)
	{
		SimulationProgramResource resource;

		gl::GenBuffers(1, &resource.timeBuffer);
		gl::GenBuffers(1, &resource.positionBuffer);
		gl::GenBuffers(1, &resource.velocityBuffer);
		gl::GenBuffers(1, &resource.newPositionBuffer);
		gl::GenBuffers(1, &resource.newVelocityBuffer);

		gl::BindBuffer(gl::GL_COPY_WRITE_BUFFER, resource.timeBuffer);
		gl::BufferData(gl::GL_COPY_WRITE_BUFFER, metadata.timeBufferSizeInBytes, nullptr, gl::GL_DYNAMIC_DRAW);

		gl::BindBuffer(gl::GL_COPY_WRITE_BUFFER, resource.positionBuffer);
		gl::BufferData(gl::GL_COPY_WRITE_BUFFER, metadata.positionBufferSizeInBytes, nullptr, gl::GL_DYNAMIC_COPY);
		gl::BindBuffer(gl::GL_COPY_WRITE_BUFFER, resource.velocityBuffer);
		gl::BufferData(gl::GL_COPY_WRITE_BUFFER, metadata.velocityBufferSizeInBytes, nullptr, gl::GL_DYNAMIC_COPY);
		gl::BindBuffer(gl::GL_COPY_WRITE_BUFFER, resource.newPositionBuffer);
		gl::BufferData(gl::GL_COPY_WRITE_BUFFER, metadata.positionBufferSizeInBytes, nullptr, gl::GL_DYNAMIC_COPY);
		gl::BindBuffer(gl::GL_COPY_WRITE_BUFFER, resource.newVelocityBuffer);
		gl::BufferData(gl::GL_COPY_WRITE_BUFFER, metadata.velocityBufferSizeInBytes, nullptr, gl::GL_DYNAMIC_COPY);

		return resource;
	}

	// generate initial data for PositionBuffer and VelocityBuffer
	void GenerateInitialData(SimulationProgramMetadata const& metadata, SimulationProgramResource const& resource)
	{
		GLuint objectCount = SimulationProgramMetadata::ObjectCount;
		GLint stride = metadata.vector3Stride;

		struct Vector3
		{
			float x;
			float y;
			float z;
			Vector3()
			{
			}
			Vector3(float x, float y, float z)
				: x(x), y(y), z(z)
			{
			}
			float Length()
			{
				return sqrt(x * x + y * y + z * z);
			}
		};

		// we need a random generator
		random_device rd;
		mt19937 generator(rd());
		std::uniform_real_distribution<float> distribution(-1, 1);


		array<Vector3, SimulationProgramMetadata::ObjectCount> initialPositions;
		// generate uniformly distributed random points in unit sphere
		for (Vector3& position : initialPositions)
		{
			do
			{
				position.x = distribution(generator);
				position.y = distribution(generator);
				position.z = distribution(generator);
			}
			while (position.Length() > 1); // until in the unit sphere
		}

		gl::BindBuffer(gl::GL_COPY_WRITE_BUFFER, resource.positionBuffer);
		unsigned char* positionPointer = static_cast<unsigned char*>(gl::MapBuffer(gl::GL_COPY_WRITE_BUFFER, gl::GL_WRITE_ONLY));
		for (int i = 0; i < SimulationProgramMetadata::ObjectCount; ++i)
		{
			reinterpret_cast<Vector3&>(*positionPointer) = initialPositions[i];
			positionPointer += metadata.vector3Stride;
		}
		gl::UnmapBuffer(gl::GL_COPY_WRITE_BUFFER);

		array<Vector3, SimulationProgramMetadata::ObjectCount> initialVelocities;
		// set all initial velocities to 0
		for (Vector3& velocity : initialVelocities)
		{
			velocity = Vector3(0, 0, 0);
		}

		gl::BindBuffer(gl::GL_COPY_WRITE_BUFFER, resource.velocityBuffer);
		unsigned char* velocityPointer = static_cast<unsigned char*>(gl::MapBuffer(gl::GL_COPY_WRITE_BUFFER, gl::GL_WRITE_ONLY));
		for (int i = 0; i < SimulationProgramMetadata::ObjectCount; ++i)
		{
			reinterpret_cast<Vector3&>(*velocityPointer) = initialVelocities[i];
			velocityPointer += metadata.vector3Stride;
		}
		gl::UnmapBuffer(gl::GL_COPY_WRITE_BUFFER);

	}

	DisplayProgramMetadata GetDisplayProgramResourceMetadata(GLuint program)
	{
		DisplayProgramMetadata programMetadata;

		GLint inputCount;
		gl::GetProgramInterfaceiv(program, gl::GL_PROGRAM_INPUT, gl::GL_ACTIVE_RESOURCES, &inputCount);
		assert(inputCount == 1);
		GLint positionInputLocation = gl::GetProgramResourceIndex(program, gl::GL_PROGRAM_INPUT, "position");
		assert(positionInputLocation != gl::GL_INVALID_INDEX);

		GLint outputCount;
		gl::GetProgramInterfaceiv(program, gl::GL_PROGRAM_OUTPUT, gl::GL_ACTIVE_RESOURCES, &outputCount);
		assert(outputCount == 1);
		GLint finalColorOutputLocation = gl::GetProgramResourceIndex(program, gl::GL_PROGRAM_OUTPUT, "finalColor");
		assert(finalColorOutputLocation != gl::GL_INVALID_INDEX);


		array<GLenum, 1> const InputOutputProperties = { gl::GL_LOCATION };

		GLint positionChannelIndex;
		gl::GetProgramResourceiv(program, gl::GL_PROGRAM_INPUT, positionInputLocation, InputOutputProperties.size(), InputOutputProperties.data(), 1, nullptr, &positionChannelIndex);
		programMetadata.positionInputIndex = positionChannelIndex;

		GLint finalColorChannelIndex;
		gl::GetProgramResourceiv(program, gl::GL_PROGRAM_OUTPUT, finalColorOutputLocation, InputOutputProperties.size(), InputOutputProperties.data(), 1, nullptr, &finalColorChannelIndex);
		programMetadata.finalColorOutputIndex = finalColorChannelIndex;

		return programMetadata;
	}

	DisplayProgramResource CreateDisplayProgramResource(SimulationProgramMetadata const& simulationMetadata, DisplayProgramMetadata const& metadata, SimulationProgramResource const& simulationResource)
	{
		DisplayProgramResource resource;

		gl::GenVertexArrays(1, &resource.vertexAttributeBinding0);
		gl::GenVertexArrays(1, &resource.vertexAttributeBinding1);

		// bind vertex array to record binding information
		gl::BindVertexArray(resource.vertexAttributeBinding0);

		gl::BindBuffer(gl::GL_ARRAY_BUFFER, simulationResource.positionBuffer);

		// enable attribute input
		gl::EnableVertexAttribArray(metadata.positionInputIndex);
		// specify the input layout
		gl::VertexAttribPointer(metadata.positionInputIndex, /*vec3 has 3 component*/3, gl::GL_FLOAT, gl::GL_FALSE, simulationMetadata.vector3Stride, reinterpret_cast<void*>(0));

		// unbind to stop recording
		gl::BindVertexArray(0);

		// bind vertex array to record binding information
		gl::BindVertexArray(resource.vertexAttributeBinding1);

		gl::BindBuffer(gl::GL_ARRAY_BUFFER, simulationResource.newPositionBuffer);

		// enable attribute input
		gl::EnableVertexAttribArray(metadata.positionInputIndex);
		// specify the input layout
		gl::VertexAttribPointer(metadata.positionInputIndex, /*vec3 has 3 component*/3, gl::GL_FLOAT, gl::GL_FALSE, simulationMetadata.vector3Stride, reinterpret_cast<void*>(0));

		// unbind to stop recording
		gl::BindVertexArray(0);

		return resource;
	}

	GLuint simulationProgramID;
	SimulationProgramMetadata simulationProgramMetadata;
	SimulationProgramResource simulationProgramResource;

	GLuint displayProgramID;
	DisplayProgramMetadata displayProgramMetadata;
	DisplayProgramResource displayProgramResource;

	float delta;

};

int main()
{
	GLComputeShader shader;

	shader.Start();

	return 0;
}

#include "../MemoryLeakCheck.hpp" // include to enable memory leak checking