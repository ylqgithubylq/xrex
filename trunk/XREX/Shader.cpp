#include "XREX.hpp"

#include "XREXContext.hpp"
#include "RenderingFactory.hpp"
#include "Shader.hpp"
#include "RenderingEffect.hpp"
#include "GLUtil.hpp"
#include "GraphicsBuffer.hpp"
#include "Texture.hpp"

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
				std::array<string, static_cast<uint32>(ShaderObject::ShaderType::CountOfShaderTypes)> macros;
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
			static string const macro = "#version 420\n\n";
			return macro;
		}

		uint32 GLShaderTypeFromShaderType(ShaderObject::ShaderType type)
		{
			static std::array<uint32, static_cast<uint32>(ShaderObject::ShaderType::CountOfShaderTypes)> const mapping = [] ()
			{
				std::array<uint32, static_cast<uint32>(ShaderObject::ShaderType::CountOfShaderTypes)> mapping;
				mapping[static_cast<uint32>(ShaderObject::ShaderType::VertexShader)] = gl::GL_VERTEX_SHADER;
				mapping[static_cast<uint32>(ShaderObject::ShaderType::FragmentShader)] = gl::GL_FRAGMENT_SHADER;
				mapping[static_cast<uint32>(ShaderObject::ShaderType::GeometryShader)] = gl::GL_GEOMETRY_SHADER;
				mapping[static_cast<uint32>(ShaderObject::ShaderType::TessellationControlShader)] = gl::GL_TESS_CONTROL_SHADER;
				mapping[static_cast<uint32>(ShaderObject::ShaderType::TessellationEvaluationShader)] = gl::GL_TESS_EVALUATION_SHADER;
				return mapping;
			} ();
			return mapping[static_cast<uint32>(type)];
		};
	}





	ShaderObject::ShaderObject(ShaderType type, string const& source) : type_(type), source_(source)
	{
		shaderID_ = gl::CreateShader(GLShaderTypeFromShaderType(type_));
		Compile();
	}

	ShaderObject::ShaderObject(ShaderType type, string&& source) : type_(type), source_(move(source))
	{
		shaderID_ = gl::CreateShader(GLShaderTypeFromShaderType(type_));
		Compile();
	}


	ShaderObject::~ShaderObject()
	{
		Destory();
	}


	void ShaderObject::Destory()
	{
		gl::DeleteShader(shaderID_);
		shaderID_ = 0;
	}


	bool ShaderObject::Compile()
	{
		if (shaderID_ == 0)
		{
			errorString_ = "Shader creation failed.";
			return false;
		}

		string const& macroToDefine = ShaderDefineMacroFromShaderType(type_);

		char const* cstring[] = { VersionMacro().c_str(), macroToDefine.c_str(), source_.c_str() };
		gl::ShaderSource(shaderID_, sizeof(cstring) / sizeof(cstring[0]), cstring, nullptr);
		gl::CompileShader(shaderID_);
	
		int32 sourceLength;
		gl::GetShaderiv(shaderID_, gl::GL_SHADER_SOURCE_LENGTH, &sourceLength); // '\0' included
		source_.resize(sourceLength);
		gl::GetShaderSource(shaderID_, sourceLength, &sourceLength, &source_[0]); // write back actual source content

		int32 compiled = 0;
		gl::GetShaderiv(shaderID_, gl::GL_COMPILE_STATUS, &compiled);
		validate_ = compiled == 1;

	#ifdef XREX_DEBUG
		if (!validate_)
		{
			int32 length = 0;
			gl::GetShaderiv(shaderID_, gl::GL_INFO_LOG_LENGTH, &length);
			if (length > 0)
			{
				errorString_.resize(length, 0);
				gl::GetShaderInfoLog(shaderID_, length, &length, &errorString_[0]);
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
	}
	ProgramObject::~ProgramObject()
	{
		Destory();
	}

	void ProgramObject::Destory()
	{
		for (int32 i = 0; i < static_cast<uint32>(ShaderObject::ShaderType::CountOfShaderTypes); ++i)
		{
			if (shaders_[i])
			{
				uint32 id = shaders_[i]->GetID();
				gl::DetachShader(glProgramID_, id);
			}
		}
		gl::DeleteProgram(glProgramID_);
		glProgramID_ = 0;
	}

	void ProgramObject::AttachShader(ShaderObjectSP& shader)
	{
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
		return validate_;
	}


	void ProgramObject::Bind()
	{
		assert(validate_);
		gl::UseProgram(glProgramID_);

		for (auto i = uniformBinders_.begin(); i != uniformBinders_.end(); ++i)
		{
			i->setter();
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
				// use string::resize to do like this in other place.
				errorString_.resize(length, 0);
				gl::GetProgramInfoLog(glProgramID_, length, &length, &errorString_[0]);
				std::cerr << errorString_ << std::endl;
			}
		}
	#endif
	}

	int32 ProgramObject::GetAttributeLocation(string const& channel) const
	{
		auto found = std::find_if(attributeBindingInformation_.begin(), attributeBindingInformation_.end(), [&channel] (AttributeBindingInformation const& bindingInformation)
		{
			return bindingInformation.channel == channel;
		});
		if (found == attributeBindingInformation_.end())
		{
			return -1;
		}
		return found->glLocation;
	}


	void ProgramObject::InitializeParameterSetters(RenderingEffect& effect)
	{
		vector<EffectParameterSP> const& parameters = effect.GetAllParameters();

		// uniforms

		uint32 availableSamplerLocation = 0;

		int32 uniformCount = 0;
		gl::GetProgramiv(glProgramID_, gl::GL_ACTIVE_UNIFORMS, &uniformCount);
		int32 maxUniformNameLength = 0;
		gl::GetProgramiv(glProgramID_, gl::GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformNameLength);

		// don't change the size of uniformBinders_ after this, reference to elements in it will be kept in uniform binder.
		uniformBinders_.resize(uniformCount);

		for (int32 i = 0; i < uniformCount; ++i)
		{
			UniformBinder& binder = uniformBinders_[i];

			int32 nameLength;
			int32 uniformSize;
			uint32 glType;
			string name;
			name.resize(maxUniformNameLength);
			gl::GetActiveUniform(glProgramID_, i, maxUniformNameLength, &nameLength, &uniformSize, &glType, &name[0]);
			int32 location = gl::GetUniformLocation(glProgramID_, name.c_str());
			name = name.substr(0, nameLength); // no '\0' included

			binder.glType = glType;
			binder.elementCount = uniformSize;
			binder.glLocation = location;


			auto resultIter = std::find_if(parameters.begin(), parameters.end(), [&name] (EffectParameterSP const& parameter)
			{
				return parameter->GetName() == name;
			});

			EffectParameterSP parameter;
			if (resultIter == parameters.end()) // not exist
			{
				switch(glType)
				{
				case gl::GL_BOOL:
					{
						parameter = MakeSP<ConcreteEffectParameter<bool>>(name);
					}
					break;
				case gl::GL_INT:
					{
						parameter = MakeSP<ConcreteEffectParameter<int32>>(name);
					}
					break;
				case gl::GL_INT_VEC2:
					{
						parameter = MakeSP<ConcreteEffectParameter<intV2>>(name);
					}
					break;
				case gl::GL_INT_VEC3:
					{
						parameter = MakeSP<ConcreteEffectParameter<intV3>>(name);
					}
					break;
				case gl::GL_INT_VEC4:
					{
						parameter = MakeSP<ConcreteEffectParameter<intV4>>(name);
					}
					break;
				case gl::GL_UNSIGNED_INT:
					{
						parameter = MakeSP<ConcreteEffectParameter<uint32>>(name);
					}
					break;
				case gl::GL_UNSIGNED_INT_VEC2:
					{
						parameter = MakeSP<ConcreteEffectParameter<uintV2>>(name);
					}
					break;
				case gl::GL_UNSIGNED_INT_VEC3:
					{
						parameter = MakeSP<ConcreteEffectParameter<uintV3>>(name);
					}
					break;
				case gl::GL_UNSIGNED_INT_VEC4:
					{
						parameter = MakeSP<ConcreteEffectParameter<uintV4>>(name);
					}
					break;
				case gl::GL_FLOAT:
					{
						parameter = MakeSP<ConcreteEffectParameter<float>>(name);
					}
					break;
				case gl::GL_FLOAT_VEC2:
					{
						parameter = MakeSP<ConcreteEffectParameter<floatV2>>(name);
					}
					break;
				case gl::GL_FLOAT_VEC3:
					{
						parameter = MakeSP<ConcreteEffectParameter<floatV3>>(name);
					}
					break;
				case gl::GL_FLOAT_VEC4:
					{
						parameter = MakeSP<ConcreteEffectParameter<floatV4>>(name);
					}
					break;
				case gl::GL_FLOAT_MAT4:
					{
						parameter = MakeSP<ConcreteEffectParameter<floatM44>>(name);
					}
					break;
				case gl::GL_SAMPLER_1D:
				case gl::GL_SAMPLER_2D:
				case gl::GL_SAMPLER_3D:
				case gl::GL_SAMPLER_CUBE:
					{
						parameter = MakeSP<ConcreteEffectParameter<TextureSP>>(name);
					}
					break;
				default:
					// not support.
					assert(false);
				}
				effect.AddParameter(parameter);
			}
			else
			{
				parameter = *resultIter;
				// check if uniform type in this shader not equals to type of parameter created by other shader.
				assert(GLTypeFromElementType(parameter->GetType()) == glType);
			}

			InitializeUniformBinder(binder, parameter, availableSamplerLocation);
		}


		// attributes

		int32 attributeCount;
		gl::GetProgramiv(glProgramID_, gl::GL_ACTIVE_ATTRIBUTES, &attributeCount);
		int32 maxAttributeNameLength;
		gl::GetProgramiv(glProgramID_, gl::GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxAttributeNameLength);

		// don't change the size of attributeBindingInformation_ after this, reference to elements in it will be kept in uniform binder.
		attributeBindingInformation_.resize(attributeCount);

		for (int32 i = 0; i < attributeCount; ++i)
		{
			AttributeBindingInformation& bindingInformation = attributeBindingInformation_[i];

			int32 nameLength;
			int32 attributeSize;
			uint32 glType;
			string name;
			name.resize(maxAttributeNameLength);
			gl::GetActiveAttrib(glProgramID_, i, maxAttributeNameLength, &nameLength, &attributeSize, &glType, &name[0]);
			int32 location = gl::GetAttribLocation(glProgramID_, name.c_str());
			name = name.substr(0, nameLength); // no '\0' included
		
			bindingInformation.channel = std::move(name);
			bindingInformation.glType = glType;
			bindingInformation.elementCount = attributeSize;
			bindingInformation.glLocation = location;
		}
		return;
	}


	void ProgramObject::InitializeUniformBinder(UniformBinder& binder, EffectParameterSP& parameter, uint32& availableSamplerLocation)
	{
		uint32 glType = binder.glType;
		//EffectParameter* parameter = parameter.get();
		if (glType == gl::GL_SAMPLER_1D || glType == gl::GL_SAMPLER_2D || glType == gl::GL_SAMPLER_3D || glType == gl::GL_SAMPLER_CUBE)
		{
			int32 samplerLocation = availableSamplerLocation++;

			switch(glType)
			{
			case gl::GL_SAMPLER_1D:
				{
					binder.setter = [&binder, parameter, samplerLocation] ()
					{
						TextureSP const& texture = parameter->GetValue<TextureSP>();
						if (texture)
						{
							texture->BindTexture(samplerLocation);
							gl::Uniform1i(binder.glLocation, samplerLocation);
						}
						else
						{
							XREXContext::GetInstance().GetRenderingFactory().GetBlackTexture1D()->BindTexture(samplerLocation);
							gl::Uniform1i(binder.glLocation, samplerLocation);
						}
					};
				}
				break;
			case gl::GL_SAMPLER_2D:
				{
					binder.setter = [&binder, parameter, samplerLocation] ()
					{
						TextureSP const& texture = parameter->GetValue<TextureSP>();
						if (texture)
						{
							texture->BindTexture(samplerLocation);
							gl::Uniform1i(binder.glLocation, samplerLocation);
						}
						else
						{
							XREXContext::GetInstance().GetRenderingFactory().GetBlackTexture2D()->BindTexture(samplerLocation);
							gl::Uniform1i(binder.glLocation, samplerLocation);
						}
					};
				}
				break;
			case gl::GL_SAMPLER_3D:
				{
					binder.setter = [&binder, parameter, samplerLocation] ()
					{
						TextureSP const& texture = parameter->GetValue<TextureSP>();
						if (texture)
						{
							texture->BindTexture(samplerLocation);
							gl::Uniform1i(binder.glLocation, samplerLocation);
						}
						else
						{
							XREXContext::GetInstance().GetRenderingFactory().GetBlackTexture3D()->BindTexture(samplerLocation);
							gl::Uniform1i(binder.glLocation, samplerLocation);
						}
					};
				}
				break;
			case gl::GL_SAMPLER_CUBE:
				{
					binder.setter = [&binder, parameter, samplerLocation] ()
					{
						TextureSP const& texture = parameter->GetValue<TextureSP>();
						if (texture)
						{
							texture->BindTexture(samplerLocation);
							gl::Uniform1i(binder.glLocation, samplerLocation);
						}
						else
						{
							XREXContext::GetInstance().GetRenderingFactory().GetBlackTextureCube()->BindTexture(samplerLocation);
							gl::Uniform1i(binder.glLocation, samplerLocation);
						}
					};
				}
				break;
			default:
				// not support.
				assert(false);
			}
		}
		else
		{
			switch(glType)
			{
			case gl::GL_BOOL:
				{
					binder.setter = [&binder, parameter] ()
					{
						gl::Uniform1i(binder.glLocation, parameter->GetValue<bool>());
					};
				}
				break;
			case gl::GL_INT:
				{
					binder.setter = [&binder, parameter] ()
					{
						gl::Uniform1i(binder.glLocation, parameter->GetValue<int32>());
					};
				}
				break;
			case gl::GL_INT_VEC2:
				{
					binder.setter = [&binder, parameter] ()
					{
						gl::Uniform2iv(binder.glLocation, 1, parameter->GetValue<intV2>().GetArray());
					};
				}
				break;
			case gl::GL_INT_VEC3:
				{
					binder.setter = [&binder, parameter] ()
					{
						gl::Uniform3iv(binder.glLocation, 1, parameter->GetValue<intV3>().GetArray());
					};
				}
				break;
			case gl::GL_INT_VEC4:
				{
					binder.setter = [&binder, parameter] ()
					{
						gl::Uniform4iv(binder.glLocation, 1, parameter->GetValue<intV4>().GetArray());
					};
				}
				break;
			case gl::GL_UNSIGNED_INT:
				{
					binder.setter = [&binder, parameter] ()
					{
						gl::Uniform1ui(binder.glLocation, parameter->GetValue<uint32>());
					};
				}
				break;
			case gl::GL_UNSIGNED_INT_VEC2:
				{
					binder.setter = [&binder, parameter] ()
					{
						gl::Uniform2uiv(binder.glLocation, 1, parameter->GetValue<uintV2>().GetArray());
					};
				}
				break;
			case gl::GL_UNSIGNED_INT_VEC3:
				{
					binder.setter = [&binder, parameter] ()
					{
						gl::Uniform3uiv(binder.glLocation, 1, parameter->GetValue<uintV3>().GetArray());
					};
				}
				break;
			case gl::GL_UNSIGNED_INT_VEC4:
				{
					binder.setter = [&binder, parameter] ()
					{
						gl::Uniform4uiv(binder.glLocation, 1, parameter->GetValue<uintV4>().GetArray());
					};
				}
				break;
			case gl::GL_FLOAT:
				{
					binder.setter = [&binder, parameter] ()
					{
						gl::Uniform1f(binder.glLocation, parameter->GetValue<float>());
					};
				}
				break;
			case gl::GL_FLOAT_VEC2:
				{
					binder.setter = [&binder, parameter] ()
					{
						gl::Uniform2fv(binder.glLocation, 1, parameter->GetValue<floatV2>().GetArray());
					};
				}
				break;
			case gl::GL_FLOAT_VEC3:
				{
					binder.setter = [&binder, parameter] ()
					{
						gl::Uniform3fv(binder.glLocation, 1, parameter->GetValue<floatV3>().GetArray());
					};
				}
				break;
			case gl::GL_FLOAT_VEC4:
				{
					binder.setter = [&binder, parameter] ()
					{
						gl::Uniform4fv(binder.glLocation, 1, parameter->GetValue<floatV4>().GetArray());
					};
				}
				break;
			case gl::GL_FLOAT_MAT4:
				{
					binder.setter = [&binder, parameter] ()
					{
						gl::UniformMatrix4fv(binder.glLocation, 1, false, parameter->GetValue<floatM44>().GetArray());
					};
				}
				break;
				// TODO
			default:
				// not support.
				assert(false);
			}
		}
	}

}
