#pragma once

#include "Declare.hpp"

#include "Scene.hpp"
#include "SceneObject.hpp"

#include <vector>
#include <algorithm>

namespace XREX
{

	class XREX_API NaiveManagedScene
		: public Scene
	{
	public:
		NaiveManagedScene();
		virtual ~NaiveManagedScene() override;


		virtual bool HasObject(std::string const& sceneObjectName) override
		{
			return std::find_if(objects_.begin(), objects_.end(), [&sceneObjectName] (SceneObjectSP const& sceneObject)
			{
				return sceneObject->GetName() == sceneObjectName;
			}) != objects_.end();
		}

		virtual SceneObjectSP const& GetObject(std::string const& sceneObjectName) override;

		virtual bool RemoveObject(std::string const& sceneObjectName) override;


		virtual bool HasObject(SceneObjectSP const& sceneObject) override
		{
			return std::find(objects_.begin(), objects_.end(), sceneObject) != objects_.end();
		}

		virtual bool AddObject(SceneObjectSP const& sceneObject) override;

		virtual bool RemoveObject(SceneObjectSP const& sceneObject) override;

		virtual int32 GetObjectCount() override
		{
			return objects_.size();
		}

		virtual void ClearAllObject() override
		{
			objects_ = std::vector<SceneObjectSP>();
			cameras_ = std::vector<SceneObjectSP>();
		}

		virtual std::vector<SceneObjectSP> GetRenderableQueue(SceneObjectSP const& camera) override;

		virtual std::vector<SceneObjectSP> GetCameras() override;

	private:
		std::vector<SceneObjectSP> objects_;
		std::vector<SceneObjectSP> cameras_;
	};

}
