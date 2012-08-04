#include "XREX.hpp"
#include "NaiveManagedScene.hpp"


NaiveManagedScene::NaiveManagedScene(void)
{
}


NaiveManagedScene::~NaiveManagedScene(void)
{
}

bool NaiveManagedScene::AddObject(SceneObjectSP const & sceneObject)
{
	if (HasObject(sceneObject))
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

bool NaiveManagedScene::RemoveObject(SceneObjectSP const & sceneObject)
{
	auto found = std::find(objects_.begin(), objects_.end(), sceneObject);
	if (found == objects_.end())
	{
		return false;
	}

	SwapBackRemove(objects_, found);
	if (sceneObject->HasComponent<Camera>())
	{
		auto cameraFound = std::find(cameras_.begin(), cameras_.end(), sceneObject);
		if (cameraFound != cameras_.end())
		{
			SwapBackRemove(cameras_, cameraFound);
		}
	}
}
