#include "XREX.hpp"

#include "Mesh.hpp"

#include "RenderingEffect.hpp"
#include "Material.hpp"


using std::string;
using std::vector;

namespace XREX
{

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

	SubMeshSP const& Mesh::CreateSubMesh(std::string const& name, RenderingLayoutSP const& layout)
	{
		subMeshes_.emplace_back(new SubMesh(*this, name, layout, nullptr, nullptr, RenderablePack::DefaultRenderingGroup));
		return subMeshes_.back();
	}
	SubMeshSP const& Mesh::CreateSubMesh(std::string const& name, RenderingLayoutSP const& layout, MaterialSP const& material, RenderingEffectSP const& effect, int32 renderingGroup /*= RenderablePack::DefaultRenderingGroup*/)
	{
		subMeshes_.emplace_back(new SubMesh(*this, name, layout, material, effect, renderingGroup));
		return subMeshes_.back();
	}



	vector<Renderable::RenderablePack> Mesh::GetRenderablePack(SceneObjectSP const& camera)
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
			clone.CreateSubMesh(subMesh->GetName(), subMesh->GetLayout(), subMesh->GetMaterial(), subMesh->GetEffect());
		}
		return cloneSP;
	}



	SubMesh::SubMesh(Mesh& mesh, std::string const& name, RenderingLayoutSP const& layout, MaterialSP const& material, RenderingEffectSP const& effect, int32 renderingGroup)
		: mesh_(mesh), name_(name), material_(material), layout_(layout), renderingGroup_(renderingGroup)
	{
		assert(layout_ != nullptr);
		SetEffect(effect);
	}

	SubMesh::~SubMesh()
	{

	}

	Renderable::RenderablePack SubMesh::GetRenderablePack(SceneObjectSP const& camera) const
	{
		// TODO camera dependence technique
		assert(effect_ != nullptr);
		return Renderable::RenderablePack(this->mesh_, layout_, material_, effect_->GetAvailableTechnique(0), renderingGroup_);
	}

	void SubMesh::SetEffect(RenderingEffectSP const& effect)
	{
		effect_ = effect;
		if (material_)
		{
			material_->BindToEffect(effect_);
		}
	}

	void SubMesh::SetMaterial(MaterialSP const& material)
	{
		material_ = material;
		if (effect_)
		{
			material_->BindToEffect(effect_);
		}
	}

}
