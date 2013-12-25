#pragma once

#include "Declare.hpp"
#include "LoadingResult.hpp"

#include <unordered_map>
#include <string>

namespace XREX
{

	class XREX_API ResourceManager
		: Noncopyable
	{
	public:
		ResourceManager(std::string const& rootPath);
		~ResourceManager();

		bool AddResourceLocation(std::string const& path);
		bool LocatePath(std::string const& relativePath, std::string* resultPath);

		TextureLoadingResultSP LoadTexture1D(std::string const& fileName);
		TextureLoadingResultSP LoadTexture2D(std::string const& fileName);
		TextureLoadingResultSP LoadTexture3D(std::string const& fileName);
		TextureLoadingResultSP LoadTextureCube(std::string const& fileName);

		MeshLoadingResultSP LoadModel(std::string const& fileName);

		TechniqueLoadingResultSP LoadTechnique(std::string const& fileName, std::vector<std::pair<std::string, std::string>> macros);
		FrameBufferLoadingResultSP LoadFrameBuffer(std::string const& fileName);

	private:
		std::unordered_map<std::string, MeshSP> meshes_;
		std::unordered_map<std::string, TextureSP> texture1Ds_;
		std::unordered_map<std::string, TextureSP> texture2Ds_;
		std::unordered_map<std::string, TextureSP> texture3Ds_;
		std::unordered_map<std::string, RenderingTechniqueSP> techniques_;
		std::unordered_map<std::string, FrameBufferSP> framebuffers_;

		std::unordered_map<std::string, MeshLoadingResultSP> meshesToLoad_;
		std::unordered_map<std::string, TextureLoadingResultSP> texture1DsToLoad_;
		std::unordered_map<std::string, TextureLoadingResultSP> texture2DsToLoad_;
		std::unordered_map<std::string, TextureLoadingResultSP> texture3DsToLoad_;
		std::unordered_map<std::string, TechniqueLoadingResultSP> techniquesToLoad_;
		std::unordered_map<std::string, FrameBufferLoadingResultSP> framebuffersToLoad_;

		struct HideFileSystemHeader;
		std::unique_ptr<HideFileSystemHeader> hideFileSystemHeader_;
	};

}
