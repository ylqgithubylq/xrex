#include "XREX.hpp"

#include "RenderingTechnique.hpp"


#include "Base/XREXContext.hpp"
#include "Rendering/RenderingFactory.hpp"
#include "Rendering/ShaderProgram.hpp"
#include "Rendering/Texture.hpp"
#include "Rendering/Sampler.hpp"
#include "Rendering/TextureImage.hpp"
#include "Rendering/FrameBuffer.hpp"

#include <algorithm>
#include <sstream>

using std::string;
using std::vector;

namespace XREX
{
	std::string const XREXPrefix = "XREX_";

	TechniqueParameterSP const TechniqueParameter::NullTechniqueParameter = nullptr;




	XREX::ElementType SimpleTextureParameter::GetType() const 
	{
		switch (GetValue()->GetType())
		{
		case Texture::TextureType::Texture1D:
			return ElementType::Sampler1D;
		case Texture::TextureType::Texture2D:
			return ElementType::Sampler2D;
		case Texture::TextureType::Texture3D:
			return ElementType::Sampler3D;
		case Texture::TextureType::TextureCube:
			return ElementType::SamplerCube;
		case Texture::TextureType::TextureBuffer:
			return ElementType::SamplerBuffer;
		default:
			assert(false);
			return ElementType::ElementTypeCount;
		}
	}


	XREX::ElementType SimpleImageParameter::GetType() const 
	{
		switch (GetValue()->GetType())
		{
		case TextureImage::ImageType::Image1D:
			return ElementType::Image1D;
		case TextureImage::ImageType::Image2D:
			return ElementType::Image2D;
		case TextureImage::ImageType::Image3D:
			return ElementType::Image3D;
		case TextureImage::ImageType::ImageCube:
			return ElementType::ImageCube;
		case TextureImage::ImageType::ImageBuffer:
			return ElementType::ImageBuffer;
		default:
			assert(false);
			return ElementType::ElementTypeCount;
		}
	}

	XREX::ElementType ImageParameter::GetType() const 
	{
		switch (GetBindingInformation().GetImageType())
		{
		case TextureImage::ImageType::Image1D:
			return ElementType::Image1D;
		case TextureImage::ImageType::Image2D:
			return ElementType::Image2D;
		case TextureImage::ImageType::Image3D:
			return ElementType::Image3D;
		case TextureImage::ImageType::ImageCube:
			return ElementType::ImageCube;
		case TextureImage::ImageType::ImageBuffer:
			return ElementType::ImageBuffer;
		default:
			assert(false);
			return ElementType::ElementTypeCount;
		}
	}


	XREX::ElementType TextureParameter::GetType() const 
	{
		switch (GetBindingInformation().GetTextureType())
		{
		case Texture::TextureType::Texture1D:
			return ElementType::Sampler1D;
		case Texture::TextureType::Texture2D:
			return ElementType::Sampler2D;
		case Texture::TextureType::Texture3D:
			return ElementType::Sampler3D;
		case Texture::TextureType::TextureCube:
			return ElementType::SamplerCube;
		case Texture::TextureType::TextureBuffer:
			return ElementType::SamplerBuffer;
		default:
			assert(false);
			return ElementType::ElementTypeCount;
		}
	}



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


	void RenderingTechnique::ConnectFrameBuffer(FrameBufferSP framebuffer)
	{
#ifdef XREX_DEBUG
		assert(framebuffer != nullptr);
		std::vector<FragmentOutputBindingInformation> const& outputLayout = program_->GetAllFragmentOutputInformations();
		FrameBufferLayoutDescriptionSP const& framebufferLayout = framebuffer->GetLayoutDescription();
		if (outputLayout.size() == framebufferLayout->GetChannelCount())
		{
			for (uint32 i = 0; i < outputLayout.size(); ++i)
			{
				if (outputLayout[i].GetTexelType() != GetTexelType(framebufferLayout->GetAllChannels()[i].GetFormat())
					|| outputLayout[i].GetChannel() != framebufferLayout->GetAllChannels()[i].GetChannel())
				{
					assert(false); // framebuffer layout mismatch
				}
			}
		}
		if (depthStencilState_->GetState().IsDepthReadEnabled() || depthStencilState_->GetState().IsDepthWriteEnabled())
		{
			assert(framebufferLayout->IsDepthEnabled());
		}
		if (depthStencilState_->GetState().IsStencilReadEnabled() || depthStencilState_->GetState().IsStencilWriteEnabled())
		{
			assert(framebufferLayout->IsStencilEnabled());
		}
#endif // XREX_DEBUG
		framebuffer_ = std::move(framebuffer);

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

		assert(framebuffer_ != nullptr);
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
			// TODO using attribute in technique BufferInformation to specify whether to create buffer.
			bool createBuffer = true;
			if (channel.size() > XREXPrefix.size() && std::equal(XREXPrefix.begin(), XREXPrefix.end(), channel.begin()))
			{ // channel start with XREXPrefix string, it is a XREX defined buffer, so do not create GraphicsBuffer for it.
				createBuffer = false;
			}
			ShaderResourceBufferSP buffer = XREXContext::GetInstance().GetRenderingFactory().CreateShaderResourceBuffer(bufferInformation, createBuffer);
			bufferParameter->SetValue(buffer);
			AddParameter(bufferParameter);

			auto bufferBinder = [bufferParameter] ()
			{
				ConcreteTechniqueParameter<ShaderResourceBufferSP> const& parameter = bufferParameter->As<ShaderResourceBufferSP>();
				assert(parameter.GetType() == ElementType::Buffer);
				BufferParameter const& resourceBufferParameter = CheckedCast<BufferParameter const&>(parameter);
				assert(parameter.GetValue() != nullptr);
				resourceBufferParameter.GetValue()->BindIndex(resourceBufferParameter.GetBindingInformation().GetBindingIndex());
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
					switch (theTextureParameter.GetBindingInformation().GetTextureType())
					{
					case Texture::TextureType::Texture1D:
						defaultTexture = XREXContext::GetInstance().GetRenderingFactory().GetBlackTexture1D();
						break;
					case Texture::TextureType::Texture2D:
						defaultTexture = XREXContext::GetInstance().GetRenderingFactory().GetBlackTexture2D();
						break;
					case Texture::TextureType::Texture3D:
						defaultTexture = XREXContext::GetInstance().GetRenderingFactory().GetBlackTexture3D();
						break;
					default:
						assert(false); // not hacked
						break;
					}
					defaultTexture->Bind(theTextureParameter.GetBindingInformation().GetBindingIndex());
				}
				else
				{
					theTextureParameter.GetValue()->Bind(theTextureParameter.GetBindingInformation().GetBindingIndex());
				}
				assert(theTextureParameter.GetSampler() != nullptr);
				theTextureParameter.GetSampler()->Bind(theTextureParameter.GetBindingInformation().GetBindingIndex());
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
				ImageBindingInformation const& imageInformation = theImageParameter.GetBindingInformation();
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


}
