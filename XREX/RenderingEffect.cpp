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

	ElementType EffectParameter::GetType() const
	{
		assert(false);
		return ElementType::ParameterTypeCount;
	}
	void EffectParameter::SetValue(bool const& value)
	{
		assert(false);
	}
	void EffectParameter::SetValue(int32 const& value)
	{
		assert(false);
	}
	void EffectParameter::SetValue(intV2 const& value)
	{
		assert(false);
	}
	void EffectParameter::SetValue(intV3 const& value)
	{
		assert(false);
	}
	void EffectParameter::SetValue(intV4 const& value)
	{
		assert(false);
	}
	void EffectParameter::SetValue(uint32 const& value)
	{
		assert(false);
	}
	void EffectParameter::SetValue(uintV2 const& value)
	{
		assert(false);
	}
	void EffectParameter::SetValue(uintV3 const& value)
	{
		assert(false);
	}
	void EffectParameter::SetValue(uintV4 const& value)
	{
		assert(false);
	}
	void EffectParameter::SetValue(float const& value)
	{
		assert(false);
	}
	void EffectParameter::SetValue(floatV2 const& value)
	{
		assert(false);
	}
	void EffectParameter::SetValue(floatV3 const& value)
	{
		assert(false);
	}
	void EffectParameter::SetValue(floatV4 const& value)
	{
		assert(false);
	}
	void EffectParameter::SetValue(floatM44 const& value)
	{
		assert(false);
	}
	void EffectParameter::SetValue(std::pair<TextureSP, SamplerSP> const& value)
	{
		assert(false);
	}
	// 	void EffectParameter::SetValue(std::vector<bool> const& value) { assert(false); }
	// 	void EffectParameter::SetValue(std::vector<int32> const& value) { assert(false); }
	// 	void EffectParameter::SetValue(std::vector<float> const& value) { assert(false); }
	// 	void EffectParameter::SetValue(std::vector<floatV2> const& value) { assert(false); }
	// 	void EffectParameter::SetValue(std::vector<floatV3> const& value) { assert(false); }
	// 	void EffectParameter::SetValue(std::vector<floatV4> const& value) { assert(false); }
	// 	void EffectParameter::SetValue(std::vector<floatM44> const& value) { assert(false); }

#pragma warning(push)
#pragma warning(disable: 4172) // return address of local variable

	EffectParameter::ParameterValueAutoConverter::operator bool const&() const
	{
		assert(false);
		return false;
	}
	EffectParameter::ParameterValueAutoConverter::operator int32 const&() const
	{
		assert(false);
		return 0;
	}
	EffectParameter::ParameterValueAutoConverter::operator intV2 const&() const
	{
		assert(false);
		return intV2();
	}
	EffectParameter::ParameterValueAutoConverter::operator intV3 const&() const
	{
		assert(false);
		return intV3();
	}
	EffectParameter::ParameterValueAutoConverter::operator intV4 const&() const
	{
		assert(false);
		return intV4();
	}
	EffectParameter::ParameterValueAutoConverter::operator uint32 const&() const
	{
		assert(false);
		return 0;
	}
	EffectParameter::ParameterValueAutoConverter::operator uintV2 const&() const
	{
		assert(false);
		return uintV2();
	}
	EffectParameter::ParameterValueAutoConverter::operator uintV3 const&() const
	{
		assert(false);
		return uintV3();
	}
	EffectParameter::ParameterValueAutoConverter::operator uintV4 const&() const
	{
		assert(false);
		return uintV4();
	}
	EffectParameter::ParameterValueAutoConverter::operator float const&() const
	{
		assert(false);
		return 0.f;
	}
	EffectParameter::ParameterValueAutoConverter::operator floatV2 const&() const
	{
		assert(false);
		return floatV2();
	}
	EffectParameter::ParameterValueAutoConverter::operator floatV3 const&() const
	{
		assert(false);
		return floatV3();
	}
	EffectParameter::ParameterValueAutoConverter::operator floatV4 const&() const
	{
		assert(false);
		return floatV4();
	}
	EffectParameter::ParameterValueAutoConverter::operator floatM44 const&() const
	{
		assert(false);
		return floatM44();
	}
	EffectParameter::ParameterValueAutoConverter::operator std::pair<TextureSP, SamplerSP> const&() const
	{
		assert(false);
		return std::make_pair(nullptr, nullptr);
	}
	// 		EffectParameter::ParameterValueAutoConverter::operator std::vector<bool> const&()  const{ assert(false); return std::vector<bool>(); }
	// 		EffectParameter::ParameterValueAutoConverter::operator std::vector<int32> const&()  const{ assert(false); return std::vector<int32>(); }
	// 		EffectParameter::ParameterValueAutoConverter::operator std::vector<float> const&()  const{ assert(false); return std::vector<float>(); }
	// 		EffectParameter::ParameterValueAutoConverter::operator std::vector<floatV2> const&()  const{ assert(false); return std::vector<floatV2>(); }
	// 		EffectParameter::ParameterValueAutoConverter::operator std::vector<floatV3> const&()  const{ assert(false); return std::vector<floatV3>(); }
	// 		EffectParameter::ParameterValueAutoConverter::operator std::vector<floatV4> const&()  const{ assert(false); return std::vector<floatV4>(); }

	// auto EffectParameter::GetValue() const -> ParameterValueAutoConverter const&
	// {
	// 	assert(false);
	// 	return ParameterValueAutoConverter();
	// }

#pragma warning(pop)



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
