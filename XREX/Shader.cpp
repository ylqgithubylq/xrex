#include "XREX.hpp"

#include "Shader.hpp"
#include "RenderingEffect.hpp"
#include "GLUtil.hpp"
#include "GraphicsBuffer.hpp"

#include <CoreGL.hpp>


#include <iostream>
#include <algorithm>



using std::string;
using std::vector;

vector<string> const ShaderObject::ShaderDefineMacros = [] ()
{
	vector<string> macros(static_cast<uint32>(ShaderObject::ShaderType::CountOfShaderTypes));
	macros[static_cast<uint32>(ShaderObject::ShaderType::VertexShader)] = "#define VS\n";
	macros[static_cast<uint32>(ShaderObject::ShaderType::FragmentShader)] = "#define FS\n";
	macros[static_cast<uint32>(ShaderObject::ShaderType::GeometryShader)] = "#define GS\n";
	macros[static_cast<uint32>(ShaderObject::ShaderType::TessellationControlShader)] = "#define TCS\n";
	macros[static_cast<uint32>(ShaderObject::ShaderType::TessellationEvaluationShader)] = "#define TES\n";
	return macros;
} ();

string const ShaderObject::VersionMacro = "#version 420\n\n";

const vector<uint32> ShaderObject::ShaderTypeToGLShaderType = [] ()
{
	vector<uint32> mapping(static_cast<uint32>(ShaderObject::ShaderType::CountOfShaderTypes));
	mapping[static_cast<uint32>(ShaderObject::ShaderType::VertexShader)] = gl::GL_VERTEX_SHADER;
	mapping[static_cast<uint32>(ShaderObject::ShaderType::FragmentShader)] = gl::GL_FRAGMENT_SHADER;
	mapping[static_cast<uint32>(ShaderObject::ShaderType::GeometryShader)] = gl::GL_GEOMETRY_SHADER;
	mapping[static_cast<uint32>(ShaderObject::ShaderType::TessellationControlShader)] = gl::GL_TESS_CONTROL_SHADER;
	mapping[static_cast<uint32>(ShaderObject::ShaderType::TessellationEvaluationShader)] = gl::GL_TESS_EVALUATION_SHADER;
	return mapping;
} ();




ShaderObject::ShaderObject(ShaderType type, string const& source) : type_(type), source_(source)
{
	shaderID_ = gl::CreateShader(ShaderTypeToGLShaderType[static_cast<uint32>(type_)]);
	Compile();
}

ShaderObject::ShaderObject(ShaderType type, string&& source) : type_(type), source_(move(source))
{
	shaderID_ = gl::CreateShader(ShaderTypeToGLShaderType[static_cast<uint32>(type_)]);
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

	string const& macroToDefine = ShaderDefineMacros[static_cast<uint32>(type_)];

	char const * cstring[] = { VersionMacro.c_str(), macroToDefine.c_str(), source_.c_str() };
	gl::ShaderSource(shaderID_, sizeof(cstring) / sizeof(cstring[0]), cstring, nullptr);
	gl::CompileShader(shaderID_);
	
	int32 sourceLength;
	gl::GetShaderiv(shaderID_, gl::GL_SHADER_SOURCE_LENGTH, &sourceLength); // '\0' included
	source_.resize(sourceLength);
	gl::GetShaderSource(shaderID_, sourceLength, &sourceLength, &source_[0]);

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
	programID_ = gl::CreateProgram();
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
			gl::DetachShader(programID_, id);
		}
	}
	gl::DeleteProgram(programID_);
	programID_ = 0;
}

void ProgramObject::AttachShader(ShaderObjectSP& shader)
{
	gl::AttachShader(programID_, shader->GetID());
	shaders_[static_cast<uint32>(shader->GetType())] = shader;
}


bool ProgramObject::Link()
{
	if (programID_ == 0)
	{
		errorString_ = "Program creation failed.";
		return false;
	}
	gl::LinkProgram(programID_);

	int32 linked = 0;
	gl::GetProgramiv(programID_, gl::GL_LINK_STATUS, &linked);
	validate_ = linked == 1;

#ifdef XREX_DEBUG
	if (!validate_)
	{
		int32 length = 0;
		gl::GetProgramiv(programID_, gl::GL_INFO_LOG_LENGTH, &length);
		if (length > 0)
		{
			errorString_.resize(length, 0);
			gl::GetProgramInfoLog(programID_, length, &length, &errorString_[0]);
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
	gl::UseProgram(programID_);

	for (auto i = uniformBinders_.begin(); i != uniformBinders_.end(); ++i)
	{
		i->setter();
	}


#ifdef XREX_DEBUG
	gl::ValidateProgram(programID_);

	int32 validated = false;
	gl::GetProgramiv(programID_, gl::GL_VALIDATE_STATUS, &validated);
	if (!validated)
	{
		int32 length = 0;
		gl::GetProgramiv(programID_, gl::GL_INFO_LOG_LENGTH, &length);
		if (length > 0)
		{
			// use string::resize to do like this in other place.
			errorString_.resize(length, 0);
			gl::GetProgramInfoLog(programID_, length, &length, &errorString_[0]);
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
	return found->location;
}


void ProgramObject::InitializeParameterSetters(RenderingEffect& effect)
{
	vector<EffectParameterSP> const& parameters = effect.GetAllParameters();

	// uniforms

	uint32 availableSamplerLocation = 0;

	int32 uniformCount = 0;
	gl::GetProgramiv(programID_, gl::GL_ACTIVE_UNIFORMS, &uniformCount);
	int32 maxUniformNameLength = 0;
	gl::GetProgramiv(programID_, gl::GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformNameLength);

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
		gl::GetActiveUniform(programID_, i, maxUniformNameLength, &nameLength, &uniformSize, &glType, &name[0]);
		int32 location = gl::GetUniformLocation(programID_, name.c_str());
		name = name.substr(0, nameLength); // no '\0' included

		binder.glType = glType;
		binder.elementCount = uniformSize;
		binder.location = location;


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
			assert(GetGLType(parameter->GetType()) == glType);
		}

		InitializeUniformBinder(binder, parameter, availableSamplerLocation);
	}


	// attributes

	int32 attributeCount;
	gl::GetProgramiv(programID_, gl::GL_ACTIVE_ATTRIBUTES, &attributeCount);
	int32 maxAttributeNameLength;
	gl::GetProgramiv(programID_, gl::GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxAttributeNameLength);

	// don't change the size of attributeBindingInformation_ after this, reference to elements in it will be kept in uniform binder.
	attributeBindingInformation_.resize(attributeCount);

	for (int32 i = 0; i < attributeCount; ++i) {
		AttributeBindingInformation& bindingInformation = attributeBindingInformation_[i];

		int32 nameLength;
		int32 attributeSize;
		uint32 glType;
		string name;
		name.resize(maxAttributeNameLength);
		gl::GetActiveAttrib(programID_, i, maxAttributeNameLength, &nameLength, &attributeSize, &glType, &name[0]);
		int32 location = gl::GetAttribLocation(programID_, name.c_str());
		name = name.substr(0, nameLength); // no '\0' included
		
		bindingInformation.channel = std::move(name);
		bindingInformation.glType = glType;
		bindingInformation.elementCount = attributeSize;
		bindingInformation.location = location;
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
				binder.setter = [&binder, parameter] ()
				{
					// TODO
				};
			}
			break;
		case gl::GL_SAMPLER_2D:
			{
				binder.setter = [&binder, parameter] ()
				{
					// TODO
				};
			}
			break;
		case gl::GL_SAMPLER_3D:
			{
				binder.setter = [&binder, parameter] ()
				{
					// TODO
				};
			}
			break;
		case gl::GL_SAMPLER_CUBE:
			{
				binder.setter = [&binder, parameter] ()
				{
					// TODO
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
					gl::Uniform1i(binder.location, parameter->GetValue<bool>());
				};
			}
			break;
		case gl::GL_INT:
			{
				binder.setter = [&binder, parameter] ()
				{
					gl::Uniform1i(binder.location, parameter->GetValue<int32>());
				};
			}
			break;
		case gl::GL_FLOAT:
			{
				binder.setter = [&binder, parameter] ()
				{
					gl::Uniform1f(binder.location, parameter->GetValue<float>());
				};
			}
			break;
		case gl::GL_FLOAT_VEC2:
			{
				binder.setter = [&binder, parameter] ()
				{
					gl::Uniform2fv(binder.location, 1, parameter->GetValue<floatV2>().GetArray());
				};
			}
			break;
		case gl::GL_FLOAT_VEC3:
			{
				binder.setter = [&binder, parameter] ()
				{
					gl::Uniform3fv(binder.location, 1, parameter->GetValue<floatV3>().GetArray());
				};
			}
			break;
		case gl::GL_FLOAT_VEC4:
			{
				binder.setter = [&binder, parameter] ()
				{
					gl::Uniform4fv(binder.location, 1, parameter->GetValue<floatV4>().GetArray());
				};
			}
			break;
		case gl::GL_FLOAT_MAT4:
			{
				binder.setter = [&binder, parameter] ()
				{
					gl::UniformMatrix4fv(binder.location, 1, false, parameter->GetValue<floatM44>().GetArray());
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
