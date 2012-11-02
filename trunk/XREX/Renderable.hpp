#pragma once
#include "Declare.hpp"

#include "Component.hpp"

#include <vector>

namespace XREX
{

	class XREX_API Renderable
		: public TemplateComponent<Renderable>, public std::enable_shared_from_this<Renderable>
	{
	public:
		struct RenderablePack
		{
			Renderable& renderable;
			MaterialSP material;
			RenderingLayoutSP layout;
			RenderingTechniqueSP technique;

			explicit RenderablePack(Renderable& ownerRenderable)
				: renderable(ownerRenderable)
			{
			}
			RenderablePack(Renderable& ownerRenderable, MaterialSP const& theMaterial, RenderingLayoutSP const& renderingLayout, RenderingTechniqueSP const& renderingTechnique)
				: renderable(ownerRenderable), material(theMaterial), layout(renderingLayout), technique(renderingTechnique)
			{
			}
		};
	public:
		Renderable();
		virtual ~Renderable() override;

		virtual std::vector<RenderablePack> GetRenderablePack(SceneObjectSP const& camera) = 0;

		bool IsVisible() const
		{
			return visible_;
		}
		void SetVisible(bool visible)
		{
			visible_ = visible;
		}

		virtual RenderableSP ShallowClone() const = 0;


	private:
		bool visible_;
	};

}