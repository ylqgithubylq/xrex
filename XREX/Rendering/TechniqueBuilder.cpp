#include "XREX.hpp"

#include "TechniqueBuilder.hpp"

#include "Base/XREXContext.hpp"
#include "Base/Logger.hpp"
#include "Base/Window.hpp"
#include "Rendering/RenderingFactory.hpp"
#include "Rendering/RenderingTechnique.hpp"

#include <sstream>

namespace XREX
{


	XREX::RenderingTechniqueSP TechniqueBuilder::GetRenderingTechnique()
	{
		if (technique_.expired())
		{
			return Create();
		}
		return technique_.lock();
	}

	namespace
	{
		template <typename Information, std::vector<Information const> const&(TechniqueBuildingInformation::*Getter)() const>
		std::vector<Information const> CollectInformation(std::vector<TechniqueBuildingInformation const*> const& includes)
		{
			std::vector<Information const> informations;
			for (auto builder : includes)
			{
				for (Information const& information : XREX_POINTER_CALL_MEMBER_FUNCTION(builder, Getter)())
				{
					informations.push_back(information);
				}
			}
			return informations;
		}
		/*
		 *	Check there is nothing in includes.
		 */
		template <typename Information, std::vector<Information const> const&(TechniqueBuildingInformation::*Getter)() const>
		bool CheckNoInclude(TechniqueBuildingInformationSP const& rootInformation, std::vector<TechniqueBuildingInformation const*> const& includes)
		{
			for (auto information : includes)
			{
				if (information != rootInformation.get())
				{
					if (!XREX_POINTER_CALL_MEMBER_FUNCTION(information, Getter)().empty())
					{
						return false;
					}
				}
			}
			return true;
		}

		std::unordered_map<std::string, SamplerState> CollectSamplerState(std::vector<TechniqueBuildingInformation const*> const& includes)
		{
			std::unordered_map<std::string, SamplerState> samplerStates;
			for (auto information : includes)
			{
				for (auto& samplerState : information->GetAllSamplerStates())
				{
					samplerStates.insert(std::move(samplerState));
				}
			}
			return samplerStates;
		}

		std::vector<FrameBufferLayoutDescriptionSP> CollectFrameBufferDescription(std::vector<TechniqueBuildingInformation const*> const& includes)
		{
			std::vector<FrameBufferLayoutDescriptionSP> descriptions;
			for (auto builder : includes)
			{
				FrameBufferLayoutDescriptionSP const& description = builder->GetFrameBufferDescription();
				if (description != nullptr)
				{
					descriptions.push_back(description);
				}
			}
			return descriptions;
		}

		std::vector<FragmentOutputInformation const> GenerateFragmentOutputInformations(FrameBufferLayoutDescriptionSP const& framebufferDescription)
		{
			std::vector<FragmentOutputInformation const> fragmentOutputs;
			for (auto& channelDescription : framebufferDescription->GetAllChannels())
			{
				fragmentOutputs.push_back(FragmentOutputInformation(channelDescription.GetChannel(), Texture::TexelTypeFromTexelFormat(channelDescription.GetFormat())));
			}
			return fragmentOutputs;
		}


	}

	RenderingTechniqueSP TechniqueBuilder::Create()
	{
		bool succeed = true;

		std::vector<TechniqueBuildingInformation const*> includes = GetIncludeList();

		BuildMacroStrings(); // build macro strings before CollectFullShaderCommonCode
		std::vector<std::string const*> collectedCommonCodes = CollectFullShaderCommonCode(includes);

		ProgramObjectSP program = XREXContext::GetInstance().GetRenderingFactory().CreateProgramObject();
		for (uint32 stageIndex = 0; stageIndex < static_cast<uint32>(ShaderObject::ShaderType::ShaderTypeCount); ++stageIndex)
		{
			ShaderObject::ShaderType stage = static_cast<ShaderObject::ShaderType>(stageIndex);
			if (!techniqueInformation_->GetStageCodes(stage).empty())
			{
				std::vector<std::string const*> codes = CollectFullShaderStageCode(collectedCommonCodes, stage);
				ShaderObjectSP shader = XREXContext::GetInstance().GetRenderingFactory().CreateShaderObject(stage);
				bool compileResult = shader->Compile(codes);
				if (compileResult)
				{
					program->AttachShader(shader);
				}
				else
				{
					XREXContext::GetInstance().GetLogger().LogLine("Shader compile failed:").LogLine(shader->GetCompileError());
					succeed = false;
				}
			}
		}

		if (techniqueInformation_->GetDepthStencilState().IsDepthReadEnabled() || techniqueInformation_->GetDepthStencilState().IsDepthWriteEnabled())
		{
			if (!techniqueInformation_->GetFrameBufferDescription()->IsDepthEnabled())
			{
				XREXContext::GetInstance().GetLogger().LogLine("Framebuffer depth component missing.");
				succeed = false;
			}
		}
		if (techniqueInformation_->GetDepthStencilState().IsStencilReadEnabled() || techniqueInformation_->GetDepthStencilState().IsStencilWriteEnabled())
		{
			if (!techniqueInformation_->GetFrameBufferDescription()->IsStencilEnabled())
			{
				XREXContext::GetInstance().GetLogger().LogLine("Framebuffer stencil component missing.");
				succeed = false;
			}
		}

		if (!CheckNoInclude<AttributeInputInformation, &TechniqueBuildingInformation::GetAllAttributeInputInformations>(techniqueInformation_, includes))
		{
			XREXContext::GetInstance().GetLogger().LogLine("Other techniques have attribute inputs.");
			succeed = false;
		}

		std::vector<FragmentOutputInformation const> fragmentOutputs;
		std::vector<FrameBufferLayoutDescriptionSP> framebufferDescriptions = CollectFrameBufferDescription(includes);
		if (framebufferDescriptions.size() > 1)
		{
			XREXContext::GetInstance().GetLogger().LogLine("Too many framebuffer descriptions exist, should only have one in all include techniques.");
			succeed = false;
		}
		else if (framebufferDescriptions.empty())
		{
			XREXContext::GetInstance().GetLogger().LogLine("No framebuffer description exist.");
			succeed = false;
		}
		else if (framebufferDescriptions.size() == 1)
		{
			fragmentOutputs = GenerateFragmentOutputInformations(framebufferDescriptions.back());
		}
		else
		{
			assert(false); // impossible
		}
		std::vector<AttributeInputInformation const> const& attributes = techniqueInformation_->GetAllAttributeInputInformations();
		std::vector<BufferInformation const> uniformBuffers = CollectInformation<BufferInformation, &TechniqueBuildingInformation::GetAllUniformBufferInformations>(includes);
		std::vector<BufferInformation const> shaderStorageBuffers = CollectInformation<BufferInformation, &TechniqueBuildingInformation::GetAllShaderStorageBufferInformations>(includes);
		std::vector<BufferInformation const> atomicCounterBuffers = CollectInformation<BufferInformation, &TechniqueBuildingInformation::GetAllAtomicCounterBufferInformations>(includes);
		std::vector<TextureInformation const> textures = CollectInformation<TextureInformation, &TechniqueBuildingInformation::GetAllTextureInformations>(includes);
		std::vector<ImageInformation const> images = CollectInformation<ImageInformation, &TechniqueBuildingInformation::GetAllImageInformations>(includes);

		bool linkResult = program->Link(ProgramObject::InformationPack(attributes, fragmentOutputs, uniformBuffers, shaderStorageBuffers, atomicCounterBuffers, textures, images));

		if (!linkResult)
		{
			XREXContext::GetInstance().GetLogger().LogLine("Shader link failed:").LogLine(program->GetLinkError());
			succeed = false;
		}

		RasterizerStateObjectSP rasterizer = XREXContext::GetInstance().GetRenderingFactory().CreateRasterizerStateObject(techniqueInformation_->GetRasterizerState());
		DepthStencilStateObjectSP depthStencil = XREXContext::GetInstance().GetRenderingFactory().CreateDepthStencilStateObject(techniqueInformation_->GetDepthStencilState());
		BlendStateObjectSP blend = XREXContext::GetInstance().GetRenderingFactory().CreateBlendStateObject(techniqueInformation_->GetBlendState());

		std::unordered_map<std::string, SamplerState> samplerStates = CollectSamplerState(includes);
		std::unordered_map<std::string, SamplerSP> samplers; // sampler channel : Sampler
		std::unordered_map<std::string, SamplerSP> createdSamplers; // sampler name : Sampler
		for (auto& samplerState : samplerStates)
		{
			SamplerSP sampler = XREXContext::GetInstance().GetRenderingFactory().CreateSampler(samplerState.second);
			createdSamplers[samplerState.first] = sampler;
		}
		for (auto& texture : textures)
		{
			std::string const& samplerChannel = texture.GetChannel();
			std::string const& samplerName = texture.GetSamplerName();
			auto found = createdSamplers.find(samplerName);
			if (found != createdSamplers.end())
			{
				samplers[samplerChannel] = found->second;
			}
			else
			{
				XREXContext::GetInstance().GetLogger().BeginLine().Log("Sampler channel: ").Log(samplerChannel)
					.Log(" mapped sampler name: ").Log(samplerName).Log(" not found.").EndLine();
				succeed = false;
			}
		}

		RenderingTechniqueSP renderingTechnique;
		if (succeed)
		{
			RenderingTechnique::ConstructerParameterPack pack(techniqueInformation_, program, rasterizer, depthStencil, blend, std::move(samplers));
			renderingTechnique = MakeSP<RenderingTechnique>(std::move(pack));
			technique_ = renderingTechnique;
		}
		else
		{
			XREXContext::GetInstance().GetLogger().BeginLine().Log("Technique: ").Log(techniqueInformation_->GetName()).Log(", build failed.").EndLine();
		}
		return renderingTechnique;
	}


	void TechniqueBuilder::BuildMacroStrings()
	{
		for (auto& macro : macros_)
		{
			std::stringstream ss;
			ss << "#define " << macro.first << " " << macro.second << std::endl;
			lastBuiltMacros_.push_back(ss.str());
		}
	}

	std::vector<TechniqueBuildingInformation const*> TechniqueBuilder::GetIncludeList() const
	{
		std::unordered_set<TechniqueBuildingInformation const*> included;
		std::vector<TechniqueBuildingInformation const*> includeList;

		// recursively add included effects in order.
		std::function<void(TechniqueBuildingInformationSP const&)> makeIncludeLists = [&makeIncludeLists, &includeList, &included] (TechniqueBuildingInformationSP const& techniqueInformation)
		{
			// process includes first
			for (auto& include : techniqueInformation->GetAllIncludes())
			{
				makeIncludeLists(include);
			}
			// then add technique itself
			if (included.find(techniqueInformation.get()) == included.end()) // only technique information not included will be added.
			{
				included.insert(techniqueInformation.get());
				includeList.push_back(techniqueInformation.get());
			};
		};

		makeIncludeLists(techniqueInformation_);
		return includeList;
	}

	std::vector<std::string const*> TechniqueBuilder::CollectFullShaderCommonCode(std::vector<TechniqueBuildingInformation const*> const& includeList) const
	{
		std::vector<std::string const*> result;
		for (std::string const& macro : lastBuiltMacros_)
		{
			result.push_back(&macro);
		}
		for (auto include : includeList)
		{
			for (std::shared_ptr<std::string> const& code : include->GetAllCommonCodes())
			{
				result.push_back(code.get());
			}
		}
		return result;
	}

	std::vector<std::string const*> TechniqueBuilder::CollectFullShaderStageCode(std::vector<std::string const*> const& commonCodes, ShaderObject::ShaderType stage) const
	{
		if (techniqueInformation_->GetStageCodes(stage).empty())
		{
			assert(false);
		}

		std::vector<std::string const*> result;
		result.reserve(result.size() + techniqueInformation_->GetStageCodes(stage).size());
		for (auto& code : commonCodes)
		{
			result.push_back(code);
		}
		for (auto& code : techniqueInformation_->GetStageCodes(stage))
		{
			result.push_back(code.get());
		}
		return result;
	}


	FrameBufferSP FrameBufferBuilder::GetFrameBuffer()
	{
		if (framebuffer_.expired())
		{
			return Create();
		}
		return framebuffer_.lock();
	}

	XREX::FrameBufferSP FrameBufferBuilder::Create()
	{
		Size<uint32, 2> size = XREXContext::GetInstance().GetMainWindow().GetClientRegionSize();
		FrameBufferLayoutDescription::SizeMode sizeMode = description_->GetSizeMode();
		switch (sizeMode)
		{
		case FrameBufferLayoutDescription::SizeMode::Fixed:
			size = description_->GetSize();
			break;
		case FrameBufferLayoutDescription::SizeMode::Sceen:
			description_->SetSize(size);
			break;
		case FrameBufferLayoutDescription::SizeMode::HalfSceen:
			size = Size<uint32, 2>(size.X() / 2, size.Y() / 2);
			description_->SetSize(size);
			break;
		default:
			assert(false);
			break;
		}
		std::unordered_map<std::string, Texture2DImageSP const> channelTextureImages;
		for (auto& channel : description_->GetAllChannels())
		{
			Texture::DataDescription<2> textureDescription(channel.GetFormat(), size);
			Texture2DSP texture = XREXContext::GetInstance().GetRenderingFactory().CreateTexture2D(textureDescription, false);
			channelTextureImages.insert(std::make_pair(channel.GetChannel(), texture->GetImage(0)));
		}

		FrameBufferLayoutDescription::DepthStencilCombinationState combinationState = description_->GetDepthStencilCombinationState();
		
		FrameBuffer::DepthStencilBinding depthStencil;
		switch (combinationState)
		{
		case FrameBufferLayoutDescription::DepthStencilCombinationState::None:
			break;
		case FrameBufferLayoutDescription::DepthStencilCombinationState::DepthOnly:
			{
				Texture::DataDescription<2> depthDescription(description_->GetDepthFormat(), size);
				Texture2DSP depthTexture = XREXContext::GetInstance().GetRenderingFactory().CreateTexture2D(depthDescription, false);
				depthStencil = FrameBuffer::DepthStencilBinding(depthTexture->GetImage(0), nullptr);
			}
			break;
		case FrameBufferLayoutDescription::DepthStencilCombinationState::StencilOnly:
			{
				Texture::DataDescription<2> stencilDescription(description_->GetStencilFormat(), size);
				Texture2DSP stencilTexture = XREXContext::GetInstance().GetRenderingFactory().CreateTexture2D(stencilDescription, false);
				depthStencil = FrameBuffer::DepthStencilBinding(nullptr, stencilTexture->GetImage(0));
			}
			break;
		case FrameBufferLayoutDescription::DepthStencilCombinationState::Separate:
			{
				Texture::DataDescription<2> depthDescription(description_->GetDepthFormat(), size);
				Texture2DSP depthTexture = XREXContext::GetInstance().GetRenderingFactory().CreateTexture2D(depthDescription, false);
				Texture::DataDescription<2> stencilDescription(description_->GetStencilFormat(), size);
				Texture2DSP stencilTexture = XREXContext::GetInstance().GetRenderingFactory().CreateTexture2D(stencilDescription, false);
				depthStencil = FrameBuffer::DepthStencilBinding(depthTexture->GetImage(0), stencilTexture->GetImage(0));
			}
			break;
		case FrameBufferLayoutDescription::DepthStencilCombinationState::Combined:
			{
				Texture::DataDescription<2> depthStencilDescription(description_->GetDepthStencilFormat(), size);
				Texture2DSP depthStencilTexture = XREXContext::GetInstance().GetRenderingFactory().CreateTexture2D(depthStencilDescription, false);
				depthStencil = FrameBuffer::DepthStencilBinding(depthStencilTexture->GetImage(0));
			}
			break;
		default:
			assert(false);
			break;
		}

		FrameBufferSP result = XREXContext::GetInstance().GetRenderingFactory().CreateFrameBuffer(description_, std::move(channelTextureImages), depthStencil);
		framebuffer_ = result;
		return result;
	}

}
