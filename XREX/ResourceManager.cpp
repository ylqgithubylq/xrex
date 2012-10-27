#include "XREX.hpp"

#include "ResourceManager.hpp"

#include "XREXContext.hpp"
#include "LocalResourceLoader.hpp"
#include "Texture.hpp"
#include "Mesh.hpp"

#include <filesystem>

namespace XREX
{
	namespace
	{
		bool Locate(std::vector<std::tr2::sys::path> const& locations, bool containsDirectory, std::tr2::sys::path const& path, std::tr2::sys::path* locatedPath)
		{
			if (path.has_root_path())
			{
				if (std::tr2::sys::exists(path) && std::tr2::sys::is_regular(path))
				{
					*locatedPath = path;
					return true;
				}
			}
			else
			{
				for (std::tr2::sys::path const& location : locations)
				{
					std::tr2::sys::path pathToTry = location / path;
					bool found = false;
					if (!containsDirectory)
					{
						found = std::tr2::sys::exists(pathToTry) && std::tr2::sys::is_regular(pathToTry);
					}
					else
					{
						found = std::tr2::sys::exists(pathToTry) && (std::tr2::sys::is_regular(pathToTry) || std::tr2::sys::is_directory(pathToTry));
					}
					if (found)
					{
						*locatedPath = pathToTry;
						return true;
					}
				}
			}
			return false;
		}
	}

	// std::string const ResourceManager::RootPath = "../../";

	struct ResourceManager::HideFileSystemHeader
	{
		std::tr2::sys::path rootPath;
		std::vector<std::tr2::sys::path> paths;
		HideFileSystemHeader(std::string const& root)
			: rootPath(std::tr2::sys::complete(std::tr2::sys::path(root)))
		{
			assert(std::tr2::sys::exists(rootPath) && std::tr2::sys::is_directory(rootPath));
			paths.push_back(rootPath);
		}
	};

	ResourceManager::ResourceManager(std::string const& rootPath)
		: hideFileSystemHeader_(MakeUP<HideFileSystemHeader>(rootPath))
	{
	}


	ResourceManager::~ResourceManager()
	{
	}

	bool ResourceManager::AddResourceLocation(std::string const& path)
	{
		std::tr2::sys::path pathObj = std::tr2::sys::path(path);
		if (!pathObj.has_root_path())
		{
			pathObj = hideFileSystemHeader_->rootPath / pathObj;
		}
		if (std::tr2::sys::exists(pathObj) && std::tr2::sys::is_directory(pathObj))
		{
			if (std::find(hideFileSystemHeader_->paths.begin(), hideFileSystemHeader_->paths.end(), pathObj) == hideFileSystemHeader_->paths.end())
			{
				hideFileSystemHeader_->paths.push_back(pathObj);
			}
			return true;
		}
		return false;
	}

	bool ResourceManager::LocatePath(std::string const& relativePath, std::string* resultPath)
	{
		std::tr2::sys::path resourceLocation;
		if (!Locate(hideFileSystemHeader_->paths, true, std::tr2::sys::path(relativePath), &resourceLocation))
		{
			return false;
		}
		*resultPath = resourceLocation.string();
		return true;
	}

	namespace
	{

		TextureSP DoGetTexture(std::vector<std::tr2::sys::path> const& paths, std::unordered_map<std::string, TextureSP>& textures, std::string const& fileName,
			TextureSP (LocalResourceLoader::* loadFunction)(std::string const& fileName)) // pointer to member function, a little brain fucking...
		{
			std::tr2::sys::path resourceLocation;
			if (!Locate(paths, false, std::tr2::sys::path(fileName), &resourceLocation))
			{
				return nullptr;
			}
			std::string fullPath = resourceLocation.string();
			auto found = textures.find(fullPath);
			if (found == textures.end())
			{
				TextureSP texture = (XREXContext::GetInstance().GetResourceLoader().*loadFunction)(fullPath);

				if (texture != nullptr)
				{
					textures[fullPath] = texture;
				}
				return texture;
			}
			else
			{
				return found->second;
			}
		}
	}

	TextureSP ResourceManager::GetTexture1D(std::string const& fileName)
	{
		auto loadFunction = &LocalResourceLoader::LoadTexture1D;
		return DoGetTexture(hideFileSystemHeader_->paths, texture1Ds_, fileName, loadFunction);
	}

	TextureSP ResourceManager::GetTexture2D(std::string const& fileName)
	{
		auto loadFunction = &LocalResourceLoader::LoadTexture2D;
		return DoGetTexture(hideFileSystemHeader_->paths, texture2Ds_, fileName, loadFunction);
	}

	TextureSP ResourceManager::GetTexture3D(std::string const& fileName)
	{
		auto loadFunction = &LocalResourceLoader::LoadTexture3D;
		return DoGetTexture(hideFileSystemHeader_->paths, texture3Ds_, fileName, loadFunction);
	}

	MeshSP ResourceManager::GetModel(std::string const& fileName)
	{
		std::tr2::sys::path resourceLocation;
		if (!Locate(hideFileSystemHeader_->paths, false, std::tr2::sys::path(fileName), &resourceLocation))
		{
			return nullptr;
		}
		std::string fullPath = resourceLocation.string();

		auto found = meshes_.find(fullPath);
		if (found == meshes_.end())
		{
			MeshSP mesh = XREXContext::GetInstance().GetResourceLoader().LoadMesh(fullPath);
			if (mesh != nullptr)
			{
				meshes_[fullPath] = mesh;
			}
			return mesh;
		}
		else
		{
			return found->second; // TODO need clone?
		}
	}

}
