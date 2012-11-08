#include "XREX.hpp"

#include "RenderingPipelineState.hpp"
#include "GLUtil.hpp"

#include <CoreGL.hpp>

namespace XREX
{
	namespace
	{
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
	}

	RasterizerState::RasterizerState()
		: polygonMode(PolygonMode::Fill), cullMode(CullMode::Back), frontFaceCCW(true), scissorEnable(false), multisampleEnable(false)
	{
	}

	DepthStencilState::DepthStencilState()
		: depthEnable(true), depthWriteMask(true), depthFunction(CompareFunction::Less), stencilEnable(false),
		frontStencilFunction(CompareFunction::AlwaysPass),
		frontStencilReadMask(std::numeric_limits<decltype(frontStencilReadMask)>::max()),
		frontStencilWriteMask(std::numeric_limits<decltype(frontStencilReadMask)>::max()),
		frontStencilFail(StencilOperation::Keep), frontStencilDepthFail(StencilOperation::Keep), frontStencilPass(StencilOperation::Keep),
		backStencilFunction(CompareFunction::AlwaysPass),
		backStencilReadMask(std::numeric_limits<decltype(backStencilReadMask)>::max()),
		backStencilWriteMask(std::numeric_limits<decltype(backStencilReadMask)>::max()),
		backStencilFail(StencilOperation::Keep), backStencilDepthFail(StencilOperation::Keep), backStencilPass(StencilOperation::Keep)
	{
	}

	BlendState::BlendState()
		: alphaToCoverageEnable(false), blendEnable(false),
		blendOperation(BlendOperation::Add), sourceBlend(AlphaBlendFactor::One), destinationBlend(AlphaBlendFactor::Zero),
		blendOperationAlpha(BlendOperation::Add), sourceBlendAlpha(AlphaBlendFactor::One), destinationBlendAlpha(AlphaBlendFactor::Zero),
		redMask(true), greenMask(true), blueMask(true)
	{
	}

	SamplerState::SamplerState()
		: borderColor(0.f, 0.f, 0.f, 0.f),
		addressingModeU(TextureAddressingMode::Wrap), addressingModeV(TextureAddressingMode::Wrap), addressingModeW(TextureAddressingMode::Wrap),
		filterOperation(TextureFilterOperation::Temp), maxAnisotropy(16), minLOD(0), maxLOD(std::numeric_limits<float>::max()), mipmapLODBias(0.f),
		compareFunction(CompareFunction::AlwaysFail)
	{
	}




	RasterizerStateObject::RasterizerStateObject(RasterizerState const& state)
		: state_(state),
		glPolygonMode_(GLPolygonModeFromPolygonMode(state.polygonMode)), glFrontFace_(state.frontFaceCCW ? gl::GL_CCW : gl::GL_CW)
	{
		switch (state_.cullMode)
		{
		case RasterizerState::CullMode::None:
			glCullFaceEnable_ = false;
			glCullFace_ = gl::GL_BACK;
			break;
		case RasterizerState::CullMode::Front:
			glCullFaceEnable_ = true;
			glCullFace_ = gl::GL_FRONT;
			break;
		case RasterizerState::CullMode::Back:
			glCullFaceEnable_ = true;
			glCullFace_ = gl::GL_BACK;
			break;
		default:
			assert(false);
			break;
		}
	}

	void RasterizerStateObject::Bind(float polygonOffsetFactor, float polygonOffsetUnits)
	{
		gl::PolygonMode(gl::GL_FRONT_AND_BACK, glPolygonMode_);
		gl::FrontFace(glFrontFace_);
		SetGLState(gl::GL_CULL_FACE, glCullFaceEnable_);
		gl::CullFace(glCullFace_);
		gl::PolygonOffset(polygonOffsetFactor, polygonOffsetUnits);
		SetGLState(gl::GL_SCISSOR_TEST, state_.scissorEnable);
		SetGLState(gl::GL_MULTISAMPLE, state_.multisampleEnable);
	}




	DepthStencilStateObject::DepthStencilStateObject(DepthStencilState const& state)
		: state_(state),
		glDepthFunction_(GLCompareFunctionFromCompareFunction(state.depthFunction)),
		glFrontStencilFunction_(GLCompareFunctionFromCompareFunction(state.frontStencilFunction)),
		glFrontStencilFail_(GLStencilOperationFromStencilOperation(state.frontStencilFail)),
		glFrontStencilDepthFail_(GLStencilOperationFromStencilOperation(state.frontStencilDepthFail)),
		glFrontStencilPass_(GLStencilOperationFromStencilOperation(state.frontStencilPass)),
		glBackStencilFunction_(GLCompareFunctionFromCompareFunction(state.backStencilFunction)),
		glBackStencilFail_(GLStencilOperationFromStencilOperation(state.backStencilFail)),
		glBackStencilDepthFail_(GLStencilOperationFromStencilOperation(state.backStencilDepthFail)),
		glBackStencilPass_(GLStencilOperationFromStencilOperation(state.backStencilPass))
	{

	}

	void DepthStencilStateObject::Bind(uint16 frontStencilReference, uint16 backStencilReference)
	{
		SetGLState(gl::GL_DEPTH_TEST, state_.depthEnable);
		SetGLState(gl::GL_STENCIL_TEST, state_.stencilEnable);
		gl::DepthMask(state_.depthWriteMask);
		gl::DepthFunc(glDepthFunction_);
		gl::StencilFuncSeparate(gl::GL_FRONT, glFrontStencilFunction_, frontStencilReference, state_.frontStencilReadMask);
		gl::StencilOpSeparate(gl::GL_FRONT, glFrontStencilFail_, glFrontStencilDepthFail_, glFrontStencilPass_);
		gl::StencilMaskSeparate(gl::GL_FRONT, state_.frontStencilWriteMask);
		gl::StencilFuncSeparate(gl::GL_BACK, glBackStencilFunction_, backStencilReference, state_.backStencilReadMask);
		gl::StencilOpSeparate(gl::GL_BACK, glBackStencilFail_, glBackStencilDepthFail_, glBackStencilPass_);
		gl::StencilMaskSeparate(gl::GL_BACK, state_.backStencilWriteMask);
	}




	BlendStateObject::BlendStateObject(BlendState const& state)
		: state_(state),
		glBlendOperation_(GLBlendOperationFromBlendOperation(state.blendOperation)),
		glBlendOperationAlpha_(GLBlendOperationFromBlendOperation(state.blendOperationAlpha)),
		glSourceBlend_(GLAlphaBlendFactorFromAlphaBlendFactor(state.sourceBlend)),
		glDestinationBlend_(GLAlphaBlendFactorFromAlphaBlendFactor(state.destinationBlend)),
		glSourceBlendAlpha_(GLAlphaBlendFactorFromAlphaBlendFactor(state.sourceBlendAlpha)),
		glDestinationBlendAlpha_(GLAlphaBlendFactorFromAlphaBlendFactor(state.destinationBlendAlpha))
	{

	}

	void BlendStateObject::Bind(Color const& blendFactor)
	{
		SetGLState(gl::GL_SAMPLE_ALPHA_TO_COVERAGE, state_.alphaToCoverageEnable);
		SetGLState(gl::GL_BLEND, state_.blendEnable);
		gl::BlendEquationSeparate(glBlendOperation_, glBlendOperationAlpha_);
		gl::BlendFuncSeparate(glSourceBlend_, glDestinationBlend_, glSourceBlendAlpha_, glDestinationBlendAlpha_);
		gl::ColorMask(state_.redMask, state_.greenMask, state_.blueMask, state_.alphaMask);
		gl::BlendColor(blendFactor.R(), blendFactor.G(), blendFactor.B(), blendFactor.A());
	}




	SamplerStateObject::SamplerStateObject(SamplerState const& state)
		: state_(state)
	{
		// TODO implement me
		assert(false);
	}

	void SamplerStateObject::Bind()
	{
		// TODO implement me
		assert(false);
	}

}
