#pragma once

#include "Declare.hpp"

#include <string>
#include <vector>


namespace XREX
{

	template <typename T>
	class ConcreteEffectParameter;

	class XREX_API EffectParameter
		: Noncopyable
	{
	public:
		static EffectParameterSP const NullEffectParameter;

	public:
		EffectParameter(std::string const& name)
			: name_(name)
		{
		}

		virtual ~EffectParameter()
		{
		}

		std::string const& GetName() const
		{
			return name_;
		}

		virtual ElementType GetType() const = 0;

		virtual void GetValueFrom(EffectParameter const& right) = 0;

		template <typename T>
		ConcreteEffectParameter<T>& As()
		{
			return *CheckedCast<ConcreteEffectParameter<T>*>(this);
		}

	protected:
			std::string name_;
	};

	template <typename T>
	class ConcreteEffectParameter
		: public EffectParameter
	{

	public:
		explicit ConcreteEffectParameter(std::string const& name)
			: EffectParameter(name)
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

	private:
		virtual ElementType GetType() const override
		{
			return TypeToElementType<T>::Type;
		}

		virtual void GetValueFrom(EffectParameter const& right) override
		{
			value_ = CheckedCast<ConcreteEffectParameter const*>(&right)->value_;
		}

	private:
		T value_;
	};

	struct XREX_API EffectPipelineParameters
		: Noncopyable
	{
		float polygonOffsetFactor;
		float polygonOffsetUnits;
		uint16 frontStencilReference;
		uint16 backStencilReference;
		Color blendFactor;
		EffectPipelineParameters();
	};




	class XREX_API RenderingEffect
		: public std::enable_shared_from_this<RenderingEffect>, Noncopyable
	{

	public:
		explicit RenderingEffect(std::string const& name);
		~RenderingEffect();

		void AddInclude(RenderingEffectSP const& effect)
		{
			includes_.push_back(effect);
		}
		std::vector<RenderingEffectSP> const& GetAllIncludes() const
		{
			return includes_;
		}

		void AddShaderCode(std::string const& code)
		{
			shaderCodes_.emplace_back(code);
		}
		void AddShaderCode(std::string&& code)
		{
			shaderCodes_.emplace_back(std::move(code));
		}
		std::vector<std::string> const& GetAllShaderCodes() const
		{
			return shaderCodes_;
		}

		/*
		 *	Get shader codes for compile, include all included effect codes.
		 */
		std::vector<std::string const*> GetFullShaderCode() const;

		std::vector<EffectParameterSP> const& GetAllParameters() const
		{
			return parameters_;
		}
		void AddParameter(EffectParameterSP const& parameter)
		{
			parameters_.push_back(parameter);
		}
		/*
		 *	@return null pointer if not exist.
		 */
		EffectParameterSP const& GetParameterByName(std::string const& name) const;

		uint32 GetTechniqueCount() const
		{
			return techniques_.size();
		}
		RenderingTechniqueSP const& GetTechnique(uint32 techniqueIndex) const
		{
			return techniques_[techniqueIndex];
		}

		RenderingTechniqueSP const& GetAvailableTechnique(int32 lodLevel) const
		{
			return techniques_[0]; // TODO
		}

		RenderingTechniqueSP const& CreateTechnique();

	private:
		std::string name_;

		std::vector<RenderingEffectSP> includes_;
		std::vector<std::string> shaderCodes_;

		std::vector<EffectParameterSP> parameters_;
		std::vector<RenderingTechniqueSP> techniques_;
	};



	class XREX_API RenderingTechnique
		: Noncopyable
	{
		friend class RenderingEffect;
		explicit RenderingTechnique(RenderingEffect& effect);

	public:
		~RenderingTechnique();

		RenderingEffect& GetEffect() const
		{
			return effect_;
		}

		uint32 GetPassCount() const
		{
			return passes_.size();
		}
		RenderingPassSP const& GetPass(uint32 passIndex) const
		{
			return passes_[passIndex];
		}
		RenderingPassSP const& CreatePass(ProgramObjectSP& program,
			RasterizerStateObjectSP& rasterizerState, DepthStencilStateObjectSP& depthStencilState, BlendStateObjectSP& blendState);

	private:
		RenderingEffect& effect_;
		std::vector<RenderingPassSP> passes_;
	};



	class XREX_API RenderingPass
		: Noncopyable
	{
		friend class RenderingTechnique;
		RenderingPass(RenderingTechnique& technique, ProgramObjectSP& program,
			RasterizerStateObjectSP& rasterizerState, DepthStencilStateObjectSP& depthStencilState, BlendStateObjectSP& blendState);

	public:
		~RenderingPass();

		RenderingTechnique& GetTechnique() const
		{
			return technique_;
		}

		void Use();

		ProgramObjectSP const& GetProgram() const
		{
			return program_;
		}

		EffectPipelineParameters& GetEffectPipelineParameters()
		{
			return pipelineParameters_;
		}

	private:
		RenderingTechnique& technique_;
		ProgramObjectSP program_;
		RasterizerStateObjectSP rasterizerState_;
		DepthStencilStateObjectSP depthStencilState_;
		BlendStateObjectSP blendState_;
	
		EffectPipelineParameters pipelineParameters_;
	};

}
