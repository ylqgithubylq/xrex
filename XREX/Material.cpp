#include "XREX.hpp"

#include "Material.hpp"

namespace XREX
{

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
		if (boundEffect_.lock() == effect)
		{
			return;
		}
		boundEffect_ = effect;
		RenderingEffectSP boundEffect = boundEffect_.lock();
		parameterMappingCache_.clear();
		if (boundEffect != nullptr)
		{
			for (auto& effectParameter : boundEffect->GetAllParameters())
			{
				EffectParameterSP materialParameter = GetParameter(effectParameter->GetName());
				if (materialParameter)
				{
					parameterMappingCache_.push_back(std::make_pair(materialParameter, std::weak_ptr<EffectParameter>(effectParameter)));
				}
			}
		}
	}

	void Material::SetAllEffectParameterValues()
	{
		for (auto& parameterPair : parameterMappingCache_)
		{
			assert(!parameterPair.second.expired());
			parameterPair.second.lock()->GetValueFrom(*parameterPair.first);
		}
	}

}
