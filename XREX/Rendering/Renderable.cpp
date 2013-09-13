#include "XREX.hpp"

#include "Renderable.hpp"

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
		ConnectorPackSP const& connectors, RenderingTechniqueSP const& renderingTechnique, int32 renderingGroup /*= DefaultRenderingGroup*/)
		: renderable(&ownerRenderable), layout(renderingLayout), material(material), connectors(connectors),
		technique(renderingTechnique), renderingGroup(renderingGroup)
	{
	}


	Renderable::SmallRenderablePack::SmallRenderablePack(Renderable& ownerRenderable, RenderingLayoutSP const& renderingLayout, MaterialSP const& material, int32 renderingGroup /*= DefaultRenderingGroup*/)
		: renderable(&ownerRenderable), layout(renderingLayout), material(material), renderingGroup(renderingGroup)
	{
	}

}
