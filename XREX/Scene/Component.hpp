#pragma once

#include "Declare.hpp"

namespace XREX
{

	class XREX_API Component
		: Noncopyable
	{
		friend class SceneObject;
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
		Component()
		{
		}
		virtual ~Component() = 0
		{
		}

		virtual ComponentType GetComponentType() const = 0;

		SceneObjectSP GetOwnerSceneObject() const
		{
			return sceneObject_.lock();
		}

	private:
		/*
		 *	Provided for SceneObject. Don't use this.
		 */
		void SetOwnerSceneObject(SceneObjectSP const& owner)
		{
			SceneObjectSP oldOwner = GetOwnerSceneObject();
			sceneObject_ = owner;
		}
	protected:
		std::weak_ptr<SceneObject> sceneObject_;
	};

	template <typename T>
	class TemplateComponent
		: public Component
	{
	public:
		virtual ~TemplateComponent() override = 0
		{
		}
	private:
		virtual ComponentType GetComponentType() const override
		{
			static_assert(TypeToComponentType<T>::Type != ComponentType::ComponentTypeCount, "");
			return TypeToComponentType<T>::Type;
		}
	};

}

