#include "XREX.hpp"

#include "Mesh.hpp"

#include "RenderingEffect.hpp"



using std::string;
using std::vector;

Mesh::Mesh(std::string const& name)
	: name_(name)
{
}



Mesh::~Mesh()
{
}


SubMeshSP const& Mesh::GetSubMesh(string const& name) const
{
	auto found = std::find_if(subMeshes_.begin(), subMeshes_.end(), [&name] (SubMeshSP const& subMesh)
	{
		return subMesh->GetName() == name;
	});
	assert(found != subMeshes_.end());
	return *found;
}

SubMeshSP const& Mesh::CreateSubMesh(string const& name, RenderingLayoutSP const& layout, RenderingEffectSP const& effect)
{
	subMeshes_.emplace_back(new SubMesh(*this, name, layout, effect));
	return subMeshes_.back();
}

vector<Renderable::LayoutAndTechnique> Mesh::GetLayoutsAndTechniques(SceneObjectSP const& camera) const 
{
	std::vector<LayoutAndTechnique> layoutAndTechnique;
	for (auto& subMesh : subMeshes_)
	{
		layoutAndTechnique.push_back(subMesh->GetLayoutAndTechnique(camera));
	}
	return layoutAndTechnique;
}



SubMesh::SubMesh(Mesh& mesh, string const& name, RenderingLayoutSP const& layout, RenderingEffectSP const& effect)
	: mesh_(mesh), name_(name), layout_(layout), effect_(effect)
{
}

SubMesh::~SubMesh()
{

}

Renderable::LayoutAndTechnique SubMesh::GetLayoutAndTechnique(SceneObjectSP const& camera) const
{
	return Renderable::LayoutAndTechnique(layout_, effect_->GetAvailableTechnique(0));
}
