#pragma once

#include "Declare.hpp"

#include "ShaderProgram.hpp"

#include <string>
#include <vector>
#include <functional>


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
#ifdef XREX_DEBUG
			auto type = GetType();
			auto rightType = right.GetType();
			assert(GetType() == right.GetType());
#endif // XREX_DEBUG
			value_ = CheckedCast<ConcreteTechniqueParameter const&>(right).value_;
		}

	private:
		T value_;
	};

	/*
	 *	Used for Material to store texture parameters.
	 */
	class XREX_API SimpleTextureParameter
		: public ConcreteTechniqueParameter<TextureSP>
	{
	public:
		explicit SimpleTextureParameter(std::string const& name)
			: ConcreteTechniqueParameter(name)
		{
		}

		virtual ElementType GetType() const override;
	};
	/*
	 *	Used for Material to store image parameters.
	 */
	class XREX_API SimpleImageParameter
		: public ConcreteTechniqueParameter<TextureImageSP>
	{
	public:
		explicit SimpleImageParameter(std::string const& name)
			: ConcreteTechniqueParameter(name)
		{
		}

		virtual ElementType GetType() const override;
	};

	/*
	 *	Technique created parameter type base.
	 */
	template <typename T, typename BindingInformation>
	class ResourceParameter
		: public ConcreteTechniqueParameter<T>
	{
	public:
		ResourceParameter(std::string const& name, BindingInformation const& information)
			: ConcreteTechniqueParameter<T>(name), information_(information)
		{
		}
		BindingInformation const& GetBindingInformation() const
		{
			return information_;
		}
	private:
		BindingInformation const& information_;
	};

	/*
	 *	Technique created parameter type for Buffer.
	 */
	class XREX_API BufferParameter
		: public ResourceParameter<ShaderResourceBufferSP, BufferBindingInformation>
	{
	public:
		BufferParameter(std::string const& name, BufferBindingInformation const& information)
			: ResourceParameter(name, information)
		{
		}

		virtual ElementType GetType() const override
		{
			return ElementType::Buffer;
		}
	};

	/*
	 *	Technique created parameter type for TextureImage.
	 */
	class XREX_API ImageParameter
		: public ResourceParameter<TextureImageSP, ImageBindingInformation>
	{
	public:
		ImageParameter(std::string const& name, ImageBindingInformation const& information)
			: ResourceParameter(name, information)
		{
		}

		virtual ElementType GetType() const override;
	};
	/*
	 *	Technique created parameter type for Texture.
	 */
	class XREX_API TextureParameter
		: public ResourceParameter<TextureSP, TextureBindingInformation>
	{
	public:
		TextureParameter(std::string const& name, TextureBindingInformation const& information, SamplerSP const& sampler)
			: ResourceParameter(name, information), sampler_(sampler)
		{
		}
		SamplerSP const& GetSampler() const
		{
			return sampler_;
		}

		virtual ElementType GetType() const override;
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
		struct ConstructerParameterPack
		{
			ProgramObjectSP program;
			RasterizerStateObjectSP rasterizerState;
			DepthStencilStateObjectSP depthStencilState;
			BlendStateObjectSP blendState;
			std::unordered_map<std::string, SamplerSP> samplers;
		};
	public:
		explicit RenderingTechnique(std::string const& name, ProgramObjectSP const& program,
			RasterizerStateObjectSP const& rasterizerState, DepthStencilStateObjectSP const& depthStencilState, BlendStateObjectSP const& blendState,
			std::unordered_map<std::string, SamplerSP>&& samplers); // TODO ConstructerParameterPack
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

		void ConnectFrameBuffer(FrameBufferSP framebuffer);

		FrameBufferSP const& GetFrameBuffer() const
		{
			return framebuffer_;
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

		FrameBufferSP framebuffer_;
	};



}
