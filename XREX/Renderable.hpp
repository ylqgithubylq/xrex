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
			static int32 const DefaultRenderingGroup = 0;

			Renderable* renderable;
			RenderingLayoutSP layout;
			MaterialSP material;
			RenderingTechniqueSP technique;
			int32 renderingGroup;

			RenderablePack(RenderablePack&& right)
				: renderable(right.renderable), layout(std::move(right.layout)), material(std::move(right.material)), technique(std::move(right.technique)), renderingGroup(right.renderingGroup)
			{
			}
			explicit RenderablePack(Renderable& ownerRenderable)
				: renderable(&ownerRenderable), renderingGroup(0)
			{
			}
			/*
			 *	@renderingGroup: smaller value will be rendered before any RenderablePack with larger value.
			 */
			RenderablePack(Renderable& ownerRenderable, RenderingLayoutSP const& renderingLayout, MaterialSP const& theMaterial, RenderingTechniqueSP const& renderingTechnique, int32 theRenderingGroup = DefaultRenderingGroup)
				: renderable(&ownerRenderable), layout(renderingLayout), material(theMaterial), technique(renderingTechnique), renderingGroup(theRenderingGroup)
			{
			}
			RenderablePack& operator =(RenderablePack&& right)
			{
				if (this != &right)
				{
					renderable = right.renderable;
					layout = right.layout;
					material = right.material;
					technique = right.technique;
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
	inline void swap<XREX::Renderable::RenderablePack>(XREX::Renderable::RenderablePack& left, XREX::Renderable::RenderablePack& right)
	{
		XREX::Renderable::RenderablePack temp = std::move(left);
		left = std::move(right);
		right = std::move(temp);
	}
}
