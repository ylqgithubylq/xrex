#pragma once

#include "Declare.hpp"

#include "Rendering/GraphicsType.hpp"
#include "Rendering/BufferView.hpp"
#include "Rendering/Texture.hpp"
#include "Rendering/TextureImage.hpp"
#include "Rendering/GraphicsBuffer.hpp"
#include "Rendering/FrameBuffer.hpp"
#include "Rendering/ShaderProgramInterface.hpp"

#include <string>
#include <vector>
#include <string>
#include <functional>


namespace XREX
{
	class XREX_API AttributeInputBindingInformation
	{
	public:
		AttributeInputBindingInformation()
			: elementType_(ElementType::Void), elementCount_(0), location_(-1)
		{
		}
		AttributeInputBindingInformation(std::string const& channel, ElementType type, int32 elementCount, int32 location)
			: channel_(channel), elementType_(type), elementCount_(elementCount), location_(location)
		{
		}

		std::string const& GetChannel() const
		{
			return channel_;
		}
		ElementType GetElementType() const
		{
			return elementType_;
		}
		int32 GetElementCount() const
		{
			return elementCount_;
		}
		int32 GetLocation() const
		{
			return location_;
		}
	private:
		std::string channel_;
		ElementType elementType_;
		int32 elementCount_;
		int32 location_;
	};

	class XREX_API FragmentOutputBindingInformation
	{
	public:
		FragmentOutputBindingInformation()
			: texelType_(ElementType::ElementTypeCount), location_(-1), index_(0)
		{
		}
		FragmentOutputBindingInformation(std::string const& channel, ElementType texelType, int32 location, int32 index)
			: channel_(channel), texelType_(texelType), location_(location), index_(index)
		{
		}

		std::string const& GetChannel() const
		{
			return channel_;
		}
		ElementType GetTexelType() const
		{
			return texelType_;
		}
		int32 GetLocation() const
		{
			return location_;
		}
		int32 GetIndex() const
		{
			return index_;
		}
	private:
		std::string channel_;
		ElementType texelType_;
		int32 location_;
		int32 index_;
	};


	class XREX_API UniformBindingInformation
	{
	public:
		UniformBindingInformation()
			: elementType_(ElementType::Void), elementCount_(0), location_(-1)
		{
		}
		UniformBindingInformation(std::string const& channel, ElementType type, int32 elementCount, int32 location)
			: channel_(channel), elementType_(type), elementCount_(elementCount), location_(location)
		{
		}

		std::string const& GetChannel() const
		{
			return channel_;
		}
		ElementType GetElementType() const
		{
			return elementType_;
		}
		int32 GetElementCount() const
		{
			return elementCount_;
		}
		int32 GetLocation() const
		{
			return location_;
		}

	private:
		std::string channel_;
		ElementType elementType_;
		int32 elementCount_;
		int32 location_;
	};

	class XREX_API TextureBindingInformation
	{
	public:
		TextureBindingInformation()
			: textureType_(Texture::TextureType::TextureTypeCount), texelType_(ElementType::ElementTypeCount), bindingIndex_(-1)
		{
		}
		TextureBindingInformation(std::string const& channel, Texture::TextureType textureType, ElementType texelType, int32 bindingIndex)
			: channel_(channel), textureType_(textureType), texelType_(texelType), bindingIndex_(bindingIndex)
		{
		}

		std::string const& GetChannel() const
		{
			return channel_;
		}
		Texture::TextureType GetTextureType() const
		{
			return textureType_;
		}
		ElementType GetTexelType() const
		{
			return texelType_;
		}
		int32 GetBindingIndex() const
		{
			return bindingIndex_;
		}
	private:
		std::string channel_;
		Texture::TextureType textureType_;
		ElementType texelType_;
		int32 bindingIndex_;
	};

	class XREX_API ImageBindingInformation
	{
	public:
		ImageBindingInformation()
			: imageType_(TextureImage::ImageType::ImageTypeCount), format_(TexelFormat::TexelFormatCount), accessType_(AccessType::ReadWrite), bindingIndex_(-1)
		{
		}
		ImageBindingInformation(std::string const& channel, TextureImage::ImageType imageType, TexelFormat format, AccessType accessType, int32 bindingIndex)
			: channel_(channel), imageType_(imageType), format_(format), accessType_(accessType), bindingIndex_(bindingIndex)
		{
		}

		std::string const& GetChannel() const
		{
			return channel_;
		}
		TextureImage::ImageType GetImageType() const
		{
			return imageType_;
		}
		TexelFormat GetTexelFormat() const
		{
			return format_;
		}
		AccessType GetAccessType() const
		{
			return accessType_;
		}
		int32 GetBindingIndex() const
		{
			return bindingIndex_;
		}
	private:
		std::string channel_;
		TextureImage::ImageType imageType_;
		TexelFormat format_;
		AccessType accessType_;
		int32 bindingIndex_;
	};

	class XREX_API BufferBindingInformation
	{
	public:

		class XREX_API BufferVariableInformation
		{
		public:
			BufferVariableInformation()
				: elementType_(ElementType::ElementTypeCount), elementCount_(-1), offset_(0), arrayStride_(-1), matrixStride_(-1)
			{
			}
			BufferVariableInformation(std::string const& name, ElementType elementType, int32 elementCount, int32 offset, int32 arrayStride, int32 matrixStride)
				: name_(name), elementType_(elementType), elementCount_(elementCount), offset_(offset), arrayStride_(arrayStride), matrixStride_(matrixStride)
			{
			}

			std::string const& GetName() const
			{
				return name_;
			}
			ElementType GetElementType() const
			{
				return elementType_;
			}
			int32 GetElementCount() const
			{
				return elementCount_;
			}
			int32 GetOffset() const
			{
				return offset_;
			}
			int32 GetArrayStride() const
			{
				return arrayStride_;
			}
			int32 GetmatrixStride() const
			{
				return matrixStride_;
			}
		private:
			std::string name_;
			ElementType elementType_;
			int32 elementCount_;
			int32 offset_;
			int32 arrayStride_;
			int32 matrixStride_;
		};

		static BufferVariableInformation const NullBufferVariableInformation;

	public:
		BufferBindingInformation()
			: bindingIndex_(-1), dataSize_(0), type_(BufferView::BufferType::TypeCount)
		{
		}
		BufferBindingInformation(std::string const& channel, BufferView::BufferType type, int32 bindingIndex, uint32 dataSize, std::vector<BufferVariableInformation>&& bufferVariableInformations)
			: channel_(channel), type_(type), bindingIndex_(bindingIndex), dataSize_(dataSize), bufferVariableInformations_(std::move(bufferVariableInformations))
		{
		}

		std::string const& GetChannel() const
		{
			return channel_;
		}
		ElementType GetElementType() const
		{
			return ElementType::Buffer;
		}
		BufferView::BufferType GetBufferType() const
		{
			return type_;
		}
		int32 GetBindingIndex() const
		{
			return bindingIndex_;
		}
		uint32 GetDataSize() const
		{
			return dataSize_;
		}

		std::pair<bool, BufferVariableInformation const&> GetBufferVariableInformation(std::string const& name) const;

		std::vector<BufferVariableInformation> const& GetAllBufferVariableInformations() const
		{
			return bufferVariableInformations_;
		}

	private:
		std::string channel_;
		BufferView::BufferType type_;
		int32 bindingIndex_;
		uint32 dataSize_;

		std::vector<BufferVariableInformation> bufferVariableInformations_;
	};

	class XREX_API ShaderObject
		: Noncopyable
	{
	public:
		enum class ShaderType
		{
			VertexShader,
			FragmentShader,
			GeometryShader,
			TessellationControlShader,
			TessellationEvaluationShader,
			ComputeShader,

			ShaderTypeCount
		};

	public:
		explicit ShaderObject(ShaderType type);
		~ShaderObject();

		ShaderType GetType() const
		{
			return type_;
		}

		bool Compile(std::vector<std::string const*> const& sources);

		bool IsValidate() const
		{
			return validate_;
		}
		std::string const& GetCompileError() const
		{
			return errorString_;
		}

		uint32 GetID() const
		{
			return glShaderID_;
		}


	private:
		ShaderType type_;
		std::string source_;
		bool validate_;
		std::string errorString_;
		uint32 glShaderID_;
	};

	class XREX_API ProgramObject
		: Noncopyable
	{
	public:
		ProgramObject();
		~ProgramObject();

		void AttachShader(ShaderObjectSP& shader);

		struct XREX_API InformationPack
		{
			std::vector<AttributeInputInformation const> const& attributeInputs;
			std::vector<FragmentOutputInformation const> const& fragmentOutputs;
			std::vector<BufferInformation const> const& uniformBuffers;
			std::vector<BufferInformation const> const& shaderStorageBuffers;
			std::vector<BufferInformation const> const& atomicCounterBuffers;
			std::vector<TextureInformation const> const& textures;
			std::vector<ImageInformation const> const& images;

			/*
			 *	The index of all Informations will be used by program as binding index.
			 */
			InformationPack(std::vector<AttributeInputInformation const> const& attributeInputs,
				std::vector<FragmentOutputInformation const> const& fragmentOutputs,
				std::vector<BufferInformation const> const& uniformBuffers,
				std::vector<BufferInformation const> const& shaderStorageBuffers,
				std::vector<BufferInformation const> const& atomicCounterBuffers,
				std::vector<TextureInformation const> const& textures,
				std::vector<ImageInformation const> const& images)
				: attributeInputs(attributeInputs), fragmentOutputs(fragmentOutputs),
				uniformBuffers(uniformBuffers), shaderStorageBuffers(shaderStorageBuffers), atomicCounterBuffers(atomicCounterBuffers),
				textures(textures), images(images)
			{
			}
		};
		/*
		 *	Informations in pack will be used only in Link and will not be stored.
		 */
		bool Link(InformationPack const& pack);

		bool IsValidate() const
		{
			return validate_;
		}
		std::string const& GetLinkError() const
		{
			return errorString_;
		}

		void Bind();

		void SetupAllUniforms();

		/*
		 *	@return: return.first indicates whether the channel is found.
		 */
		std::pair<bool, AttributeInputBindingInformation const&> GetAttributeInformation(std::string const& channel) const;

		std::vector<AttributeInputBindingInformation> const& GetAllAttributeInformations() const
		{
			return attributeInformations_;
		}

		/*
		 *	@return: return.first indicates whether the channel is found.
		 */
		std::pair<bool, UniformBindingInformation const&> GetUniformInformation(std::string const& channel) const;

		std::vector<UniformBindingInformation> const& GetAllUniformInformations() const
		{
			return uniformInformations_;
		}

		/*
		 *	@return: return.first indicates whether the channel is found.
		 */
		std::pair<bool, TextureBindingInformation const&> GetTextureInformation(std::string const& channel) const;

		std::vector<TextureBindingInformation> const& GetAllTextureInformations() const
		{
			return textureInformations_;
		}

		/*
		 *	@return: return.first indicates whether the channel is found.
		 */
		std::pair<bool, ImageBindingInformation const&> GetImageInformation(std::string const& channel) const;

		std::vector<ImageBindingInformation> const& GetAllImageInformations() const
		{
			return imageInformations_;
		}

		/*
		 *	@channel: if BufferType is AtomicCounterBuffer, channel is just numerical buffer binding index to string.
		 *	@return: return.first indicates whether the channel is found.
		 */
		std::pair<bool, BufferBindingInformation const&> GetBufferInformation(std::string const& channel) const;

		std::vector<BufferBindingInformation> const& GetAllBufferInformations() const
		{
			return bufferInformations_;
		}

		/*
		 *	@return: return.first indicates whether the channel is found.
		 */
		std::pair<bool, FragmentOutputBindingInformation const&> GetFragmentOutputInformation(std::string const& channel) const;

		std::vector<FragmentOutputBindingInformation> const& GetAllFragmentOutputInformations() const
		{
			return fragmentOutputInformations_;
		}

		void ConnectUniformParameter(std::string const& channel, TechniqueParameterSP const& parameter);

	private:

		struct UniformBinder
		{
			UniformBindingInformation const& uniformInformation;
			std::function<void(UniformBindingInformation const& uniformInformation)> setter;
			explicit UniformBinder(UniformBindingInformation const& uniformInformation)
				: uniformInformation(uniformInformation)
			{
			}
		};
		UniformBinder& CreateUniformBinder(std::string const& channel);

		void SpecifyAllInterfaceBindingsBeforeLink(InformationPack const& pack);

		void SpecifyAllInterfaceBindingsAfterLink(InformationPack const& pack);

		void InitializeBindingInformations(InformationPack const& pack);
	private:
		std::vector<ShaderObjectSP> shaders_;
		bool validate_;
		std::string errorString_;
		uint32 glProgramID_;

	
		std::vector<UniformBindingInformation> uniformInformations_;
		std::vector<TextureBindingInformation> textureInformations_;
		std::vector<ImageBindingInformation> imageInformations_;
		std::vector<BufferBindingInformation> bufferInformations_;
		std::vector<AttributeInputBindingInformation> attributeInformations_;
		std::vector<FragmentOutputBindingInformation> fragmentOutputInformations_;

		std::vector<UniformBinder> uniformBinders_;

	};



	class XREX_API ShaderResourceBuffer
		: public BufferView
	{
		friend class BufferMapper;

	public:
		class XREX_API BufferMapper
			: XREX::Noncopyable
		{
			friend class ShaderResourceBuffer;
		private:
			explicit BufferMapper(ShaderResourceBuffer& buffer);

		public:
			BufferMapper(BufferMapper&& right);

			void Finish()
			{
				mapper_.Finish();
			}

		private:
			ShaderResourceBuffer& buffer_;
			GraphicsBuffer::BufferMapper mapper_;
		};

		class XREX_API VariableSetter
		{
			friend class ShaderResourceBuffer;
		private:
			
#ifdef XREX_DEBUG
			VariableSetter(BufferBindingInformation::BufferVariableInformation const& variableInformation, ShaderResourceBuffer const& buffer)
				: variableInformation_(variableInformation), buffer_(&buffer)
			{
			}
#else
			explicit VariableSetter(BufferBindingInformation::BufferVariableInformation const& variableInformation)
				: variableInformation_(variableInformation)
			{
			}
#endif

		public:
			/*
			 *	Default constructed object cannot work.
			 */
			VariableSetter();

			template <typename T>
			void SetValue(BufferMapper& mapper, T const& value)
			{
				assert(variableInformation_.GetElementType() == TypeToElementType<T>::Type);
				assert(&mapper.buffer_ == buffer_);
				uint8* pointer = mapper.mapper_.GetPointer<uint8>();
				*reinterpret_cast<T*>(pointer + variableInformation_.GetOffset()) = value;
			}


			template <typename T>
			void SetArrayValue(BufferMapper& mapper, std::vector<T> const& value)
			{
				assert(variableInformation_.GetElementType() == TypeToElementType<T>::Type);
				assert(variableInformation_.GetElementCount() == value.size());
				assert(&mapper.buffer_ == buffer_);
				uint8* pointer = mapper_.GetPointer<uint8>();
				uint8* start = pointer + variableInformation_.GetOffset();
				if (variableInformation_.GetArrayStride() == 0) // tightly packed data
				{
					for (uint32 i = 0; i < variableInformation_.GetElementCount(); ++i)
					{
						*reinterpret_cast<T*>(start)[i] = value[i];
					}
				}
				else
				{
					for (uint32 i = 0; i < variableInformation_.GetElementCount(); ++i)
					{
						*static_cast<T*>(start + variableInformation_.GetArrayStride() * i) = value[i];
					}
				}
			}
		private:
			BufferBindingInformation::BufferVariableInformation variableInformation_;
#ifdef XREX_DEBUG
			ShaderResourceBuffer const* buffer_;
#endif
		};
	public:
		explicit ShaderResourceBuffer(BufferBindingInformation const& information);
		ShaderResourceBuffer(BufferBindingInformation const& information, GraphicsBufferSP const& buffer);
		virtual ~ShaderResourceBuffer() override;

		BufferBindingInformation const& GetBufferInformation() const
		{
			return information_;
		}

		BufferMapper GetMapper()
		{
			assert(HaveBuffer());
			return BufferMapper(*this);
		}

		std::pair<bool, VariableSetter> GetSetter(std::string const& name);

	private:
		virtual bool SetBufferCheck(GraphicsBufferSP const& newBuffer) override;

	private:
		BufferBindingInformation const& information_;
	};
}
