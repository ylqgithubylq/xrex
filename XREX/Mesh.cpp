#include "XREX.hpp"

#include "Mesh.hpp"

#include "RenderingEffect.hpp"
#include "Material.hpp"


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

SubMeshSP const& Mesh::CreateSubMesh(string const& name, MaterialSP const& material, RenderingLayoutSP const& layout, RenderingEffectSP const& effect)
{
	subMeshes_.emplace_back(new SubMesh(*this, name, material, layout, effect));
	return subMeshes_.back();
}



vector<Renderable::RenderablePack> Mesh::GetRenderablePack(SceneObjectSP const& camera) const 
{
	std::vector<RenderablePack> renderablePacks;
	for (uint32 i = 0; i < subMeshes_.size(); ++i)
	{
		renderablePacks.push_back(subMeshes_[i]->GetRenderablePack(camera));
	}
	return renderablePacks;
}

RenderableSP Mesh::ShallowClone() const
{
	MeshSP cloneSP = MakeSP<Mesh>(name_);
	Mesh& clone = *cloneSP;
	clone.SetVisible(IsVisible());
	for (auto& subMesh : subMeshes_)
	{
		clone.CreateSubMesh(subMesh->GetName(), subMesh->GetMaterial(), subMesh->layout_, subMesh->effect_);
	}
	return cloneSP;
}



SubMesh::SubMesh(Mesh& mesh, string const& name, MaterialSP const& material, RenderingLayoutSP const& layout, RenderingEffectSP const& effect)
	: mesh_(mesh), name_(name), material_(material), layout_(layout)
{
	SetEffect(effect);
}

SubMesh::~SubMesh()
{

}

Renderable::RenderablePack SubMesh::GetRenderablePack(SceneObjectSP const& camera) const
{
	// TODO camera dependence technique
	return Renderable::RenderablePack(this->mesh_, material_, layout_, effect_->GetAvailableTechnique(0));
}

void SubMesh::SetEffect(RenderingEffectSP const& effect)
{
	effect_ = effect;
	if (material_)
	{
		material_->BindToEffect(effect_);
	}
}

