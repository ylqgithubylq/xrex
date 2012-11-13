#pragma once

#include "Declare.hpp"
#include "Shader.hpp"

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

		// TODO not implement yet
		RenderingEffectSP LoadEffect(std::string const& fileName, std::vector<std::string> const& macros);

		// temp
		ProgramObjectSP LoadProgram(std::string const& fileName, std::vector<std::string> const& macros);

		MeshSP LoadMesh(std::string const& fileName);
		TextureSP LoadTexture1D(std::string const& fileName, bool generateMipmap);
		TextureSP LoadTexture2D(std::string const& fileName, bool generateMipmap);
		TextureSP LoadTexture3D(std::string const& fileName, bool generateMipmap);
		TextureSP LoadTextureCube(std::string const& fileName, bool generateMipmap);

	private:
		std::unique_ptr<MeshLoader> meshLoader_;
		std::unique_ptr<TextureLoader> textureLoader_;
	};

}
