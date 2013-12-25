#include "XREX.hpp"

#include "Renderable.hpp"
#include "Rendering/RenderingTechnique.hpp"

namespace XREX
{

	Renderable::Renderable()
		: visible_(true)
	{
	};


	Renderable::~Renderable()
	{
	}


	Renderable::RenderablePack::RenderablePack(Renderable& ownerRenderable, RenderingLayoutSP const& renderingLayout, MaterialSP const& material,
		LayoutAndProgramConnectorSP const& connector, RenderingTechniqueSP const& renderingTechnique, int32 renderingGroup /*= DefaultRenderingGroup*/)
		: renderable(&ownerRenderable), layout(renderingLayout), material(material), connector(connector),
		technique(renderingTechnique), renderingGroup(renderingGroup)
	{
	}

	Renderable::RenderablePack::RenderablePack(RenderablePack&& right)
		: renderable(right.renderable), layout(std::move(right.layout)), material(std::move(right.material)),
		connector(std::move(right.connector)), technique(std::move(right.technique)), renderingGroup(right.renderingGroup)
	{
	}

	Renderable::RenderablePack& Renderable::RenderablePack::operator =(RenderablePack&& right)
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


	Renderable::SmallRenderablePack::SmallRenderablePack(Renderable& ownerRenderable, RenderingLayoutSP const& renderingLayout, MaterialSP const& material, int32 renderingGroup /*= DefaultRenderingGroup*/)
		: renderable(&ownerRenderable), layout(renderingLayout), material(material), renderingGroup(renderingGroup)
	{
	}

	Renderable::SmallRenderablePack::SmallRenderablePack(SmallRenderablePack&& right)
		: renderable(right.renderable), layout(std::move(right.layout)), material(std::move(right.material)), renderingGroup(right.renderingGroup)
	{
	}

	Renderable::SmallRenderablePack& Renderable::SmallRenderablePack::operator=(SmallRenderablePack&& right)
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

}
