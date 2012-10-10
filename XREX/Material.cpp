#include "XREX.hpp"

#include "Material.hpp"


Material::Material(std::string const& name)
	: name_(name)
{
}


Material::~Material()
{
}


EffectParameterSP const& Material::GetParameter(std::string const& parameterName)
{
	auto found = parameters_.find(parameterName);
	if (found != parameters_.end())
	{
		return found->second;
	}
	else
	{
		return EffectParameter::NullEffectParameter;
	}
}

void Material::BindToEffect(RenderingEffectSP const& effect)
{
	boundEffect_ = effect;
	parameterMappingCache_.clear();
	if (boundEffect_ != nullptr)
	{
		for (auto& effectParameter : boundEffect_->GetAllParameters())
		{
			EffectParameterSP materialParameter = GetParameter(effectParameter->GetName());
			if (materialParameter)
			{
				parameterMappingCache_.push_back(std::make_pair(materialParameter, effectParameter));
			}
		}
	}
}

void Material::SetAllEffectParameterValues()
{
	for (auto& parameterPair : parameterMappingCache_)
	{
		parameterPair.second->GetValueFrom(*parameterPair.first);
	}
}
