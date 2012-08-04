#pragma once

#include "Declare.hpp"

#include "Scene.hpp"
#include "SceneObject.hpp"

#include <vector>
#include <algorithm>

class NaiveManagedScene
	: public Scene
{
public:
	NaiveManagedScene(void);
	virtual ~NaiveManagedScene(void);


	virtual bool HasObject(std::string const & sceneObjectName) override
	{
		return std::find_if(objects_.begin(), objects_.end(), [&sceneObjectName] (SceneObjectSP const & sceneObject)
		{
			return sceneObject->GetName() == sceneObjectName;
		}) != objects_.end();
	}

	virtual SceneObjectSP const & GetObject(std::string const & sceneObjectName) override
	{
		auto found = std::find_if(objects_.begin(), objects_.end(), [&sceneObjectName] (SceneObjectSP const & sceneObject)
		{
			return sceneObject->GetName() == sceneObjectName;
		});
		if (found == objects_.end())
		{
			return SceneObject::NullSceneObject;
		}
		return *found;
	}

	virtual bool RemoveObject(std::string const & sceneObjectName) override
	{
		auto found = std::find_if(objects_.begin(), objects_.end(), [&sceneObjectName] (SceneObjectSP const & sceneObject)
		{
			return sceneObject->GetName() == sceneObjectName;
		});

	}

	virtual bool HasObject(SceneObjectSP const & sceneObject) override
	{
		return std::find(objects_.begin(), objects_.end(), sceneObject) != objects_.end();
	}

	virtual bool AddObject(SceneObjectSP const & sceneObject) override;

	virtual bool RemoveObject(SceneObjectSP const & sceneObject) override;

	virtual std::vector<SceneObjectSP> GetRenderableQueue(SceneObjectSP const & camera) override
	{
		std::vector<SceneObjectSP> resultObjects;
		std::_For_each(objects_.begin(), objects_.end(), [&resultObjects] (SceneObjectSP const & sceneObject)
		{
			if (sceneObject->HasComponent<Renderable>())
			{
				resultObjects.push_back(sceneObject);
			}
		});
		return std::move(resultObjects);
	}

	virtual std::vector<SceneObjectSP> GetCameras() override
	{
		return cameras_; // a copy of cameras_
	}

private:
	std::vector<SceneObjectSP> objects_;
	std::vector<SceneObjectSP> cameras_;
};

