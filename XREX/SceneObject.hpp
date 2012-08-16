#pragma once

#include "Declare.hpp"
#include "Component.hpp"

#include <vector>



class SceneObject
	: public std::enable_shared_from_this<SceneObject>, Noncopyable
{
public:
	static SceneObjectSP const NullSceneObject;
public:
	SceneObject();
	explicit SceneObject(std::string const & name);
	~SceneObject();

	std::string const & GetName() const
	{
		return name_;
	}

	bool HasComponent(Component::ComponentType type) const
	{
		return components_[static_cast<uint32>(type)] != nullptr;
	}
	template <typename T>
	bool HasComponent() const
	{
		return components_[static_cast<uint32>(Component::TypeToComponentType<T>::Type)] != nullptr;
	}

	void SetComponent(ComponentSP const & component)
	{
		components_[component->GetComponentType()] = component;
		component->SetOwnerSceneObject(shared_from_this());
	}

	ComponentSP const & GetComponent(Component::ComponentType type) const
	{
		return	components_[static_cast<uint32>(type)];
	}
	template <typename T>
	std::shared_ptr<T> GetComponent() const
	{
		ComponentSP comonent = components_[static_cast<uint32>(Component::TypeToComponentType<T>::Type)];
		return CheckedSPCast<T>(comonent);
	}

private:
	std::vector<ComponentSP> components_;
	std::string name_;
};

