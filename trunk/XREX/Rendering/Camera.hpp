#pragma once

#include "Declare.hpp"

#include "Scene/Component.hpp"
#include "Scene/SceneObject.hpp"
#include "Scene/Transformation.hpp"


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

	protected:
		Camera();
	public:
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

		enum class ViewportOrigin
		{
			/*
			 *	Use viewport center as origin, position ranges: [(-1, -1), (1, 1)]
			 */
			ViewportCenter,
			/*
			 *	Use viewport lower left corner as origin, position ranges: [(0, 0), (1, 1)]
			 */
			ViewportLowerLeft,
		};

		/*
		 *	@position: position in the camera image plane, different @origin have different range. See ViewportOrigin.
		 */
		Ray GetViewRay(floatV2 const& position, ViewportOrigin origin);

	private:
		void Update() const;


	protected:

		floatM44 mutable viewMatrix_;
		floatM44 mutable projectionMatrix_;

	private:
		Color backgroundColor_;

		ViewportSP viewport_;

		bool active_;

		bool mutable dirty_;

	};

	class XREX_API PerspectiveCamera
		: public Camera
	{
	public:
		PerspectiveCamera(float fieldOfView, float aspectRatio, float near, float far);
		virtual ~PerspectiveCamera() override;


		float GetFieldOfView() const
		{
			return fieldOfView_;
		}
		float GetAspectRatio() const
		{
			return aspectRatio_;
		}
		float GetNear() const
		{
			return near_;
		}
		float GetFar() const
		{
			return far_;
		}
	private:
		float fieldOfView_;
		float aspectRatio_;
		float near_;
		float far_;

	};

	class OrthogonalCamera
		: public Camera
	{
	public:
		OrthogonalCamera(float width, float height, float depth);
		~OrthogonalCamera();

		float GetWidth() const
		{
			return width_;
		}
		float GetHeight() const
		{
			return height_;
		}
		float GetDepth() const
		{
			return depth_;
		}
	private:
		float width_;
		float height_;
		float depth_;
	};


}

