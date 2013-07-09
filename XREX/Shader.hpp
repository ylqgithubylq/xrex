#pragma once

#include "Declare.hpp"
#include "BufferView.hpp"
#include "GraphicsBuffer.hpp"


#include <string>
#include <vector>
#include <string>
#include <functional>

namespace XREX
{

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
			AttributeInformation(std::string&& channel_, ElementType type, int32 elementCount, int32 location)
				: channel_(std::move(channel_)), elementType_(type), elementCount_(elementCount), location_(location)
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
			UniformInformation(std::string&& channel, ElementType type, int32 elementCount, int32 location)
				: channel_(std::move(channel)), elementType_(type), elementCount_(elementCount), location_(location)
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
				BufferVariableInformation(std::string&& name, ElementType elementType, int32 elementCount, int32 offset, int32 arrayStride, int32 matrixStride)
					: name_(std::move(name)), elementType_(elementType), elementCount_(elementCount), offset_(offset), arrayStride_(arrayStride), matrixStride_(matrixStride)
				{
				}

				std::string const& GetName() const
				{
					return name_;
				}
				ElementType GetType() const
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
		public:
			BufferInformation()
				: bindingIndex_(-1), dataSize_(0), type_(BufferView::BufferType::TypeCount)
			{
			}
			BufferInformation(std::string const& name, BufferView::BufferType type, int32 bindingIndex, uint32 dataSize, std::vector<BufferVariableInformation>&& bufferVariableInformations)
				: name_(name), type_(type), bindingIndex_(bindingIndex), dataSize_(dataSize), bufferVariableInformations_(std::move(bufferVariableInformations))
			{
			}
			BufferInformation(std::string&& name, BufferView::BufferType type, int32 bindingIndex, uint32 dataSize, std::vector<BufferVariableInformation>&& bufferVariableInformations)
				: name_(std::move(name)), type_(type), bindingIndex_(bindingIndex), dataSize_(dataSize), bufferVariableInformations_(std::move(bufferVariableInformations))
			{
			}

			std::string const& GetName() const
			{
				return name_;
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
			std::string name_;
			BufferView::BufferType type_;
			int32 bindingIndex_;
			uint32 dataSize_;

			std::vector<BufferVariableInformation> bufferVariableInformations_;
		};

	public:
		ProgramObject();
		~ProgramObject();

		void AttachShader(ShaderObjectSP& shader);
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

		/*
		 *	@return: return.first indicates whether the channel is found.
		 */
		std::pair<bool, AttributeInformation> GetAttributeInformation(std::string const& channel) const;

		std::vector<AttributeInformation> const& GetAllAttributeInformations() const
		{
			return attributeInformations_;
		}

		/*
		 *	@return: return.first indicates whether the channel is found.
		 */
		std::pair<bool, UniformInformation> GetUniformInformation(std::string const& channel) const;

		std::vector<UniformInformation> const& GetAllUniformInformations() const
		{
			return uniformInformations_;
		}

		/*
		 *	@bufferName: if BufferType is AtomicCounterBuffer, bufferName is just numerical buffer binding index to string.
		 *	@return: return.first indicates whether the channel is found.
		 */
		std::pair<bool, BufferInformation> GetBufferInformation(std::string const& bufferName) const;

		std::vector<BufferInformation> const& GetAllBufferInformations() const
		{
			return bufferInformations_;
		}

		void CreateUniformBinder(std::string const& channel, EffectParameterSP const& parameter);
		void CreateSamplerUniformBinder(std::string const& channel, EffectParameterSP const& parameter, uint32 samplerLocation);
		void CreateImageUniformBinder(std::string const& channel, EffectParameterSP const& parameter, uint32 imageLocation);
		void CreateBufferBinder(std::string const& name, EffectParameterSP const& parameter);

	private:

		static AttributeInformation const NullAttributeInformation;
		static UniformInformation const NullUniformInformation;
		static BufferInformation const NullBufferInformation;
		static BufferInformation::BufferVariableInformation const NullBufferVariableInformation;

		struct UniformBinder
		{
			UniformInformation const& uniformInformation;
			std::function<void(UniformInformation const& uniformInformation)> setter;
			explicit UniformBinder(UniformInformation const& uniformInformation)
				: uniformInformation(uniformInformation)
			{
			}
		};
		UniformBinder& DoCreateUniformBinder(std::string const& channel);

		struct BufferBinder
		{
			BufferInformation const& bufferInformation;
			std::function<void(BufferInformation const& bufferInformation)> setter;
			explicit BufferBinder(BufferInformation const& bufferInformation)
				: bufferInformation(bufferInformation)
			{
			}
		};
		BufferBinder& DoCreateBufferBinder(std::string const& name);

	private:
		std::vector<ShaderObjectSP> shaders_;
		bool validate_;
		std::string errorString_;
		uint32 glProgramID_;
	
		std::vector<UniformInformation> uniformInformations_;
		std::vector<AttributeInformation> attributeInformations_;
		std::vector<BufferInformation> bufferInformations_;

		std::vector<UniformBinder> uniformBinders_;
		std::vector<BufferBinder> bufferBinders_;
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
			BufferVariableSetter(BufferVariableSetter&& right);

			template <typename T>
			bool SetValue(std::string const& name, T const& value)
			{
				std::pair<bool, ProgramObject::BufferInformation::BufferVariableInformation> result = buffer_.GetBufferInformation().GetBufferVariableInformation(name);
				if (result.first)
				{
					uint8* pointer = mapper_.GetPointer<uint8>();
					*reinterpret_cast<T*>(pointer + result.second.GetOffset()) = value;
					return true;
				}
				return false;
			}

			template <typename T>
			bool SetArrayValue(std::string const& name, std::vector<T> const& value)
			{
				std::pair<bool, ProgramObject::BufferInformation::BufferVariableInformation> result = buffer_.GetBufferInformation().GetBufferVariableInformation(name);
				if (result.first)
				{
					assert(result.second.GetElementCount() == value.size());
					uint8* pointer = mapper_.GetPointer<uint8>();
					uint8* start = pointer + result.second.GetOffset();
					if (result.second.GetArrayStride() == 0) // tightly packed data
					{
						for (uint32 i = 0; i < result.second.GetElementCount(); ++i)
						{
							*reinterpret_cast<T*>(start)[i] = value[i];
						}
					}
					else
					{
						for (uint32 i = 0; i < result.second.GetElementCount(); ++i)
						{
							*static_cast<T*>(start + result.second.GetArrayStride() * i) = value[i];
						}
					}
					return true;
				}
				return false;
			}

			void Finish();

		private:
			ShaderResourceBuffer& buffer_;
			GraphicsBuffer::BufferMapper mapper_;
		};

	public:
		explicit ShaderResourceBuffer(ProgramObject::BufferInformation const& information);
		ShaderResourceBuffer(ProgramObject::BufferInformation const& information, GraphicsBufferSP const& buffer);
		virtual ~ShaderResourceBuffer() override;

		ProgramObject::BufferInformation const& GetBufferInformation() const
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
		ProgramObject::BufferInformation const& information_;
	};
}
