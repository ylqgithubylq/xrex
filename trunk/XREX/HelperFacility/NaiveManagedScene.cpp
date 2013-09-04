#include "XREX.hpp"

#include "NaiveManagedScene.hpp"

#include "Rendering/Renderable.hpp"
#include "Rendering/Camera.hpp"

using std::vector;

namespace XREX
{

	NaiveManagedScene::NaiveManagedScene()
	{
	}


	NaiveManagedScene::~NaiveManagedScene()
	{
	}

	bool NaiveManagedScene::RemoveObject(std::string const& sceneObjectName)
	{

		auto found = std::find_if(objects_.begin(), objects_.end(), [&sceneObjectName] (SceneObjectSP const& sceneObject)
		{
			return sceneObject->GetName() == sceneObjectName;
		});
		if (found == objects_.end())
		{
			return false;
		}

		if ((*found)->HasComponent<Camera>())
		{
			auto cameraFound = std::find(cameras_.begin(), cameras_.end(), (*found));
			if (cameraFound != cameras_.end())
			{
				SwapBackRemove(cameras_, cameraFound);
			}
		}
		SwapBackRemove(objects_, found);
		return true;
	}

	bool NaiveManagedScene::AddObject(SceneObjectSP const& sceneObject)
	{
		assert(sceneObject != nullptr);
		if (HasObject(sceneObject))
		{
			return false;
		}
		if (HasObject(sceneObject->GetName()))
		{
			return false;
		}
		objects_.push_back(sceneObject);
		if (sceneObject->HasComponent<Camera>())
		{
			cameras_.push_back(sceneObject);
		}
		return true;
	}

	SceneObjectSP const& NaiveManagedScene::GetObject(std::string const& sceneObjectName)
	{
		auto found = std::find_if(objects_.begin(), objects_.end(), [&sceneObjectName] (SceneObjectSP const& sceneObject)
		{
			return sceneObject->GetName() == sceneObjectName;
		});
		if (found == objects_.end())
		{
			return SceneObject::NullSceneObject;
		}
		return *found;
	}

	bool NaiveManagedScene::RemoveObject(SceneObjectSP const& sceneObject)
	{
		assert(sceneObject != nullptr);
		auto found = std::find(objects_.begin(), objects_.end(), sceneObject);
		if (found == objects_.end())
		{
			return false;
		}

		if (sceneObject->HasComponent<Camera>())
		{
			auto cameraFound = std::find(cameras_.begin(), cameras_.end(), sceneObject);
			if (cameraFound != cameras_.end())
			{
				SwapBackRemove(cameras_, cameraFound);
			}
		}
		SwapBackRemove(objects_, found);
		return true;
	}

	vector<SceneObjectSP> NaiveManagedScene::GetRenderableQueue(SceneObjectSP const& camera)
	{
		vector<SceneObjectSP> resultObjects;
		for(auto& sceneObject : objects_)
		{
			if (sceneObject->HasComponent<Renderable>() && sceneObject->GetComponent<Renderable>()->IsVisible())
			{
				resultObjects.push_back(sceneObject);
			}
		}
		return resultObjects;
	}

	std::vector<SceneObjectSP> NaiveManagedScene::GetCameras()
	{
		vector<SceneObjectSP> cameras;
		for(auto& camera : cameras_)
		{
			if (camera->HasComponent<Camera>() && camera->GetComponent<Camera>()->IsActive())
			{
				cameras.push_back(camera);
			}
		}
		return cameras;
	}

}