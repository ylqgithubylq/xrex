#pragma once

#include "Declare.hpp"
#include "Shader.hpp"
#include "LoadingResult.hpp"

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

		MeshLoadingResultSP LoadMesh(std::string const& fileName);
		TextureLoadingResultSP LoadTexture1D(std::string const& fileName, bool generateMipmap);
		TextureLoadingResultSP LoadTexture2D(std::string const& fileName, bool generateMipmap);
		TextureLoadingResultSP LoadTexture3D(std::string const& fileName, bool generateMipmap);
		TextureLoadingResultSP LoadTextureCube(std::string const& fileName, bool generateMipmap);

	private:
		std::unique_ptr<MeshLoader> meshLoader_;
		std::unique_ptr<TextureLoader> textureLoader_;
	};

}
