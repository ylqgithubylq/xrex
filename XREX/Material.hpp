#pragma once

#include "Declare.hpp"

#include "RenderingEffect.hpp"

#include <string>
#include <unordered_map>

class Material
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

private:
// 	floatV3 ambient_;
// 	floatV3 diffuse_;
// 	floatV3 specular_;
// 	floatV3 emit_;
// 	float opacity_;
// 	float specularLevel_;
// 	float shininess_;
	std::string name_;

	std::unordered_map<std::string, EffectParameterSP> parameters_;
};

