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
			static std::array<std::string, static_cast<uint32>(ShaderObject::ShaderType::ShaderTypeCount)> const mapping = [] ()
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





	std::pair<bool, BufferBindingInformation::BufferVariableInformation> BufferBindingInformation::GetBufferVariableInformation(std::string const& name) const
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


	BufferBindingInformation::BufferVariableInformation const BufferBindingInformation::NullBufferVariableInformation;



	ProgramObject::ProgramObject()
		: shaders_(static_cast<uint32>(ShaderObject::ShaderType::ShaderTypeCount)), validate_(false)
	{
		glProgramID_ = gl::CreateProgram();
		assert(glProgramID_ != 0);
	}
	ProgramObject::~ProgramObject()
	{
		for (int32 i = 0; i < static_cast<uint32>(ShaderObject::ShaderType::ShaderTypeCount); ++i)
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

	bool ProgramObject::Link(InformationPack const& pack)
	{
		if (glProgramID_ == 0)
		{
			errorString_ = "Program creation failed.";
			return false;
		}

		SpecifyAllInterfaceBindingsBeforeLink(pack);

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
			SpecifyAllInterfaceBindingsAfterLink(pack);
			InitializeBindingInformations(pack);
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
			static AttributeInputBindingInformation const NullInformation;
		};
		AttributeInputBindingInformation const AttributeDefault::NullInformation;
		struct UniformDefault
		{
			static UniformBindingInformation const NullInformation;
		};
		UniformBindingInformation const UniformDefault::NullInformation;
		struct TextureDefault
		{
			static TextureBindingInformation const NullInformation;
		};
		TextureBindingInformation const TextureDefault::NullInformation;
		struct ImageDefault
		{
			static ImageBindingInformation const NullInformation;
		};
		ImageBindingInformation const ImageDefault::NullInformation;
		struct BufferDefault
		{
			static BufferBindingInformation const NullInformation;
		};
		BufferBindingInformation const BufferDefault::NullInformation;
		struct FragmentOutputDefault
		{
			static FragmentOutputBindingInformation const NullInformation;
		};
		FragmentOutputBindingInformation const FragmentOutputDefault::NullInformation;
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

	std::pair<bool, AttributeInputBindingInformation const&> ProgramObject::GetAttributeInformation(std::string const& channel) const
	{
		return FindAndReturn<AttributeDefault>(attributeInformations_, channel);
	}

	std::pair<bool, UniformBindingInformation const&> ProgramObject::GetUniformInformation(std::string const& channel) const
	{
		return FindAndReturn<UniformDefault>(uniformInformations_, channel);
	}

	std::pair<bool, TextureBindingInformation const&> ProgramObject::GetTextureInformation(std::string const& channel) const
	{
		return FindAndReturn<TextureDefault>(textureInformations_, channel);
	}

	std::pair<bool, ImageBindingInformation const&> ProgramObject::GetImageInformation(std::string const& channel) const
	{
		return FindAndReturn<ImageDefault>(imageInformations_, channel);
	}

	std::pair<bool, BufferBindingInformation const&> ProgramObject::GetBufferInformation(std::string const& channel) const
	{
		return FindAndReturn<BufferDefault>(bufferInformations_, channel);
	}

	std::pair<bool, FragmentOutputBindingInformation const&> ProgramObject::GetFragmentOutputInformation(std::string const& channel) const
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
				binder.setter = [parameter] (UniformBindingInformation const& uniformInformation)
				{
					gl::Uniform1i(uniformInformation.GetLocation(), parameter->As<bool>().GetValue());
				};
			}
			break;
		case ElementType::Int32:
			{
				binder.setter = [parameter] (UniformBindingInformation const& uniformInformation)
				{
					gl::Uniform1i(uniformInformation.GetLocation(), parameter->As<int32>().GetValue());
				};
			}
			break;
		case ElementType::IntV2:
			{
				binder.setter = [parameter] (UniformBindingInformation const& uniformInformation)
				{
					gl::Uniform2iv(uniformInformation.GetLocation(), 1, parameter->As<intV2>().GetValue().GetArray());
				};
			}
			break;
		case ElementType::IntV3:
			{
				binder.setter = [parameter] (UniformBindingInformation const& uniformInformation)
				{
					gl::Uniform3iv(uniformInformation.GetLocation(), 1, parameter->As<intV3>().GetValue().GetArray());
				};
			}
			break;
		case ElementType::IntV4:
			{
				binder.setter = [parameter] (UniformBindingInformation const& uniformInformation)
				{
					gl::Uniform4iv(uniformInformation.GetLocation(), 1, parameter->As<intV4>().GetValue().GetArray());
				};
			}
			break;
		case ElementType::Uint32:
			{
				binder.setter = [parameter] (UniformBindingInformation const& uniformInformation)
				{
					gl::Uniform1ui(uniformInformation.GetLocation(), parameter->As<uint32>().GetValue());
				};
			}
			break;
		case ElementType::UintV2:
			{
				binder.setter = [parameter] (UniformBindingInformation const& uniformInformation)
				{
					gl::Uniform2uiv(uniformInformation.GetLocation(), 1, parameter->As<uintV2>().GetValue().GetArray());
				};
			}
			break;
		case ElementType::UintV3:
			{
				binder.setter = [parameter] (UniformBindingInformation const& uniformInformation)
				{
					gl::Uniform3uiv(uniformInformation.GetLocation(), 1, parameter->As<uintV3>().GetValue().GetArray());
				};
			}
			break;
		case ElementType::UintV4:
			{
				binder.setter = [parameter] (UniformBindingInformation const& uniformInformation)
				{
					gl::Uniform4uiv(uniformInformation.GetLocation(), 1, parameter->As<uintV4>().GetValue().GetArray());
				};
			}
			break;
		case ElementType::Float:
			{
				binder.setter = [parameter] (UniformBindingInformation const& uniformInformation)
				{
					gl::Uniform1f(uniformInformation.GetLocation(), parameter->As<float>().GetValue());
				};
			}
			break;
		case ElementType::FloatV2:
			{
				binder.setter = [parameter] (UniformBindingInformation const& uniformInformation)
				{
					gl::Uniform2fv(uniformInformation.GetLocation(), 1, parameter->As<floatV2>().GetValue().GetArray());
				};
			}
			break;
		case ElementType::FloatV3:
			{
				binder.setter = [parameter] (UniformBindingInformation const& uniformInformation)
				{
					gl::Uniform3fv(uniformInformation.GetLocation(), 1, parameter->As<floatV3>().GetValue().GetArray());
				};
			}
			break;
		case ElementType::FloatV4:
			{
				binder.setter = [parameter] (UniformBindingInformation const& uniformInformation)
				{
					gl::Uniform4fv(uniformInformation.GetLocation(), 1, parameter->As<floatV4>().GetValue().GetArray());
				};
			}
			break;
		case ElementType::Double:
			{
				binder.setter = [parameter] (UniformBindingInformation const& uniformInformation)
				{
					gl::Uniform1d(uniformInformation.GetLocation(), parameter->As<double>().GetValue());
				};
			}
			break;
		case ElementType::DoubleV2:
			{
				binder.setter = [parameter] (UniformBindingInformation const& uniformInformation)
				{
					gl::Uniform2dv(uniformInformation.GetLocation(), 1, parameter->As<doubleV2>().GetValue().GetArray());
				};
			}
			break;
		case ElementType::DoubleV3:
			{
				binder.setter = [parameter] (UniformBindingInformation const& uniformInformation)
				{
					gl::Uniform3dv(uniformInformation.GetLocation(), 1, parameter->As<doubleV3>().GetValue().GetArray());
				};
			}
			break;
		case ElementType::DoubleV4:
			{
				binder.setter = [parameter] (UniformBindingInformation const& uniformInformation)
				{
					gl::Uniform4dv(uniformInformation.GetLocation(), 1, parameter->As<doubleV4>().GetValue().GetArray());
				};
			}
			break;
		case ElementType::FloatM44:
			{
				binder.setter = [parameter] (UniformBindingInformation const& uniformInformation)
				{
					gl::UniformMatrix4fv(uniformInformation.GetLocation(), 1, false, parameter->As<floatM44>().GetValue().GetArray());
				};
			}
			break;
		case ElementType::DoubleM44:
			{
				binder.setter = [parameter] (UniformBindingInformation const& uniformInformation)
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

	// Only can be called after InitializeAllInterfaceInformations. Do not store the return value.
	ProgramObject::UniformBinder& ProgramObject::CreateUniformBinder(std::string const& channel)
	{
		auto found = std::find_if(uniformInformations_.begin(), uniformInformations_.end(), [&channel] (UniformBindingInformation const& uniformInformation)
		{
			return uniformInformation.GetChannel() == channel;
		});
		assert(found != uniformInformations_.end());

		uniformBinders_.push_back(UniformBinder(*found));
		return uniformBinders_.back();
	}

	void ProgramObject::SpecifyAllInterfaceBindingsBeforeLink(InformationPack const& pack)
	{
		// attribute inputs
		for (uint32 i = 0, location = 0; i < pack.attributeInputs.size(); ++i)
		{
			AttributeInputInformation const& attributeInput = pack.attributeInputs[i];
			uint32 elementCount = attributeInput.GetElementCount();
			ElementType elementType = attributeInput.GetElementType();

			bool matrixElement = false;
			switch (elementType)
			{
			case ElementType::FloatM44:
			case ElementType::DoubleM44:
				matrixElement = true;
				break;
			case ElementType::Bool:
			case ElementType::Uint8:
			case ElementType::Uint16:
			case ElementType::Uint32:
			case ElementType::Int8:
			case ElementType::Int16:
			case ElementType::Int32:
			case ElementType::IntV2:
			case ElementType::IntV3:
			case ElementType::IntV4:
			case ElementType::UintV2:
			case ElementType::UintV3:
			case ElementType::UintV4:
			case ElementType::Float:
			case ElementType::FloatV2:
			case ElementType::FloatV3:
			case ElementType::FloatV4:
			case ElementType::Double:
			case ElementType::DoubleV2:
			case ElementType::DoubleV3:
			case ElementType::DoubleV4:
				break;
			default:
				assert(false);
			}
			if (elementCount == 0) // non-array element
			{
				elementCount = 1;
			}
			for (uint32 offset = 0; offset < elementCount; ++offset) // TODO array attribute location can be bound like this?
			{
				gl::BindAttribLocation(glProgramID_, location, attributeInput.GetChannel().c_str());
				location += matrixElement ? 4 : 1;
			}
		}

		// fragment outputs
		for (uint32 i = 0, location = 0; i < pack.fragmentOutputs.size(); ++i)
		{
			FragmentOutputInformation const& fragmentOutput = pack.fragmentOutputs[i];
			gl::BindFragDataLocation(glProgramID_, location, fragmentOutput.GetChannel().c_str());
			location += 1;
		}

	}

	void ProgramObject::SpecifyAllInterfaceBindingsAfterLink(InformationPack const& pack)
	{
		// GL_INVALID_INDEX is -1 actually (signed integer overflow)
		int32 uniformBufferCount = 0;
		gl::GetProgramiv(glProgramID_, gl::GL_ACTIVE_UNIFORM_BLOCKS, &uniformBufferCount);
		assert(static_cast<uint32>(uniformBufferCount) <= pack.uniformBuffers.size());

		{ // uniform buffers, link will set all of these binding to 0
			for (uint32 i = 0; i < pack.uniformBuffers.size(); ++i)
			{
				BufferInformation const& uniformBuffer = pack.uniformBuffers[i];
				uint32 glIndex = gl::GetUniformBlockIndex(glProgramID_, uniformBuffer.GetChannel().c_str());
				if (glIndex != gl::GL_INVALID_INDEX)
				{
					// unspecified binding buffer will be all 0
					gl::UniformBlockBinding(glProgramID_, glIndex, i);
				}
			}
		}

		int32 shaderStorageBufferCount = 0;
		gl::GetProgramInterfaceiv(glProgramID_, gl::GL_SHADER_STORAGE_BLOCK, gl::GL_ACTIVE_RESOURCES, &shaderStorageBufferCount);
		assert(static_cast<uint32>(shaderStorageBufferCount) <= pack.shaderStorageBuffers.size());

		{ // shader storage buffers, link will set all of these binding to 0
			for (uint32 i = 0; i < pack.shaderStorageBuffers.size(); ++i)
			{
				BufferInformation const& shaderStorageBuffer = pack.shaderStorageBuffers[i];
				uint32 glIndex = gl::GetProgramResourceLocationIndex(glProgramID_, gl::GL_SHADER_STORAGE_BLOCK, shaderStorageBuffer.GetChannel().c_str());
				if (glIndex != gl::GL_INVALID_INDEX)
				{
					// unspecified binding buffer will be all 0
					gl::ShaderStorageBlockBinding(glProgramID_, glIndex, i);
				}
			}
		}

		{ // textures
			for (uint32 i = 0; i < pack.textures.size(); ++i)
			{
				TextureInformation const& texture = pack.textures[i];
				int32 location = gl::GetUniformLocation(glProgramID_, texture.GetChannel().c_str());
				if (location != -1) // note: not GL_INVALID_INDEX
				{
					gl::ProgramUniform1i(glProgramID_, location, i);
				}
			}
		}
		
		{ // images
			for (uint32 i = 0; i < pack.images.size(); ++i)
			{
				ImageInformation const& image = pack.images[i];
				int32 location = gl::GetUniformLocation(glProgramID_, image.GetChannel().c_str());
				if (location != -1) // note: not GL_INVALID_INDEX
				{
					gl::ProgramUniform1i(glProgramID_, location, i);
				}
			}
		}
	}

	void ProgramObject::InitializeBindingInformations(InformationPack const& pack)
	{
		int32 maxUniformNameLength = 0;
		gl::GetProgramiv(glProgramID_, gl::GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformNameLength);

		std::string nameBuffer;
		nameBuffer.resize(maxUniformNameLength);

		{ // attribute inputs
			for (AttributeInputInformation const& attributeInput : pack.attributeInputs)
			{
				int32 location = gl::GetAttribLocation(glProgramID_, attributeInput.GetChannel().c_str());
				if (location != -1) // note: not GL_INVALID_INDEX
				{
					attributeInformations_.push_back(AttributeInputBindingInformation(attributeInput.GetChannel(), attributeInput.GetElementType(), attributeInput.GetElementCount(), location));
				}
			}
		}
		{ // fragment outputs
			for (FragmentOutputInformation const& fragmentOutput : pack.fragmentOutputs)
			{
				int32 location = gl::GetFragDataLocation(glProgramID_, fragmentOutput.GetChannel().c_str());
				int32 index = gl::GetFragDataIndex(glProgramID_, fragmentOutput.GetChannel().c_str());
				fragmentOutputInformations_.push_back(FragmentOutputBindingInformation(fragmentOutput.GetChannel(), location, index));
			}
		}
		{ // textures
			for (TextureInformation const& texture : pack.textures)
			{
				int32 location = gl::GetUniformLocation(glProgramID_, texture.GetChannel().c_str());
				if (location != -1) // note: not GL_INVALID_INDEX
				{
					int32 samplerIndex = -1;
					gl::GetUniformiv(glProgramID_, location, &samplerIndex);
					assert(samplerIndex != -1);
					textureInformations_.push_back(TextureBindingInformation(texture.GetChannel(), texture.GetTextureType(), texture.GetTexelType(), samplerIndex));
				}
			}
		}
		{ // images
			for (ImageInformation const& image : pack.images)
			{
				int32 location = gl::GetUniformLocation(glProgramID_, image.GetChannel().c_str());
				if (location != -1) // note: not GL_INVALID_INDEX
				{
					int32 imageIndex = -1;
					gl::GetUniformiv(glProgramID_, location, &imageIndex);
					assert(imageIndex != -1);
					imageInformations_.push_back(ImageBindingInformation(image.GetChannel(), image.GetImageType(), image.GetTexelFormat(), image.GetAccessType(), imageIndex));
				}
			}
		}
		{ // uniforms, TODO remove this
			int32 uniformCount = 0;
			gl::GetProgramiv(glProgramID_, gl::GL_ACTIVE_UNIFORMS, &uniformCount);

			for (int32 i = 0; i < uniformCount; ++i)
			{
				int32 nameLength;
				int32 uniformSize;
				uint32 glType;
				std::string name;
				gl::GetActiveUniform(glProgramID_, i, maxUniformNameLength, &nameLength, &uniformSize, &glType, &nameBuffer[0]);
				name = nameBuffer.substr(0, nameLength); // no '\0' included
				int32 location = gl::GetUniformLocation(glProgramID_, name.c_str());
				if (location != gl::GL_INVALID_INDEX) // if GL_INVALID_INDEX, it is in a buffer. this is handled at buffer part
				{
					ElementType type = ElementTypeFromeGLType(glType);
					if (IsSamplerType(type))
					{
					}
					else if (IsImageType(type))
					{
					}
#ifdef XREX_DEBUG
					else if (IsAtomicBufferType(type))
					{
						assert(false);
					}
#endif // XREX_DEBUG
					else
					{
						uniformInformations_.push_back(UniformBindingInformation(name, type, uniformSize, location));
					}
				}
			}
		}
		{ // uniform buffer
			for (BufferInformation const& uniformBuffer : pack.uniformBuffers)
			{
				uint32 glIndex = gl::GetUniformBlockIndex(glProgramID_, uniformBuffer.GetChannel().c_str());
				if (glIndex != gl::GL_INVALID_INDEX)
				{
					int32 bindingIndex;
					int32 dataSize;
					int32 variableCount;
					std::vector<int32> variableIndices;
					std::vector<int32> variableOffsets;
					std::vector<int32> arrayStrides;
					std::vector<int32> matrixStrides;
					gl::GetActiveUniformBlockiv(glProgramID_, glIndex, gl::GL_UNIFORM_BLOCK_BINDING, &bindingIndex);
					gl::GetActiveUniformBlockiv(glProgramID_, glIndex, gl::GL_UNIFORM_BLOCK_DATA_SIZE, &dataSize);
					gl::GetActiveUniformBlockiv(glProgramID_, glIndex, gl::GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &variableCount);
					variableIndices.resize(variableCount);
					gl::GetActiveUniformBlockiv(glProgramID_, glIndex, gl::GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, &variableIndices[0]);
					variableOffsets.resize(variableCount);
					arrayStrides.resize(variableCount);
					matrixStrides.resize(variableCount);
					gl::GetActiveUniformsiv(glProgramID_, variableCount, reinterpret_cast<uint32 const*>(variableIndices.data()), gl::GL_UNIFORM_OFFSET, &variableOffsets[0]);
					gl::GetActiveUniformsiv(glProgramID_, variableCount, reinterpret_cast<uint32 const*>(variableIndices.data()), gl::GL_UNIFORM_ARRAY_STRIDE, &arrayStrides[0]);
					gl::GetActiveUniformsiv(glProgramID_, variableCount, reinterpret_cast<uint32 const*>(variableIndices.data()), gl::GL_UNIFORM_MATRIX_STRIDE, &matrixStrides[0]);
					std::vector<BufferBindingInformation::BufferVariableInformation> bufferVariableInformations;
					for (int32 j = 0; j < variableCount; ++j) // use informations get from gl, as gl uses flat variable hierarchies
					{
						int32 index = variableIndices[j];
						int32 nameLength;
						int32 variableSize;
						uint32 glType;
						std::string variableName;
						gl::GetActiveUniform(glProgramID_, index, maxUniformNameLength, &nameLength, &variableSize, &glType, &nameBuffer[0]);
						variableName = nameBuffer.substr(0, nameLength); // no '\0' included

						bufferVariableInformations.push_back(BufferBindingInformation::BufferVariableInformation(
							variableName, ElementTypeFromeGLType(glType), variableSize, variableOffsets[j], arrayStrides[j], matrixStrides[j]));
					}
					bufferInformations_.push_back(BufferBindingInformation(
						uniformBuffer.GetChannel(), BufferView::BufferType::Uniform, bindingIndex, dataSize, std::move(bufferVariableInformations)));
				}
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
				std::vector<BufferBindingInformation::BufferVariableInformation> bufferVariableInformations;
				for (int32 j = 0; j < variableCount; ++j)
				{
					int32 index = variableIndices[j];
					int32 nameLength;
					int32 variableSize;
					uint32 glType;
					std::string variableName;
					gl::GetActiveUniform(glProgramID_, index, maxUniformNameLength, &nameLength, &variableSize, &glType, &nameBuffer[0]);
					variableName = nameBuffer.substr(0, nameLength); // no '\0' included

					bufferVariableInformations.push_back(BufferBindingInformation::BufferVariableInformation(
						variableName, ElementTypeFromeGLType(glType), variableSize, variableOffsets[j], arrayStrides[j], 0));
				}
				BufferInformation const& atomicCounterBuffer = pack.atomicCounterBuffers[bindingIndex];
				bufferInformations_.push_back(BufferBindingInformation(
					atomicCounterBuffer.GetChannel(), BufferView::BufferType::AtomicCounter, bindingIndex, dataSize, std::move(bufferVariableInformations)));
			}
		}

		{ // TODO transform feedback

		}
		
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


	ShaderResourceBuffer::BufferMapper::BufferMapper(ShaderResourceBuffer& buffer)
		: buffer_(buffer), mapper_(buffer.GetBuffer()->GetMapper(AccessType::WriteOnly))
	{
	}

	ShaderResourceBuffer::BufferMapper::BufferMapper(BufferMapper&& right)
		: buffer_(right.buffer_), mapper_(std::move(right.mapper_))
	{
	}






	ShaderResourceBuffer::ShaderResourceBuffer(BufferBindingInformation const& information)
		: BufferView(information.GetBufferType()), information_(information)
	{
		assert(IsResourceBufferType(information.GetBufferType()));
	}

	ShaderResourceBuffer::ShaderResourceBuffer(BufferBindingInformation const& information, GraphicsBufferSP const& buffer)
		: BufferView(information.GetBufferType(), buffer), information_(information)
	{
		assert(IsResourceBufferType(information.GetBufferType()));
	}


	ShaderResourceBuffer::~ShaderResourceBuffer()
	{
	}


	std::pair<bool, ShaderResourceBuffer::VariableSetter> ShaderResourceBuffer::GetSetter(std::string const& name)
	{
		std::pair<bool, BufferBindingInformation::BufferVariableInformation const&> result = GetBufferInformation().GetBufferVariableInformation(name);
#ifdef XREX_DEBUG
		return std::make_pair(result.first, VariableSetter(result.second, *this));
#else
		return std::make_pair(result.first, VariableSetter(result.second));
#endif
	}

	bool ShaderResourceBuffer::SetBufferCheck(GraphicsBufferSP const& newBuffer)
	{
		if (newBuffer)
		{
			return newBuffer->GetSize() == information_.GetDataSize();
		}
		return true;
	}



	ShaderResourceBuffer::VariableSetter::VariableSetter()
		: variableInformation_()
#ifdef XREX_DEBUG
		, buffer_(nullptr)
#endif
	{
	}

}
