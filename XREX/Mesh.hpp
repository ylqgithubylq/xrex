#pragma once
#include "Declare.hpp"

#include "Renderable.hpp"

#include <map>
#include <string>

class StaticMesh
	: public Renderable
{
public:
	StaticMesh(std::map<std::string, RenderingLayoutSP> const & layouts);
	virtual ~StaticMesh();

	void SetEffect(std::string const & layout, RenderingEffectSP& effect);
	RenderingEffectSP const & GetEffect(std::string const & layout) const;

	virtual std::vector<LayoutAndEffect> const & GetLayoutsAndEffects(SceneObjectSP camera) const override
	{
		return layoutsAndEffects_;
	}



private:
	std::vector<LayoutAndEffect> layoutsAndEffects_;
	std::map<std::string, uint32> nameToIndex_;
};

