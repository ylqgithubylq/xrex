#pragma once

#include "Declare.hpp"
#include "Component.hpp"

#include <vector>

namespace XREX
{

	class XREX_API SceneObject
		: public std::enable_shared_from_this<SceneObject>, Noncopyable
	{
	public:
		static SceneObjectSP const NullSceneObject;
	public:
		SceneObject();
		explicit SceneObject(std::string const& name);
		~SceneObject();

		std::string const& GetName() const
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

		ComponentSP RemoveComponent(Component::ComponentType type)
		{
			ComponentSP component = components_[static_cast<uint32>(type)];
			components_[static_cast<uint32>(type)] = nullptr;
			return component;
		}

		template <typename T>
		std::shared_ptr<T> RemoveComponent()
		{
			ComponentSP component = components_[static_cast<uint32>(Component::TypeToComponentType<T>::Type)];
			components_[static_cast<uint32>(Component::TypeToComponentType<T>::Type)] = nullptr;
			return CheckedSPCast<T>(component);
		}

		/*
		 *	Set an instance of a subclass of Component into this object, one instance per a subclass can be set in.
		 */
		void SetComponent(ComponentSP const& component);

		ComponentSP const& GetComponent(Component::ComponentType type) const
		{
			return	components_[static_cast<uint32>(type)];
		}
		template <typename T>
		std::shared_ptr<T> GetComponent() const
		{
			ComponentSP const& comonent = components_[static_cast<uint32>(Component::TypeToComponentType<T>::Type)];
			return CheckedSPCast<T>(comonent); // shared_ptr_cast create a new shared_ptr object.
		}

	private:
		std::vector<ComponentSP> components_;
		std::string name_;
	};

}
