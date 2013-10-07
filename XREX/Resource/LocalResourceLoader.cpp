#include "XREX.hpp"

#include "LocalResourceLoader.hpp"
#include "Resource/MeshLoader.hpp"
#include "Resource/TextureLoader.hpp"
#include "Resource/ResourceManager.hpp"
#include "Base/XREXContext.hpp"
#include "Rendering/RenderingFactory.hpp"

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

	std::shared_ptr<std::string> LocalResourceLoader::LoadString(string const& fileName)
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

			return MakeSP<std::string>(move(temp));
		}
		return nullptr;
	}

	std::shared_ptr<std::wstring> LocalResourceLoader::LoadWString(string const& fileName)
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

			return MakeSP<std::wstring>(move(temp));
		}
		return nullptr;
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
