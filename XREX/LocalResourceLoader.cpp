#include "XREX.hpp"

#include "LocalResourceLoader.hpp"
#include "MeshLoader.hpp"
#include "TextureLoader.hpp"
#include "ResourceManager.hpp"
#include "XREXContext.hpp"
#include "RenderingFactory.hpp"

#include <fstream>
#include <vector>


using std::vector;
using std::string;
using std::wstring;
using std::ifstream;
using std::wifstream;
using std::ios;

namespace XREX
{

	LocalResourceLoader::LocalResourceLoader()
		: meshLoader_(MakeUP<MeshLoader>()), textureLoader_(MakeUP<TextureLoader>())
	{
	}


	LocalResourceLoader::~LocalResourceLoader()
	{
	}

	bool LocalResourceLoader::LoadString(string const& fileName, string* result)
	{
		ifstream file(fileName, ios::in | ios::binary);

		if (file)
		{
			file.seekg(0, ios::end);
			int32 length = static_cast<int32>(file.tellg());
			file.seekg(0, ios::beg);

			string temp;
			temp.resize(length + 1);
			file.read(&temp[0], length);

			*result = move(temp);

			return true;
		}
		return false;
	}

	bool LocalResourceLoader::LoadWString(string const& fileName, wstring* result)
	{
		wifstream file(fileName, ios::in | ios::binary);

		if (file)
		{
			file.seekg(0, ios::end);
			int32 length = static_cast<int32>(file.tellg());
			file.seekg(0, ios::beg);

			wstring temp;
			temp.resize(length + 1);
			file.read(&temp[0], length);

			*result = move(temp);

			return true;
		}
		return false;
	}

	ProgramObjectSP LocalResourceLoader::LoadProgram(std::string const& fileName, std::vector<std::string> const& macros)
	{
		ProgramObjectSP program = XREXContext::GetInstance().GetRenderingFactory().CreateProgramObject();
		string shaderString;
		string fullPath;
		if (!XREXContext::GetInstance().GetResourceManager().LocatePath(fileName, &fullPath))
		{
			return nullptr;
		}
		if (!XREXContext::GetInstance().GetResourceLoader().LoadString(fullPath, &shaderString))
		{
			assert(false); // impossible, if happened, find the bug in the LocatePath
			return nullptr;
		}

		ShaderObjectSP vs =XREXContext::GetInstance().GetRenderingFactory().CreateShaderObject(ShaderObject::ShaderType::VertexShader);
		ShaderObjectSP fs = XREXContext::GetInstance().GetRenderingFactory().CreateShaderObject(ShaderObject::ShaderType::FragmentShader);

		std::vector<string const*> shaderStrings(1, &shaderString);
		vs->Compile(shaderStrings);
		fs->Compile(shaderStrings);

		if (!vs->IsValidate())
		{
			assert(false);
		}
		if (!fs->IsValidate())
		{
			assert(false);
		}
		program->AttachShader(vs);
		program->AttachShader(fs);
		program->Link();
		if (!program->IsValidate())
		{
			assert(false);
		}
		return program;
	}

	MeshLoadingResultSP LocalResourceLoader::LoadMesh(std::string const& fileName)
	{
		return meshLoader_->LoadMesh(fileName);
	}

	TextureLoadingResultSP LocalResourceLoader::LoadTexture1D(std::string const& fileName, bool generateMipmap)
	{
		return textureLoader_->LoadTexture1D(fileName, generateMipmap);
	}
	TextureLoadingResultSP LocalResourceLoader::LoadTexture2D(std::string const& fileName, bool generateMipmap)
	{
		return textureLoader_->LoadTexture2D(fileName, generateMipmap);
	}
	TextureLoadingResultSP LocalResourceLoader::LoadTexture3D(std::string const& fileName, bool generateMipmap)
	{
		return textureLoader_->LoadTexture3D(fileName, generateMipmap);
	}
	TextureLoadingResultSP LocalResourceLoader::LoadTextureCube(std::string const& fileName, bool generateMipmap)
	{
		return textureLoader_->LoadTextureCube(fileName, generateMipmap);
	}


}
