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

	virtual std::vector<LayoutAndTechnique> GetLayoutsAndTechniques(SceneObjectSP const& camera) const override;

	SubMeshSP const& CreateSubMesh(std::string const& name, RenderingLayoutSP const& layout, RenderingEffectSP const& effect);

private:
	std::string name_;
	std::vector<SubMeshSP> subMeshes_;
};


class SubMesh
{
	friend class Mesh;
	SubMesh(Mesh& mesh, std::string const& name, RenderingLayoutSP const& layout, RenderingEffectSP const& effect);

public:
	~SubMesh();

	std::string const& GetName() const
	{
		return name_;
	}

	Renderable::LayoutAndTechnique GetLayoutAndTechnique(SceneObjectSP const& camera) const;

private:
	Mesh& mesh_;
	std::string name_;
	RenderingLayoutSP layout_;
	RenderingEffectSP effect_;
};

