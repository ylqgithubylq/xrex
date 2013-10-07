#include "XREX.hpp"

#include "Mesh.hpp"

#include "Base/XREXContext.hpp"
#include "Rendering/RenderingFactory.hpp"
#include "Rendering/RenderingTechnique.hpp"
#include "Rendering/Material.hpp"


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
	SubMeshSP const& Mesh::CreateSubMesh(std::string const& name, RenderingLayoutSP const& layout, MaterialSP const& material, RenderingTechniqueSP const& technique, int32 renderingGroup /*= RenderablePack::DefaultRenderingGroup*/)
	{
		subMeshes_.emplace_back(new SubMesh(*this, name, layout, material, technique, renderingGroup));
		return subMeshes_.back();
	}



	void Mesh::GetRenderablePack(RenderablePackCollector& collector, SceneObjectSP const& camera)
	{
		for (uint32 i = 0; i < subMeshes_.size(); ++i)
		{
			collector.AddRenderablePack(subMeshes_[i]->GetRenderablePack(camera));
		}
	}

	void Mesh::GetSmallRenderablePack(RenderablePackCollector& collector, SceneObjectSP const& camera)
	{
		for (uint32 i = 0; i < subMeshes_.size(); ++i)
		{
			collector.AddSmallRenderablePack(subMeshes_[i]->GetSmallRenderablePack(camera));
		}
	}

	RenderableSP Mesh::ShallowClone() const
	{
		MeshSP cloneSP = MakeSP<Mesh>(name_);
		Mesh& clone = *cloneSP;
		clone.SetVisible(IsVisible());
		for (auto& subMesh : subMeshes_)
		{
			clone.CreateSubMesh(subMesh->GetName(), subMesh->GetLayout(), subMesh->GetMaterial(), subMesh->GetTechnique());
		}
		return cloneSP;
	}



	SubMesh::SubMesh(Mesh& mesh, std::string const& name, RenderingLayoutSP const& layout, MaterialSP const& material, RenderingTechniqueSP const& technique, int32 renderingGroup)
		: mesh_(mesh), name_(name), material_(material), layout_(layout), renderingGroup_(renderingGroup)
	{
		assert(layout_ != nullptr);
		SetTechnique(technique);
	}

	SubMesh::~SubMesh()
	{

	}

	Renderable::RenderablePack SubMesh::GetRenderablePack(SceneObjectSP const& camera) const
	{
		assert(technique_ != nullptr);
		assert(connector_ != nullptr);
		assert(layout_ != nullptr);
		return Renderable::RenderablePack(this->mesh_, layout_, material_, connector_, technique_, renderingGroup_);
	}

	Renderable::SmallRenderablePack SubMesh::GetSmallRenderablePack(SceneObjectSP const& camera) const
	{
		assert(layout_ != nullptr);
		return Renderable::SmallRenderablePack(this->mesh_, layout_, material_, renderingGroup_);
	}

	void SubMesh::SetTechnique(RenderingTechniqueSP const& technique)
	{
		technique_ = technique;
		if (technique_)
		{
			SetConnector();
		}
		if (material_)
		{
			if (technique_)
			{
				material_->BindToTechnique(technique_);
			}
			else
			{
				material_->BindToTechnique(nullptr);
			}
		}
	}

	void SubMesh::SetMaterial(MaterialSP const& material)
	{
		material_ = material;
		if (material_ && technique_)
		{
			material_->BindToTechnique(technique_);
		}
	}

	void SubMesh::SetConnector()
	{
		connector_ = XREXContext::GetInstance().GetRenderingFactory().GetConnector(layout_, technique_);
	}

}
