#include "XREX.hpp"

#include "Sampler.hpp"

#include "Rendering/GL/GLUtil.hpp"

#include <CoreGL.hpp>

namespace XREX
{

	SamplerState::SamplerState()
		: borderColor(0, 0, 0, 1),
		addressingModeS(TextureAddressingMode::Repeat), addressingModeT(TextureAddressingMode::Repeat), addressingModeR(TextureAddressingMode::Repeat),
		minFilterOperation(TextureFilterOperation::Nearest), magFilterOperation(TextureFilterOperation::Nearest), maxAnisotropy(16),
		minLOD(0), maxLOD(std::numeric_limits<float>::max()), mipmapLODBias(0.f), compareFunction(CompareFunction::AlwaysFail)
	{
	}



	Sampler::Sampler(SamplerState const& state)
		: state_(state),
		glAddressingModeS_(GLTextureAddressingModeFromAddressingMode(state_.addressingModeS)),
		glAddressingModeT_(GLTextureAddressingModeFromAddressingMode(state_.addressingModeT)),
		glAddressingModeR_(GLTextureAddressingModeFromAddressingMode(state_.addressingModeR)),
		glCompareFunction_(GLCompareFunctionFromCompareFunction(state_.compareFunction))
	{
		assert(state_.magFilterOperation == SamplerState::TextureFilterOperation::Nearest
			|| state_.magFilterOperation == SamplerState::TextureFilterOperation::Linear
			|| state_.magFilterOperation == SamplerState::TextureFilterOperation::Anisotropic);

		if (state_.minFilterOperation == SamplerState::TextureFilterOperation::Anisotropic || state_.magFilterOperation == SamplerState::TextureFilterOperation::Anisotropic)
		{
			state_.minFilterOperation = SamplerState::TextureFilterOperation::Anisotropic;
			state_.magFilterOperation = SamplerState::TextureFilterOperation::Anisotropic;
			glMinFilter_ = GLFilterOperationFromTextureFilterOperation(SamplerState::TextureFilterOperation::LinearMipmapLinear);
			glMagFilter_ = GLFilterOperationFromTextureFilterOperation(SamplerState::TextureFilterOperation::Linear);
		}
		else
		{
			glMinFilter_ = GLFilterOperationFromTextureFilterOperation(state_.minFilterOperation);
			glMagFilter_ = GLFilterOperationFromTextureFilterOperation(state_.magFilterOperation);
		}
		
		gl::GenSamplers(1, &glSamplerID_);
		assert(glSamplerID_ != 0);

		gl::SamplerParameteri(glSamplerID_, gl::GL_TEXTURE_WRAP_S, glAddressingModeS_);
		gl::SamplerParameteri(glSamplerID_, gl::GL_TEXTURE_WRAP_T, glAddressingModeT_);
		gl::SamplerParameteri(glSamplerID_, gl::GL_TEXTURE_WRAP_R, glAddressingModeR_);
		gl::SamplerParameterfv(glSamplerID_, gl::GL_TEXTURE_BORDER_COLOR, state.borderColor.GetArray());

		gl::SamplerParameteri(glSamplerID_, gl::GL_TEXTURE_MIN_FILTER, glMinFilter_);
		gl::SamplerParameteri(glSamplerID_, gl::GL_TEXTURE_MAG_FILTER, glMagFilter_);
		if (state_.minFilterOperation == SamplerState::TextureFilterOperation::Anisotropic)
		{
			gl::SamplerParameteri(glSamplerID_, gl::GL_TEXTURE_MAX_ANISOTROPY_EXT, state.maxAnisotropy);
		}
		else
		{
			gl::SamplerParameteri(glSamplerID_, gl::GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);
		}

		gl::SamplerParameterf(glSamplerID_, gl::GL_TEXTURE_MIN_LOD, state.minLOD);
		gl::SamplerParameterf(glSamplerID_, gl::GL_TEXTURE_MAX_LOD, state.maxLOD);

		gl::SamplerParameterf(glSamplerID_, gl::GL_TEXTURE_LOD_BIAS, state.mipmapLODBias);

		if (state_.compareFunction != RenderingPipelineState::CompareFunction::AlwaysFail) // TODO how does depth texture compare function work?
		{
			gl::SamplerParameteri(glSamplerID_, gl::GL_TEXTURE_COMPARE_MODE, gl::GL_COMPARE_REF_TO_TEXTURE);
		}
		else
		{
			gl::SamplerParameteri(glSamplerID_, gl::GL_TEXTURE_COMPARE_MODE, gl::GL_NONE);
		}
		gl::SamplerParameteri(glSamplerID_, gl::GL_TEXTURE_COMPARE_FUNC, glCompareFunction_);

	}


	Sampler::~Sampler()
	{
		if (glSamplerID_ != 0)
		{
			gl::DeleteSamplers(1, &glSamplerID_);
			glSamplerID_ = 0;
		}
	}

	void Sampler::Bind(uint32 textureChannel)
	{
		gl::BindSampler(textureChannel, glSamplerID_);
	}

}

