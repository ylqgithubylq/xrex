#pragma once

#include "Declare.hpp"

class Component
	: Noncopyable
{
public:
	enum ComponentType
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

	virtual void Update() = 0;
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



