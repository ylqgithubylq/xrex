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

	public:
		Material(std::string const& name);
		~Material();

		/*
		 *	Will updated until next call to UpdateBindingValues or bind to other RenderingEffect.
		 */
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
		/*
		 *	@return: tuple<(success), (factor), (units)>
		 */
		std::tuple<bool, float, float> GetPolygonOffset(uint32 techniqueIndex, uint32 passIndex) const;
		bool RemovePolygonOffset(uint32 techniqueIndex, uint32 passIndex);

		void SetStencilReference(uint32 techniqueIndex, uint32 passIndex, uint16 front, uint16 back);
		/*
		 *	@return: tuple<(success), (front), (back)>
		 */
		std::tuple<bool, uint16, uint16> GetStencilReference(uint32 techniqueIndex, uint32 passIndex) const;
		bool RemoveStencilReference(uint32 techniqueIndex, uint32 passIndex);

		void SetBlendFactor(uint32 techniqueIndex, uint32 passIndex, Color value);
		/*
		 *	@return: tuple<(success), (factor)>
		 */
		std::tuple<bool, Color> GetBlendFactor(uint32 techniqueIndex, uint32 passIndex) const;
		bool RemoveBlendFactor(uint32 techniqueIndex, uint32 passIndex);


		void BindToEffect(RenderingEffectSP const& effect);

		void UpdateBindingValues();

		void SetAllEffectParameterValues();

	private:
		bool HavePipelineParameter(uint32 techniqueIndex, uint32 passIndex) const;
		void ShrinkPipelineParameter(uint32 techniqueIndex, uint32 passIndex);
		void ResizePipelineParameterOnRequest(uint32 techniqueIndex, uint32 passIndex);
	private:
		// when use default values, material will not set corresponding parameters.
		struct EffectPipelineParameterSettings
		{
			EffectPipelineParameters parameters;
			bool useDefaultPolygonOffset;
			bool useDefaultStencilReference;
			bool useDefaultBlendFactor;
			EffectPipelineParameterSettings();
		};
	private:
		std::string name_;

		std::unordered_map<std::string, EffectParameterSP> parameters_;
		// vector<(technique index) vector<(pass index) EffectPipelineParameterSettings>>
		std::vector<std::vector<std::unique_ptr<EffectPipelineParameterSettings>>> pipelineParameters_;

		std::weak_ptr<RenderingEffect> boundEffect_;
		std::vector<std::pair<EffectParameterSP, std::weak_ptr<EffectParameter>>> parameterMappingCache_;

	};

}
