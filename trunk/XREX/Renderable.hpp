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
			: XREX::Noncopyable
		{
			Renderable* renderable;
			MaterialSP material;
			RenderingLayoutSP layout;
			RenderingTechniqueSP technique;
			int32 renderingGroup;

			RenderablePack(RenderablePack&& rhs)
				: renderable(rhs.renderable), material(std::move(rhs.material)), layout(std::move(rhs.layout)), technique(std::move(rhs.technique)), renderingGroup(rhs.renderingGroup)
			{
			}
			explicit RenderablePack(Renderable& ownerRenderable)
				: renderable(&ownerRenderable), renderingGroup(0)
			{
			}
			/*
			 *	@renderingGroup: smaller value will be rendered before any RenderablePack with larger value.
			 */
			RenderablePack(Renderable& ownerRenderable, MaterialSP const& theMaterial, RenderingLayoutSP const& renderingLayout, RenderingTechniqueSP const& renderingTechnique, int32 theRenderingGroup = 0)
				: renderable(&ownerRenderable), material(theMaterial), layout(renderingLayout), technique(renderingTechnique), renderingGroup(theRenderingGroup)
			{
			}
			RenderablePack& operator =(RenderablePack&& rhs)
			{
				if (this != &rhs)
				{
					renderable = rhs.renderable;
					material = rhs.material;
					layout = rhs.layout;
					technique = rhs.technique;
				}
				return *this;
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

namespace std
{
	template<>
	inline void swap<XREX::Renderable::RenderablePack>(XREX::Renderable::RenderablePack& lhs, XREX::Renderable::RenderablePack& rhs)
	{
		XREX::Renderable::RenderablePack temp = std::move(lhs);
		lhs = std::move(rhs);
		rhs = std::move(temp);
	}
}
