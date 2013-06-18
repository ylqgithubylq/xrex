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
		class AttributeInformation
		{
		public:
			AttributeInformation()
				: elementType(ElementType::Void), elementCount(0), location(-1)
			{
			}
			AttributeInformation(std::string const& channel, ElementType type, int32 elementCount, int32 location)
				: channel(channel), elementType(type), elementCount(elementCount), location(location)
			{
			}
			AttributeInformation(std::string&& channel, ElementType type, int32 elementCount, int32 location)
				: channel(std::move(channel)), elementType(type), elementCount(elementCount), location(location)
			{
			}
			std::string const& GetChannel() const
			{
				return channel;
			}
			ElementType GetElementType() const
			{
				return elementType;
			}
			int32 GetElementCount() const
			{
				return elementCount;
			}
			int32 GetLocation() const
			{
				return location;
			}
		private:
			std::string channel;
			ElementType elementType;
			int32 elementCount;
			int32 location;
		};

		class UniformInformation
		{
		public:
			UniformInformation()
				: elementType(ElementType::Void), elementCount(0), location(-1)
			{
			}
			UniformInformation(std::string const& channel, ElementType type, int32 elementCount, int32 location)
				: channel(channel), elementType(type), elementCount(elementCount), location(location)
			{
			}
			UniformInformation(std::string&& channel, ElementType type, int32 elementCount, int32 location)
				: channel(std::move(channel)), elementType(type), elementCount(elementCount), location(location)
			{
			}

			std::string const& GetChannel() const
			{
				return channel;
			}
			ElementType GetElementType() const
			{
				return elementType;
			}
			int32 GetElementCount() const
			{
				return elementCount;
			}
			int32 GetLocation() const
			{
				return location;
			}

		private:
			std::string channel;
			ElementType elementType;
			int32 elementCount;
			int32 location;
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

		void CreateUniformBinder(std::string const& channel, EffectParameterSP const& parameter);
		void CreateSamplerUniformBinder(std::string const& channel, EffectParameterSP const& parameter, uint32 samplerLocation);
		void CreateImageUniformBinder(std::string const& channel, EffectParameterSP const& parameter, uint32 imageLocation);

	private:

		static AttributeInformation const NullAttributeInformation;
		static UniformInformation const NullUniformInformation;

		struct UniformBinder
		{
			UniformInformation const& uniformInformation;
			std::function<void(UniformInformation const& uniformInformation)> setter;
			explicit UniformBinder(UniformInformation const& uniformInformation)
				: uniformInformation(uniformInformation)
			{
			}
		};
		UniformBinder& CreateBinder(std::string const& channel);

	private:
		std::vector<ShaderObjectSP> shaders_;
		bool validate_;
		std::string errorString_;
		uint32 glProgramID_;
	
		std::vector<UniformInformation> uniformInformations_;
		std::vector<AttributeInformation> attributeInformations_;

		std::vector<UniformBinder> uniformBinders_;
	};

}
