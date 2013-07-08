#pragma once

#include "Declare.hpp"


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
			enum class BufferType
			{
				UniformBuffer,
				AtomicCounterBuffer,
				ShaderStorageBuffer,

				BufferTypeCount,
			};

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
				: bindingIndex_(-1), dataSize_(0), type_(BufferType::BufferTypeCount)
			{
			}
			BufferInformation(std::string const& name, BufferType type, int32 bindingIndex, uint32 dataSize, std::vector<BufferVariableInformation>&& bufferVariableInformations)
				: name_(name), type_(type), bindingIndex_(bindingIndex), dataSize_(dataSize), bufferVariableInformations_(std::move(bufferVariableInformations))
			{
			}
			BufferInformation(std::string&& name, BufferType type, int32 bindingIndex, uint32 dataSize, std::vector<BufferVariableInformation>&& bufferVariableInformations)
				: name_(std::move(name)), type_(type), bindingIndex_(bindingIndex), dataSize_(dataSize), bufferVariableInformations_(std::move(bufferVariableInformations))
			{
			}

			std::string const& GetName() const
			{
				return name_;
			}
			BufferType GetBufferType() const
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

		private:
			std::string name_;
			BufferType type_;
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

}
