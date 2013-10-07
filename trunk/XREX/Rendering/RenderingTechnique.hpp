#pragma once

#include "Declare.hpp"

#include "Rendering/ShaderProgram.hpp"
#include "Rendering/RenderingPipelineState.hpp"
#include "Rendering/Sampler.hpp"

#include <string>
#include <vector>
#include <functional>

using namespace XREX;

namespace XREX
{

	template <typename T>
	class ConcreteTechniqueParameter;

	class XREX_API TechniqueParameter
		: Noncopyable
	{
	public:
		static TechniqueParameterSP const NullTechniqueParameter;
	public:
		TechniqueParameter(std::string const& name)
			: name_(name)
		{
		}


		virtual ~TechniqueParameter()
		{
		}

		std::string const& GetName() const
		{
			return name_;
		}

		virtual ElementType GetType() const = 0;

		virtual void GetValueFrom(TechniqueParameter const& right) = 0;

		template <typename T>
		ConcreteTechniqueParameter<T>& As()
		{
			assert(this != nullptr);
			return CheckedCast<ConcreteTechniqueParameter<T>&>(*this);
		}

	private:
			std::string name_;
	};

	template <typename T>
	class ConcreteTechniqueParameter
		: public TechniqueParameter
	{
	public:
		explicit ConcreteTechniqueParameter(std::string const& name)
			: TechniqueParameter(name)
		{
		}

		T const& GetValue() const
		{
			return value_;
		}

		void SetValue(T const& value)
		{
			value_ = value;
		}

		virtual ElementType GetType() const override
		{
			return TypeToElementType<T>::Type;
		}

		virtual void GetValueFrom(TechniqueParameter const& right) override
		{
			assert(GetType() == right.GetType());
			value_ = CheckedCast<ConcreteTechniqueParameter const&>(right).value_;
		}

	private:
		T value_;
	};

	template <typename T, typename Information>
	class ResourceParameter
		: public ConcreteTechniqueParameter<T>
	{
	public:
		ResourceParameter(std::string const& name, Information const& information)
			: ConcreteTechniqueParameter<T>(name), information_(information)
		{
		}
		Information const& GetInformation() const
		{
			return information_;
		}
	private:
		Information const& information_;
	};

	typedef ResourceParameter<ShaderResourceBufferSP, BufferInformation> BufferParameter;
	typedef ResourceParameter<TextureImageSP, ImageInformation> ImageParameter;

	class TextureParameter
		: public ResourceParameter<TextureSP, TextureInformation>
	{
	public:
		TextureParameter(std::string const& name, TextureInformation const& information, SamplerSP const& sampler)
			: ResourceParameter(name, information), sampler_(sampler)
		{
		}
		SamplerSP const& GetSampler() const
		{
			return sampler_;
		}
	private:
		SamplerSP sampler_;
	};


	struct XREX_API TechniquePipelineParameters
	{
		float polygonOffsetFactor;
		float polygonOffsetUnits;
		uint16 frontStencilReference;
		uint16 backStencilReference;
		Color blendFactor;
		TechniquePipelineParameters();
	};



	class XREX_API RenderingTechnique
		: Noncopyable
	{
	public:
		explicit RenderingTechnique(std::string const& name, ProgramObjectSP const& program,
			RasterizerStateObjectSP const& rasterizerState, DepthStencilStateObjectSP const& depthStencilState, BlendStateObjectSP const& blendState,
			std::unordered_map<std::string, SamplerSP>&& samplers);
		~RenderingTechnique();


		std::string const& GetName() const
		{
			return name_;
		}

		ProgramObjectSP const& GetProgram() const
		{
			return program_;
		}

		TechniquePipelineParameters& GetPipelineParameters() // non const
		{
			return pipelineParameters_;
		}
		void SetPipelineParameters(TechniquePipelineParameters const& pipelineParameters)
		{
			pipelineParameters_ = pipelineParameters;
		}

		std::vector<TechniqueParameterSP> const& GetAllParameters() const
		{
			return parameters_;
		}

		/*
		 *	@return null pointer if not exist.
		 */
		TechniqueParameterSP const& GetParameterByName(std::string const& name);


		void Use();

		void SetupAllResources();

	private:
		void InitializeParameterInformations();
		void AddParameter(TechniqueParameterSP const& parameter);

	private:
		std::string name_;
		
		std::vector<TechniqueParameterSP> parameters_;

		std::vector<std::function<void()>> parameterSetters_;

		ProgramObjectSP program_;

		RasterizerStateObjectSP rasterizerState_;
		DepthStencilStateObjectSP depthStencilState_;
		BlendStateObjectSP blendState_;

		std::unordered_map<std::string, SamplerSP> samplers_;

		TechniquePipelineParameters pipelineParameters_;
	};


	class XREX_API TechniqueBuilder
	{
	public:
		TechniqueBuilder(std::string const& name);

		void SpecifyFragmentOutput(std::string const& channel); // TEMP or should be systematic added
		void SpecifyImageFormat(std::string const& channel, TexelFormat format, AccessType accessType); // TEMP or should be systematic added


		void AddInclude(TechniqueBuilderSP const& technique)
		{
			includes_.push_back(technique);
		}
		std::vector<TechniqueBuilderSP> const& GetAllIncludes() const
		{
			return includes_;
		}
		void AddMacros(std::pair<std::string, std::string> const& macro)
		{
			macros_.push_back(macro);
		}

		void AddCommonCode(std::shared_ptr<std::string> const& code)
		{
			commonCodes_.push_back(code);
		}
		void SetStageCode(ShaderObject::ShaderType stage, std::shared_ptr<std::string> const& code)
		{
			stageCodes_[static_cast<uint32>(stage)] = code;
		}

		std::vector<std::shared_ptr<std::string>> const& GetAllCommonCodes() const
		{
			return commonCodes_;
		}
		std::shared_ptr<std::string> const& GetStageCode(ShaderObject::ShaderType stage) const
		{
			return stageCodes_[static_cast<uint32>(stage)];
		}

		void SetRasterizerState(RasterizerState const& rasterizerState)
		{
			rasterizerState_ = rasterizerState;
		}
		void SetDepthStencilState(DepthStencilState const& depthStencilState)
		{
			depthStencilState_ = depthStencilState;
		}
		void SetBlendState(BlendState const& blendState)
		{
			blendState_ = blendState;
		}
		/*
		 *	Set this before SetSamplerChannelToSamplerStateMapping.
		 */
		void SetSamplerState(std::string const& samplerName, SamplerState const& samplerState)
		{
			samplerStates_[samplerName] = samplerState;
		}
		/*
		 *	Set this after SetSamplerState.
		 */
		void SetSamplerChannelToSamplerStateMapping(std::string const& channel, std::string samplerName);

		RenderingTechniqueSP GetRenderingTechnique()
		{
			if (technique_.expired())
			{
				Create();
			}
			return technique_.lock();
		}


	private:
		void Create();

		void BuildMacroStrings();

		/*
		 *	Get shader codes for compile, include all included technique codes.
		 */
		std::vector<std::string const*> GetFullShaderCode(ShaderObject::ShaderType stage) const;

	private:
		std::string name_;

		std::vector<std::string> fragmentOutputChannels_; // TEMP
		std::vector<std::tuple<std::string, TexelFormat, AccessType>> imageChannelInformations_; // TEMP

		std::vector<TechniqueBuilderSP> includes_;
		std::vector<std::shared_ptr<std::string>> commonCodes_;
		std::vector<std::shared_ptr<std::string>> stageCodes_;
		std::vector<std::pair<std::string, std::string>> macros_;

		std::vector<std::string> generatedMacros_;

		RasterizerState rasterizerState_;
		DepthStencilState depthStencilState_;
		BlendState blendState_;
		std::unordered_map<std::string, SamplerState> samplerStates_;
		std::unordered_map<std::string, std::string> samplerChannelToSamplerStateMappings_;

		std::weak_ptr<RenderingTechnique> technique_;

	};
}
