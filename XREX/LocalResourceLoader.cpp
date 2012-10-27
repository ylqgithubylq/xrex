#include "XREX.hpp"

#include "LocalResourceLoader.hpp"
#include "MeshLoader.hpp"
#include "TextureLoader.hpp"

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

	MeshSP LocalResourceLoader::LoadMesh(std::string const& fileName)
	{
		return meshLoader_->LoadMesh(fileName);
	}

	TextureSP LocalResourceLoader::LoadTexture1D(std::string const& fileName)
	{
		return textureLoader_->LoadTexture1D(fileName);
	}
	TextureSP LocalResourceLoader::LoadTexture2D(std::string const& fileName)
	{
		return textureLoader_->LoadTexture2D(fileName);
	}
	TextureSP LocalResourceLoader::LoadTexture3D(std::string const& fileName)
	{
		return textureLoader_->LoadTexture3D(fileName);
	}
	TextureSP LocalResourceLoader::LoadTextureCube(std::string const& fileName)
	{
		return textureLoader_->LoadTextureCube(fileName);
	}

}
