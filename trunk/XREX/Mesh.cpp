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



vector<Renderable::LayoutAndTechnique> Mesh::GetLayoutsAndTechniques(SceneObjectSP const& camera) const 
{
	std::vector<LayoutAndTechnique> layoutAndTechnique;
	for (uint32 i = 0; i < subMeshes_.size(); ++i)
	{
		layoutAndTechnique.push_back(subMeshes_[i]->GetLayoutAndTechnique(camera));
		layoutAndTechnique.back().userCustomData = i; // for updating effect parameters use
	}
	return layoutAndTechnique;
}

void Mesh::OnLayoutBeforeRendered(LayoutAndTechnique& layoutAndTechnique)
{
	subMeshes_[layoutAndTechnique.userCustomData]->BindAllParameterValue();
}



SubMesh::SubMesh(Mesh& mesh, string const& name, MaterialSP const& material, RenderingLayoutSP const& layout, RenderingEffectSP const& effect)
	: mesh_(mesh), name_(name), material_(material), layout_(layout), effect_(effect)
{
}

SubMesh::~SubMesh()
{

}

Renderable::LayoutAndTechnique SubMesh::GetLayoutAndTechnique(SceneObjectSP const& camera) const
{
	return Renderable::LayoutAndTechnique(this->mesh_, layout_, effect_->GetAvailableTechnique(0));
}

void SubMesh::SetEffect(RenderingEffectSP const& effect)
{
	effect_ = effect;
	for (auto& effectParameter : effect_->GetAllParameters())
	{
		EffectParameterSP materialParameter = material_->GetParameter(effectParameter->GetName());
		if (materialParameter)
		{
			parameterMappingCache_.push_back(std::make_pair(materialParameter, effectParameter));
		}
	}
}

void SubMesh::BindAllParameterValue()
{
	for (auto& parameterPair : parameterMappingCache_)
	{
		parameterPair.second->GetValueFrom(*parameterPair.first);
	}
}
