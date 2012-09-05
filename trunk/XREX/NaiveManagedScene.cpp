#include "XREX.hpp"
#include "NaiveManagedScene.hpp"


using std::vector;
using std::find;

NaiveManagedScene::NaiveManagedScene(void)
{
}


NaiveManagedScene::~NaiveManagedScene(void)
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
		auto cameraFound = find(cameras_.begin(), cameras_.end(), (*found));
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
	auto found = find(objects_.begin(), objects_.end(), sceneObject);
	if (found == objects_.end())
	{
		return false;
	}

	if (sceneObject->HasComponent<Camera>())
	{
		auto cameraFound = find(cameras_.begin(), cameras_.end(), sceneObject);
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
	std::_For_each(objects_.begin(), objects_.end(), [&resultObjects] (SceneObjectSP const& sceneObject)
	{
		if (sceneObject->HasComponent<Renderable>())
		{
			resultObjects.push_back(sceneObject);
		}
	});
	return resultObjects;
}
