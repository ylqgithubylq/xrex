#pragma once
#include "Declare.hpp"

#include "Component.hpp"

#include <vector>


class Renderable
	: public TemplateComponent<Renderable>
{
public:
	struct LayoutAndEffect
	{
		RenderingLayoutSP layout;
		RenderingEffectSP effect;
		LayoutAndEffect()
		{
		}
		LayoutAndEffect(RenderingLayoutSP const& renderingLayout, RenderingEffectSP const& renderingEffect)
			: layout(renderingLayout), effect(renderingEffect)
		{
		}
	};
public:
	Renderable();
	virtual ~Renderable();

	virtual void Update() override
	{
		// nothing to do for renderable
	}

	virtual std::vector<LayoutAndEffect> const& GetLayoutsAndEffects(SceneObjectSP camera) const = 0;

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

