#pragma once

#include "Declare.hpp"

#include <string>
#include <vector>


namespace XREX
{

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

		virtual std::string const& GetName() const
		{
			return name_;
		}

		virtual ElementType GetType() const;


		virtual void SetValue(bool const& value);
		virtual void SetValue(int32 const& value);
		virtual void SetValue(intV2 const& value);
		virtual void SetValue(intV3 const& value);
		virtual void SetValue(intV4 const& value);
		virtual void SetValue(uint32 const& value);
		virtual void SetValue(uintV2 const& value);
		virtual void SetValue(uintV3 const& value);
		virtual void SetValue(uintV4 const& value);
		virtual void SetValue(float const& value);
		virtual void SetValue(floatV2 const& value);
		virtual void SetValue(floatV3 const& value);
		virtual void SetValue(floatV4 const& value);
		virtual void SetValue(double const& value);
		virtual void SetValue(doubleV2 const& value);
		virtual void SetValue(doubleV3 const& value);
		virtual void SetValue(doubleV4 const& value);
		virtual void SetValue(floatM44 const& value);
		virtual void SetValue(doubleM44 const& value);
		virtual void SetValue(std::pair<TextureSP, SamplerSP> const& value);
	// 	
	// 	virtual void SetValue(std::vector<bool> const& value);
	// 	virtual void SetValue(std::vector<int32> const& value);
	// 	virtual void SetValue(std::vector<float> const& value);
	// 	virtual void SetValue(std::vector<floatV2> const& value);
	// 	virtual void SetValue(std::vector<floatV3> const& value);
	// 	virtual void SetValue(std::vector<floatV4> const& value);
	// 	virtual void SetValue(std::vector<floatM44> const& value);

		// the mass above is due to no template virtual member functions

		// a little brain fucking...
		// a helper struct to help return value automatically convert to sorts of types.
		struct XREX_API ParameterValueAutoConverter
			: Noncopyable
		{
			virtual operator bool const&() const;
			virtual operator int32 const&() const;
			virtual operator intV2 const&() const;
			virtual operator intV3 const&() const;
			virtual operator intV4 const&() const;
			virtual operator uint32 const&() const;
			virtual operator uintV2 const&() const;
			virtual operator uintV3 const&() const;
			virtual operator uintV4 const&() const;
			virtual operator float const&() const;
			virtual operator floatV2 const&() const;
			virtual operator floatV3 const&() const;
			virtual operator floatV4 const&() const;
			virtual operator double const&() const;
			virtual operator doubleV2 const&() const;
			virtual operator doubleV3 const&() const;
			virtual operator doubleV4 const&() const;
			virtual operator floatM44 const&() const;
			virtual operator doubleM44 const&() const;
			virtual operator std::pair<TextureSP, SamplerSP> const&() const;
	// 		virtual operator std::vector<bool> const&() const;
	// 		virtual operator std::vector<int32> const&() const;
	// 		virtual operator std::vector<float> const&() const;
	// 		virtual operator std::vector<floatV2> const&() const;
	// 		virtual operator std::vector<floatV3> const&() const;
	// 		virtual operator std::vector<floatV4> const&() const;
		};

		// a bit magic, do it this way can overload functions by return value.
		template <typename T>
		T const& GetValue() const
		{
			return DoGetValue();
		}

		virtual void GetValueFrom(EffectParameter const& rhs) = 0;

	private:
		/*
		 *	@return ParameterValueAutoConverter: a helper struct that can convert to other type automatically.
		 */
		virtual ParameterValueAutoConverter const& DoGetValue() const = 0;

	protected:
			std::string name_;
	};


	template <typename T>
	class ConcreteEffectParameter
		: public EffectParameter
	{
	public:
		typedef T ValueType;

#pragma warning(push)
#pragma warning(disable: 4355) // 'this' used in base member initializer list
		explicit ConcreteEffectParameter(std::string const& name)
			: EffectParameter(name), converter_(this)
		{
		}
#pragma warning(pop)

		virtual ElementType GetType() const override
		{
			return TypeToElementType<T>::Type;
		}

		virtual void SetValue(T const& value) override
		{
			value_ = value;
		}
	
		virtual void GetValueFrom(EffectParameter const& rhs) override
		{
			value_ = CheckedCast<ConcreteEffectParameter const*>(&rhs)->value_;
		}

	private:
//		friend class ConcreteParameterValueAutoConverter;
		struct ConcreteParameterValueAutoConverter
			: public ParameterValueAutoConverter
		{
		public:
			explicit ConcreteParameterValueAutoConverter(ConcreteEffectParameter* parameter)
				: parameter_(parameter)
			{
			}
			virtual operator T const&() const override
			{
				return parameter_->value_;
			}
// 			ConcreteParameterValueAutoConverter(EffectParameter* parameter)
// 				: parameter_(parameter)
// 			{
// 			}
		private:
			ConcreteEffectParameter* parameter_;
		};

	private:
		virtual ParameterValueAutoConverter const& DoGetValue() const override
		{
			return converter_;
		}


	protected:
		T value_;
		ConcreteParameterValueAutoConverter converter_;
	};


// 	template <>
// 	class ConcreteEffectParameter<std::pair<TextureSP, SamplerSP>>
// 		: public EffectParameter
// 	{
// 	public:
// 		ConcreteEffectParameter(std::string const& name)
// 			: EffectParameter(name)
// 		{
// 
// 		}
// 
// 		virtual void GetValueFrom(EffectParameter const& rhs) override
// 		{
// 			value_ = CheckedCast<ConcreteEffectParameter const*>(&rhs)->value_;
// 		}
// 
// 		virtual ParameterValueAutoConverter const& DoGetValue() const override
// 		{
// 			throw std::exception("The method or operation is not implemented.");
// 		}
// 
// 	private:	
// 		struct ConcreteParameterValueAutoConverter
// 			: public ParameterValueAutoConverter
// 		{
// 		public:
// 			explicit ConcreteParameterValueAutoConverter(ConcreteEffectParameter* parameter)
// 				: parameter_(parameter)
// 			{
// 			}
// 			virtual operator std::pair<TextureSP, SamplerSP> const&() const override
// 			{
// 				return parameter_->value_;
// 			}
// 
// 		private:
// 			ConcreteEffectParameter* parameter_;
// 		};
// 
// 	private:
// 		std::pair<TextureSP, SamplerSP> value_;
// 	};

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

		std::vector<EffectParameterSP> const& GetAllParameters()
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
