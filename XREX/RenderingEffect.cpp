#include "XREX.hpp"

#include "RenderingEffect.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "RenderingPipelineState.hpp"

#include <CoreGL.hpp>

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
	void EffectParameter::SetValue(TextureSP const& value)
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
	EffectParameter::ParameterValueAutoConverter::operator float const&() const
	{
		assert(false);
		return 0.0;
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
	EffectParameter::ParameterValueAutoConverter::operator TextureSP const&() const
	{
		assert(false);
		return nullptr;
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
		: technique_(technique), program_(program), rasterizerState_(rasterizerState), depthStencilState_(depthStencilState), blendState_(blendState),
		// TODO get variable values below from out side
		frontStencilReference_(0), backStencilReference_(0), blendFactor_(1.f, 1.f, 1.f, 1.f)
	{
		program->InitializeParameterSetters(technique_.GetEffect());
	}
	RenderingPass::~RenderingPass()
	{
	}

	void RenderingPass::Use()
	{
		rasterizerState_->Bind();
		depthStencilState_->Bind(frontStencilReference_, backStencilReference_);
		blendState_->Bind(blendFactor_);
		program_->Bind();
	}

}