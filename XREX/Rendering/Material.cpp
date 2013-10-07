#include "XREX.hpp"

#include "Material.hpp"

namespace XREX
{

	Material::TechniquePipelineParameterSettings::TechniquePipelineParameterSettings()
		: useDefaultPolygonOffset(true), useDefaultStencilReference(true), useDefaultBlendFactor(true)
	{
	}


	Material::Material(std::string const& name)
		: name_(name), cacheDirty_(false)
	{
	}


	Material::~Material()
	{
	}


	TechniqueParameterSP const& Material::GetParameter(std::string const& parameterName)
	{
		auto found = parameters_.find(parameterName);
		if (found != parameters_.end())
		{
			return found->second;
		}
		else
		{
			return TechniqueParameter::NullTechniqueParameter;
		}
	}

	void Material::SetPolygonOffset(float factor, float units)
	{
		pipelineParameter_.parameters.polygonOffsetFactor = factor;
		pipelineParameter_.parameters.polygonOffsetUnits = units;
		pipelineParameter_.useDefaultPolygonOffset = false;
	}

	std::tuple<bool, float, float> Material::GetPolygonOffset() const
	{
		bool useDefault = pipelineParameter_.useDefaultPolygonOffset;
		float outFactor = pipelineParameter_.parameters.polygonOffsetFactor;
		float outUnits = pipelineParameter_.parameters.polygonOffsetUnits;
		return std::make_tuple(useDefault, outFactor, outUnits);
	}

	void Material::RemovePolygonOffset()
	{
		pipelineParameter_.useDefaultPolygonOffset = true;
	}

	void Material::SetStencilReference(uint16 front, uint16 back)
	{
		pipelineParameter_.parameters.frontStencilReference = front;
		pipelineParameter_.parameters.frontStencilReference = back;
		pipelineParameter_.useDefaultStencilReference = false;
	}

	std::tuple<bool, uint16, uint16> Material::GetStencilReference() const
	{
		bool useDefault = pipelineParameter_.useDefaultStencilReference;
		uint16 outFront = pipelineParameter_.parameters.frontStencilReference;
		uint16 outBack = pipelineParameter_.parameters.backStencilReference;
		return std::make_tuple(useDefault, outFront, outBack);
	}

	void Material::RemoveStencilReference()
	{
		pipelineParameter_.useDefaultStencilReference = true;
	}

	void Material::SetBlendFactor(Color value)
	{
		pipelineParameter_.parameters.blendFactor = value;
		pipelineParameter_.useDefaultBlendFactor = false;
	}

	std::tuple<bool, Color> Material::GetBlendFactor() const
	{
		bool useDefault = pipelineParameter_.useDefaultBlendFactor;
		Color color = pipelineParameter_.parameters.blendFactor;
		return std::make_tuple(useDefault, color);
	}

	void Material::RemoveBlendFactor()
	{
		pipelineParameter_.useDefaultBlendFactor = true;
	}

	void Material::BindToTechnique(RenderingTechniqueSP const& technique)
	{
		if (boundTechnique_.lock() == technique)
		{
			return;
		}
		boundTechnique_ = technique;
	}


	void Material::SetAllTechniqueParameterValues()
	{
		if (cacheDirty_)
		{
			UpdateBindingMapping();
		}

		for (auto& parameterPair : parameterMappingCache_)
		{
			assert(!parameterPair.second.expired());
			parameterPair.second.lock()->GetValueFrom(*parameterPair.first);
		}

		RenderingTechniqueSP boundTechnique = boundTechnique_.lock();
		assert(boundTechnique);

		TechniquePipelineParameters& parameters = boundTechnique->GetPipelineParameters();
		if (!pipelineParameter_.useDefaultPolygonOffset)
		{
			parameters.polygonOffsetFactor = pipelineParameter_.parameters.polygonOffsetFactor;
			parameters.polygonOffsetUnits = pipelineParameter_.parameters.polygonOffsetUnits;
		}
		if (!pipelineParameter_.useDefaultStencilReference)
		{
			parameters.frontStencilReference = pipelineParameter_.parameters.frontStencilReference;
			parameters.backStencilReference = pipelineParameter_.parameters.backStencilReference;
		}
		if (!pipelineParameter_.useDefaultBlendFactor)
		{
			parameters.blendFactor = pipelineParameter_.parameters.blendFactor;
		}
	}


	void Material::UpdateBindingMapping()
	{
		cacheDirty_ = false;

		RenderingTechniqueSP boundTechnique = boundTechnique_.lock();
		parameterMappingCache_.clear();
		if (boundTechnique != nullptr)
		{
			for (auto& techniqueParameter : boundTechnique->GetAllParameters())
			{
				TechniqueParameterSP materialParameter = GetParameter(techniqueParameter->GetName());
				if (materialParameter)
				{
					parameterMappingCache_.push_back(std::make_pair(materialParameter, std::weak_ptr<TechniqueParameter>(techniqueParameter)));
				}
			}
		}
	}

}
