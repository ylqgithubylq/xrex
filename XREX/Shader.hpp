#pragma once

#include "Declare.hpp"


#include <string>
#include <vector>
#include <string>
#include <functional>



class ShaderObject
	: Noncopyable
{
public:
	enum ShaderType
	{
		VertexShader,
		FragmentShader,
		GeometryShader,
		TessellationControlShader,
		TessellationEvaluationShader,

		CountOfShaderTypes
	};

public:
	ShaderObject(ShaderType type, std::string const & source);
	ShaderObject(ShaderType type, std::string&& source);
	~ShaderObject();

	void Destory();

	ShaderType GetType() const
	{
		return type_;
	}

	uint32 GetID() const
	{
		return shaderID_;
	}

	bool IsValidate() const
	{
		return validate_;
	}
	std::string const & GetCompileError() const
	{
		return errorString_;
	}

private:

	static std::string const VERSION_MACRO;

	static std::vector<std::string> InitializeShaderMacros();
	static std::vector<std::string> const SHADER_DEFINE_MACROS;

	static std::vector<uint32> InitializeGLShaderTypeMapping();
	static std::vector<uint32> const SHADER_TYPE_TO_GL_SHADER_TYPE;


private:
	static uint32 GetGLShaderType(ShaderType type);


private:
	bool Compile();

private:
	ProgramObject* program_;
	ShaderType type_;
	std::string source_;
	bool validate_;
	std::string errorString_;
	uint32 shaderID_;
};

class ProgramObject
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
	std::string const & GetLinkError() const
	{
		return errorString_;
	}

	void Bind();
	/*
	 *	@return: -1 indicates not found.
	 */
	int32 GetAttributeLocation(std::string const & channel) const;

	/*
	 *	@effect: new parameters will be added to it if this shader have uniform names that not in parameters of effect.
	 */
	void InitializeParameterSetters(RenderingEffect& effect);

private:
	struct UniformBinder
	{
		uint32 glType;
		int32 elementCount;
		int32 location;
		std::function<void()> setter;
	};
	struct AttributeBindingInformation
	{
		std::string channel;
		uint32 glType;
		int32 elementCount; // always 1?
		int32 location;
	};
private:
	void InitializeUniformBinder(UniformBinder& binder, EffectParameterSP& parameter, uint32& availableSamplerLocation);


private:
	std::vector<ShaderObjectSP> shaders_;
	bool validate_;
	std::string errorString_;
	uint32 programID_;
	
	std::vector<UniformBinder> uniformBinders_;
	std::vector<AttributeBindingInformation> attributeBindingInformation_;
};

