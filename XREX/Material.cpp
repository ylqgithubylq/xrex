#include "XREX.hpp"

#include "Material.hpp"

namespace XREX
{

	Material::EffectPipelineParameterSettings::EffectPipelineParameterSettings()
		: useDefaultPolygonOffset(true), useDefaultStencilReference(true), useDefaultBlendFactor(true)
	{
	}


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

	void Material::SetPolygonOffset(uint32 techniqueIndex, uint32 passIndex, float factor, float units)
	{
		ResizePipelineParameterOnRequest(techniqueIndex, passIndex);
		std::unique_ptr<EffectPipelineParameterSettings>& parameters = pipelineParameters_[techniqueIndex][passIndex];
		if (!parameters)
		{
			parameters = MakeUP<EffectPipelineParameterSettings>();
		}
		parameters->parameters.polygonOffsetFactor = factor;
		parameters->parameters.polygonOffsetUnits = units;
		parameters->useDefaultPolygonOffset = false;
	}

	bool Material::GetPolygonOffset(uint32 techniqueIndex, uint32 passIndex, float* outFactor, float* outUnits) const
	{
		if (HavePipelineParameter(techniqueIndex, passIndex))
		{
			*outFactor = pipelineParameters_[techniqueIndex][passIndex]->parameters.polygonOffsetFactor;
			*outUnits = pipelineParameters_[techniqueIndex][passIndex]->parameters.polygonOffsetUnits;
			return true;
		}
		return false;
	}

	bool Material::RemovePolygonOffset(uint32 techniqueIndex, uint32 passIndex)
	{
		if (HavePipelineParameter(techniqueIndex, passIndex) && !pipelineParameters_[techniqueIndex][passIndex]->useDefaultPolygonOffset)
		{
			std::unique_ptr<EffectPipelineParameterSettings>& parameters = pipelineParameters_[techniqueIndex][passIndex];
			parameters->parameters.polygonOffsetFactor = 0;
			parameters->parameters.polygonOffsetUnits = 0;
			parameters->useDefaultPolygonOffset = true;
			ShrinkPipelineParameter(techniqueIndex, passIndex);
			return true;
		}
		return false;
	}

	void Material::SetStencilReference(uint32 techniqueIndex, uint32 passIndex, uint16 front, uint16 back)
	{
		ResizePipelineParameterOnRequest(techniqueIndex, passIndex);
		std::unique_ptr<EffectPipelineParameterSettings>& parameters = pipelineParameters_[techniqueIndex][passIndex];
		if (!parameters)
		{
			parameters = MakeUP<EffectPipelineParameterSettings>();
		}
		parameters->parameters.frontStencilReference = front;
		parameters->parameters.frontStencilReference = back;
		parameters->useDefaultStencilReference = false;

	}

	bool Material::GetStencilReference(uint32 techniqueIndex, uint32 passIndex, uint16* outFront, uint16* outBack) const
	{
		if (HavePipelineParameter(techniqueIndex, passIndex))
		{
			*outFront = pipelineParameters_[techniqueIndex][passIndex]->parameters.frontStencilReference;
			*outBack = pipelineParameters_[techniqueIndex][passIndex]->parameters.backStencilReference;
			return true;
		}
		return false;
	}

	bool Material::RemoveStencilReference(uint32 techniqueIndex, uint32 passIndex)
	{
		if (HavePipelineParameter(techniqueIndex, passIndex) && !pipelineParameters_[techniqueIndex][passIndex]->useDefaultStencilReference)
		{
			std::unique_ptr<EffectPipelineParameterSettings>& parameters = pipelineParameters_[techniqueIndex][passIndex];
			parameters->parameters.frontStencilReference = 0;
			parameters->parameters.backStencilReference = 0;
			parameters->useDefaultStencilReference = true;
			ShrinkPipelineParameter(techniqueIndex, passIndex);
			return true;
		}
		return false;
	}

	void Material::SetBlendFactor(uint32 techniqueIndex, uint32 passIndex, Color value)
	{
		ResizePipelineParameterOnRequest(techniqueIndex, passIndex);
		std::unique_ptr<EffectPipelineParameterSettings>& parameters = pipelineParameters_[techniqueIndex][passIndex];
		if (!parameters)
		{
			parameters = MakeUP<EffectPipelineParameterSettings>();
		}
		parameters->parameters.blendFactor = value;
		parameters->useDefaultBlendFactor = false;

	}

	bool Material::GetBlendFactor(uint32 techniqueIndex, uint32 passIndex, Color* outColor) const
	{
		if (HavePipelineParameter(techniqueIndex, passIndex))
		{
			*outColor = pipelineParameters_[techniqueIndex][passIndex]->parameters.blendFactor;
			return true;
		}
		return false;
	}

	bool Material::RemoveBlendFactor(uint32 techniqueIndex, uint32 passIndex)
	{
		if (HavePipelineParameter(techniqueIndex, passIndex) && !pipelineParameters_[techniqueIndex][passIndex]->useDefaultBlendFactor)
		{
			pipelineParameters_[techniqueIndex][passIndex]->parameters.blendFactor = Color(1.f, 1.f, 1.f, 1.f);
			pipelineParameters_[techniqueIndex][passIndex]->useDefaultBlendFactor = true;
			ShrinkPipelineParameter(techniqueIndex, passIndex);
			return true;
		}
		return false;
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

		RenderingEffectSP boundEffect = boundEffect_.lock();
		for (uint32 i = 0; i < boundEffect->GetTechniqueCount() && i < pipelineParameters_.size(); ++i)
		{
			std::vector<std::unique_ptr<EffectPipelineParameterSettings>>& techniqueParameters = pipelineParameters_[i];
			RenderingTechniqueSP const& technique = boundEffect->GetTechnique(i);

			for (uint32 j = 0; j < technique->GetPassCount() && j < techniqueParameters.size(); ++j)
			{
				std::unique_ptr<EffectPipelineParameterSettings>& passParameters = techniqueParameters[j];
				if (passParameters)
				{
					RenderingPassSP const& pass = technique->GetPass(j);

					EffectPipelineParameters& parameters = pass->GetEffectPipelineParameters();
					if (!passParameters->useDefaultPolygonOffset)
					{
						parameters.polygonOffsetFactor = passParameters->parameters.polygonOffsetFactor;
						parameters.polygonOffsetUnits = passParameters->parameters.polygonOffsetUnits;
					}
					if (!passParameters->useDefaultStencilReference)
					{
						parameters.frontStencilReference = passParameters->parameters.frontStencilReference;
						parameters.backStencilReference = passParameters->parameters.backStencilReference;
					}
					if (!passParameters->useDefaultBlendFactor)
					{
						parameters.blendFactor = passParameters->parameters.blendFactor;
					}
				}

			}
		}
	}


	bool Material::HavePipelineParameter(uint32 techniqueIndex, uint32 passIndex) const
	{
		return techniqueIndex < pipelineParameters_.size() && passIndex < pipelineParameters_[techniqueIndex].size() && pipelineParameters_[techniqueIndex][passIndex];
	}

	void Material::ShrinkPipelineParameter(uint32 techniqueIndex, uint32 passIndex)
	{
		assert(HavePipelineParameter(techniqueIndex, passIndex));
		std::unique_ptr<EffectPipelineParameterSettings>& parameters = pipelineParameters_[techniqueIndex][passIndex];
		assert(parameters);
		bool needRemove = parameters->useDefaultPolygonOffset
			&& parameters->useDefaultStencilReference
			&& parameters->useDefaultBlendFactor;
		if (needRemove)
		{
			parameters.reset();
			bool canRemovePassParameters = false;
			canRemovePassParameters = std::all_of(pipelineParameters_[techniqueIndex].begin(), pipelineParameters_[techniqueIndex].end(), [] (std::unique_ptr<EffectPipelineParameterSettings> const& entry)
			{
				return entry == nullptr;
			});
			if (canRemovePassParameters)
			{
				pipelineParameters_[techniqueIndex].clear();

				bool canRemoveTechniqueParameters = false;
				canRemoveTechniqueParameters = std::all_of(pipelineParameters_.begin(), pipelineParameters_.end(), [] (std::vector<std::unique_ptr<EffectPipelineParameterSettings>> const& entry)
				{
					return entry.size() == 0;
				});
				if (canRemoveTechniqueParameters)
				{
					pipelineParameters_.clear();
				}
			}

		}
	}


	void Material::ResizePipelineParameterOnRequest(uint32 techniqueIndex, uint32 passIndex)
	{
		if (techniqueIndex >= pipelineParameters_.size())
		{
			pipelineParameters_.resize(techniqueIndex + 1);
			if (passIndex <= pipelineParameters_[techniqueIndex].size())
			{
				pipelineParameters_[techniqueIndex].resize(passIndex + 1);
			}
		}
	}



}
