#pragma once

#include "Declare.hpp"

#include <string>

namespace XREX
{

	class XREX_API LocalResourceLoader
		: Noncopyable
	{
	public:
		LocalResourceLoader();
		~LocalResourceLoader();

		bool LoadString(std::string const& fileName, std::string* result);
		bool LoadWString(std::string const& fileName, std::wstring* result);

		MeshSP LoadMesh(std::string const& fileName);
		TextureSP LoadTexture1D(std::string const& fileName);
		TextureSP LoadTexture2D(std::string const& fileName);
		TextureSP LoadTexture3D(std::string const& fileName);
		TextureSP LoadTextureCube(std::string const& fileName);

	private:
		std::unique_ptr<MeshLoader> meshLoader_;
		std::unique_ptr<TextureLoader> textureLoader_;
	};

}
