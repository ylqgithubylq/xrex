#pragma once

#include "Declare.hpp"

#include <vector>
#include <string>

class Scene
	: Noncopyable
{
public:
	Scene();
	virtual ~Scene();

	/*
	 *	@return: true if succeed.
	 */
	virtual bool HasObject(std::string const& sceneObjectName) = 0;

	virtual SceneObjectSP const& GetObject(std::string const& sceneObjectName) = 0;

	/*
	 *	@return: true if succeed.
	 */
	virtual bool RemoveObject(std::string const& sceneObjectName) = 0;
	/*
	 *	@return: true if succeed.
	 */
	virtual bool HasObject(SceneObjectSP const& sceneObject) = 0;
	/*
	 *	@return: true if succeed.
	 */
	virtual bool AddObject(SceneObjectSP const& sceneObject) = 0;
	/*
	 *	@return: true if succeed.
	 */
	virtual bool RemoveObject(SceneObjectSP const& sceneObject) = 0;

	virtual int32 GetObjectCount() = 0;

	virtual void ClearAllObject() = 0;

	virtual std::vector<SceneObjectSP> GetRenderableQueue(SceneObjectSP const& camera) = 0;

	virtual std::vector<SceneObjectSP> GetCameras() = 0;
};

