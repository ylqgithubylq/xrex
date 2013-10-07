#include "XREX.hpp"

#include "ShaderProgram.hpp"

#include "Base/XREXContext.hpp"
#include "Base/Logger.hpp"
#include "Rendering/RenderingFactory.hpp"
#include "Rendering/RenderingTechnique.hpp"
#include "Rendering/GraphicsBuffer.hpp"
#include "Rendering/Texture.hpp"
#include "Rendering/Sampler.hpp"
#include "Rendering/TextureImage.hpp"

#include "Rendering/GL/GLUtil.hpp"

#include <CoreGL.hpp>


#include <algorithm>
#include <array>



namespace XREX
{

	namespace
	{
		std::string const& ShaderDefineMacroFromShaderType(ShaderObject::ShaderType type)
		{
			static std::array<std::string, static_cast<uint32>(ShaderObject::ShaderType::CountOfShaderTypes)> const mapping = [] ()
			{
				std::remove_const<decltype(mapping)>::type macros;
				macros[static_cast<uint32>(ShaderObject::ShaderType::VertexShader)] = "#define VS\n";
				macros[static_cast<uint32>(ShaderObject::ShaderType::FragmentShader)] = "#define FS\n";
				macros[static_cast<uint32>(ShaderObject::ShaderType::GeometryShader)] = "#define GS\n";
				macros[static_cast<uint32>(ShaderObject::ShaderType::TessellationControlShader)] = "#define TCS\n";
				macros[static_cast<uint32>(ShaderObject::ShaderType::TessellationEvaluationShader)] = "#define TES\n";
				macros[static_cast<uint32>(ShaderObject::ShaderType::ComputeShader)] = "#define CS\n";
				return macros;
			} ();
			return mapping[static_cast<uint32>(type)];
		}

		std::string const& VersionMacro()
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
		std::string const& macroToDefine = ShaderDefineMacroFromShaderType(type_);

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
			XREXContext::GetInstance().GetLogger().LogLine(errorString_);
		}
#endif

		return validate_;
	}





	std::pair<bool, BufferInformation::BufferVariableInformation> BufferInformation::GetBufferVariableInformation(std::string const& name) const
	{
		//auto found = bufferVariableInformations_.begin();
		auto found = std::find_if(bufferVariableInformations_.begin(), bufferVariableInformations_.end(), [&name] (BufferVariableInformation const& bufferVariableInformation)
		{
			return bufferVariableInformation.GetName() == name;
		});
		if (found == bufferVariableInformations_.end())
		{
			return std::make_pair(false, NullBufferVariableInformation);
		}
		return std::make_pair(true, *found);
	}


	BufferInformation::BufferVariableInformation const BufferInformation::NullBufferVariableInformation;



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


	void ProgramObject::SpecifyFragmentOutput(std::string const& channel)
	{
		fragmentOutputChannels_.push_back(channel);
	}

	void ProgramObject::SpecifyImageFormat(std::string const& channel, TexelFormat format, AccessType accessType)
	{
		imageChannelInformations_.push_back(std::make_tuple(channel, format, accessType));
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
			XREXContext::GetInstance().GetLogger().LogLine(errorString_);
		}
#endif

		if (validate_)
		{
			InitializeAllInterfaceInformations();
		}

		return validate_;
	}


	void ProgramObject::Bind()
	{
		assert(validate_);
		gl::UseProgram(glProgramID_);

		SetupAllUniforms();

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
				XREXContext::GetInstance().GetLogger().LogLine(errorString_);
			}
		}
	#endif
	}


	void ProgramObject::SetupAllUniforms()
	{

		for (auto uniformBinder : uniformBinders_)
		{
			uniformBinder.setter(uniformBinder.uniformInformation);
		}
	}

	namespace
	{

		struct AttributeDefault
		{
			static AttributeInformation const NullInformation;
		};
		AttributeInformation const AttributeDefault::NullInformation;
		struct UniformDefault
		{
			static UniformInformation const NullInformation;
		};
		UniformInformation const UniformDefault::NullInformation;
		struct TextureDefault
		{
			static TextureInformation const NullInformation;
		};
		TextureInformation const TextureDefault::NullInformation;
		struct ImageDefault
		{
			static ImageInformation const NullInformation;
		};
		ImageInformation const ImageDefault::NullInformation;
		struct BufferDefault
		{
			static BufferInformation const NullInformation;
		};
		BufferInformation const BufferDefault::NullInformation;
		struct FragmentOutputDefault
		{
			static FragmentOutputInformation const NullInformation;
		};
		FragmentOutputInformation const FragmentOutputDefault::NullInformation;
		template <typename DefaultInformation, typename Information>
		std::pair<bool, Information const&> FindAndReturn(std::vector<Information> const& informations, std::string const& channel)
		{
			auto found = std::find_if(informations.begin(), informations.end(), [&channel] (Information const& information)
			{
				return information.GetChannel() == channel;
			});
			if (found == informations.end())
			{
				return std::pair<bool, Information const&>(false, DefaultInformation::NullInformation);
			}
			return std::pair<bool, Information const&>(true, *found);
		}
	}

	std::pair<bool, AttributeInformation const&> ProgramObject::GetAttributeInformation(std::string const& channel) const
	{
		return FindAndReturn<AttributeDefault>(attributeInformations_, channel);
	}

	std::pair<bool, UniformInformation const&> ProgramObject::GetUniformInformation(std::string const& channel) const
	{
		return FindAndReturn<UniformDefault>(uniformInformations_, channel);
	}

	std::pair<bool, TextureInformation const&> ProgramObject::GetTextureInformation(std::string const& channel) const
	{
		return FindAndReturn<TextureDefault>(textureInformations_, channel);
	}

	std::pair<bool, ImageInformation const&> ProgramObject::GetImageInformation(std::string const& channel) const
	{
		return FindAndReturn<ImageDefault>(imageInformations_, channel);
	}

	std::pair<bool, BufferInformation const&> ProgramObject::GetBufferInformation(std::string const& channel) const
	{
		return FindAndReturn<BufferDefault>(bufferInformations_, channel);
	}

	std::pair<bool, FragmentOutputInformation const&> ProgramObject::GetFragmentOutputInformation(std::string const& channel) const
	{
		return FindAndReturn<FragmentOutputDefault>(fragmentOutputInformations_, channel);
	}
	void ProgramObject::ConnectUniformParameter(std::string const& channel, TechniqueParameterSP const& parameter)
	{
		assert(parameter != nullptr);
		UniformBinder& binder = CreateUniformBinder(channel);

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

	void ProgramObject::ClearAllParameterConnections()
	{
		//uniformBinders_.swap(decltype(uniformBinders_)());
	}

	// Only can be called after InitializeAllInterfaceInformations. Do not store the return value.
	ProgramObject::UniformBinder& ProgramObject::CreateUniformBinder(std::string const& channel)
	{
		auto found = std::find_if(uniformInformations_.begin(), uniformInformations_.end(), [&channel] (UniformInformation const& uniformInformation)
		{
			return uniformInformation.GetChannel() == channel;
		});
		assert(found != uniformInformations_.end());

		uniformBinders_.push_back(UniformBinder(*found));
		return uniformBinders_.back();
	}


	void ProgramObject::InitializeAllInterfaceInformations()
	{
		gl::UseProgram(glProgramID_);
		int32 maxUniformNameLength = 0;
		gl::GetProgramiv(glProgramID_, gl::GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformNameLength);

		{ // uniform and assign sampler & image binding indices
			int32 uniformCount = 0;
			gl::GetProgramiv(glProgramID_, gl::GL_ACTIVE_UNIFORMS, &uniformCount);

			int32 availableSamplerIndex = 0;
			int32 availableImageIndex = 0;
			for (int32 i = 0; i < uniformCount; ++i)
			{
				int32 nameLength;
				int32 uniformSize;
				uint32 glType;
				std::string name;
				name.resize(maxUniformNameLength);
				gl::GetActiveUniform(glProgramID_, i, maxUniformNameLength, &nameLength, &uniformSize, &glType, &name[0]);
				name = name.substr(0, nameLength); // no '\0' included
				int32 location = gl::GetUniformLocation(glProgramID_, name.c_str());
				if (location != -1) // if -1, it is in a buffer. this is handled at buffer part
				{
					ElementType type = ElementTypeFromeGLType(glType);
					if (IsSamplerType(type))
					{
						gl::Uniform1i(location, availableSamplerIndex);
						textureInformations_.push_back(TextureInformation(name, type, availableSamplerIndex));
						availableSamplerIndex += 1;
					}
					else if (IsImageType(type))
					{
						auto found = std::find_if(imageChannelInformations_.begin(), imageChannelInformations_.end(), [&name] (std::tuple<std::string, TexelFormat, AccessType> const& image)
						{
							return std::get<0>(image) == name;
						});
						assert(found != imageChannelInformations_.end()); // image channel not specified.
						gl::Uniform1i(location, availableImageIndex);
						imageInformations_.push_back(ImageInformation(name, type, std::get<1>(*found), std::get<2>(*found), availableImageIndex));
						availableImageIndex += 1;
					}
#ifdef XREX_DEBUG
					else if (IsAtomicBufferType(type))
					{
						assert(false);
					}
#endif // XREX_DEBUG
					else
					{
						uniformInformations_.push_back(UniformInformation(name, type, uniformSize, location));
					}
				}
			}
		}
		{ // attribute
			int32 attributeCount = 0;
			gl::GetProgramiv(glProgramID_, gl::GL_ACTIVE_ATTRIBUTES, &attributeCount);
			int32 maxAttributeNameLength = 0;
			gl::GetProgramiv(glProgramID_, gl::GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxAttributeNameLength);
			for (int32 i = 0; i < attributeCount; ++i)
			{
				int32 nameLength;
				int32 attributeSize;
				uint32 glType;
				std::string name;
				name.resize(maxAttributeNameLength);
				gl::GetActiveAttrib(glProgramID_, i, maxAttributeNameLength, &nameLength, &attributeSize, &glType, &name[0]);
				name = name.substr(0, nameLength); // no '\0' included
				int32 location = gl::GetAttribLocation(glProgramID_, name.c_str());
				assert(location != -1);

				attributeInformations_.emplace_back(AttributeInformation(name, ElementTypeFromeGLType(glType), attributeSize, location));
			}
		}
		{ // uniform buffer
			int32 uniformBufferCount = 0;
			gl::GetProgramiv(glProgramID_, gl::GL_ACTIVE_UNIFORM_BLOCKS, &uniformBufferCount);
			for (int32 i = 0; i < uniformBufferCount; ++i)
			{
				int32 bindingIndex;
				int32 dataSize;
				int32 nameLength;
				std::string bufferName;
				int32 variableCount;
				std::vector<int32> variableIndices;
				std::vector<int32> variableOffsets;
				std::vector<int32> arrayStrides;
				std::vector<int32> matrixStrides;
				gl::GetActiveUniformBlockiv(glProgramID_, i, gl::GL_UNIFORM_BLOCK_BINDING, &bindingIndex);
				gl::GetActiveUniformBlockiv(glProgramID_, i, gl::GL_UNIFORM_BLOCK_DATA_SIZE, &dataSize);
				gl::GetActiveUniformBlockiv(glProgramID_, i, gl::GL_UNIFORM_BLOCK_NAME_LENGTH, &nameLength);
				bufferName.resize(nameLength);
				gl::GetActiveUniformBlockiv(glProgramID_, i, gl::GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &variableCount);
				variableIndices.resize(variableCount);
				gl::GetActiveUniformBlockiv(glProgramID_, i, gl::GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, &variableIndices[0]);
				gl::GetActiveUniformBlockName(glProgramID_, i, nameLength, &nameLength, &bufferName[0]);
				variableOffsets.resize(variableCount);
				arrayStrides.resize(variableCount);
				matrixStrides.resize(variableCount);
				gl::GetActiveUniformsiv(glProgramID_, variableCount, reinterpret_cast<uint32 const*>(variableIndices.data()), gl::GL_UNIFORM_OFFSET, &variableOffsets[0]);
				gl::GetActiveUniformsiv(glProgramID_, variableCount, reinterpret_cast<uint32 const*>(variableIndices.data()), gl::GL_UNIFORM_ARRAY_STRIDE, &arrayStrides[0]);
				gl::GetActiveUniformsiv(glProgramID_, variableCount, reinterpret_cast<uint32 const*>(variableIndices.data()), gl::GL_UNIFORM_MATRIX_STRIDE, &matrixStrides[0]);
				std::vector<BufferInformation::BufferVariableInformation> bufferVariableInformations;
				for (int32 j = 0; j < variableCount; ++j)
				{
					int32 index = variableIndices[j];
					int32 nameLength;
					int32 variableSize;
					uint32 glType;
					std::string variableName;
					variableName.resize(maxUniformNameLength);
					gl::GetActiveUniform(glProgramID_, index, maxUniformNameLength, &nameLength, &variableSize, &glType, &variableName[0]);
					variableName = variableName.substr(0, nameLength); // no '\0' included

					bufferVariableInformations.emplace_back(BufferInformation::BufferVariableInformation(
						variableName, ElementTypeFromeGLType(glType), variableSize, variableOffsets[j], arrayStrides[j], matrixStrides[j]));
				}
				bufferInformations_.emplace_back(BufferInformation(
					bufferName, BufferView::BufferType::Uniform, bindingIndex, dataSize, std::move(bufferVariableInformations)));
			}
		}
		{ // atomic counter
			int32 atomicBufferCount = 0;
			gl::GetProgramiv(glProgramID_, gl::GL_ACTIVE_ATOMIC_COUNTER_BUFFERS, &atomicBufferCount);
			for (int32 i = 0; i < atomicBufferCount; ++i)
			{
				int32 bindingIndex;
				int32 dataSize;
				int32 variableCount;
				std::vector<int32> variableIndices;
				std::vector<int32> variableOffsets;
				std::vector<int32> arrayStrides;
				gl::GetActiveAtomicCounterBufferiv(glProgramID_, i, gl::GL_ATOMIC_COUNTER_BUFFER_BINDING, &bindingIndex);
				gl::GetActiveAtomicCounterBufferiv(glProgramID_, i, gl::GL_ATOMIC_COUNTER_BUFFER_DATA_SIZE, &dataSize);
				gl::GetActiveAtomicCounterBufferiv(glProgramID_, i, gl::GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTERS, &variableCount);
				variableIndices.resize(variableCount);
				gl::GetActiveAtomicCounterBufferiv(glProgramID_, i, gl::GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTER_INDICES, &variableIndices[0]);
				variableOffsets.resize(variableCount);
				arrayStrides.resize(variableCount);
				gl::GetActiveUniformsiv(glProgramID_, variableCount, reinterpret_cast<uint32 const*>(variableIndices.data()), gl::GL_UNIFORM_OFFSET, &variableOffsets[0]);
				gl::GetActiveUniformsiv(glProgramID_, variableCount, reinterpret_cast<uint32 const*>(variableIndices.data()), gl::GL_UNIFORM_ARRAY_STRIDE, &arrayStrides[0]);
				std::vector<BufferInformation::BufferVariableInformation> bufferVariableInformations;
				for (int32 j = 0; j < variableCount; ++j)
				{
					int32 index = variableIndices[j];
					int32 nameLength;
					int32 variableSize;
					uint32 glType;
					std::string variableName;
					variableName.resize(maxUniformNameLength);
					gl::GetActiveUniform(glProgramID_, index, maxUniformNameLength, &nameLength, &variableSize, &glType, &variableName[0]);
					variableName = variableName.substr(0, nameLength); // no '\0' included

					bufferVariableInformations.emplace_back(BufferInformation::BufferVariableInformation(
						variableName, ElementTypeFromeGLType(glType), variableSize, variableOffsets[j], arrayStrides[j], 0));
				}
				bufferInformations_.emplace_back(BufferInformation(
					std::to_string(bindingIndex), BufferView::BufferType::AtomicCounter, bindingIndex, dataSize, std::move(bufferVariableInformations)));
			}
		}

		{ // fragment output
			for (uint32 i = 0; i < fragmentOutputChannels_.size(); ++i)
			{
				int32 location;
				int32 index;
				location = gl::GetFragDataLocation(glProgramID_, fragmentOutputChannels_[i].c_str());
				index = gl::GetFragDataIndex(glProgramID_, fragmentOutputChannels_[i].c_str());
				fragmentOutputInformations_.emplace_back(fragmentOutputChannels_[i], location, index);
			}
		}
		{ // TODO transform feedback

		}
		gl::UseProgram(0);
	}

	namespace
	{
		bool IsResourceBufferType(BufferView::BufferType type)
		{
			switch (type)
			{
			case BufferView::BufferType::Uniform:
			case BufferView::BufferType::AtomicCounter:
			case BufferView::BufferType::ShaderStorage:
				return true;
				break;
			case BufferView::BufferType::TypeCount:
				assert(false);
				return false;
			default:
				return false;
				break;
			}
		}
	}


	ShaderResourceBuffer::BufferVariableSetter::BufferVariableSetter(ShaderResourceBuffer& buffer)
		: buffer_(buffer), mapper_(buffer.GetBuffer()->GetMapper(AccessType::WriteOnly))
	{
	}

	ShaderResourceBuffer::BufferVariableSetter::BufferVariableSetter(BufferVariableSetter&& right)
		: buffer_(right.buffer_), mapper_(std::move(right.mapper_))
	{
	}

	void ShaderResourceBuffer::BufferVariableSetter::Finish()
	{
		mapper_.Finish();
	}





	ShaderResourceBuffer::ShaderResourceBuffer(BufferInformation const& information)
		: BufferView(information.GetBufferType()), information_(information)
	{
		assert(IsResourceBufferType(information.GetBufferType()));
	}

	ShaderResourceBuffer::ShaderResourceBuffer(BufferInformation const& information, GraphicsBufferSP const& buffer)
		: BufferView(information.GetBufferType(), buffer), information_(information)
	{
		assert(IsResourceBufferType(information.GetBufferType()));
	}


	ShaderResourceBuffer::~ShaderResourceBuffer()
	{
	}


	bool ShaderResourceBuffer::SetBufferCheck(GraphicsBufferSP const& newBuffer)
	{
		if (newBuffer)
		{
			return newBuffer->GetSize() == information_.GetDataSize();
		}
		return true;
	}

}
