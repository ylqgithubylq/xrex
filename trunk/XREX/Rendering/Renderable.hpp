#pragma once
#include "Declare.hpp"

#include "Scene/Component.hpp"

#include <vector>

namespace XREX
{
	class RenderablePackCollector;

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
			LayoutAndProgramConnectorSP connector;
			RenderingTechniqueSP technique;
			int32 renderingGroup;

			RenderablePack(RenderablePack&& right)
				: renderable(right.renderable), layout(std::move(right.layout)), material(std::move(right.material)),
				connector(std::move(right.connector)), technique(std::move(right.technique)), renderingGroup(right.renderingGroup)
			{
			}
			explicit RenderablePack(Renderable& ownerRenderable)
				: renderable(&ownerRenderable), renderingGroup(DefaultRenderingGroup)
			{
			}
			/*
			 *	@renderingGroup: smaller value will be rendered before any RenderablePack with larger value.
			 */
			RenderablePack(Renderable& ownerRenderable, RenderingLayoutSP const& renderingLayout, MaterialSP const& material,
				LayoutAndProgramConnectorSP const& connector, RenderingTechniqueSP const& renderingTechnique, int32 renderingGroup = DefaultRenderingGroup);
			
			RenderablePack& operator =(RenderablePack&& right)
			{
				if (this != &right)
				{
					renderable = right.renderable;
					layout = std::move(right.layout);
					material = std::move(right.material);
					connector = std::move(right.connector);
					technique = std::move(right.technique);
					renderingGroup = right.renderingGroup;
				}
				return *this;
			}
		};

		struct SmallRenderablePack
		{
			static int32 const DefaultRenderingGroup = 0;

			Renderable* renderable;
			RenderingLayoutSP layout;
			MaterialSP material;
			int32 renderingGroup;

			SmallRenderablePack(SmallRenderablePack&& right)
				: renderable(right.renderable), layout(std::move(right.layout)), material(std::move(right.material)), renderingGroup(right.renderingGroup)
			{
			}
			explicit SmallRenderablePack(Renderable& ownerRenderable)
				: renderable(&ownerRenderable), renderingGroup(DefaultRenderingGroup)
			{
			}
			/*
			 *	@renderingGroup: smaller value will be rendered before any SmallRenderablePack with larger value.
			 */
			SmallRenderablePack(Renderable& ownerRenderable, RenderingLayoutSP const& renderingLayout, MaterialSP const& material, int32 renderingGroup = DefaultRenderingGroup);
			
			SmallRenderablePack& operator =(SmallRenderablePack&& right)
			{
				if (this != &right)
				{
					renderable = right.renderable;
					layout = std::move(right.layout);
					material = std::move(right.material);
					renderingGroup = right.renderingGroup;
				}
				return *this;
			}
		};


	public:
		Renderable();

		virtual ~Renderable() override;

		virtual void GetRenderablePack(RenderablePackCollector& collector, SceneObjectSP const& camera) = 0;

		virtual void GetSmallRenderablePack(RenderablePackCollector& collector, SceneObjectSP const& camera) = 0;

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

	/*
	 * Can be used by RenderingProcess to collect renderable packs.
	 */
	class RenderablePackCollector
		: XREX::Noncopyable
	{
	public:

		void AddRenderablePack(Renderable::RenderablePack&& pack)
		{
			packs_.emplace_back(std::move(pack));
		}
		
		void AddSmallRenderablePack(Renderable::SmallRenderablePack&& pack)
		{
			smallPacks_.emplace_back(std::move(pack));
		}

		std::vector<Renderable::RenderablePack> const& GetRenderablePacks()
		{
			return packs_;
		}

		std::vector<Renderable::SmallRenderablePack> const& GetSmallRenderablePacks()
		{
			return smallPacks_;
		}

		std::vector<Renderable::RenderablePack> ExtractRenderablePacks()
		{
			decltype(packs_) toReturn;
			toReturn.swap(packs_);
			return toReturn;
		}

		std::vector<Renderable::SmallRenderablePack> ExtractSmallRenderablePack()
		{
			decltype(smallPacks_) toReturn;
			toReturn.swap(smallPacks_);
			return toReturn;
		}

	private:
		std::vector<Renderable::RenderablePack> packs_;
		std::vector<Renderable::SmallRenderablePack> smallPacks_;
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

	template<>
	inline void swap<XREX::Renderable::SmallRenderablePack>(XREX::Renderable::SmallRenderablePack& left, XREX::Renderable::SmallRenderablePack& right)
	{
		XREX::Renderable::SmallRenderablePack temp = std::move(left);
		left = std::move(right);
		right = std::move(temp);
	}
}
