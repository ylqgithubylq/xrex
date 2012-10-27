#pragma once
#include "Declare.hpp"

#include "Renderable.hpp"

#include <string>

namespace XREX
{

	class XREX_API Mesh
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

		virtual std::vector<RenderablePack> GetRenderablePack(SceneObjectSP const& camera) const override;

		/*
		 *	Only mesh and sub meshes are cloned. Material, layout, effect are shared.
		 */
		virtual RenderableSP ShallowClone() const override;

		/*
		 *	Short cut for ShallowClone().
		 *	Only mesh and sub meshes are cloned. Material, layout, effect are shared.
		 */
		MeshSP GetShallowClone() const
		{
			return std::static_pointer_cast<Mesh>(ShallowClone());
		}

	private:
		std::string name_;
		std::vector<SubMeshSP> subMeshes_;
	};


	class XREX_API SubMesh
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

		Renderable::RenderablePack GetRenderablePack(SceneObjectSP const& camera) const;

	private:
		SubMesh(Mesh& mesh, std::string const& name, MaterialSP const& material, RenderingLayoutSP const& layout, RenderingEffectSP const& effect);

	private:
		Mesh& mesh_;
		std::string name_;
		MaterialSP material_;
		RenderingLayoutSP layout_;
		RenderingEffectSP effect_;

	};

}
