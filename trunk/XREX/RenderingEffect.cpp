#include "XREX.hpp"

#include "RenderingEffect.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Sampler.hpp"
#include "RenderingPipelineState.hpp"
#include "XREXContext.hpp"
#include "RenderingFactory.hpp"

#include <algorithm>

using std::string;
using std::vector;

namespace XREX
{

	EffectParameterSP const EffectParameter::NullEffectParameter = nullptr;

	EffectPipelineParameters::EffectPipelineParameters()
		: polygonOffsetFactor(0), polygonOffsetUnits(0), frontStencilReference(0), backStencilReference(0), blendFactor(1.f, 1.f, 1.f, 1.f)
	{
	}




	RenderingTechnique::RenderingTechniqueInternal::RenderingTechniqueInternal(RenderingEffect& effect, std::string const& name)
		: effect_(effect), name_(name)
	{
	}

	RenderingTechnique::RenderingTechnique(RenderingTechniqueInternal& techniqueInternal)
		: techniqueInternal_(techniqueInternal), effect_(techniqueInternal.effect_.shared_from_this())
	{
	}

	RenderingTechnique::~RenderingTechnique()
	{
	}

	XREX::RenderingEffectSP RenderingTechnique::GetEffect() const
	{
		return techniqueInternal_.effect_.shared_from_this();
	}

	RenderingPassSP const& RenderingTechnique::CreatePass(ProgramObjectSP& program,
		RasterizerStateObjectSP& rasterizerState, DepthStencilStateObjectSP& depthStencilState, BlendStateObjectSP& blendState)
	{
		auto pass = RenderingPassSP(new RenderingPass(this->techniqueInternal_, program, rasterizerState, depthStencilState, blendState)); // private member cannot be accessed by shared_ptr
		techniqueInternal_.passes_.emplace_back(pass);
		return techniqueInternal_.passes_.back();
	}






	RenderingPass::RenderingPass(RenderingTechnique::RenderingTechniqueInternal& technique, ProgramObjectSP& program,
		RasterizerStateObjectSP& rasterizerState, DepthStencilStateObjectSP& depthStencilState, BlendStateObjectSP& blendState)
		: technique_(technique), program_(program), rasterizerState_(rasterizerState), depthStencilState_(depthStencilState), blendState_(blendState)
	{
		InitializeParameterSetters();
	}
	RenderingPass::~RenderingPass()
	{
	}

	XREX::RenderingTechniqueSP RenderingPass::GetTechnique() const
	{
		return RenderingTechniqueSP(new RenderingTechnique(technique_)); // private member cannot be accessed by shared_ptr
	}



	void RenderingPass::Use()
	{
		rasterizerState_->Bind(pipelineParameters_.polygonOffsetFactor, pipelineParameters_.polygonOffsetUnits);
		depthStencilState_->Bind(pipelineParameters_.frontStencilReference, pipelineParameters_.backStencilReference);
		blendState_->Bind(pipelineParameters_.blendFactor);
		program_->Bind();
	}

	void RenderingPass::InitializeParameterSetters()
	{
		vector<EffectParameterSP> const& parameters = technique_.effect_.GetAllParameters();

		uint32 availableSamplerLocation = 0;
		uint32 availableImageLocation = 0;
		uint32 availableBufferLocation = 0; // not used currently

		for (auto& bufferInformation : program_->GetAllBufferInformations())
		{
			std::string const& bufferName = bufferInformation.GetName();
			auto resultIter = std::find_if(parameters.begin(), parameters.end(), [&bufferName] (EffectParameterSP const& parameter)
			{
				return parameter->GetName() == bufferName;
			});

			EffectParameterSP parameter;
			if (resultIter == parameters.end()) // not exist
			{
				std::shared_ptr<ConcreteEffectParameter<ShaderResourceBufferSP>> bufferParameter = MakeSP<ConcreteEffectParameter<ShaderResourceBufferSP>>(bufferName);
				ShaderResourceBufferSP buffer = XREXContext::GetInstance().GetRenderingFactory().CreateShaderResourceBuffer(bufferInformation, true);
				bufferParameter->SetValue(buffer);
				parameter = bufferParameter;
				technique_.effect_.AddParameter(parameter);
			}
			else
			{
				parameter = *resultIter;
				// check if uniform type in this shader not equals to type of parameter created by other shader.
				assert(parameter->GetType() == ElementType::Buffer);
			}

			program_->CreateBufferBinder(bufferName, parameter);
		}

		for (auto& uniformInformation : program_->GetAllUniformInformations())
		{
			std::string const& name = uniformInformation.GetChannel();

			auto resultIter = std::find_if(parameters.begin(), parameters.end(), [&name] (EffectParameterSP const& parameter)
			{
				return parameter->GetName() == name;
			});

			EffectParameterSP parameter;
			if (resultIter == parameters.end()) // not exist
			{
				switch(uniformInformation.GetElementType())
				{
				case ElementType::Bool:
					{
						parameter = MakeSP<ConcreteEffectParameter<bool>>(name);
					}
					break;
				case ElementType::Int32:
					{
						parameter = MakeSP<ConcreteEffectParameter<int32>>(name);
					}
					break;
				case ElementType::IntV2:
					{
						parameter = MakeSP<ConcreteEffectParameter<intV2>>(name);
					}
					break;
				case ElementType::IntV3:
					{
						parameter = MakeSP<ConcreteEffectParameter<intV3>>(name);
					}
					break;
				case ElementType::IntV4:
					{
						parameter = MakeSP<ConcreteEffectParameter<intV4>>(name);
					}
					break;
				case ElementType::Uint32:
					{
						parameter = MakeSP<ConcreteEffectParameter<uint32>>(name);
					}
					break;
				case ElementType::UintV2:
					{
						parameter = MakeSP<ConcreteEffectParameter<uintV2>>(name);
					}
					break;
				case ElementType::UintV3:
					{
						parameter = MakeSP<ConcreteEffectParameter<uintV3>>(name);
					}
					break;
				case ElementType::UintV4:
					{
						parameter = MakeSP<ConcreteEffectParameter<uintV4>>(name);
					}
					break;
				case ElementType::Float:
					{
						parameter = MakeSP<ConcreteEffectParameter<float>>(name);
					}
					break;
				case ElementType::FloatV2:
					{
						parameter = MakeSP<ConcreteEffectParameter<floatV2>>(name);
					}
					break;
				case ElementType::FloatV3:
					{
						parameter = MakeSP<ConcreteEffectParameter<floatV3>>(name);
					}
					break;
				case ElementType::FloatV4:
					{
						parameter = MakeSP<ConcreteEffectParameter<floatV4>>(name);
					}
					break;
				case ElementType::Double:
					{
						parameter = MakeSP<ConcreteEffectParameter<double>>(name);
					}
					break;
				case ElementType::DoubleV2:
					{
						parameter = MakeSP<ConcreteEffectParameter<doubleV2>>(name);
					}
					break;
				case ElementType::DoubleV3:
					{
						parameter = MakeSP<ConcreteEffectParameter<doubleV3>>(name);
					}
					break;
				case ElementType::DoubleV4:
					{
						parameter = MakeSP<ConcreteEffectParameter<doubleV4>>(name);
					}
					break;
				case ElementType::FloatM44:
					{
						parameter = MakeSP<ConcreteEffectParameter<floatM44>>(name);
					}
					break;
				case ElementType::DoubleM44:
					{
						parameter = MakeSP<ConcreteEffectParameter<doubleM44>>(name);
					}
					break;

				default:
					if (IsSamplerType(uniformInformation.GetElementType()))
					{
						parameter = MakeSP<ConcreteEffectParameter<std::pair<TextureSP, SamplerSP>>>(name);
					}
					else if (IsImageType(uniformInformation.GetElementType()))
					{
						parameter = MakeSP<ConcreteEffectParameter<TextureImageSP>>(name);
					}
					else
					{
						// not support.
						assert(false);
					}
				}
				technique_.effect_.AddParameter(parameter);
			}
			else
			{
				parameter = *resultIter;
				// check if uniform type in this shader not equals to type of parameter created by other shader.
				assert(parameter->GetType() == uniformInformation.GetElementType());
			}

			ElementType type = uniformInformation.GetElementType();

			if (IsSamplerType(type))
			{
				program_->CreateSamplerUniformBinder(uniformInformation.GetChannel(), parameter, availableSamplerLocation);
				availableSamplerLocation++;
			}
			else if (IsImageType(type))
			{
				program_->CreateImageUniformBinder(uniformInformation.GetChannel(), parameter, availableImageLocation);
				availableImageLocation++;
			}
			else if (IsAtomicBufferType(type))
			{
				assert(false); // should be handled in buffer part
			}
			else
			{
				program_->CreateUniformBinder(uniformInformation.GetChannel(), parameter);
			}
		}

	}




	RenderingTechniqueSP const RenderingEffect::NullRenderingTechnique = nullptr;

	RenderingEffect::RenderingEffect(string const& name)
		: name_(name)
	{
	}


	RenderingEffect::~RenderingEffect()
	{
	}

	std::vector<std::string const*> RenderingEffect::GetFullShaderCode() const
	{
		std::unordered_set<RenderingEffect const*> included;
		std::vector<RenderingEffect const*> includeList;

		// recursively add included effects in order.
		std::function<void(RenderingEffect const& effect)> makeIncludeLists = [&makeIncludeLists, &includeList, &included] (RenderingEffect const& effect)
		{
			// process includes first
			for (auto& include : effect.GetAllIncludes())
			{
				makeIncludeLists(*include);
			}
			// then add effect itself
			if (included.find(&effect) == included.end()) // only effect not included will be added.
			{
				included.insert(&effect);
				includeList.push_back(&effect);
			};
		};

		makeIncludeLists(*this);

		std::vector<std::string const*> result;
		for (auto include : includeList)
		{
			for (auto& code : include->GetAllShaderCodes())
			{
				result.push_back(&code);
			}
		}
		return result;
	}


	EffectParameterSP const& RenderingEffect::GetParameterByName(string const& name) const
	{
		auto i = std::find_if(parameters_.begin(), parameters_.end(), [&name] (EffectParameterSP const& parameter)
		{
			return parameter->GetName() == name;
		});
		if (i == parameters_.end())
		{
			return EffectParameter::NullEffectParameter;
		}
		return *i;
	}

	XREX::RenderingTechniqueSP RenderingEffect::GetTechnique(uint32 techniqueIndex) const
	{
		return RenderingTechniqueSP(new RenderingTechnique(*techniques_[techniqueIndex])); // private member cannot be accessed by shared_ptr
	}
	
	RenderingTechniqueSP RenderingEffect::GetTechniqueByName(std::string const& name) const
	{
		auto i = std::find_if(techniques_.begin(), techniques_.end(), [&name] (std::unique_ptr<RenderingTechnique::RenderingTechniqueInternal> const& technique)
		{
			return technique->name_ == name;
		});
		if (i == techniques_.end())
		{
			return NullRenderingTechnique;
		}
		return RenderingTechniqueSP(new RenderingTechnique(**i)); // private member cannot be accessed by shared_ptr
	}

	RenderingTechniqueSP RenderingEffect::CreateTechnique(std::string const& name)
	{
		techniques_.emplace_back(MakeUP<RenderingTechnique::RenderingTechniqueInternal>(*this, name));
		return RenderingTechniqueSP(new RenderingTechnique(*techniques_.back())); // private member cannot be accessed by shared_ptr
	}

}
