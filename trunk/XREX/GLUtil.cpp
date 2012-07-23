#include "XREX.hpp"

#include "GLUtil.hpp"

#include <CoreGL.hpp>

#include <vector>
#include <utility>

using std::vector;

namespace
{
	std::vector<uint32> InitializeTypeMap()
	{
		vector<uint32> parameterMap(static_cast<uint32>(ElementType::ParameterTypeCount));
		parameterMap[static_cast<uint32>(ElementType::Bool)] = gl::GL_BOOL;
		parameterMap[static_cast<uint32>(ElementType::Uint8)] = gl::GL_UNSIGNED_BYTE;
		parameterMap[static_cast<uint32>(ElementType::Uint16)] = gl::GL_UNSIGNED_SHORT;
		parameterMap[static_cast<uint32>(ElementType::Uint32)] = gl::GL_UNSIGNED_INT;
		parameterMap[static_cast<uint32>(ElementType::Int8)] = gl::GL_BYTE;
		parameterMap[static_cast<uint32>(ElementType::Int16)] = gl::GL_SHORT;
		parameterMap[static_cast<uint32>(ElementType::Int32)] = gl::GL_INT;
		parameterMap[static_cast<uint32>(ElementType::Float)] = gl::GL_FLOAT;
		parameterMap[static_cast<uint32>(ElementType::FloatV2)] = gl::GL_FLOAT_VEC2;
		parameterMap[static_cast<uint32>(ElementType::FloatV3)] = gl::GL_FLOAT_VEC3;
		parameterMap[static_cast<uint32>(ElementType::FloatV4)] = gl::GL_FLOAT_VEC4;
		parameterMap[static_cast<uint32>(ElementType::Sampler1D)] = gl::GL_SAMPLER_1D;
		parameterMap[static_cast<uint32>(ElementType::Sampler2D)] = gl::GL_SAMPLER_2D;
		parameterMap[static_cast<uint32>(ElementType::Sampler3D)] = gl::GL_SAMPLER_3D;
		parameterMap[static_cast<uint32>(ElementType::SamplerCube)] = gl::GL_SAMPLER_CUBE;
		return std::move(parameterMap);
	}
}

uint32 GetGLType(ElementType type)
{
	static vector<uint32> const ELEMENT_TYPE_TO_GL_TYPE = InitializeTypeMap();
	return ELEMENT_TYPE_TO_GL_TYPE[static_cast<uint32>(type)];
}
