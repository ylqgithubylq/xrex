#include "XREX.hpp"

#include "Sampler.hpp"

#include "Rendering/GL/GLUtil.hpp"

#include <CoreGL.hpp>

namespace XREX
{

	SamplerState::SamplerState()
		: borderColor(0, 0, 0, 1),
		addressingModeS(TextureAddressingMode::Repeat), addressingModeT(TextureAddressingMode::Repeat), addressingModeR(TextureAddressingMode::Repeat),
		minFilterMode(TextureFilterMode::Nearest), magFilterMode(TextureFilterMode::Nearest), maxAnisotropy(16),
		minLOD(0), maxLOD(std::numeric_limits<float>::max()), mipmapLODBias(0.f), compareEnable(false), compareFunction(CompareFunction::AlwaysFail)
	{
	}



	Sampler::Sampler(SamplerState const& state)
		: state_(state),
		glAddressingModeS_(GLTextureAddressingModeFromAddressingMode(state_.addressingModeS)),
		glAddressingModeT_(GLTextureAddressingModeFromAddressingMode(state_.addressingModeT)),
		glAddressingModeR_(GLTextureAddressingModeFromAddressingMode(state_.addressingModeR)),
		glCompareFunction_(GLCompareFunctionFromCompareFunction(state_.compareFunction))
	{
		assert(state_.magFilterMode == SamplerState::TextureFilterMode::Nearest
			|| state_.magFilterMode == SamplerState::TextureFilterMode::Linear
			|| state_.magFilterMode == SamplerState::TextureFilterMode::Anisotropic);

		if (state_.minFilterMode == SamplerState::TextureFilterMode::Anisotropic || state_.magFilterMode == SamplerState::TextureFilterMode::Anisotropic)
		{
			state_.minFilterMode = SamplerState::TextureFilterMode::Anisotropic;
			state_.magFilterMode = SamplerState::TextureFilterMode::Anisotropic;
			glMinFilter_ = GLFilterOperationFromTextureFilterOperation(SamplerState::TextureFilterMode::LinearMipmapLinear);
			glMagFilter_ = GLFilterOperationFromTextureFilterOperation(SamplerState::TextureFilterMode::Linear);
		}
		else
		{
			glMinFilter_ = GLFilterOperationFromTextureFilterOperation(state_.minFilterMode);
			glMagFilter_ = GLFilterOperationFromTextureFilterOperation(state_.magFilterMode);
		}
		
		gl::GenSamplers(1, &glSamplerID_);
		assert(glSamplerID_ != 0);

		gl::SamplerParameteri(glSamplerID_, gl::GL_TEXTURE_WRAP_S, glAddressingModeS_);
		gl::SamplerParameteri(glSamplerID_, gl::GL_TEXTURE_WRAP_T, glAddressingModeT_);
		gl::SamplerParameteri(glSamplerID_, gl::GL_TEXTURE_WRAP_R, glAddressingModeR_);
		gl::SamplerParameterfv(glSamplerID_, gl::GL_TEXTURE_BORDER_COLOR, state.borderColor.GetArray());

		gl::SamplerParameteri(glSamplerID_, gl::GL_TEXTURE_MIN_FILTER, glMinFilter_);
		gl::SamplerParameteri(glSamplerID_, gl::GL_TEXTURE_MAG_FILTER, glMagFilter_);
		if (state_.minFilterMode == SamplerState::TextureFilterMode::Anisotropic)
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

		if (state_.compareEnable)
		{
			gl::SamplerParameteri(glSamplerID_, gl::GL_TEXTURE_COMPARE_MODE, gl::GL_COMPARE_REF_TO_TEXTURE);
			gl::SamplerParameteri(glSamplerID_, gl::GL_TEXTURE_COMPARE_FUNC, glCompareFunction_);
		}
		else
		{
			gl::SamplerParameteri(glSamplerID_, gl::GL_TEXTURE_COMPARE_MODE, gl::GL_NONE);
		}

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

