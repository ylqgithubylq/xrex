#include "XREX.hpp"

#include "GLUtil.hpp"

#include <CoreGL.hpp>

#include <array>

namespace XREX
{
	void SetGLState(uint32 glState, bool on)
	{
		// 	static decltype(&gl::Enable) Functions[] = { gl::Enable, gl::Disable };
		// 	Functions[on](glState);
		if (on)
		{
			gl::Enable(glState);
		}
		else
		{
			gl::Disable(glState);
		}
	}

	uint32 GLTypeFromElementType(ElementType type)
	{
		switch (type)
		{
		case ElementType::Void:
			assert(false);
			return 0;
		case ElementType::Bool:
			return gl::GL_BOOL;
		case ElementType::Uint8:
			return gl::GL_UNSIGNED_BYTE;
		case ElementType::Uint16:
			return gl::GL_UNSIGNED_SHORT;
		case ElementType::Uint32:
			return gl::GL_UNSIGNED_INT;
		case ElementType::Int8:
			return gl::GL_BYTE;
		case ElementType::Int16:
			return gl::GL_SHORT;
		case ElementType::Int32:
			return gl::GL_INT;
		case ElementType::Float:
			return gl::GL_FLOAT;
		case ElementType::FloatV2:
			return gl::GL_FLOAT_VEC2;
		case ElementType::FloatV3:
			return gl::GL_FLOAT_VEC3;
		case ElementType::FloatV4:
			return gl::GL_FLOAT_VEC4;
		case ElementType::FloatM44:
			assert(false);
			return 0;
		case ElementType::Sampler1D:
			return gl::GL_SAMPLER_1D;
		case ElementType::Sampler2D:
			return gl::GL_SAMPLER_2D;
		case ElementType::Sampler3D:
			return gl::GL_SAMPLER_3D;
		case ElementType::SamplerCube:
			return gl::GL_SAMPLER_CUBE;
		case ElementType::ParameterTypeCount:
			assert(false);
			return 0;
		default:
			assert(false);
			return 0;
		}
// 		static std::array<uint32, static_cast<uint32>(ElementType::ParameterTypeCount)> const mapping = [] ()
// 		{
// 			std::remove_const<decltype(mapping)>::type parameterMap;
// 			parameterMap[static_cast<uint32>(ElementType::Bool)] = gl::GL_BOOL;
// 			parameterMap[static_cast<uint32>(ElementType::Uint8)] = gl::GL_UNSIGNED_BYTE;
// 			parameterMap[static_cast<uint32>(ElementType::Uint16)] = gl::GL_UNSIGNED_SHORT;
// 			parameterMap[static_cast<uint32>(ElementType::Uint32)] = gl::GL_UNSIGNED_INT;
// 			parameterMap[static_cast<uint32>(ElementType::Int8)] = gl::GL_BYTE;
// 			parameterMap[static_cast<uint32>(ElementType::Int16)] = gl::GL_SHORT;
// 			parameterMap[static_cast<uint32>(ElementType::Int32)] = gl::GL_INT;
// 			parameterMap[static_cast<uint32>(ElementType::Float)] = gl::GL_FLOAT;
// 			parameterMap[static_cast<uint32>(ElementType::FloatV2)] = gl::GL_FLOAT_VEC2;
// 			parameterMap[static_cast<uint32>(ElementType::FloatV3)] = gl::GL_FLOAT_VEC3;
// 			parameterMap[static_cast<uint32>(ElementType::FloatV4)] = gl::GL_FLOAT_VEC4;
// 			parameterMap[static_cast<uint32>(ElementType::Sampler1D)] = gl::GL_SAMPLER_1D;
// 			parameterMap[static_cast<uint32>(ElementType::Sampler2D)] = gl::GL_SAMPLER_2D;
// 			parameterMap[static_cast<uint32>(ElementType::Sampler3D)] = gl::GL_SAMPLER_3D;
// 			parameterMap[static_cast<uint32>(ElementType::SamplerCube)] = gl::GL_SAMPLER_CUBE;
// 			return parameterMap;
// 		} ();
// 		return mapping[static_cast<uint32>(type)];
	}

	uint32 GLPolygonModeFromPolygonMode(RasterizerState::PolygonMode polygonMode)
	{
		switch (polygonMode)
		{
		case RasterizerState::PolygonMode::Point:
			return gl::GL_POINT;
		case RasterizerState::PolygonMode::Line:
			return gl::GL_LINE;
		case RasterizerState::PolygonMode::Fill:
			return gl::GL_FILL;
		default:
			assert(false);
			return gl::GL_FILL;
		}
	}

	uint32 GLCompareFunctionFromCompareFunction(DepthStencilState::CompareFunction compareFunction)
	{
		switch (compareFunction)
		{
		case DepthStencilState::CompareFunction::AlwaysFail:
			return gl::GL_NEVER;
		case DepthStencilState::CompareFunction::AlwaysPass:
			return gl::GL_ALWAYS;
		case DepthStencilState::CompareFunction::Less:
			return gl::GL_LESS;
		case DepthStencilState::CompareFunction::LessEqual:
			return gl::GL_LEQUAL;
		case DepthStencilState::CompareFunction::Equal:
			return gl::GL_EQUAL;
		case DepthStencilState::CompareFunction::NotEqual:
			return gl::GL_NOTEQUAL;
		case DepthStencilState::CompareFunction::Greater:
			return gl::GL_GREATER;
		case DepthStencilState::CompareFunction::GreaterEqual:
			return gl::GL_GEQUAL;
		default:
			assert(false);
			return gl::GL_EQUAL;
		}
	}

	uint32 GLStencilOperationFromStencilOperation(DepthStencilState::StencilOperation stencilOperation)
	{
		switch (stencilOperation)
		{
		case DepthStencilState::StencilOperation::Keep:
			return gl::GL_KEEP;
		case DepthStencilState::StencilOperation::Zero:
			return gl::GL_ZERO;
		case DepthStencilState::StencilOperation::Replace:
			return gl::GL_REPLACE;
		case DepthStencilState::StencilOperation::Increase:
			return gl::GL_INCR;
		case DepthStencilState::StencilOperation::Decrease:
			return gl::GL_DECR;
		case DepthStencilState::StencilOperation::Invert:
			return gl::GL_INVERT;
		case DepthStencilState::StencilOperation::IncreaseWarp:
			return gl::GL_INCR_WRAP;
		case DepthStencilState::StencilOperation::DecreaseWarp:
			return gl::GL_DECR_WRAP;
		default:
			assert(false);
			return gl::GL_KEEP;
		}
	}

	uint32 GLBlendOperationFromBlendOperation(BlendState::BlendOperation blendOperation)
	{
		switch (blendOperation)
		{
		case RenderingPipelineState::BlendOperation::Add:
			return gl::GL_FUNC_ADD;
		case RenderingPipelineState::BlendOperation::Sub:
			return gl::GL_FUNC_SUBTRACT;
		case RenderingPipelineState::BlendOperation::ReverseSub:
			return gl::GL_FUNC_REVERSE_SUBTRACT;
		case RenderingPipelineState::BlendOperation::Min:
			return gl::GL_MIN;
		case RenderingPipelineState::BlendOperation::Max:
			return gl::GL_MAX;
		default:
			assert(false);
			return gl::GL_FUNC_ADD;
		}
	}

	uint32 GLAlphaBlendFactorFromAlphaBlendFactor(BlendState::AlphaBlendFactor alphaBlendFactor)
	{
		switch (alphaBlendFactor)
		{
		case RenderingPipelineState::AlphaBlendFactor::Zero:
			return gl::GL_ZERO;
		case RenderingPipelineState::AlphaBlendFactor::One:
			return gl::GL_ONE;
		case RenderingPipelineState::AlphaBlendFactor::SourceAlpha:
			return gl::GL_SRC_ALPHA;
		case RenderingPipelineState::AlphaBlendFactor::DestinationAlpha:
			return gl::GL_DST_ALPHA;
		case RenderingPipelineState::AlphaBlendFactor::OneMinusSourceAlpha:
			return gl::GL_ONE_MINUS_SRC_ALPHA;
		case RenderingPipelineState::AlphaBlendFactor::OneMinusDestinationAlpha:
			return gl::GL_ONE_MINUS_DST_ALPHA;
		case RenderingPipelineState::AlphaBlendFactor::SourceColor:
			return gl::GL_SRC_COLOR;
		case RenderingPipelineState::AlphaBlendFactor::DestinationColor:
			return gl::GL_DST_COLOR;
		case RenderingPipelineState::AlphaBlendFactor::OneMinusSourceColor:
			return gl::GL_ONE_MINUS_SRC_COLOR;
		case RenderingPipelineState::AlphaBlendFactor::OneMinusDestinationColor:
			return gl::GL_ONE_MINUS_DST_COLOR;
		case RenderingPipelineState::AlphaBlendFactor::SourceAlphaSaturate:
			return gl::GL_SRC_ALPHA_SATURATE;
		default:
			assert(false);
			return gl::GL_ZERO;
		}
	}

	uint32 GLTextureAddressingModeFromAddressingMode(SamplerState::TextureAddressingMode addressingMode)
	{
		switch (addressingMode)
		{
		case SamplerState::TextureAddressingMode::Repeat:
			return gl::GL_REPEAT;
		case SamplerState::TextureAddressingMode::MirroredRepeat:
			return gl::GL_MIRRORED_REPEAT;
		case SamplerState::TextureAddressingMode::ClampToEdge:
			return gl::GL_CLAMP_TO_EDGE;
		case SamplerState::TextureAddressingMode::ClampToBorder:
			return gl::GL_CLAMP_TO_BORDER;
		default:
			assert(false);
			return gl::GL_REPEAT;
		}
	}

	uint32 GLFilterOperationFromTextureFilterOperation(SamplerState::TextureFilterOperation filterOperation)
	{
		switch (filterOperation)
		{
		case SamplerState::TextureFilterOperation::Nearest:
			return gl::GL_NEAREST;
		case SamplerState::TextureFilterOperation::Linear:
			return gl::GL_LINEAR;
		case SamplerState::TextureFilterOperation::NearestMipmapNearest:
			return gl::GL_NEAREST_MIPMAP_NEAREST;
		case SamplerState::TextureFilterOperation::LinearMipmapNearest:
			return gl::GL_LINEAR_MIPMAP_NEAREST;
		case SamplerState::TextureFilterOperation::NearestMipmapLinear:
			return gl::GL_NEAREST_MIPMAP_LINEAR;
		case SamplerState::TextureFilterOperation::LinearMipmapLinear:
			return gl::GL_LINEAR_MIPMAP_LINEAR;
		case SamplerState::TextureFilterOperation::Anisotropic:
			assert(false);
			return gl::GL_NEAREST;
		default:
			assert(false);
			return gl::GL_NEAREST;
		}
	}

}

