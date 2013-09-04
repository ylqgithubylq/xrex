#include "XREX.hpp"

#include "ResourceManager.hpp"

#include "Base/XREXContext.hpp"
#include "Resource/LocalResourceLoader.hpp"
#include "Rendering/Texture.hpp"
#include "Rendering/Mesh.hpp"


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

		bool LocatePathString(std::vector<std::tr2::sys::path> const& locations, bool containsDirectory, std::string const& relativePath, std::string* resultPath)
		{
			std::tr2::sys::path resourceLocation;
			if (!Locate(locations, containsDirectory, std::tr2::sys::path(relativePath), &resourceLocation))
			{
				return false;
			}
			*resultPath = resourceLocation.string();
			return true;
		}
	}


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
		return LocatePathString(hideFileSystemHeader_->paths, true, relativePath, resultPath);
	}


	namespace
	{
		template <typename Type>
		struct LoadingResultProxy
			: LoadingResult<Type>
		{
			std::shared_ptr<LoadingResult<Type>> actualResult;
			std::shared_ptr<Type> object;
			std::function<void(std::shared_ptr<Type> const& loadedObject)> onLoad;

			LoadingResultProxy()
			{
			}

			LoadingResultProxy(std::shared_ptr<LoadingResult<Type>> const& result)
				: actualResult(result)
			{
			}
			LoadingResultProxy(std::shared_ptr<Type> const& theObject)
				: object(theObject)
			{
			}

			void SetOnLoad(std::function<void(std::shared_ptr<Type> const& loadedObject)>&& callback)
			{
				onLoad = std::move(callback);
			}

			virtual bool Succeeded() const override
			{
				return object || (actualResult && actualResult->Succeeded());
			}
			virtual std::shared_ptr<Type> Create() override
			{
				if (object)
				{
					return object; // already loaded
				}
				if (actualResult && actualResult->Succeeded())
				{
					object = actualResult->Create();
					onLoad(object); // the lambda in DoLoadTexture
					return object;
				}
				else
				{
					return nullptr; // file not found
				}
			}
		};

		template <typename Type>
		std::shared_ptr<LoadingResult<Type>> DoLoad(std::vector<std::tr2::sys::path> const& paths,
			std::unordered_map<std::string, std::shared_ptr<Type>>& objects, std::unordered_map<std::string, std::shared_ptr<LoadingResult<Type>>>& objectLoadingCache,
			std::string const& fileName, std::function<std::shared_ptr<LoadingResult<Type>>(std::string const& fileName)> const& loadingFunction)
		{
			std::string fullPath;
			if (!LocatePathString(paths, false, fileName, &fullPath))
			{
				return MakeSP<LoadingResultProxy<Type>>(); // not found
			}
			auto found = objects.find(fullPath);
			if (found == objects.end())
			{ // not in object cache
				auto foundInToLoad = objectLoadingCache.find(fullPath);
				if (foundInToLoad == objectLoadingCache.end())
				{ // and not in object loading cache
					std::shared_ptr<LoadingResult<Type>> result = loadingFunction(fullPath);
					std::shared_ptr<LoadingResultProxy<Type>> proxy = MakeSP<LoadingResultProxy<Type>>(result);
					objectLoadingCache.insert(std::make_pair(fullPath, proxy));

					// callback to do caching
					auto onLoadHandler = [&objects, &objectLoadingCache, fullPath] (std::shared_ptr<Type> const& object)
					{
						auto found = objectLoadingCache.find(fullPath);
						if (found == objectLoadingCache.end())
						{
							assert(false);
						}
						objectLoadingCache.erase(found);
						objects.insert(std::make_pair(fullPath, object));
					};
					proxy->SetOnLoad(std::move(onLoadHandler));

					return proxy;
				}
				else // loaded but not created
				{
					return foundInToLoad->second;
				}
			}
			else // already created object
			{
				std::shared_ptr<LoadingResultProxy<Type>> proxy = MakeSP<LoadingResultProxy<Type>>(found->second);
				return proxy;
			}
		}
	}

	TextureLoadingResultSP ResourceManager::LoadTexture1D(std::string const& fileName)
	{
		return DoLoad<Texture>(hideFileSystemHeader_->paths, texture1Ds_, texture1DsToLoad_, fileName, [] (std::string const& fullPath)
		{
			return XREXContext::GetInstance().GetResourceLoader().LoadTexture1D(fullPath, true);
		});
	}

	TextureLoadingResultSP ResourceManager::LoadTexture2D(std::string const& fileName)
	{
		return DoLoad<Texture>(hideFileSystemHeader_->paths, texture2Ds_, texture2DsToLoad_, fileName, [] (std::string const& fullPath)
		{
			return XREXContext::GetInstance().GetResourceLoader().LoadTexture2D(fullPath, true);
		});
	}

	TextureLoadingResultSP ResourceManager::LoadTexture3D(std::string const& fileName)
	{
		return DoLoad<Texture>(hideFileSystemHeader_->paths, texture3Ds_, texture3DsToLoad_, fileName, [] (std::string const& fullPath)
		{
			return XREXContext::GetInstance().GetResourceLoader().LoadTexture3D(fullPath, true);
		});
	}

	MeshLoadingResultSP ResourceManager::LoadModel(std::string const& fileName)
	{
		return DoLoad<Mesh>(hideFileSystemHeader_->paths, meshes_, meshesToLoad_, fileName, [] (std::string const& fullPath)
		{
			return XREXContext::GetInstance().GetResourceLoader().LoadMesh(fullPath);
		});
	}
	
}
