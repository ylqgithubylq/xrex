#pragma once

#include "Declare.hpp"

#include "RenderingEffect.hpp"

#include <string>
#include <unordered_map>

namespace XREX
{

	class XREX_API Material
		: Noncopyable
	{
	public:
		Material(std::string const& name);
		~Material();

		template <typename T>
		void SetParameter(std::string const& parameterName, T const& value)
		{
			auto found = parameters_.find(parameterName);
			if (found == parameters_.end())
			{
				EffectParameterSP parameter = MakeSP<ConcreteEffectParameter<T>>(parameterName);
				parameter->SetValue(value);
				parameters_[parameterName] = std::move(parameter);
			}
			else
			{
				found->second->SetValue(value);
			}
		}

		EffectParameterSP const& GetParameter(std::string const& parameterName);

		void BindToEffect(RenderingEffectSP const& effect);

		void SetAllEffectParameterValues();

	private:
		std::string name_;

		std::unordered_map<std::string, EffectParameterSP> parameters_;

		std::weak_ptr<RenderingEffect> boundEffect_;
		std::vector<std::pair<EffectParameterSP, std::weak_ptr<EffectParameter>>> parameterMappingCache_;

	};

}
