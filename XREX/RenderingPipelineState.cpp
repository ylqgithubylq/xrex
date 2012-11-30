#include "XREX.hpp"

#include "RenderingPipelineState.hpp"
#include "GLUtil.hpp"

#include <CoreGL.hpp>

namespace XREX
{
	namespace
	{

	}

	RasterizerState::RasterizerState()
		: polygonMode(PolygonMode::Fill), cullMode(CullMode::Back), frontFaceCCW(true), multisampleEnable(false)
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


}
