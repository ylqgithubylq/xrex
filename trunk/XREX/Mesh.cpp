#include "XREX.hpp"

#include "Mesh.hpp"



using std::map;
using std::string;
using std::vector;

StaticMesh::StaticMesh(map<string, RenderingLayoutSP> const & layouts)
	: layoutsAndEffects_(layouts.size())
{
	int32 index = 0;
	for (auto i = layouts.begin(); i != layouts.end(); ++i)
	{
		nameToIndex_[i->first] = index;
		layoutsAndEffects_[index].layout = i->second;
	}
}



StaticMesh::~StaticMesh()
{
}

void StaticMesh::SetEffect(std::string const & layout, RenderingEffectSP& effect)
{
	auto found = std::find_if(nameToIndex_.begin(), nameToIndex_.end(), [&layout] (std::pair<std::string, uint32> const & nameToindex)
	{
		return layout == nameToindex.first;
	});
	assert(found != nameToIndex_.end());
	layoutsAndEffects_[found->second].effect = effect;
}

RenderingEffectSP const & StaticMesh::GetEffect(std::string const & layout) const
{
	auto found = std::find_if(nameToIndex_.begin(), nameToIndex_.end(), [&layout] (std::pair<std::string, uint32> const & nameToindex)
	{
		return layout == nameToindex.first;
	});
	assert(found != nameToIndex_.end());
	return layoutsAndEffects_[found->second].effect;
}


