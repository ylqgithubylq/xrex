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
		RenderingLayoutSP layout;
		RenderingTechniqueSP technique;
		LayoutAndTechnique()
		{
		}
		LayoutAndTechnique(RenderingLayoutSP const& renderingLayout, RenderingTechniqueSP const& renderingTechnique)
			: layout(renderingLayout), technique(renderingTechnique)
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

private:
	bool visible_;
};

