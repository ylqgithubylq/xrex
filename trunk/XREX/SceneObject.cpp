#include "XREX.hpp"

#include "SceneObject.hpp"

#include "Transformation.hpp"


SceneObjectSP const SceneObject::NullSceneObject = nullptr;


SceneObject::SceneObject()
	: components_(static_cast<uint32>(Component::ComponentType::ComponentTypeCount))
{
	components_[0] = MakeSP<Transformation>();
}

SceneObject::SceneObject(std::string const & name)
	: name_(name), components_(static_cast<uint32>(Component::ComponentType::ComponentTypeCount))
{
	components_[0] = MakeSP<Transformation>();
}


SceneObject::~SceneObject()
{
}


