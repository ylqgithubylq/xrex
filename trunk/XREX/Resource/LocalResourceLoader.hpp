#pragma once

#include "Declare.hpp"
#include "Resource/LoadingResult.hpp"

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
		TechniqueLoadingResultSP LoadTechnique(std::string const& fileName, std::vector<std::pair<std::string, std::string>> macros);
		FrameBufferLoadingResultSP LoadFrameBuffer(std::string const& fileName);


	private:
		std::unique_ptr<MeshLoader> meshLoader_;
		std::unique_ptr<TextureLoader> textureLoader_;
		std::unique_ptr<TechniqueLoader> techniqueLoader_;
	};

}
