#pragma once

#include "Declare.hpp"

class Component
	: Noncopyable
{
public:
	enum class ComponentType
	{
		TransformationType,
		RenderableType,
		CameraType,

		ComponentTypeCount
	};

	template <typename T>
	struct TypeToComponentType
	{
		static ComponentType const Type = ComponentType::ComponentTypeCount;
	};
	template <>
	struct TypeToComponentType<Transformation>
	{
		static ComponentType const Type = ComponentType::TransformationType;
	};
	template <>
	struct TypeToComponentType<Renderable>
	{
		static ComponentType const Type = ComponentType::RenderableType;
	};
	template <>
	struct TypeToComponentType<Camera>
	{
		static ComponentType const Type = ComponentType::CameraType;
	};

public:
	Component();
	virtual ~Component();

	virtual ComponentType GetComponentType() const = 0;

	/*
	 *	Used for SceneObject. Don't use this.
	 */
	void SetOwnerSceneObject(SceneObjectSP const& owner)
	{
		SceneObjectSP oldOwner = GetOwnerSceneObject();
		sceneObject_ = owner;
		if (oldOwner != owner)
		{
			OnOwnerObjectChanged(oldOwner, owner);
		}
	}
	SceneObjectSP GetOwnerSceneObject() const
	{
		return sceneObject_.lock();
	}

protected:
	virtual void OnOwnerObjectChanged(SceneObjectSP const& oldOwnerObject, SceneObjectSP const& newOwnerObject)
	{
	}

protected:
	std::weak_ptr<SceneObject> sceneObject_;
};

template <typename T>
class TemplateComponent
	: public Component
{
	virtual ComponentType GetComponentType() const override
	{
		static_assert(TypeToComponentType<T>::Type != ComponentType::ComponentTypeCount, "");
		return TypeToComponentType<T>::Type;
	}
};



