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

		SubMeshSP const& CreateSubMesh(std::string const& name, RenderingLayoutSP const& layout);
		SubMeshSP const& CreateSubMesh(std::string const& name, RenderingLayoutSP const& layout, MaterialSP const& material, RenderingEffectSP const& effect, int32 renderingGroup = RenderablePack::DefaultRenderingGroup);

		virtual std::vector<RenderablePack> GetRenderablePack(SceneObjectSP const& camera) override;

		/*
		 *	Only mesh and sub meshes are cloned. Material, layout, effect are shared.
		 */
		virtual RenderableSP ShallowClone() const override;

		/*
		 *	Short cut for ShallowClone() because shared_ptr<Mesh> are not a covariance of shared_ptr<Renderable>.
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

		RenderingLayoutSP const& GetLayout() const
		{
			return layout_;
		}

		RenderingEffectSP const& GetEffect() const
		{
			return effect_;
		}
		void SetEffect(RenderingEffectSP const& effect);

		MaterialSP const& GetMaterial() const
		{
			return material_;
		}
		void SetMaterial(MaterialSP const& material);

		int32 GetRenderingGroup() const
		{
			return renderingGroup_;
		}
		void SetRenderingGroup(int32 renderingGroup)
		{
			renderingGroup_ = renderingGroup;
		}

		Renderable::RenderablePack GetRenderablePack(SceneObjectSP const& camera) const;

	private:
		SubMesh(Mesh& mesh, std::string const& name, RenderingLayoutSP const& layout, MaterialSP const& material, RenderingEffectSP const& effect, int32 renderingGroup);

	private:
		Mesh& mesh_;
		std::string name_;
		MaterialSP material_;
		RenderingLayoutSP layout_;
		RenderingEffectSP effect_;
		int32 renderingGroup_;
	};

}