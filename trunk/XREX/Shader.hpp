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
		ShaderObject(ShaderType type, std::string const& source);
		ShaderObject(ShaderType type, std::string&& source);
		~ShaderObject();

		void Destory();

		ShaderType GetType() const
		{
			return type_;
		}

		bool IsValidate() const
		{
			return validate_;
		}
		std::string const& GetCompileError() const
		{
			return errorString_;
		}



	private:
		bool Compile();

		uint32 GetGLID() const
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

		void Destory();

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
		 *	@return: -1 indicates not found.
		 */
		int32 GetAttributeLocation(std::string const& channel) const;

		/*
		 *	Do not call this. Used by RenderingPass.
		 *	@effect: new parameters will be added to it if this shader have uniform names that not in parameters of effect.
		 */
		void InitializeParameterSetters(RenderingEffect& effect);

	private:
		struct UniformBinder
		{
			uint32 glType;
			int32 elementCount;
			int32 glLocation;
			std::function<void()> setter;
		};
		struct AttributeBindingInformation
		{
			std::string channel;
			uint32 glType;
			int32 elementCount; // always 1?
			int32 glLocation;
		};
	private:
		void InitializeUniformBinder(UniformBinder& binder, EffectParameterSP& parameter, uint32& availableSamplerLocation);


	private:
		std::vector<ShaderObjectSP> shaders_;
		bool validate_;
		std::string errorString_;
		uint32 glProgramID_;
	
		std::vector<UniformBinder> uniformBinders_;
		std::vector<AttributeBindingInformation> attributeBindingInformation_;
	};

}
