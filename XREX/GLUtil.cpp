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
			return gl::GL_FLOAT_MAT4;
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
			return 0;
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
			return 0;
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
			return 0;
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
			return 0;
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
			return 0;
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
			return 0;
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
			return 0;
		default:
			assert(false);
			return 0;
		}
	}

	uint32 GLShaderTypeFromShaderType(ShaderObject::ShaderType type)
	{
		switch (type)
		{
		case ShaderObject::ShaderType::VertexShader:
			return gl::GL_VERTEX_SHADER;
		case ShaderObject::ShaderType::FragmentShader:
			return gl::GL_FRAGMENT_SHADER;
		case ShaderObject::ShaderType::GeometryShader:
			return gl::GL_GEOMETRY_SHADER;
		case ShaderObject::ShaderType::TessellationControlShader:
			gl::GL_TESS_CONTROL_SHADER;
		case ShaderObject::ShaderType::TessellationEvaluationShader:
			gl::GL_TESS_EVALUATION_SHADER;
		case ShaderObject::ShaderType::CountOfShaderTypes:
			assert(false);
			return 0;
		default:
			assert(false);
			return 0;
		}
	}


	uint32 GLBufferTypeFromBufferType(BufferView::BufferType type)
	{
		switch (type)
		{
		case BufferView::BufferType::Vertex:
			return gl::GL_ARRAY_BUFFER;
		case BufferView::BufferType::Index:
			return gl::GL_ELEMENT_ARRAY_BUFFER;
		case BufferView::BufferType::Uniform:
			return gl::GL_UNIFORM_BUFFER;
		case BufferView::BufferType::TransformFeedback:
			return gl::GL_TRANSFORM_FEEDBACK_BUFFER;
		case BufferView::BufferType::Texture:
			return gl::GL_TEXTURE_BUFFER;
		case BufferView::BufferType::TypeCount:
			assert(false);
			return 0;
		default:
			assert(false);
			return 0;
		}
	}

	uint32 GLTextureTypeFromTextureType(Texture::TextureType type)
	{
		switch (type)
		{
		case Texture::TextureType::Texture1D:
			return gl::GL_TEXTURE_1D;
		case Texture::TextureType::Texture2D:
			return gl::GL_TEXTURE_2D;
		case Texture::TextureType::Texture3D:
			return gl::GL_TEXTURE_3D;
		case Texture::TextureType::TextureCube:
			return gl::GL_TEXTURE_CUBE_MAP;
		case Texture::TextureType::TextureTypeCount:
			assert(false);
			return 0;
		default:
			assert(false);
			return 0;
		}
	}


	uint32 GLUsageFromUsage(GraphicsBuffer::Usage usage)
	{
		switch (usage)
		{
		case GraphicsBuffer::Usage::Static:
			return gl::GL_STATIC_DRAW;
		case GraphicsBuffer::Usage::Dynamic:
			return gl::GL_DYNAMIC_DRAW;
		case GraphicsBuffer::Usage::Stream:
			return gl::GL_STREAM_DRAW;
		case GraphicsBuffer::Usage::UsageCount:
			assert(false);
			return 0;
		default:
			assert(false);
			return 0;
		}
	}

	uint32 GlAccessTypeFromAccessType(GraphicsBuffer::AccessType type)
	{
		switch (type)
		{
		case GraphicsBuffer::AccessType::ReadOnly:
			return gl::GL_READ_ONLY;
		case GraphicsBuffer::AccessType::WriteOnly:
			return gl::GL_WRITE_ONLY;
		case GraphicsBuffer::AccessType::ReadWrite:
			return gl::GL_READ_WRITE;
		default:
			assert(false);
			return 0;
		}
	}

	GLTextureFormat const& GLTextureFormatFromTexelFormat(TexelFormat format)
	{
		switch (format)
		{
		case TexelFormat::R8:
			{
				static GLTextureFormat const Format(gl::GL_R8, gl::GL_RED, gl::GL_UNSIGNED_BYTE);
				return Format;
			}
		case TexelFormat::RG8:
			{
				static GLTextureFormat const Format(gl::GL_RG8, gl::GL_RG, gl::GL_UNSIGNED_BYTE);
				return Format;
			}
		case TexelFormat::RGB8:
			{
				static GLTextureFormat const Format(gl::GL_RGB8, gl::GL_RGB, gl::GL_UNSIGNED_BYTE);
				return Format;
			}
		case TexelFormat::RGBA8:
			{
				static GLTextureFormat const Format(gl::GL_RGBA8, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE);
				return Format;
			}
		case TexelFormat::R16I:
			{
				static GLTextureFormat const Format(gl::GL_R16I, gl::GL_RED_INTEGER, gl::GL_SHORT);
				return Format;
			}
		case TexelFormat::RG16I:
			{
				static GLTextureFormat const Format(gl::GL_RG16I, gl::GL_RG_INTEGER, gl::GL_SHORT);
				return Format;
			}
		case TexelFormat::RGB16I:
			{
				static GLTextureFormat const Format(gl::GL_RGB16I, gl::GL_RGB_INTEGER, gl::GL_SHORT);
				return Format;
			}
		case TexelFormat::RGBA16I:
			{
				static GLTextureFormat const Format(gl::GL_RGBA16I, gl::GL_RGBA_INTEGER, gl::GL_SHORT);
				return Format;
			}
		case TexelFormat::R16UI:
			{
				static GLTextureFormat const Format(gl::GL_R16UI, gl::GL_RED_INTEGER, gl::GL_UNSIGNED_SHORT);
				return Format;
			}
		case TexelFormat::RG16UI:
			{
				static GLTextureFormat const Format(gl::GL_RG16UI, gl::GL_RG_INTEGER, gl::GL_UNSIGNED_SHORT);
				return Format;
			}
		case TexelFormat::RGB16UI:
			{
				static GLTextureFormat const Format(gl::GL_RGB16UI, gl::GL_RGB_INTEGER, gl::GL_UNSIGNED_SHORT);
				return Format;
			}
		case TexelFormat::RGBA16UI:
			{
				static GLTextureFormat const Format(gl::GL_RGBA16UI, gl::GL_RGBA_INTEGER, gl::GL_UNSIGNED_SHORT);
				return Format;
			}
		case TexelFormat::R16F:
			{
				static GLTextureFormat const Format(gl::GL_R16F, gl::GL_RED, gl::GL_FLOAT);
				return Format;
			}
		case TexelFormat::RG16F:
			{
				static GLTextureFormat const Format(gl::GL_RG16F, gl::GL_RG, gl::GL_FLOAT);
				return Format;
			}
		case TexelFormat::RGB16F:
			{
				static GLTextureFormat const Format(gl::GL_RGB16F, gl::GL_RGB, gl::GL_FLOAT);
				return Format;
			}
		case TexelFormat::RGBA16F:
			{
				static GLTextureFormat const Format(gl::GL_RGBA16F, gl::GL_RGBA, gl::GL_FLOAT);
				return Format;
			}
		case TexelFormat::R32F:
			{
				static GLTextureFormat const Format(gl::GL_R32F, gl::GL_RED, gl::GL_FLOAT);
				return Format;
			}
		case TexelFormat::RG32F:
			{
				static GLTextureFormat const Format(gl::GL_RG32F, gl::GL_RG, gl::GL_FLOAT);
				return Format;
			}
		case TexelFormat::RGB32F:
			{
				static GLTextureFormat const Format(gl::GL_RGB32F, gl::GL_RGB, gl::GL_FLOAT);
				return Format;
			}
		case TexelFormat::RGBA32F:
			{
				static GLTextureFormat const Format(gl::GL_RGBA32F, gl::GL_RGBA, gl::GL_FLOAT);
				return Format;
			}


		case TexelFormat::BGR8:
			{
				static GLTextureFormat const Format(gl::GL_RGB8, gl::GL_BGR, gl::GL_UNSIGNED_BYTE);
				return Format;
			}
		case TexelFormat::BGRA8:
			{
				static GLTextureFormat const Format(gl::GL_RGBA8, gl::GL_BGRA, gl::GL_UNSIGNED_BYTE);
				return Format;
			}
		case TexelFormat::BGR16F:
			{
				static GLTextureFormat const Format(gl::GL_RGB16F, gl::GL_BGR, gl::GL_FLOAT);
				return Format;
			}
		case TexelFormat::BGRA16F:
			{
				static GLTextureFormat const Format(gl::GL_RGBA16F, gl::GL_BGRA, gl::GL_FLOAT);
				return Format;
			}
		case TexelFormat::BGR32F:
			{
				static GLTextureFormat const Format(gl::GL_RGB32F, gl::GL_BGR, gl::GL_FLOAT);
				return Format;
			}
		case TexelFormat::BGRA32F:
			{
				static GLTextureFormat const Format(gl::GL_RGBA32F, gl::GL_BGRA, gl::GL_FLOAT);
				return Format;
			}

		default:
			{
				static GLTextureFormat const Format;
				assert(false);
				return Format;
			}
		}
	}


	ElementType ElementTypeFromeGLType(uint32 glType)
	{
		switch (glType)
		{
		case gl::GL_BOOL:
			return ElementType::Bool;
		case gl::GL_INT:
			return ElementType::Int32;
		case gl::GL_INT_VEC2:
			return ElementType::IntV2;
		case gl::GL_INT_VEC3:
			return ElementType::IntV3;
		case gl::GL_INT_VEC4:
			return ElementType::IntV4;
		case gl::GL_UNSIGNED_INT:
			return ElementType::Uint32;
		case gl::GL_UNSIGNED_INT_VEC2:
			return ElementType::UintV2;
		case gl::GL_UNSIGNED_INT_VEC3:
			return ElementType::UintV3;
		case gl::GL_UNSIGNED_INT_VEC4:
			return ElementType::UintV4;
		case gl::GL_FLOAT:
			return ElementType::Float;
		case gl::GL_FLOAT_VEC2:
			return ElementType::FloatV2;
		case gl::GL_FLOAT_VEC3:
			return ElementType::FloatV3;
		case gl::GL_FLOAT_VEC4:
			return ElementType::FloatV4;
		case gl::GL_DOUBLE:
			return ElementType::Double;
		case gl::GL_DOUBLE_VEC2:
			return ElementType::DoubleV2;
		case gl::GL_DOUBLE_VEC3:
			return ElementType::DoubleV3;
		case gl::GL_DOUBLE_VEC4:
			return ElementType::DoubleV4;
		case gl::GL_FLOAT_MAT4:
			return ElementType::FloatM44;
		case gl::GL_DOUBLE_MAT4:
			return ElementType::DoubleM44;
		case gl::GL_SAMPLER_1D:
			return ElementType::Sampler1D;
		case gl::GL_SAMPLER_2D:
			return ElementType::Sampler2D;
		case gl::GL_SAMPLER_3D:
			return ElementType::Sampler3D;
		case gl::GL_SAMPLER_CUBE:
			return ElementType::SamplerCube;
		case gl::GL_IMAGE_1D:
			return ElementType::Image1D;
		case gl::GL_IMAGE_2D:
			return ElementType::Image2D;
		case gl::GL_IMAGE_3D:
			return ElementType::Image3D;
		case gl::GL_IMAGE_CUBE:
			return ElementType::ImageCube;
		default:
			// not support.
			assert(false);
			return ElementType::Void;
		}
	}

}

