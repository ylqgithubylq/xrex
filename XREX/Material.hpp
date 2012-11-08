#pragma once

#include "Declare.hpp"

#include "RenderingEffect.hpp"

#include <string>
#include <vector>
#include <unordered_map>

namespace XREX
{

	class XREX_API Material
		: Noncopyable
	{
		// when use default parameters, material will not set corresponding parameters.
		struct EffectPipelineParameterSettings
		{
			EffectPipelineParameters parameters;
			bool useDefaultPolygonOffset;
			bool useDefaultStencilReference;
			bool useDefaultBlendFactor;
			EffectPipelineParameterSettings();
		};
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

		uint32 GetPipelineParameterTechniqueCount() const
		{
			return pipelineParameters_.size();
		}
		uint32 GetPipelineParameterPassCount(uint32 techniqueIndex) const
		{
			assert(techniqueIndex < pipelineParameters_.size());
			return pipelineParameters_[techniqueIndex].size();
		}

		void SetPolygonOffset(uint32 techniqueIndex, uint32 passIndex, float factor, float units);
		bool GetPolygonOffset(uint32 techniqueIndex, uint32 passIndex, float* outFactor, float* outUnits) const;
		bool RemovePolygonOffset(uint32 techniqueIndex, uint32 passIndex);

		void SetStencilReference(uint32 techniqueIndex, uint32 passIndex, uint16 front, uint16 back);
		bool GetStencilReference(uint32 techniqueIndex, uint32 passIndex, uint16* outFront, uint16* outBack) const;
		bool RemoveStencilReference(uint32 techniqueIndex, uint32 passIndex);

		void SetBlendFactor(uint32 techniqueIndex, uint32 passIndex, Color value);
		bool GetBlendFactor(uint32 techniqueIndex, uint32 passIndex, Color* outColor) const;
		bool RemoveBlendFactor(uint32 techniqueIndex, uint32 passIndex);


		void BindToEffect(RenderingEffectSP const& effect);

		void SetAllEffectParameterValues();

	private:
		bool HavePipelineParameter(uint32 techniqueIndex, uint32 passIndex) const;
		void ShrinkPipelineParameter(uint32 techniqueIndex, uint32 passIndex);
		void ResizePipelineParameterOnRequest(uint32 techniqueIndex, uint32 passIndex);
	private:

		std::string name_;

		std::unordered_map<std::string, EffectParameterSP> parameters_;
		// vector<(technique index) vector<(pass index) EffectPipelineParameterSettings>>
		std::vector<std::vector<std::unique_ptr<EffectPipelineParameterSettings>>> pipelineParameters_;

		std::weak_ptr<RenderingEffect> boundEffect_;
		std::vector<std::pair<EffectParameterSP, std::weak_ptr<EffectParameter>>> parameterMappingCache_;

	};

}
