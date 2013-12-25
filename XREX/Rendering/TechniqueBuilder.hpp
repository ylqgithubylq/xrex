#pragma once

#include "Declare.hpp"

#include "Rendering/FrameBuffer.hpp"
#include "Rendering/ShaderProgram.hpp"
#include "Rendering/RenderingPipelineState.hpp"
#include "Rendering/Sampler.hpp"
#include "Rendering/ShaderProgramInterface.hpp"

namespace XREX
{
	class XREX_API TechniqueBuildingInformation
	{
	public:
		TechniqueBuildingInformation(std::string name)
			: name_(std::move(name))
		{
		}

		std::string const& GetName() const
		{
			return name_;
		}

		void AddInclude(TechniqueBuildingInformationSP technique)
		{
			includes_.push_back(std::move(technique));
		}
		/*
		 *	Only directly included technique informations are returned.
		 */
		std::vector<TechniqueBuildingInformationSP> const& GetAllIncludes() const
		{
			return includes_;
		}

		void AddCommonCode(std::shared_ptr<std::string> code)
		{
			commonCodes_.push_back(std::move(code));
		}
		void AddStageCode(ShaderObject::ShaderType stage, std::shared_ptr<std::string> code)
		{
			stageCodes_[static_cast<uint32>(stage)].push_back(std::move(code));
		}

		std::vector<std::shared_ptr<std::string>> const& GetAllCommonCodes() const
		{
			return commonCodes_;
		}
		std::vector<std::shared_ptr<std::string>> const& GetStageCodes(ShaderObject::ShaderType stage) const
		{
			return stageCodes_[static_cast<uint32>(stage)];
		}

		RasterizerState const& GetRasterizerState() const
		{
			return rasterizerState_;
		}
		void SetRasterizerState(RasterizerState const& rasterizerState)
		{
			rasterizerState_ = rasterizerState;
		}
		DepthStencilState GetDepthStencilState() const
		{
			return depthStencilState_;
		}
		void SetDepthStencilState(DepthStencilState const& depthStencilState)
		{
			depthStencilState_ = depthStencilState;
		}
		BlendState GetBlendState() const
		{
			return blendState_;
		}
		void SetBlendState(BlendState const& blendState)
		{
			blendState_ = blendState;
		}
		void AddSamplerState(std::string const& samplerName, SamplerState const& samplerState)
		{
			auto result = samplerStates_.insert(std::make_pair(samplerName, samplerState));
			assert(result.second); // make sure insert success
		}
		std::unordered_map<std::string, SamplerState const> const& GetAllSamplerStates() const
		{
			return samplerStates_;
		}

		FrameBufferLayoutDescriptionSP const& GetFrameBufferDescription() const
		{
			return framebuffeDescription_;
		}
		void SetFrameBufferDescription(FrameBufferLayoutDescriptionSP framebufferDescription)
		{
			framebuffeDescription_ = std::move(framebufferDescription);
		}

		std::vector<AttributeInputInformation const> const& GetAllAttributeInputInformations() const
		{
			return attributeInputInformations_;
		}
		void AddAttributeInputInformation(AttributeInputInformation const& information)
		{
			attributeInputInformations_.push_back(information);
		}
		std::vector<FragmentOutputInformation const> const& GetAllFragmentOutputInformations() const
		{
			return fragmentOutputInformations_;
		}
		// FragmentOutput is generated from FrameBufferLayoutInformation
// 		void AddFragmentOutputInformation(FragmentOutputInformation const& information)
// 		{
// 			fragmentOutputInformations_.push_back(information);
// 		}
		std::vector<BufferInformation const> const& GetAllUniformBufferInformations() const
		{
			return uniformBufferInformations_;
		}
		void AddUniformBufferInformation(BufferInformation const& information)
		{
			uniformBufferInformations_.push_back(information);
		}
		std::vector<BufferInformation const> const& GetAllShaderStorageBufferInformations() const
		{
			return shaderStorageBufferInformations_;
		}
		void AddShaderStorageBufferInformation(BufferInformation const& information)
		{
			shaderStorageBufferInformations_.push_back(information);
		}
		std::vector<BufferInformation const> const& GetAllAtomicCounterBufferInformations() const
		{
			return atomicCounterBufferInformations_;
		}
		void AddAtomicCounterBufferInformation(BufferInformation const& information)
		{
			atomicCounterBufferInformations_.push_back(information);
		}
		std::vector<TextureInformation const> const& GetAllTextureInformations() const
		{
			return textureInformations_;
		}
		void AddTextureInformation(TextureInformation const& information)
		{
			textureInformations_.push_back(information);
		}
		std::vector<ImageInformation const> const& GetAllImageInformations() const
		{
			return imageInformations_;
		}
		void AddImageInformation(ImageInformation const& information)
		{
			imageInformations_.push_back(information);
		}


	private:
		std::string name_;

		std::vector<TechniqueBuildingInformationSP> includes_;
		std::vector<std::shared_ptr<std::string>> commonCodes_;
		std::array<std::vector<std::shared_ptr<std::string>>, static_cast<uint32>(ShaderObject::ShaderType::ShaderTypeCount)> stageCodes_;

		RasterizerState rasterizerState_;
		DepthStencilState depthStencilState_;
		BlendState blendState_;
		std::unordered_map<std::string, SamplerState const> samplerStates_; // sampler name : SamplerState

		FrameBufferLayoutDescriptionSP framebuffeDescription_;

		std::vector<AttributeInputInformation const> attributeInputInformations_;
		std::vector<FragmentOutputInformation const> fragmentOutputInformations_;
		std::vector<BufferInformation const> uniformBufferInformations_;
		std::vector<BufferInformation const> shaderStorageBufferInformations_;
		std::vector<BufferInformation const> atomicCounterBufferInformations_;
		std::vector<TextureInformation const> textureInformations_;
		std::vector<ImageInformation const> imageInformations_;
		
	};




	class XREX_API TechniqueBuilder
	{
	public:
		TechniqueBuilder(TechniqueBuildingInformationSP techniqueInformation)
			: techniqueInformation_(std::move(techniqueInformation))
		{
		}

		void AddMacros(std::pair<std::string, std::string> const& macro)
		{
			macros_.push_back(macro);
		}

		RenderingTechniqueSP GetRenderingTechnique();

	private:
		RenderingTechniqueSP Create();

		void BuildMacroStrings();

		/*
		 *	Get included technique informations. This technique information also included.
		 */
		std::vector<TechniqueBuildingInformation const*> GetIncludeList() const;

		/*
		 *	Get shader common codes, include all included technique codes.
		 */
		std::vector<std::string const*> CollectFullShaderCommonCode(std::vector<TechniqueBuildingInformation const*> const& includeList) const;
		
		/*
		 *	Get shader stage codes for compile, include all included technique codes.
		 */
		std::vector<std::string const*> CollectFullShaderStageCode(std::vector<std::string const*> const& commonCodes, ShaderObject::ShaderType stage) const;

	private:
		std::vector<std::pair<std::string, std::string>> macros_;
		std::vector<std::string> mutable lastBuiltMacros_;

		TechniqueBuildingInformationSP techniqueInformation_;

		std::weak_ptr<RenderingTechnique> technique_;
	};



	class XREX_API FrameBufferBuilder
	{
	public:
		explicit FrameBufferBuilder(FrameBufferLayoutDescriptionSP description)
			: description_(std::move(description))
		{
		}

		FrameBufferSP GetFrameBuffer();

	private:
		FrameBufferSP Create();

	private:
		FrameBufferLayoutDescriptionSP description_;

		std::weak_ptr<FrameBuffer> framebuffer_;
	};

}

