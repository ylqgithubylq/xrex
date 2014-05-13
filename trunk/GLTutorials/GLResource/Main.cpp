#include <XREXAll.hpp>

#include <CoreGL.hpp>

#include <iostream>
#include <numeric>

#include "../GLTutorialFramework.hpp"



using namespace std;


struct Resource
	: TutorialFramework
{

	Resource()
		: TutorialFramework(XREX::Size<XREX::uint32, 2>(800, 600))
	{
	}

	virtual void Initialize() override
	{
		enum class ShowCase
		{
			CreateAndFillBuffer,
			CreateAndFillTexture,
		};

		ShowCase showCase = ShowCase::CreateAndFillTexture; // change this to select a sample

		switch (showCase)
		{
		case ShowCase::CreateAndFillBuffer:
			CreateAndFillBuffer();
			break;
		case ShowCase::CreateAndFillTexture:
			CreateAndFillTexture();
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


	void CreateAndFillBuffer()
	{
		GLuint buffer = 0; // will store the handle to the buffer

		// generate buffer
		gl::GenBuffers(1, &buffer);

		// bind buffer to port GL_COPY_WRITE_BUFFER for following operations
		gl::BindBuffer(gl::GL_COPY_WRITE_BUFFER, buffer);

		GLuint const bufferSize = sizeof(float);
		// create buffer storage for buffer bound at GL_COPY_WRITE_BUFFER, with 4 size in byte, with usage hint GL_DYNAMIC_DRAW
		gl::BufferData(gl::GL_COPY_WRITE_BUFFER, bufferSize, nullptr, gl::GL_DYNAMIC_DRAW);

		float bufferData = 4;
		// fill data into the buffer bound at GL_COPY_WRITE_BUFFER
		gl::BufferSubData(gl::GL_COPY_WRITE_BUFFER, 0, bufferSize, &bufferData);

		float retrievedBufferData = 0;
		// retrieve the data from the buffer back
		gl::GetBufferSubData(gl::GL_COPY_WRITE_BUFFER, 0, bufferSize, &retrievedBufferData);

		if (retrievedBufferData != bufferData)
		{
			// some thing goes wrong
			assert(false);
		}

		// delete the buffer
		gl::DeleteBuffers(1, &buffer);
	}

	void CreateAndFillTexture()
	{
		GLuint texture = 0; // will store the handle to the texture

		// generate texture
		gl::GenTextures(1, &texture);

		// bind texture to port GL_TEXTURE_3D for following operations
		gl::BindTexture(gl::GL_TEXTURE_3D, texture);

		// create storage of texture level 0 for texture bound at GL_TEXTURE_3D.
		// specify GPU format GL_R32F(red_float_32). width, height, depth are 2. border must be 0 (legacy parameter).
		// data not provide so parameter format and type are not used here.
		gl::TexImage3D(gl::GL_TEXTURE_3D, 0, gl::GL_R32F, 2, 2, 2, 0, gl::GL_RED, gl::GL_FLOAT, nullptr);

		array<float, 8> textureData = {0.f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f};

		// fill data into the texture level 0 bound at GL_TEXTURE_3D.
		// source data format is GL_RED(single element texel), using float per element.
		gl::TexSubImage3D(gl::GL_TEXTURE_3D, 0, 0, 0, 0, 2, 2, 2, gl::GL_RED, gl::GL_FLOAT, &textureData);


		array<float, 8> retrievedTextureData;
		// retrieve the data from the texture back
		gl::GetTexImage(gl::GL_TEXTURE_3D, 0, gl::GL_RED, gl::GL_FLOAT, &retrievedTextureData);

		if (retrievedTextureData != textureData) // be careful, floating point equality test
		{
			// some thing goes wrong
			assert(false);
		}

		// generate mipmap (only level 1 will be generated)
		gl::GenerateMipmap(gl::GL_TEXTURE_3D);

		float retrivedTextureLevel1Data;
		// retrieve the level 1 data from the texture back
		gl::GetTexImage(gl::GL_TEXTURE_3D, 1, gl::GL_RED, gl::GL_FLOAT, &retrivedTextureLevel1Data);

		float average = accumulate(textureData.begin(), textureData.end(), 0.f) / 8;
		if (abs(retrivedTextureLevel1Data - average) > numeric_limits<float>::epsilon()) // floating point equality test
		{
			// some thing goes wrong
			assert(false);
		}

		// delete the texture
		gl::DeleteTextures(1, &texture);
	}
};

int main()
{
	Resource resource;

	resource.Start();

	return 0;
}

#include "../MemoryLeakCheck.hpp" // include to enable memory leak checking