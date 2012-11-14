#pragma once

#include "Declare.hpp"

#include "Component.hpp"
#include "SceneObject.hpp"
#include "Transformation.hpp"


namespace XREX
{
	/*
	 *	+z is the front direction. +y is the up direction.
	 */
	class XREX_API Camera
		: public TemplateComponent<Camera>
	{
	public:
		static Color const DefaultBackgroundColor;
	public:
		Camera(float fieldOfView, float aspectRatio, float near, float far);
		virtual ~Camera() override;

		bool IsActive() const
		{
			return active_;
		}
		void SetActive(bool active)
		{
			active_ = active;
		}

		floatM44 GetViewMatrix() const
		{
			Update();
			return viewMatrix_;
		}

		floatM44 GetProjectionMatrix() const
		{
			Update();
			return projectionMatrix_;
		}

		void SetBackgroundColor(Color const& color)
		{
			backgroundColor_ = color;
		}
		Color const& GetBackgroundColor() const
		{
			return backgroundColor_;
		}

		void AttachToViewport(ViewportSP const& viewport)
		{
			viewport_ = viewport;
		}

		ViewportSP const& GetViewport() const
		{
			return viewport_;
		}

	private:
		void Update() const;


	private:
		float fieldOfView_;
		float aspectRatio_;
		float near_;
		float far_;

		floatM44 mutable viewMatrix_;
		floatM44 mutable projectionMatrix_;


		Color backgroundColor_;

		ViewportSP viewport_;

		bool active_;

		bool mutable dirty_;

	};

}

