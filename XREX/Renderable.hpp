#pragma once
#include "Declare.hpp"


#include <vector>

class Renderable
	: Noncopyable
{
public:
	struct LayoutAndEffect
	{
		RenderingLayoutSP layout;
		RenderingEffectSP effect;
		LayoutAndEffect()
		{
		}
		LayoutAndEffect(RenderingLayoutSP const & renderingLayout, RenderingEffectSP const & renderingEffect)
			: layout(renderingLayout), effect(renderingEffect)
		{
		}
	};
public:
	Renderable();
	virtual ~Renderable();

	virtual std::vector<LayoutAndEffect> const & GetLayoutsAndEffects() const = 0;

};

