#pragma once

#include "Declare.hpp"
#include "Component.hpp"

#include <vector>


class SceneObject
	: Noncopyable
{
public:
	SceneObject();
	explicit SceneObject(std::string const & name);
	~SceneObject();

	std::string const & GetName() const
	{
		return name_;
	}

	void SetComponent(ComponentSP const & component)
	{
		components_[component->GetComponentType()] = component;
	}

	ComponentSP const & GetComponent(Component::ComponentType type) const
	{
		return	components_[static_cast<uint32>(type)];
	}
	template <typename T>
	std::shared_ptr<T> GetComponent() const
	{
		return CheckedSPCast<T>(components_[static_cast<uint32>(Component::TypeToComponentType<T>::Type)]);
	}

private:
	std::vector<ComponentSP> components_;
	std::string name_;
};

