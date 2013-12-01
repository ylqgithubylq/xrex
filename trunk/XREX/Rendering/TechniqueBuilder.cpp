#include "XREX.hpp"

#include "TechniqueBuilder.hpp"

#include "Base/XREXContext.hpp"
#include "Base/Logger.hpp"
#include "Rendering/RenderingFactory.hpp"
#include "Rendering/RenderingTechnique.hpp"

#include <sstream>

namespace XREX
{
	TechniqueBuilder::TechniqueBuilder(std::string const& name)
		: name_(name), stageCodes_(static_cast<uint32>(ShaderObject::ShaderType::ShaderTypeCount))
	{
	}

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
		template <typename Information, std::vector<Information const> const&(TechniqueBuilder::*Getter)() const>
		std::vector<Information const> CollectInformation(std::vector<TechniqueBuilder const*> const& includes)
		{
			std::vector<Information const> informations;
			for (auto builder : includes)
			{
				for (Information const& information : (builder->*Getter)())
				{
					informations.push_back(information);
				}
			}
			return informations;
		}
	}

	RenderingTechniqueSP TechniqueBuilder::Create()
	{
		bool succeed = true;

		std::vector<TechniqueBuilder const*> includes = GetIncludeList();

		ProgramObjectSP program = XREXContext::GetInstance().GetRenderingFactory().CreateProgramObject();
		for (uint32 stageIndex = 0; stageIndex < static_cast<uint32>(ShaderObject::ShaderType::ShaderTypeCount); ++stageIndex)
		{
			ShaderObject::ShaderType stage = static_cast<ShaderObject::ShaderType>(stageIndex);
			if (GetStageCode(stage) != nullptr)
			{
				std::vector<std::string const*> codes = CollectFullShaderCode(includes, stage);
				ShaderObjectSP shader = XREXContext::GetInstance().GetRenderingFactory().CreateShaderObject(stage);
				bool compileResult = shader->Compile(codes);
				if (!compileResult)
				{
					XREXContext::GetInstance().GetLogger().LogLine("shader compile failed:").LogLine(shader->GetCompileError());
					succeed = false;
				}
				else
				{
					program->AttachShader(shader);
				}
			}
		}

		std::vector<AttributeInputInformation const> attributes = CollectInformation<AttributeInputInformation, &TechniqueBuilder::GetAllAttributeInputInformations>(includes);
		std::vector<FragmentOutputInformation const> fragmentOutputs = CollectInformation<FragmentOutputInformation, &TechniqueBuilder::GetAllFragmentOutputInformations>(includes);
		std::vector<BufferInformation const> uniformBuffers = CollectInformation<BufferInformation, &TechniqueBuilder::GetAllUniformBufferInformations>(includes);
		std::vector<BufferInformation const> shaderStorageBuffers = CollectInformation<BufferInformation, &TechniqueBuilder::GetAllShaderStorageBufferInformations>(includes);
		std::vector<BufferInformation const> atomicCounterBuffers = CollectInformation<BufferInformation, &TechniqueBuilder::GetAllAtomicCounterBufferInformations>(includes);
		std::vector<TextureInformation const> textures = CollectInformation<TextureInformation, &TechniqueBuilder::GetAllTextureInformations>(includes);
		std::vector<ImageInformation const> images = CollectInformation<ImageInformation, &TechniqueBuilder::GetAllImageInformations>(includes);

		bool linkResult = program->Link(ProgramObject::InformationPack(attributes, fragmentOutputs, uniformBuffers, shaderStorageBuffers, atomicCounterBuffers, textures, images));

		if (!linkResult)
		{
			XREXContext::GetInstance().GetLogger().LogLine("shader link failed:").LogLine(program->GetLinkError());
			succeed = false;
		}

		RasterizerStateObjectSP rasterizer = XREXContext::GetInstance().GetRenderingFactory().CreateRasterizerStateObject(rasterizerState_);
		DepthStencilStateObjectSP depthStencil = XREXContext::GetInstance().GetRenderingFactory().CreateDepthStencilStateObject(depthStencilState_);
		BlendStateObjectSP blend = XREXContext::GetInstance().GetRenderingFactory().CreateBlendStateObject(blendState_);

		std::unordered_map<std::string, SamplerSP> samplers; // sampler channel : Sampler
		std::unordered_map<std::string, SamplerSP> createdSamplers; // sampler name : Sampler
		for (auto& samplerState : samplerStates_)
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
				XREXContext::GetInstance().GetLogger().BeginLine().Log("sampler channel: ").Log(samplerChannel)
					.Log(" mapped sampler name: ").Log(samplerName).Log(" not found.").EndLine();
				succeed = false;
			}
		}

		RenderingTechniqueSP renderingTechnique;
		if (succeed)
		{
			renderingTechnique = MakeSP<RenderingTechnique>(name_, program, rasterizer, depthStencil, blend, std::move(samplers));
			technique_ = renderingTechnique;
		}
		return renderingTechnique;
	}


	std::vector<std::string> TechniqueBuilder::BuildMacroStrings() const
	{
		std::vector<std::string> generatedMacros;
		for (auto& macro : macros_)
		{
			std::stringstream ss;
			ss << "#define " << macro.first << " " << macro.second << std::endl;
			generatedMacros.push_back(ss.str());
		}
		return generatedMacros;
	}

	std::vector<TechniqueBuilder const*> TechniqueBuilder::GetIncludeList() const
	{
		std::unordered_set<TechniqueBuilder const*> included;
		std::vector<TechniqueBuilder const*> includeList;

		// recursively add included effects in order.
		std::function<void(TechniqueBuilder const& technique)> makeIncludeLists = [&makeIncludeLists, &includeList, &included] (TechniqueBuilder const& technique)
		{
			// process includes first
			for (auto& include : technique.GetAllIncludes())
			{
				makeIncludeLists(*include);
			}
			// then add technique itself
			if (included.find(&technique) == included.end()) // only technique not included will be added.
			{
				included.insert(&technique);
				includeList.push_back(&technique);
			};
		};

		makeIncludeLists(*this);
		return includeList;
	}

	std::vector<std::string const*> TechniqueBuilder::CollectFullShaderCode(std::vector<TechniqueBuilder const*> const& includeList, ShaderObject::ShaderType stage) const
	{
		if (GetStageCode(stage) == nullptr)
		{
			assert(false);
		}

		std::vector<std::string const*> result;
		for (auto include : includeList)
		{
			for (std::string const& macro : include->BuildMacroStrings())
			{
				result.push_back(&macro);
			}
			for (std::shared_ptr<std::string> const& code : include->GetAllCommonCodes())
			{
				result.push_back(code.get());
			}
		}
		result.push_back(GetStageCode(stage).get());

		return result;
	}

}
