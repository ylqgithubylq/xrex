#include "XREX.hpp"

#include "RenderingEffect.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Sampler.hpp"
#include "RenderingPipelineState.hpp"

#include <algorithm>

using std::string;
using std::vector;

namespace XREX
{

	EffectParameterSP const EffectParameter::NullEffectParameter = nullptr;


	EffectPipelineParameters::EffectPipelineParameters()
		: polygonOffsetFactor(0), polygonOffsetUnits(0), frontStencilReference(0), backStencilReference(0), blendFactor(1.f, 1.f, 1.f, 1.f)
	{
	}





	RenderingEffect::RenderingEffect(string const& name)
		: name_(name)
	{
	}


	RenderingEffect::~RenderingEffect()
	{
	}

	std::vector<std::string const*> RenderingEffect::GetFullShaderCode() const
	{
		std::unordered_set<RenderingEffect const*> included;
		std::vector<RenderingEffect const*> includeList;

		// recursively add included effects in order.
		std::function<void(RenderingEffect const& effect)> makeIncludeLists = [&makeIncludeLists, &includeList, &included] (RenderingEffect const& effect)
		{
			// process includes first
			for (auto& include : effect.GetAllIncludes())
			{
				makeIncludeLists(*include);
			}
			// then add effect itself
			if (included.find(&effect) == included.end()) // only effect not included will be added.
			{
				included.insert(&effect);
				includeList.push_back(&effect);
			};
		};

		makeIncludeLists(*this);

		std::vector<std::string const*> result;
		for (auto include : includeList)
		{
			for (auto& code : include->GetAllShaderCodes())
			{
				result.push_back(&code);
			}
		}
		return result;
	}


	EffectParameterSP const& RenderingEffect::GetParameterByName(string const& name) const
	{
		auto i = std::find_if(parameters_.begin(), parameters_.end(), [&name] (EffectParameterSP const& parameter)
		{
			return parameter->GetName() == name;
		});
		if (i == parameters_.end())
		{
			return EffectParameter::NullEffectParameter;
		}
		return *i;
	}

	RenderingTechniqueSP const& RenderingEffect::CreateTechnique()
	{
		techniques_.emplace_back(new RenderingTechnique(*this));
		return techniques_.back();
	}





	RenderingTechnique::RenderingTechnique(RenderingEffect& effect)
		: effect_(effect)
	{
	}

	RenderingTechnique::~RenderingTechnique()
	{

	}

	RenderingPassSP const& RenderingTechnique::CreatePass(ProgramObjectSP& program,
		RasterizerStateObjectSP& rasterizerState, DepthStencilStateObjectSP& depthStencilState, BlendStateObjectSP& blendState)
	{
		passes_.emplace_back(new RenderingPass(*this, program, rasterizerState, depthStencilState, blendState));
		return passes_.back();
	}







	RenderingPass::RenderingPass(RenderingTechnique& technique, ProgramObjectSP& program,
		RasterizerStateObjectSP& rasterizerState, DepthStencilStateObjectSP& depthStencilState, BlendStateObjectSP& blendState)
		: technique_(technique), program_(program), rasterizerState_(rasterizerState), depthStencilState_(depthStencilState), blendState_(blendState)
	{
		program->InitializeParameterSetters(technique_.GetEffect());
	}
	RenderingPass::~RenderingPass()
	{
	}

	void RenderingPass::Use()
	{
		rasterizerState_->Bind(pipelineParameters_.polygonOffsetFactor, pipelineParameters_.polygonOffsetUnits);
		depthStencilState_->Bind(pipelineParameters_.frontStencilReference, pipelineParameters_.backStencilReference);
		blendState_->Bind(pipelineParameters_.blendFactor);
		program_->Bind();
	}

}
