#include "XREX.hpp"

#include "RenderingTechnique.hpp"


#include "Base/XREXContext.hpp"
#include "Base/Logger.hpp"
#include "Rendering/ShaderProgram.hpp"
#include "Rendering/Texture.hpp"
#include "Rendering/Sampler.hpp"
#include "Rendering/TextureImage.hpp"
#include "Rendering/RenderingPipelineState.hpp"
#include "Rendering/RenderingFactory.hpp"

#include <algorithm>
#include <sstream>

using std::string;
using std::vector;

namespace XREX
{

	TechniqueParameterSP const TechniqueParameter::NullTechniqueParameter = nullptr;




	TechniquePipelineParameters::TechniquePipelineParameters()
		: polygonOffsetFactor(0), polygonOffsetUnits(0), frontStencilReference(0), backStencilReference(0), blendFactor(1.f, 1.f, 1.f, 1.f)
	{
	}


	RenderingTechnique::RenderingTechnique(std::string const& name, ProgramObjectSP const& program,
		RasterizerStateObjectSP const& rasterizerState, DepthStencilStateObjectSP const& depthStencilState, BlendStateObjectSP const& blendState,
		std::unordered_map<std::string, SamplerSP>&& samplers)
		: name_(name), program_(program), rasterizerState_(rasterizerState), depthStencilState_(depthStencilState), blendState_(blendState),
		samplers_(std::move(samplers))
	{
		InitializeParameterInformations();
	}

	RenderingTechnique::~RenderingTechnique()
	{
	}


	void RenderingTechnique::SetFrameBuffer(FrameBufferSP const& framebuffer)
	{
#ifdef XREX_DEBUG
		FrameBufferLayoutDescription const& outputLayout = program_->GetFragmentOutputLayout();
		FrameBufferLayoutDescription const& framebufferLayout = framebuffer->GetLayoutDescription();
		if (outputLayout.GetColorChannelCount() == framebufferLayout.GetColorChannelCount())
		{
			for (uint32 i = 0; i < outputLayout.GetAllColorChannels().size(); ++i)
			{
				if (outputLayout.GetAllColorChannels()[i].GetChannel() != framebufferLayout.GetAllColorChannels()[i].GetChannel())
				{
					assert(false);
				}
			}
		}
		if (outputLayout.GetDepthEnabled())
		{
			assert(framebufferLayout.GetDepthEnabled());
		}
		if (outputLayout.GetStencilEnabled())
		{
			assert(framebufferLayout.GetStencilEnabled());
		}
#endif // XREX_DEBUG
		framebuffer_ = framebuffer;

	}


	TechniqueParameterSP const& RenderingTechnique::GetParameterByName(string const& name)
	{
		auto i = find_if(parameters_.begin(), parameters_.end(), [&name] (TechniqueParameterSP const& parameter)
		{
			return parameter->GetName() == name;
		});
		if (i == parameters_.end())
		{
			return TechniqueParameter::NullTechniqueParameter;
		}
		return *i;
	}


	void RenderingTechnique::Use()
	{
		program_->Bind();
		SetupAllResources();
	}

	void RenderingTechnique::SetupAllResources()
	{
		rasterizerState_->Bind(pipelineParameters_.polygonOffsetFactor, pipelineParameters_.polygonOffsetUnits);
		depthStencilState_->Bind(pipelineParameters_.frontStencilReference, pipelineParameters_.backStencilReference);
		blendState_->Bind(pipelineParameters_.blendFactor);

		framebuffer_->BindWrite();

		for (auto& resourceSetter : parameterSetters_)
		{
			resourceSetter();
		}
		program_->SetupAllUniforms();
	}


	void RenderingTechnique::InitializeParameterInformations()
	{
		for (auto& bufferInformation : program_->GetAllBufferInformations())
		{
			std::string const& channel = bufferInformation.GetChannel();

			std::shared_ptr<BufferParameter> bufferParameter = MakeSP<BufferParameter>(channel, bufferInformation);
			ShaderResourceBufferSP buffer = XREXContext::GetInstance().GetRenderingFactory().CreateShaderResourceBuffer(bufferInformation, true);
			bufferParameter->SetValue(buffer);
			AddParameter(bufferParameter);

			auto bufferBinder = [bufferParameter] ()
			{
				ConcreteTechniqueParameter<ShaderResourceBufferSP> const& parameter = bufferParameter->As<ShaderResourceBufferSP>();
				assert(parameter.GetType() == ElementType::ShaderResourceBuffer);
				BufferParameter const& resourceBufferParameter = CheckedCast<BufferParameter const&>(parameter);
				assert(parameter.GetValue() != nullptr);
				resourceBufferParameter.GetValue()->BindIndex(resourceBufferParameter.GetInformation().GetBindingIndex());
			};
			parameterSetters_.push_back(std::move(bufferBinder));
		}

		for (auto& textureInformation : program_->GetAllTextureInformations())
		{
			std::string const& channel = textureInformation.GetChannel();
			auto found = samplers_.find(channel);
			assert(found != samplers_.end());
			std::shared_ptr<TextureParameter> textureParameter = MakeSP<TextureParameter>(channel, textureInformation, found->second);
			AddParameter(textureParameter);

			auto textureBinder = [textureParameter] ()
			{
				ConcreteTechniqueParameter<TextureSP> const& parameter = textureParameter->As<TextureSP>();
				assert(IsSamplerType(parameter.GetType()));
				TextureParameter const& theTextureParameter = CheckedCast<TextureParameter const&>(parameter);
				//assert(theTextureParameter.GetValue() != nullptr);
				if (theTextureParameter.GetValue() == nullptr)
				{
					TextureSP defaultTexture;
					switch (theTextureParameter.GetInformation().GetElementType())
					{
					case ElementType::Sampler1D:
						defaultTexture = XREXContext::GetInstance().GetRenderingFactory().GetBlackTexture1D();
						break;
					case ElementType::Sampler2D:
						defaultTexture = XREXContext::GetInstance().GetRenderingFactory().GetBlackTexture2D();
						break;
					case ElementType::Sampler3D:
						defaultTexture = XREXContext::GetInstance().GetRenderingFactory().GetBlackTexture3D();
						break;
					default:
						assert(false); // not hacked
						break;
					}
					defaultTexture->Bind(theTextureParameter.GetInformation().GetBindingIndex());
				}
				else
				{
					theTextureParameter.GetValue()->Bind(theTextureParameter.GetInformation().GetBindingIndex());
				}
				assert(theTextureParameter.GetSampler() != nullptr);
				theTextureParameter.GetSampler()->Bind(theTextureParameter.GetInformation().GetBindingIndex());
			};
			parameterSetters_.push_back(std::move(textureBinder));
		}

		for (auto& imageInformation : program_->GetAllImageInformations())
		{
			std::string const& channel = imageInformation.GetChannel();
			std::shared_ptr<ImageParameter> imageParameter = MakeSP<ImageParameter>(channel, imageInformation);
			AddParameter(imageParameter);

			auto imageBinder = [imageParameter] ()
			{
				ConcreteTechniqueParameter<TextureImageSP> const& parameter = imageParameter->As<TextureImageSP>();
				assert(IsImageType(parameter.GetType()));
				ImageParameter const& theImageParameter = CheckedCast<ImageParameter const&>(parameter);
				assert(theImageParameter.GetValue() != nullptr);
				ImageInformation const& imageInformation = theImageParameter.GetInformation();
				theImageParameter.GetValue()->Bind(imageInformation.GetBindingIndex(), imageInformation.GetTexelFormat(), imageInformation.GetAccessType());
			};
			parameterSetters_.push_back(std::move(imageBinder));
		}

		for (auto& uniformInformation : program_->GetAllUniformInformations())
		{
			std::string const& name = uniformInformation.GetChannel();

			TechniqueParameterSP parameter;
			switch(uniformInformation.GetElementType())
			{
			case ElementType::Bool:
				{
					parameter = MakeSP<ConcreteTechniqueParameter<bool>>(name);
				}
				break;
			case ElementType::Int32:
				{
					parameter = MakeSP<ConcreteTechniqueParameter<int32>>(name);
				}
				break;
			case ElementType::IntV2:
				{
					parameter = MakeSP<ConcreteTechniqueParameter<intV2>>(name);
				}
				break;
			case ElementType::IntV3:
				{
					parameter = MakeSP<ConcreteTechniqueParameter<intV3>>(name);
				}
				break;
			case ElementType::IntV4:
				{
					parameter = MakeSP<ConcreteTechniqueParameter<intV4>>(name);
				}
				break;
			case ElementType::Uint32:
				{
					parameter = MakeSP<ConcreteTechniqueParameter<uint32>>(name);
				}
				break;
			case ElementType::UintV2:
				{
					parameter = MakeSP<ConcreteTechniqueParameter<uintV2>>(name);
				}
				break;
			case ElementType::UintV3:
				{
					parameter = MakeSP<ConcreteTechniqueParameter<uintV3>>(name);
				}
				break;
			case ElementType::UintV4:
				{
					parameter = MakeSP<ConcreteTechniqueParameter<uintV4>>(name);
				}
				break;
			case ElementType::Float:
				{
					parameter = MakeSP<ConcreteTechniqueParameter<float>>(name);
				}
				break;
			case ElementType::FloatV2:
				{
					parameter = MakeSP<ConcreteTechniqueParameter<floatV2>>(name);
				}
				break;
			case ElementType::FloatV3:
				{
					parameter = MakeSP<ConcreteTechniqueParameter<floatV3>>(name);
				}
				break;
			case ElementType::FloatV4:
				{
					parameter = MakeSP<ConcreteTechniqueParameter<floatV4>>(name);
				}
				break;
			case ElementType::Double:
				{
					parameter = MakeSP<ConcreteTechniqueParameter<double>>(name);
				}
				break;
			case ElementType::DoubleV2:
				{
					parameter = MakeSP<ConcreteTechniqueParameter<doubleV2>>(name);
				}
				break;
			case ElementType::DoubleV3:
				{
					parameter = MakeSP<ConcreteTechniqueParameter<doubleV3>>(name);
				}
				break;
			case ElementType::DoubleV4:
				{
					parameter = MakeSP<ConcreteTechniqueParameter<doubleV4>>(name);
				}
				break;
			case ElementType::FloatM44:
				{
					parameter = MakeSP<ConcreteTechniqueParameter<floatM44>>(name);
				}
				break;
			case ElementType::DoubleM44:
				{
					parameter = MakeSP<ConcreteTechniqueParameter<doubleM44>>(name);
				}
				break;

			default:
				// not support.
				assert(false);
			}
			AddParameter(parameter);

			program_->ConnectUniformParameter(uniformInformation.GetChannel(), parameter);
		}

	}


	void RenderingTechnique::AddParameter(TechniqueParameterSP const& parameter)
	{
		parameters_.push_back(parameter);
	}






	TechniqueBuilder::TechniqueBuilder(std::string const& name)
		: name_(name), stageCodes_(static_cast<uint32>(ShaderObject::ShaderType::CountOfShaderTypes))
	{
	}

	void TechniqueBuilder::SpecifyFragmentOutput(FrameBufferLayoutDescription const& description)
	{
		framebufferDescription_ = description;
	}

	void TechniqueBuilder::SpecifyImageFormat(std::string const& channel, TexelFormat format, AccessType accessType)
	{
		imageChannelInformations_.push_back(std::make_tuple(channel, format, accessType));
	}


	void TechniqueBuilder::SetSamplerChannelToSamplerStateMapping(std::string const& channel, std::string samplerName)
	{
		auto found = samplerStates_.find(samplerName);
		assert(found != samplerStates_.end());
		samplerChannelToSamplerStateMappings_[channel] = samplerName;
	}
	
	XREX::RenderingTechniqueSP TechniqueBuilder::GetRenderingTechnique()
	{
		if (technique_.expired())
		{
			return Create();
		}
		return technique_.lock();
	}

	RenderingTechniqueSP TechniqueBuilder::Create()
	{
		bool succeed = true;
		BuildMacroStrings();

		ProgramObjectSP program = XREXContext::GetInstance().GetRenderingFactory().CreateProgramObject();
		for (uint32 stageIndex = 0; stageIndex < static_cast<uint32>(ShaderObject::ShaderType::CountOfShaderTypes); ++stageIndex)
		{
			ShaderObject::ShaderType stage = static_cast<ShaderObject::ShaderType>(stageIndex);
			if (GetStageCode(stage) != nullptr)
			{
				std::vector<std::string const*> codes = GetFullShaderCode(stage);
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

		program->SpecifyFragmentOutputs(framebufferDescription_);
		for (auto& image : imageChannelInformations_)
		{
			program->SpecifyImageFormat(std::get<0>(image), std::get<1>(image), std::get<2>(image));
		}
		bool linkResult = program->Link();
		if (!linkResult)
		{
			XREXContext::GetInstance().GetLogger().LogLine("shader link failed:").LogLine(program->GetLinkError());
			succeed = false;
		}

		if (depthStencilState_.depthEnable)
		{
			if (!framebufferDescription_.GetDepthEnabled())
			{
				XREXContext::GetInstance().GetLogger().LogLine("depth state error: depth in technique is enabled but frame buffer is not.");
				succeed = false;
			}
		}
		if (depthStencilState_.stencilEnable)
		{
			if (!framebufferDescription_.GetStencilEnabled())
			{
				XREXContext::GetInstance().GetLogger().LogLine("depth state error: stencil in technique is enabled but frame buffer is not.");
				succeed = false;
			}
		}

		RasterizerStateObjectSP rasterizer = XREXContext::GetInstance().GetRenderingFactory().CreateRasterizerStateObject(rasterizerState_);
		DepthStencilStateObjectSP depthStencil = XREXContext::GetInstance().GetRenderingFactory().CreateDepthStencilStateObject(depthStencilState_);
		BlendStateObjectSP blend = XREXContext::GetInstance().GetRenderingFactory().CreateBlendStateObject(blendState_);

		std::unordered_map<std::string, SamplerSP> samplers;
		std::unordered_map<std::string, SamplerSP> createdSamplers;
		for (auto& samplerState : samplerStates_)
		{
			SamplerSP sampler = XREXContext::GetInstance().GetRenderingFactory().CreateSampler(samplerState.second);
			createdSamplers[samplerState.first] = sampler;
		}
		for (auto& samplerChannel : samplerChannelToSamplerStateMappings_)
		{
			auto found = createdSamplers.find(samplerChannel.second);
			if (found != createdSamplers.end())
			{
				samplers[samplerChannel.first] = found->second;
			}
			else
			{
				XREXContext::GetInstance().GetLogger().BeginLine().Log("sampler channel: ").Log(samplerChannel.first)
					.Log(" mapped sampler name: ").Log(samplerChannel.second).Log(" not found.").EndLine();
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


	void TechniqueBuilder::BuildMacroStrings()
	{
		for (auto& macro : macros_)
		{
			std::stringstream ss;
			ss << "#define " << macro.first << " " << macro.second << std::endl;
			std::string s = ss.str();
			generatedMacros_.push_back(std::move(s));
		}
	}

	std::vector<std::string const*> TechniqueBuilder::GetFullShaderCode(ShaderObject::ShaderType stage) const
	{
		if (GetStageCode(stage) == nullptr)
		{
			assert(false);
		}
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

		std::vector<std::string const*> result;
		for (auto& generatedMacro : generatedMacros_)
		{
			result.push_back(&generatedMacro);
		}
		for (auto include : includeList)
		{
			for (auto code : include->GetAllCommonCodes())
			{
				result.push_back(code.get());
			}
		}
		result.push_back(GetStageCode(stage).get());

		return result;
	}

}
