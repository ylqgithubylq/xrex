#pragma once

#include "BasicType.hpp"
#include "RenderingPipelineState.hpp"
#include "Sampler.hpp"
#include "Shader.hpp"
#include "GraphicsBuffer.hpp"


namespace XREX
{

	XREX_API void SetGLState(uint32 glState, bool on);

	XREX_API uint32 GLTypeFromElementType(ElementType type);

	XREX_API uint32 GLPolygonModeFromPolygonMode(RenderingPipelineState::PolygonMode polygonMode);

	XREX_API uint32 GLCompareFunctionFromCompareFunction(RenderingPipelineState::CompareFunction compareFunction);

	XREX_API uint32 GLStencilOperationFromStencilOperation(RenderingPipelineState::StencilOperation stencilOperation);

	XREX_API uint32 GLBlendOperationFromBlendOperation(RenderingPipelineState::BlendOperation blendOperation);

	XREX_API uint32 GLAlphaBlendFactorFromAlphaBlendFactor(RenderingPipelineState::AlphaBlendFactor alphaBlendFactor);

	XREX_API uint32 GLTextureAddressingModeFromAddressingMode(SamplerState::TextureAddressingMode addressingMode);

	XREX_API uint32 GLFilterOperationFromTextureFilterOperation(SamplerState::TextureFilterOperation filterOperation);

	XREX_API uint32 GLShaderTypeFromShaderType(ShaderObject::ShaderType type);

	XREX_API uint32 GLBufferTypeFromBufferType(GraphicsBuffer::BufferType type);

	XREX_API uint32 GLUsageFromUsage(GraphicsBuffer::Usage usage);

	XREX_API uint32 GlAccessTypeFromAccessType(GraphicsBuffer::AccessType type);

	XREX_API ElementType ElementTypeFromeGLType(uint32 glType);

}
