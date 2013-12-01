#pragma once

#include "Declare.hpp"

#include "Rendering/FrameBuffer.hpp"
#include "Rendering/ShaderProgram.hpp"
#include "Rendering/RenderingPipelineState.hpp"
#include "Rendering/Sampler.hpp"
#include "Rendering/ShaderProgramInterface.hpp"

namespace XREX
{
	class XREX_API TechniqueBuilder
	{
	public:
		TechniqueBuilder(std::string const& name);

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
		void AddSamplerState(std::string const& samplerName, SamplerState const& samplerState)
		{
			auto result = samplerStates_.insert(std::make_pair(samplerName, samplerState));
			assert(result.second); // make sure insert success
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
		void AddFragmentOutputInformation(FragmentOutputInformation const& information)
		{
			fragmentOutputInformations_.push_back(information);
		}
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


		RenderingTechniqueSP GetRenderingTechnique();


	private:
		RenderingTechniqueSP Create();

		std::vector<std::string> BuildMacroStrings() const;

		/*
		 *	Get included techniques. This technique also included.
		 */
		std::vector<TechniqueBuilder const*> GetIncludeList() const;
		/*
		 *	Get shader codes for compile, include all included technique codes.
		 */
		std::vector<std::string const*> CollectFullShaderCode(std::vector<TechniqueBuilder const*> const& includeList, ShaderObject::ShaderType stage) const;

	private:
		std::string name_;

		std::vector<TechniqueBuilderSP> includes_;
		std::vector<std::shared_ptr<std::string>> commonCodes_;
		std::vector<std::shared_ptr<std::string>> stageCodes_;
		std::vector<std::pair<std::string, std::string>> macros_;


		RasterizerState rasterizerState_;
		DepthStencilState depthStencilState_;
		BlendState blendState_;
		std::unordered_map<std::string, SamplerState> samplerStates_; // sampler name : SamplerState

		std::vector<AttributeInputInformation const> attributeInputInformations_;
		std::vector<FragmentOutputInformation const> fragmentOutputInformations_;
		std::vector<BufferInformation const> uniformBufferInformations_;
		std::vector<BufferInformation const> shaderStorageBufferInformations_;
		std::vector<BufferInformation const> atomicCounterBufferInformations_;
		std::vector<TextureInformation const> textureInformations_;
		std::vector<ImageInformation const> imageInformations_;

		std::weak_ptr<RenderingTechnique> technique_;

	};


}

