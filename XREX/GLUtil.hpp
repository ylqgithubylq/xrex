#pragma once

#include "BasicType.hpp"
#include "RenderingPipelineState.hpp"
#include "Sampler.hpp"

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
}
