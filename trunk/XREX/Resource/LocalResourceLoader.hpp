#pragma once

#include "Declare.hpp"
#include "Rendering/ShaderProgram.hpp"
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

		std::shared_ptr<std::string> LoadString(std::string const& fileName);
		std::shared_ptr<std::wstring> LoadWString(std::string const& fileName);

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
