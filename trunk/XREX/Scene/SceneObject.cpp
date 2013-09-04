#include "XREX.hpp"

#include "SceneObject.hpp"

#include "Scene/Transformation.hpp"

namespace XREX
{

	SceneObjectSP const SceneObject::NullSceneObject = nullptr;


	SceneObject::SceneObject(std::string const& name)
		: name_(name), components_(static_cast<uint32>(Component::ComponentType::ComponentTypeCount))
	{
		components_[0] = MakeSP<Transformation>();
	}


	SceneObject::~SceneObject()
	{
	}

	void SceneObject::SetComponent(ComponentSP const& component)
	{
		assert(component);
		assert(component->GetOwnerSceneObject() == nullptr);
		ComponentSP const& original = components_[static_cast<uint32>(component->GetComponentType())];
		if (original != nullptr)
		{
			original->SetOwnerSceneObject(SceneObject::NullSceneObject);
		}

		components_[static_cast<uint32>(component->GetComponentType())] = component;
		component->SetOwnerSceneObject(shared_from_this());
	}

}
