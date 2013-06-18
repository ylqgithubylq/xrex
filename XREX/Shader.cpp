#include "XREX.hpp"

#include "XREXContext.hpp"
#include "RenderingFactory.hpp"
#include "Shader.hpp"
#include "RenderingEffect.hpp"
#include "GLUtil.hpp"
#include "GraphicsBuffer.hpp"
#include "Texture.hpp"
#include "Sampler.hpp"
#include "TextureImage.hpp"

#include "GLUtil.hpp"

#include <CoreGL.hpp>


#include <iostream>
#include <algorithm>
#include <array>


using std::string;
using std::vector;

namespace XREX
{

	namespace
	{
		string const& ShaderDefineMacroFromShaderType(ShaderObject::ShaderType type)
		{
			static std::array<string, static_cast<uint32>(ShaderObject::ShaderType::CountOfShaderTypes)> const mapping = [] ()
			{
				std::remove_const<decltype(mapping)>::type macros;
				macros[static_cast<uint32>(ShaderObject::ShaderType::VertexShader)] = "#define VS\n";
				macros[static_cast<uint32>(ShaderObject::ShaderType::FragmentShader)] = "#define FS\n";
				macros[static_cast<uint32>(ShaderObject::ShaderType::GeometryShader)] = "#define GS\n";
				macros[static_cast<uint32>(ShaderObject::ShaderType::TessellationControlShader)] = "#define TCS\n";
				macros[static_cast<uint32>(ShaderObject::ShaderType::TessellationEvaluationShader)] = "#define TES\n";
				return macros;
			} ();
			return mapping[static_cast<uint32>(type)];
		}

		string const& VersionMacro()
		{
			return XREXContext::GetInstance().GetRenderingFactory().GetGLSLVersionString();
		}

	}



	ShaderObject::ShaderObject(ShaderType type)
		: type_(type), validate_(false)
	{
		glShaderID_ = gl::CreateShader(GLShaderTypeFromShaderType(type_));
		assert(glShaderID_ != 0);
	}


	ShaderObject::~ShaderObject()
	{
		if (glShaderID_ != 0)
		{
			gl::DeleteShader(glShaderID_);
			glShaderID_ = 0;
		}
	}




	bool ShaderObject::Compile(std::vector<std::string const*> const& sources)
	{
		string const& macroToDefine = ShaderDefineMacroFromShaderType(type_);

		std::vector<char const*> cstrings;
		cstrings.push_back(VersionMacro().c_str());
		cstrings.push_back(macroToDefine.c_str());
		for (auto source : sources)
		{
			cstrings.push_back(source->c_str());
		}
		gl::ShaderSource(glShaderID_, cstrings.size(), cstrings.data(), nullptr);
		gl::CompileShader(glShaderID_);

		int32 sourceLength;
		gl::GetShaderiv(glShaderID_, gl::GL_SHADER_SOURCE_LENGTH, &sourceLength); // '\0' included
		source_.resize(sourceLength);
		gl::GetShaderSource(glShaderID_, sourceLength, &sourceLength, &source_[0]); // write back actual source content

		int32 compiled = 0;
		gl::GetShaderiv(glShaderID_, gl::GL_COMPILE_STATUS, &compiled);
		validate_ = compiled == 1;

	#ifdef XREX_DEBUG
		if (!validate_)
		{
			int32 length = 0;
			gl::GetShaderiv(glShaderID_, gl::GL_INFO_LOG_LENGTH, &length);
			if (length > 0)
			{
				errorString_.resize(length, 0);
				gl::GetShaderInfoLog(glShaderID_, length, &length, &errorString_[0]);
			}
			else
			{
				errorString_ = "Unknown compiling error.";
			}
			std::cerr << errorString_ << std::endl;
		}
	#endif

		return validate_;
	}








	ProgramObject::ProgramObject()
		: shaders_(static_cast<uint32>(ShaderObject::ShaderType::CountOfShaderTypes)), validate_(false)
	{
		glProgramID_ = gl::CreateProgram();
		assert(glProgramID_ != 0);
	}
	ProgramObject::~ProgramObject()
	{
		for (int32 i = 0; i < static_cast<uint32>(ShaderObject::ShaderType::CountOfShaderTypes); ++i)
		{
			if (shaders_[i])
			{
				uint32 id = shaders_[i]->GetID();
				gl::DetachShader(glProgramID_, id);
			}
		}
		if (glProgramID_ != 0)
		{
			gl::DeleteProgram(glProgramID_);
			glProgramID_ = 0;
		}
	}

	void ProgramObject::AttachShader(ShaderObjectSP& shader)
	{
		assert(shader->IsValidate());
		gl::AttachShader(glProgramID_, shader->GetID());
		shaders_[static_cast<uint32>(shader->GetType())] = shader;
	}


	bool ProgramObject::Link()
	{
		if (glProgramID_ == 0)
		{
			errorString_ = "Program creation failed.";
			return false;
		}
		gl::LinkProgram(glProgramID_);

		int32 linked = 0;
		gl::GetProgramiv(glProgramID_, gl::GL_LINK_STATUS, &linked);
		validate_ = linked == 1;

	#ifdef XREX_DEBUG
		if (!validate_)
		{
			int32 length = 0;
			gl::GetProgramiv(glProgramID_, gl::GL_INFO_LOG_LENGTH, &length);
			if (length > 0)
			{
				errorString_.resize(length, 0);
				gl::GetProgramInfoLog(glProgramID_, length, &length, &errorString_[0]);
			}
			else
			{
				errorString_ = "Unknown linking error.";
			}
			std::cerr << errorString_ << std::endl;
		}
	#endif

		if (validate_)
		{
			int32 uniformCount = 0;
			gl::GetProgramiv(glProgramID_, gl::GL_ACTIVE_UNIFORMS, &uniformCount);
			int32 maxUniformNameLength = 0;
			gl::GetProgramiv(glProgramID_, gl::GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformNameLength);

			for (int32 i = 0; i < uniformCount; ++i)
			{
				int32 nameLength;
				int32 uniformSize;
				uint32 glType;
				string name;
				name.resize(maxUniformNameLength);
				gl::GetActiveUniform(glProgramID_, i, maxUniformNameLength, &nameLength, &uniformSize, &glType, &name[0]);
				int32 location = gl::GetUniformLocation(glProgramID_, name.c_str());
				name = name.substr(0, nameLength); // no '\0' included

				uniformInformations_.emplace_back(UniformInformation(name, ElementTypeFromeGLType(glType), uniformSize, location));

			}

			int32 attributeCount = 0;
			gl::GetProgramiv(glProgramID_, gl::GL_ACTIVE_ATTRIBUTES, &attributeCount);
			int32 maxAttributeNameLength = 0;
			gl::GetProgramiv(glProgramID_, gl::GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxAttributeNameLength);


			for (int32 i = 0; i < attributeCount; ++i)
			{
				int32 nameLength;
				int32 attributeSize;
				uint32 glType;
				string name;
				name.resize(maxAttributeNameLength);
				gl::GetActiveAttrib(glProgramID_, i, maxAttributeNameLength, &nameLength, &attributeSize, &glType, &name[0]);
				int32 location = gl::GetAttribLocation(glProgramID_, name.c_str());
				name = name.substr(0, nameLength); // no '\0' included

				attributeInformations_.emplace_back(AttributeInformation(std::move(name), ElementTypeFromeGLType(glType), attributeSize, location));
			}

		}

		return validate_;
	}


	void ProgramObject::Bind()
	{
		assert(validate_);
		gl::UseProgram(glProgramID_);

		for (auto i = uniformBinders_.begin(); i != uniformBinders_.end(); ++i)
		{
			i->setter(i->uniformInformation);
		}


	#ifdef XREX_DEBUG
		gl::ValidateProgram(glProgramID_);

		int32 validated = false;
		gl::GetProgramiv(glProgramID_, gl::GL_VALIDATE_STATUS, &validated);
		if (!validated)
		{
			int32 length = 0;
			gl::GetProgramiv(glProgramID_, gl::GL_INFO_LOG_LENGTH, &length);
			if (length > 0)
			{
				errorString_.resize(length, 0);
				gl::GetProgramInfoLog(glProgramID_, length, &length, &errorString_[0]);
				std::cerr << errorString_ << std::endl;
			}
		}
	#endif
	}

	std::pair<bool, ProgramObject::AttributeInformation> ProgramObject::GetAttributeInformation(std::string const& channel) const
	{
		auto found = std::find_if(attributeInformations_.begin(), attributeInformations_.end(), [&channel] (AttributeInformation const& attributeInformation)
		{
			return attributeInformation.GetChannel() == channel;
		});
		if (found == attributeInformations_.end())
		{
			return std::make_pair(false, NullAttributeInformation);
		}
		return std::make_pair(true, *found);
	}


	std::pair<bool, ProgramObject::UniformInformation> ProgramObject::GetUniformInformation(std::string const& channel) const
	{
		auto found = std::find_if(uniformInformations_.begin(), uniformInformations_.end(), [&channel] (UniformInformation const& uniformInformation)
		{
			return uniformInformation.GetChannel() == channel;
		});
		if (found == uniformInformations_.end())
		{
			return std::make_pair(false, NullUniformInformation);
		}
		return std::make_pair(true, *found);
	}



	void ProgramObject::CreateUniformBinder(std::string const& channel, EffectParameterSP const& parameter)
	{
		UniformBinder& binder = CreateBinder(channel);

		ElementType type = binder.uniformInformation.GetElementType();

		switch(type)
		{
		case ElementType::Bool:
			{
				binder.setter = [parameter] (UniformInformation const& uniformInformation)
				{
					gl::Uniform1i(uniformInformation.GetLocation(), parameter->As<bool>().GetValue());
				};
			}
			break;
		case ElementType::Int32:
			{
				binder.setter = [parameter] (UniformInformation const& uniformInformation)
				{
					gl::Uniform1i(uniformInformation.GetLocation(), parameter->As<int32>().GetValue());
				};
			}
			break;
		case ElementType::IntV2:
			{
				binder.setter = [parameter] (UniformInformation const& uniformInformation)
				{
					gl::Uniform2iv(uniformInformation.GetLocation(), 1, parameter->As<intV2>().GetValue().GetArray());
				};
			}
			break;
		case ElementType::IntV3:
			{
				binder.setter = [parameter] (UniformInformation const& uniformInformation)
				{
					gl::Uniform3iv(uniformInformation.GetLocation(), 1, parameter->As<intV3>().GetValue().GetArray());
				};
			}
			break;
		case ElementType::IntV4:
			{
				binder.setter = [parameter] (UniformInformation const& uniformInformation)
				{
					gl::Uniform4iv(uniformInformation.GetLocation(), 1, parameter->As<intV4>().GetValue().GetArray());
				};
			}
			break;
		case ElementType::Uint32:
			{
				binder.setter = [parameter] (UniformInformation const& uniformInformation)
				{
					gl::Uniform1ui(uniformInformation.GetLocation(), parameter->As<uint32>().GetValue());
				};
			}
			break;
		case ElementType::UintV2:
			{
				binder.setter = [parameter] (UniformInformation const& uniformInformation)
				{
					gl::Uniform2uiv(uniformInformation.GetLocation(), 1, parameter->As<uintV2>().GetValue().GetArray());
				};
			}
			break;
		case ElementType::UintV3:
			{
				binder.setter = [parameter] (UniformInformation const& uniformInformation)
				{
					gl::Uniform3uiv(uniformInformation.GetLocation(), 1, parameter->As<uintV3>().GetValue().GetArray());
				};
			}
			break;
		case ElementType::UintV4:
			{
				binder.setter = [parameter] (UniformInformation const& uniformInformation)
				{
					gl::Uniform4uiv(uniformInformation.GetLocation(), 1, parameter->As<uintV4>().GetValue().GetArray());
				};
			}
			break;
		case ElementType::Float:
			{
				binder.setter = [parameter] (UniformInformation const& uniformInformation)
				{
					gl::Uniform1f(uniformInformation.GetLocation(), parameter->As<float>().GetValue());
				};
			}
			break;
		case ElementType::FloatV2:
			{
				binder.setter = [parameter] (UniformInformation const& uniformInformation)
				{
					gl::Uniform2fv(uniformInformation.GetLocation(), 1, parameter->As<floatV2>().GetValue().GetArray());
				};
			}
			break;
		case ElementType::FloatV3:
			{
				binder.setter = [parameter] (UniformInformation const& uniformInformation)
				{
					gl::Uniform3fv(uniformInformation.GetLocation(), 1, parameter->As<floatV3>().GetValue().GetArray());
				};
			}
			break;
		case ElementType::FloatV4:
			{
				binder.setter = [parameter] (UniformInformation const& uniformInformation)
				{
					gl::Uniform4fv(uniformInformation.GetLocation(), 1, parameter->As<floatV4>().GetValue().GetArray());
				};
			}
			break;
		case ElementType::Double:
			{
				binder.setter = [parameter] (UniformInformation const& uniformInformation)
				{
					gl::Uniform1d(uniformInformation.GetLocation(), parameter->As<double>().GetValue());
				};
			}
			break;
		case ElementType::DoubleV2:
			{
				binder.setter = [parameter] (UniformInformation const& uniformInformation)
				{
					gl::Uniform2dv(uniformInformation.GetLocation(), 1, parameter->As<doubleV2>().GetValue().GetArray());
				};
			}
			break;
		case ElementType::DoubleV3:
			{
				binder.setter = [parameter] (UniformInformation const& uniformInformation)
				{
					gl::Uniform3dv(uniformInformation.GetLocation(), 1, parameter->As<doubleV3>().GetValue().GetArray());
				};
			}
			break;
		case ElementType::DoubleV4:
			{
				binder.setter = [parameter] (UniformInformation const& uniformInformation)
				{
					gl::Uniform4dv(uniformInformation.GetLocation(), 1, parameter->As<doubleV4>().GetValue().GetArray());
				};
			}
			break;
		case ElementType::FloatM44:
			{
				binder.setter = [parameter] (UniformInformation const& uniformInformation)
				{
					gl::UniformMatrix4fv(uniformInformation.GetLocation(), 1, false, parameter->As<floatM44>().GetValue().GetArray());
				};
			}
			break;
		case ElementType::DoubleM44:
			{
				binder.setter = [parameter] (UniformInformation const& uniformInformation)
				{
					gl::UniformMatrix4dv(uniformInformation.GetLocation(), 1, false, parameter->As<doubleM44>().GetValue().GetArray());
				};
			}
			break;
			// TODO
		default:
			// not support.
			assert(false);
		}
	}

	namespace
	{
		struct DefaultTextureGetter1D
		{
			static TextureSP const& Get()
			{
				return XREXContext::GetInstance().GetRenderingFactory().GetBlackTexture1D();
			}
		};
		struct DefaultTextureGetter2D
		{
			static TextureSP const& Get()
			{
				return XREXContext::GetInstance().GetRenderingFactory().GetBlackTexture2D();
			}
		};
		struct DefaultTextureGetter3D
		{
			static TextureSP const& Get()
			{
				return XREXContext::GetInstance().GetRenderingFactory().GetBlackTexture3D();
			}
		};
		struct DefaultTextureGetterCube
		{
			static TextureSP const& Get()
			{
				return XREXContext::GetInstance().GetRenderingFactory().GetBlackTextureCube();
			}
		};

		template <typename DefaultTextureGetter>
		struct SamplerBinder
		{
			EffectParameterSP parameter;
			uint32 samplerLocation;
			SamplerBinder(EffectParameterSP const& parameter, uint32 samplerLocation)
				: parameter(parameter), samplerLocation(samplerLocation)
			{
			}

			void operator() (ProgramObject::UniformInformation const& uniformInformation)
			{
				std::pair<TextureSP, SamplerSP> const& texture = parameter->As<std::pair<TextureSP, SamplerSP>>().GetValue();
				if (texture.first)
				{
					texture.first->Bind(samplerLocation);
				}
				else
				{
					DefaultTextureGetter::Get()->Bind(samplerLocation);
				}
				gl::Uniform1i(uniformInformation.GetLocation(), samplerLocation);
				if (texture.second)
				{
					texture.second->Bind(samplerLocation);
				}
				else
				{
					XREXContext::GetInstance().GetRenderingFactory().GetDefaultSampler()->Bind(samplerLocation);
				}
			}
		};
	}

	void ProgramObject::CreateSamplerUniformBinder(std::string const& channel, EffectParameterSP const& parameter, uint32 samplerLocation)
	{
		UniformBinder& binder = CreateBinder(channel);

		ElementType type = binder.uniformInformation.GetElementType();

		switch(type)
		{
		case ElementType::Sampler1D:
			{
				binder.setter = SamplerBinder<DefaultTextureGetter1D>(parameter, samplerLocation);
			}
			break;
		case ElementType::Sampler2D:
			{
				binder.setter = SamplerBinder<DefaultTextureGetter2D>(parameter, samplerLocation);
			}
			break;
		case ElementType::Sampler3D:
			{
				binder.setter = SamplerBinder<DefaultTextureGetter3D>(parameter, samplerLocation);
			}
			break;
		case ElementType::SamplerCube:
			{
				binder.setter = SamplerBinder<DefaultTextureGetterCube>(parameter, samplerLocation);
			}
			break;
		default:
			// not support.
			assert(false);
		}
	}

	namespace
	{
		struct ImageBinder
		{
			EffectParameterSP parameter;
			TexelFormat format;
			uint32 imageLocation;
			ImageBinder(EffectParameterSP const& parameter, TexelFormat format, uint32 imageLocation)
				: parameter(parameter), format(format), imageLocation(imageLocation)
			{
			}

			void operator() (ProgramObject::UniformInformation const& uniformInformation)
			{
				TextureImageSP const& image = parameter->As<TextureImageSP>().GetValue();
				assert(image);
				assert(GetTexelSizeInBytes(image->GetFormat()) == GetTexelSizeInBytes(format));
				// TODO format should be the format declared in shader, not the image format
				// image->Bind(imageLocation, format);
				image->Bind(imageLocation, image->GetBindingFormat_TEMP());
				gl::Uniform1i(uniformInformation.GetLocation(), imageLocation);
			}
		};
	}

	void ProgramObject::CreateImageUniformBinder(std::string const& channel, EffectParameterSP const& parameter, uint32 imageLocation)
	{
		UniformBinder& binder = CreateBinder(channel);

		ElementType type = binder.uniformInformation.GetElementType();

		switch(type)
		{
		case ElementType::Image1D:
		case ElementType::Image2D:
		case ElementType::Image3D:
		case ElementType::ImageCube:
			{
				 // TODO format should be the format declared in shader, not the image format
				binder.setter = ImageBinder(parameter, TexelFormat::BGRA32F, imageLocation);
			}
			break;
		default:
			// not support.
			assert(false);
		}
	}


	ProgramObject::UniformBinder& ProgramObject::CreateBinder(std::string const& channel)
	{
		auto found = std::find_if(uniformInformations_.begin(), uniformInformations_.end(), [&channel] (UniformInformation const& uniformInformation)
		{
			return uniformInformation.GetChannel() == channel;
		});
		assert(found != uniformInformations_.end());

		uniformBinders_.emplace_back(UniformBinder(*found));
		return uniformBinders_.back();
	}

	ProgramObject::AttributeInformation const ProgramObject::NullAttributeInformation;
	ProgramObject::UniformInformation const ProgramObject::NullUniformInformation;

}
