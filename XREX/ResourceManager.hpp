#pragma once

#include "Declare.hpp"

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
		// TODO Locate method return string

		TextureSP GetTexture1D(std::string const& fileName);
		TextureSP GetTexture2D(std::string const& fileName);
		TextureSP GetTexture3D(std::string const& fileName);
		TextureSP GetTextureCube(std::string const& fileName);

		MeshSP GetModel(std::string const& fileName);

	private:
		std::unordered_map<std::string, MeshSP> meshes_;
		std::unordered_map<std::string, TextureSP> texture1Ds_;
		std::unordered_map<std::string, TextureSP> texture2Ds_;
		std::unordered_map<std::string, TextureSP> texture3Ds_;

		static std::string const RootPath;
		struct HideFileSystemHeader;
		std::unique_ptr<HideFileSystemHeader> hideFileSystemHeader_;
	};

}
