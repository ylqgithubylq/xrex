#pragma once
#include "Declare.hpp"

#include "Component.hpp"

#include <vector>


class Renderable
	: public TemplateComponent<Renderable>
{
public:
	struct LayoutAndTechnique
	{
		Renderable& renderable;
		RenderingLayoutSP layout;
		RenderingTechniqueSP technique;
		int32 userCustomData;

		LayoutAndTechnique(Renderable& ownerRenderable)
			: renderable(ownerRenderable)
		{
		}
		LayoutAndTechnique(Renderable& ownerRenderable, RenderingLayoutSP const& renderingLayout, RenderingTechniqueSP const& renderingTechnique)
			: renderable(ownerRenderable), layout(renderingLayout), technique(renderingTechnique)
		{
		}
	};
public:
	Renderable();
	virtual ~Renderable() override;

	virtual std::vector<LayoutAndTechnique> GetLayoutsAndTechniques(SceneObjectSP const& camera) const = 0;

	bool IsVisible() const
	{
		return visible_;
	}
	void SetVisible(bool visible)
	{
		visible_ = visible;
	}

	virtual void OnLayoutBeforeRendered(LayoutAndTechnique& layoutAndTechnique)
	{
	}

private:
	bool visible_;
};

