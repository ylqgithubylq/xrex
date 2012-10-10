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
