#include "XREX.hpp"

#include "FrameBuffer.hpp"
#include "Rendering/Texture.hpp"
#include "Rendering/TextureImage.hpp"
#include "Rendering/RenderingPipelineState.hpp"

#include <CoreGL.hpp>

namespace XREX
{

	void FrameBufferLayoutDescription::AddColorChannel(ColorChannelDescription const& description)
	{
#ifdef XREX_DEBUG
		auto foundByChannel = std::find_if(colorChannels_.begin(), colorChannels_.end(), [&description] (ColorChannelDescription const& value)
		{
			return description.GetChannel() == value.GetChannel();
		});
		assert(foundByChannel == colorChannels_.end());
#endif // XREX_DEBUG

		colorChannels_.push_back(description);
	}


	FrameBuffer::FrameBuffer(FrameBufferLayoutDescription const& description)
		: description_(description), glFrameBufferID_(0)
	{
	}

	FrameBuffer::FrameBuffer(FrameBufferLayoutDescription const& description,
		std::unordered_map<std::string, TextureImageSP>&& colorTextures, DepthStencilBinding const& depthStencil)
		: description_(description), colorTextures_(std::move(colorTextures)), depthStencil_(depthStencil)
	{
#ifdef XREX_DEBUG
		TextureCheck();
#endif // XREX_DEBUG

		gl::GenFramebuffers(1, &glFrameBufferID_);
		assert(glFrameBufferID_ != 0);

		BindWrite();

		if (description_.GetAllColorChannels().empty() && !description_.GetDepthEnabled() && !description_.GetStencilEnabled())
		{ // no texture attached
			gl::FramebufferParameteri(gl::GL_DRAW_FRAMEBUFFER, gl::GL_FRAMEBUFFER_DEFAULT_WIDTH, description_.GetSizes().X());
			gl::FramebufferParameteri(gl::GL_DRAW_FRAMEBUFFER, gl::GL_FRAMEBUFFER_DEFAULT_HEIGHT, description_.GetSizes().Y());
		}

		std::vector<FrameBufferLayoutDescription::ColorChannelDescription> const& colorChannels = description_.GetAllColorChannels();
		for (uint32 i = 0; i < colorChannels.size(); ++i)
		{
			FrameBufferLayoutDescription::ColorChannelDescription const& colorChannel = colorChannels[i];
			auto found = colorTextures_.find(colorChannel.GetChannel());
			assert(found != colorTextures_.end());
			TextureImageSP textureImage = found->second;
			gl::FramebufferTexture2D(gl::GL_DRAW_FRAMEBUFFER, gl::GL_COLOR_ATTACHMENT0 + i, gl::GL_TEXTURE_2D,
				textureImage->GetTexture()->GetID(), textureImage->GetLevel());
		}
		if (description_.GetDepthEnabled() && description_.GetStencilEnabled() && depthStencil_.GetDepthStencilCombinatationState() == DepthStencilCombinatationState::Combinated)
		{
			gl::FramebufferTexture2D(gl::GL_DRAW_FRAMEBUFFER, gl::GL_DEPTH_STENCIL_ATTACHMENT, gl::GL_TEXTURE_2D,
				depthStencil_.GetDepthStencil()->GetTexture()->GetID(), depthStencil_.GetDepthStencil()->GetLevel());
		}
		else
		{
			if (depthStencil_.GetDepth() != nullptr)
			{
				gl::FramebufferTexture2D(gl::GL_DRAW_FRAMEBUFFER, gl::GL_DEPTH_ATTACHMENT, gl::GL_TEXTURE_2D,
					depthStencil_.GetDepth()->GetTexture()->GetID(), depthStencil_.GetDepth()->GetLevel());
			}
			if (depthStencil_.GetStencil() != nullptr)
			{
				gl::FramebufferTexture2D(gl::GL_DRAW_FRAMEBUFFER, gl::GL_STENCIL_ATTACHMENT, gl::GL_TEXTURE_2D,
					depthStencil_.GetStencil()->GetTexture()->GetID(), depthStencil_.GetStencil()->GetLevel());
			}
		}

		std::vector<uint32> glDrawBuffers;
		glDrawBuffers.resize(colorChannels.size());
		for (uint32 i = 0, drawBuffer = gl::GL_COLOR_ATTACHMENT0; i < colorChannels.size(); ++i, ++drawBuffer)
		{
			glDrawBuffers[i] = drawBuffer;
		}
		gl::DrawBuffers(glDrawBuffers.size(), glDrawBuffers.data());

#ifdef XREX_DEBUG
		uint32 glCheckResult = gl::CheckFramebufferStatus(gl::GL_DRAW_FRAMEBUFFER);
		switch (glCheckResult)
		{
		case gl::GL_FRAMEBUFFER_COMPLETE:
			break;
		case gl::GL_FRAMEBUFFER_UNDEFINED:
			assert(false);
			break;
		case gl::GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			assert(false);
			break;
		case gl::GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			assert(false);
			break;
		case gl::GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			assert(false);
			break;
		case gl::GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			assert(false);
			break;
		case gl::GL_FRAMEBUFFER_UNSUPPORTED:
			assert(false);
			break;
		case gl::GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			assert(false);
			break;
		case gl::GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			assert(false);
			break;
		default:
			assert(false);
			break;
		}
#endif // XREX_DEBUG
		gl::BindFramebuffer(gl::GL_DRAW_FRAMEBUFFER, 0);
	}

	FrameBuffer::~FrameBuffer()
	{
		if (glFrameBufferID_ != 0)
		{
			gl::DeleteFramebuffers(1, &glFrameBufferID_);
			glFrameBufferID_ = 0;
		}
	}


	void FrameBuffer::BindWrite()
	{
		gl::BindFramebuffer(gl::GL_DRAW_FRAMEBUFFER, glFrameBufferID_);
	}

	void FrameBuffer::BindRead()
	{
		gl::BindFramebuffer(gl::GL_READ_FRAMEBUFFER, glFrameBufferID_);
	}


	void FrameBuffer::Clear(ClearMask clearMask, Color const& clearColor, float clearDepth, uint16 clearStencil)
	{
		BindWrite();
		if (static_cast<uint32>(clearMask) & static_cast<uint32>(ClearMask::Color))
		{
			gl::ColorMask(true, true, true, true);
			for (uint32 i = 0; i < description_.GetColorChannelCount(); ++i)
			{
				gl::ClearBufferfv(gl::GL_COLOR, i, clearColor.GetArray());
			}
		}
		if (static_cast<uint32>(clearMask) & static_cast<uint32>(ClearMask::Depth)
			&& static_cast<uint32>(clearMask) & static_cast<uint32>(ClearMask::Stencil))
		{
			gl::DepthMask(true);
			gl::StencilMask(true);
			int32 stencil = clearStencil;
			gl::ClearBufferfi(gl::GL_DEPTH_STENCIL, 0, clearDepth, stencil);
		}
		else
		{
			if (static_cast<uint32>(clearMask) & static_cast<uint32>(ClearMask::Depth))
			{
				gl::DepthMask(true);
				gl::ClearBufferfv(gl::GL_DEPTH, 0, &clearDepth);
			}
			if (static_cast<uint32>(clearMask) & static_cast<uint32>(ClearMask::Stencil))
			{
				gl::StencilMask(true);
				int32 stencil = clearStencil;
				gl::ClearBufferiv(gl::GL_STENCIL, 0, &stencil);
			}
		}

	}


	void FrameBuffer::TextureCheck()
	{
		Size<uint32, 2> frameBufferSize = description_.GetSizes();
		std::vector<FrameBufferLayoutDescription::ColorChannelDescription> const& colorChannels = description_.GetAllColorChannels();
		for (uint32 i = 0; i < colorChannels.size(); ++i)
		{
			FrameBufferLayoutDescription::ColorChannelDescription const& colorChannel = colorChannels[i];
			auto found = colorTextures_.find(colorChannel.GetChannel());
			assert(found != colorTextures_.end());
			TextureImageSP textureImage = found->second;
			TextureSP texture = textureImage->GetTexture();
			Texture::TextureType textureType = texture->GetType();
			switch (textureType)
			{
			case Texture::TextureType::Texture1D:
				assert(false); // not supported
				break;
			case Texture::TextureType::Texture2D:
				{
					auto texture2D = CheckedSPCast<Texture2D>(texture);
					Size<uint32, 2> sizes = texture2D->GetDescription().GetSizes();
					assert(frameBufferSize.X() == sizes.X() && frameBufferSize.Y() == sizes.Y());
					assert(colorChannel.GetFormat() == textureImage->GetFormat());
				}
				break;
			case Texture::TextureType::Texture3D:
				assert(false); // not supported
				break;
			case Texture::TextureType::TextureCube:
				assert(false);
				break;
			case Texture::TextureType::TextureBuffer:
				assert(false); // error
				break;
			case Texture::TextureType::TextureTypeCount:
				assert(false);
				break;
			default:
				assert(false);
				break;
			}
		}

		if (description_.GetDepthEnabled())
		{
			assert(depthStencil_.GetDepth() != nullptr);
			auto texture2DDepth = CheckedSPCast<Texture2D>(depthStencil_.GetDepth()->GetTexture());
			Size<uint32, 2> depthSizes = texture2DDepth->GetDescription().GetSizes();
			assert(frameBufferSize.X() == depthSizes.X() && frameBufferSize.Y() == depthSizes.Y());
		}
		if (description_.GetStencilEnabled())
		{
			assert(depthStencil_.GetStencil() != nullptr);
			auto texture2DStencil = CheckedSPCast<Texture2D>(depthStencil_.GetStencil()->GetTexture());
			Size<uint32, 2> stencilSizes = texture2DStencil->GetDescription().GetSizes();
			assert(frameBufferSize.X() == stencilSizes.X() && frameBufferSize.Y() == stencilSizes.Y());
		}

	}

}