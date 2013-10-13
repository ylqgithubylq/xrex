#pragma once

#include "Declare.hpp"
#include "Rendering/BufferView.hpp"
#include "Rendering/Texture.hpp"
#include "Rendering/GraphicsBuffer.hpp"
#include "Rendering/FrameBuffer.hpp"


#include <string>
#include <vector>
#include <string>
#include <functional>

using namespace XREX;

namespace XREX
{
	class XREX_API AttributeInformation
	{
	public:
		AttributeInformation()
			: elementType_(ElementType::Void), elementCount_(0), location_(-1)
		{
		}
		AttributeInformation(std::string const& channel, ElementType type, int32 elementCount, int32 location)
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

	class XREX_API UniformInformation
	{
	public:
		UniformInformation()
			: elementType_(ElementType::Void), elementCount_(0), location_(-1)
		{
		}
		UniformInformation(std::string const& channel, ElementType type, int32 elementCount, int32 location)
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

	class XREX_API TextureInformation
	{
	public:
		TextureInformation()
			: type_(ElementType::Sampler), bindingIndex_(-1)
		{
		}
		TextureInformation(std::string const& channel, ElementType type, int32 bindingIndex)
			: channel_(channel), type_(type), bindingIndex_(bindingIndex)
		{
		}

		std::string const& GetChannel() const
		{
			return channel_;
		}
		ElementType GetElementType() const
		{
			return type_;
		}
		int32 GetBindingIndex() const
		{
			return bindingIndex_;
		}
	private:
		std::string channel_;
		ElementType type_;
		int32 bindingIndex_;
	};

	class XREX_API ImageInformation
	{
	public:
		ImageInformation()
			: type_(ElementType::Image), format_(TexelFormat::TexelFormatCount), accessType_(AccessType::ReadWrite), bindingIndex_(-1)
		{
		}
		ImageInformation(std::string const& channel, ElementType type, TexelFormat format, AccessType accessType, int32 bindingIndex)
			: channel_(channel), type_(type), format_(format), accessType_(accessType), bindingIndex_(bindingIndex)
		{
		}

		std::string const& GetChannel() const
		{
			return channel_;
		}
		ElementType GetElementType() const
		{
			return type_;
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
		ElementType type_;
		TexelFormat format_;
		AccessType accessType_;
		int32 bindingIndex_;
	};

	class XREX_API BufferInformation
	{
	public:

		class XREX_API BufferVariableInformation
		{
		public:
			BufferVariableInformation()
				: elementType_(ElementType::Void), elementCount_(-1), offset_(0), arrayStride_(-1), matrixStride_(-1)
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
		BufferInformation()
			: bindingIndex_(-1), dataSize_(0), type_(BufferView::BufferType::TypeCount)
		{
		}
		BufferInformation(std::string const& channel, BufferView::BufferType type, int32 bindingIndex, uint32 dataSize, std::vector<BufferVariableInformation>&& bufferVariableInformations)
			: channel_(channel), type_(type), bindingIndex_(bindingIndex), dataSize_(dataSize), bufferVariableInformations_(std::move(bufferVariableInformations))
		{
		}

		std::string const& GetChannel() const
		{
			return channel_;
		}
		ElementType GetElementType() const
		{
			return ElementType::ShaderResourceBuffer;
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

		std::pair<bool, BufferVariableInformation> GetBufferVariableInformation(std::string const& name) const;

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

	class XREX_API FragmentOutputInformation
	{
	public:
		FragmentOutputInformation()
			: location_(-1), index_(0)
		{
		}
		FragmentOutputInformation(std::string const& channel, int32 location, int32 index)
			: channel_(channel), location_(location), index_(index)
		{
		}

		std::string const& GetChannel() const
		{
			return channel_;
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
		int32 location_;
		int32 index_;
	};


	class XREX_API ShaderObject
		: Noncopyable
	{
		friend class ProgramObject;
	public:
		enum class ShaderType
		{
			VertexShader,
			FragmentShader,
			GeometryShader,
			TessellationControlShader,
			TessellationEvaluationShader,
			ComputeShader,

			CountOfShaderTypes
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

		void SpecifyFragmentOutputs(FrameBufferLayoutDescription const& description);

		FrameBufferLayoutDescription const& GetFragmentOutputLayout() const
		{
			return framebufferDescription_;
		}

		void SpecifyImageFormat(std::string const& channel, TexelFormat format, AccessType accessType); // TEMP or should be systematic added

		bool Link();

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
		std::pair<bool, AttributeInformation const&> GetAttributeInformation(std::string const& channel) const;

		std::vector<AttributeInformation> const& GetAllAttributeInformations() const
		{
			return attributeInformations_;
		}

		/*
		 *	@return: return.first indicates whether the channel is found.
		 */
		std::pair<bool, UniformInformation const&> GetUniformInformation(std::string const& channel) const;

		std::vector<UniformInformation> const& GetAllUniformInformations() const
		{
			return uniformInformations_;
		}

		/*
		 *	@return: return.first indicates whether the channel is found.
		 */
		std::pair<bool, TextureInformation const&> GetTextureInformation(std::string const& channel) const;

		std::vector<TextureInformation> const& GetAllTextureInformations() const
		{
			return textureInformations_;
		}

		/*
		 *	@return: return.first indicates whether the channel is found.
		 */
		std::pair<bool, ImageInformation const&> GetImageInformation(std::string const& channel) const;

		std::vector<ImageInformation> const& GetAllImageInformations() const
		{
			return imageInformations_;
		}

		/*
		 *	@channel: if BufferType is AtomicCounterBuffer, channel is just numerical buffer binding index to string.
		 *	@return: return.first indicates whether the channel is found.
		 */
		std::pair<bool, BufferInformation const&> GetBufferInformation(std::string const& channel) const;

		std::vector<BufferInformation> const& GetAllBufferInformations() const
		{
			return bufferInformations_;
		}

		/*
		 *	@return: return.first indicates whether the channel is found.
		 */
		std::pair<bool, FragmentOutputInformation const&> GetFragmentOutputInformation(std::string const& channel) const;

		std::vector<FragmentOutputInformation> const& GetAllFragmentOutputInformations() const
		{
			return fragmentOutputInformations_;
		}

		void ConnectUniformParameter(std::string const& channel, TechniqueParameterSP const& parameter);

	private:

		struct UniformBinder
		{
			UniformInformation const& uniformInformation;
			std::function<void(UniformInformation const& uniformInformation)> setter;
			explicit UniformBinder(UniformInformation const& uniformInformation)
				: uniformInformation(uniformInformation)
			{
			}
		};
		UniformBinder& CreateUniformBinder(std::string const& channel);

		void SpecifyAllInterfaceBindings();
		void InitializeAllInterfaceInformations();
	private:
		std::vector<ShaderObjectSP> shaders_;
		bool validate_;
		std::string errorString_;
		uint32 glProgramID_;

		FrameBufferLayoutDescription framebufferDescription_;
		std::vector<std::tuple<std::string, TexelFormat, AccessType>> imageChannelInformations_; // TEMP
	
		std::vector<UniformInformation> uniformInformations_;
		std::vector<TextureInformation> textureInformations_;
		std::vector<ImageInformation> imageInformations_;
		std::vector<BufferInformation> bufferInformations_;
		std::vector<AttributeInformation> attributeInformations_;

		std::vector<FragmentOutputInformation> fragmentOutputInformations_;

		std::vector<UniformBinder> uniformBinders_;

	};



	class XREX_API ShaderResourceBuffer
		: public BufferView
	{
		friend class BufferVariableSetter;

	public:
		class XREX_API BufferVariableSetter
			: XREX::Noncopyable
		{
			friend class ShaderResourceBuffer;
		private:
			explicit BufferVariableSetter(ShaderResourceBuffer& buffer);

		public:
			class XREX_API Setter
			{
				friend class BufferVariableSetter;
			private:

#ifdef XREX_DEBUG
				Setter(BufferInformation::BufferVariableInformation const& variableInformation, ShaderResourceBuffer const& buffer)
					: variableInformation_(variableInformation), buffer_(buffer)
				{
				}
#else
				explicit Setter(BufferInformation::BufferVariableInformation const& variableInformation)
					: variableInformation_(variableInformation)
				{
				}
#endif

			public:
				template <typename T>
				void SetValue(BufferVariableSetter& setter, T const& value)
				{
					assert(&setter.buffer_ == &buffer_);
					uint8* pointer = setter.mapper_.GetPointer<uint8>();
					*reinterpret_cast<T*>(pointer + variableInformation_.GetOffset()) = value;
				}


				template <typename T>
				void SetArrayValue(BufferVariableSetter& setter, std::vector<T> const& value)
				{
					assert(variableInformation_.GetElementCount() == value.size());
					assert(&setter.buffer_ == &buffer_);
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
				BufferInformation::BufferVariableInformation variableInformation_;
#ifdef XREX_DEBUG
				ShaderResourceBuffer const& buffer_;
#endif
			};

		public:
			BufferVariableSetter(BufferVariableSetter&& right);

			std::pair<bool, Setter> GetSetter(std::string const& name)
			{
				std::pair<bool, BufferInformation::BufferVariableInformation const&> result = buffer_.GetBufferInformation().GetBufferVariableInformation(name);
#ifdef XREX_DEBUG
				return std::make_pair(result.first, Setter(result.second, buffer_));
#else
				return std::make_pair(result.first, Setter(result.second));
#endif
			}
		
			void Finish();

		private:
			ShaderResourceBuffer& buffer_;
			GraphicsBuffer::BufferMapper mapper_;
		};

	public:
		explicit ShaderResourceBuffer(BufferInformation const& information);
		ShaderResourceBuffer(BufferInformation const& information, GraphicsBufferSP const& buffer);
		virtual ~ShaderResourceBuffer() override;

		BufferInformation const& GetBufferInformation() const
		{
			return information_;
		}

		BufferVariableSetter GetVariableSetter()
		{
			assert(HaveBuffer());
			return BufferVariableSetter(*this);
		}

	private:
		virtual bool SetBufferCheck(GraphicsBufferSP const& newBuffer) override;

	private:
		BufferInformation const& information_;
	};
}
