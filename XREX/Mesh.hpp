#pragma once
#include "Declare.hpp"

#include "Renderable.hpp"

#include <string>

class Mesh
	: public Renderable
{
public:
	Mesh(std::string const& name);
	virtual ~Mesh() override;

	std::string const& GetName() const
	{
		return name_;
	}

	SubMeshSP const& GetSubMesh(std::string const& subMeshName) const;
	std::vector<SubMeshSP> const& GetAllSubMeshes() const
	{
		return subMeshes_;
	}

	SubMeshSP const& CreateSubMesh(std::string const& name, MaterialSP const& material, RenderingLayoutSP const& layout, RenderingEffectSP const& effect);

	virtual std::vector<LayoutAndTechnique> GetLayoutsAndTechniques(SceneObjectSP const& camera) const override;

	virtual void OnLayoutBeforeRendered(LayoutAndTechnique& layoutAndTechnique) override;



private:
	std::string name_;
	std::vector<SubMeshSP> subMeshes_;
};


class SubMesh
	: Noncopyable
{
	friend class Mesh;

public:
	~SubMesh();

	std::string const& GetName() const
	{
		return name_;
	}

	void SetEffect(RenderingEffectSP const& effect);

	MaterialSP& GetMaterial()
	{
		return material_;
	}

	Renderable::LayoutAndTechnique GetLayoutAndTechnique(SceneObjectSP const& camera) const;

private:
	SubMesh(Mesh& mesh, std::string const& name, MaterialSP const& material, RenderingLayoutSP const& layout, RenderingEffectSP const& effect);

	void BindAllParameterValue();

private:
	Mesh& mesh_;
	std::string name_;
	MaterialSP material_;
	RenderingLayoutSP layout_;
	RenderingEffectSP effect_;
};

