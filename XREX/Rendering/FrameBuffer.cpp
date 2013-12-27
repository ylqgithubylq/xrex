#include "XREX.hpp"

#include "FrameBuffer.hpp"
#include "Rendering/Texture.hpp"
#include "Rendering/TextureImage.hpp"
#include "Rendering/RenderingPipelineState.hpp"

#include <CoreGL.hpp>

namespace XREX
{

	void FrameBufferLayoutDescription::AddChannel(ChannelDescription const& channelDescription)
	{
#ifdef XREX_DEBUG
		auto found = std::find_if(framebufferChannels_.begin(), framebufferChannels_.end(), [&channelDescription] (ChannelDescription const& information)
		{
			return information.GetChannel() == channelDescription.GetChannel();
		});
		assert(found == framebufferChannels_.end());
#endif // XREX_DEBUG

		framebufferChannels_.push_back(channelDescription);
	}

	void FrameBufferLayoutDescription::SetDepth(TexelFormat format)
	{
		assert(combined_ != DepthStencilCombinationState::Combined);
		depth_ = format;
		combined_ = (combined_ == DepthStencilCombinationState::None || combined_ == DepthStencilCombinationState::DepthOnly)
			? DepthStencilCombinationState::DepthOnly : DepthStencilCombinationState::Separate;
	}

	void FrameBufferLayoutDescription::SetStencil(TexelFormat format)
	{
		assert(combined_ != DepthStencilCombinationState::Combined);
		stencil_ = format;
		combined_ = (combined_ == DepthStencilCombinationState::None || combined_ == DepthStencilCombinationState::StencilOnly)
			? DepthStencilCombinationState::StencilOnly : DepthStencilCombinationState::Separate;
	}

	void FrameBufferLayoutDescription::SetDepthStencil(TexelFormat format)
	{
		assert(combined_ != DepthStencilCombinationState::DepthOnly && combined_ != DepthStencilCombinationState::StencilOnly);
		depth_ = format;
		stencil_ = format;
		combined_ = DepthStencilCombinationState::Combined;
	}



	FrameBuffer::DepthStencilBinding::DepthStencilBinding(Texture2DImageSP depth, Texture2DImageSP stencil)
		: depth_(std::move(depth)), stencil_(std::move(stencil))
	{
		if (depth_ != nullptr && stencil_ != nullptr)
		{
			combined_ = FrameBufferLayoutDescription::DepthStencilCombinationState::Separate;
		}
		else if (depth_ != nullptr)
		{
			combined_ = FrameBufferLayoutDescription::DepthStencilCombinationState::DepthOnly;
		}
		else if (stencil_ != nullptr)
		{
			combined_ = FrameBufferLayoutDescription::DepthStencilCombinationState::StencilOnly;
		}
		else
		{
			combined_ = FrameBufferLayoutDescription::DepthStencilCombinationState::None;
		}
	}

	FrameBuffer::DepthStencilBinding::DepthStencilBinding(Texture2DImageSP const& depthStencil_)
		: depth_(depthStencil_), stencil_(depthStencil_), combined_(FrameBufferLayoutDescription::DepthStencilCombinationState::Combined)
	{
		assert(depthStencil_ != nullptr);
	}



	FrameBuffer::FrameBuffer(FrameBufferLayoutDescriptionSP description)
		: description_(std::move(description)), glFrameBufferID_(0)
	{
	}

	FrameBuffer::FrameBuffer(FrameBufferLayoutDescriptionSP description,
		std::unordered_map<std::string, Texture2DImageSP const>&& colorTextures, DepthStencilBinding const& depthStencil)
		: description_(std::move(description)), colorTextures_(std::move(colorTextures)), depthStencil_(depthStencil)
	{
#ifdef XREX_DEBUG
		assert(description_->GetDepthStencilCombinationState() == depthStencil_.GetDepthStencilCombinatationState());
		TextureCheck();
#endif // XREX_DEBUG

		gl::GenFramebuffers(1, &glFrameBufferID_);
		assert(glFrameBufferID_ != 0);

		BindWrite();

		if (description_->GetAllChannels().empty() && !description_->IsDepthEnabled() && !description_->IsStencilEnabled())
		{ // no texture attached
			gl::FramebufferParameteri(gl::GL_DRAW_FRAMEBUFFER, gl::GL_FRAMEBUFFER_DEFAULT_WIDTH, description_->GetSize().X());
			gl::FramebufferParameteri(gl::GL_DRAW_FRAMEBUFFER, gl::GL_FRAMEBUFFER_DEFAULT_HEIGHT, description_->GetSize().Y());
		}

		std::vector<FrameBufferLayoutDescription::ChannelDescription const> const& channels = description_->GetAllChannels();
		for (uint32 i = 0; i < channels.size(); ++i)
		{
			FrameBufferLayoutDescription::ChannelDescription const& channel = channels[i];
			auto found = colorTextures_.find(channel.GetChannel());
			assert(found != colorTextures_.end());
			Texture2DImageSP textureImage = found->second;
			gl::FramebufferTexture2D(gl::GL_DRAW_FRAMEBUFFER, gl::GL_COLOR_ATTACHMENT0 + i, gl::GL_TEXTURE_2D,
				textureImage->GetTexture()->GetID(), textureImage->GetLevel());
		}
		if (description_->IsDepthEnabled() && description_->IsStencilEnabled()
			&& depthStencil_.GetDepthStencilCombinatationState() == FrameBufferLayoutDescription::DepthStencilCombinationState::Combined)
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
		glDrawBuffers.resize(channels.size());
		for (uint32 i = 0, drawBuffer = gl::GL_COLOR_ATTACHMENT0; i < channels.size(); ++i, ++drawBuffer)
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

	XREX::Texture2DImageSP FrameBuffer::GetColorAttachment(std::string const& channel)
	{
		auto found = colorTextures_.find(channel);
		if (found == colorTextures_.end())
		{
			return nullptr;
		}
		else
		{
			return found->second;
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
			for (uint32 i = 0; i < description_->GetChannelCount(); ++i)
			{ // TODO different format need to call different ClearBuffer, e.g. gl::ClearBufferiv() gl::ClearBufferuiv()
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
		Size<uint32, 2> frameBufferSize = description_->GetSize();
		std::vector<FrameBufferLayoutDescription::ChannelDescription const> const& channels = description_->GetAllChannels();
		for (uint32 i = 0; i < channels.size(); ++i)
		{
			FrameBufferLayoutDescription::ChannelDescription const& channel = channels[i];
			auto found = colorTextures_.find(channel.GetChannel());
			assert(found != colorTextures_.end());
			Texture2DImageSP textureImage = found->second;
			assert(textureImage->GetType() == TextureImage::ImageType::Image2D);
			Size<uint32, 2> size = textureImage->GetSize();
			assert(frameBufferSize.X() == size.X() && frameBufferSize.Y() == size.Y());
			assert(channel.GetFormat() == textureImage->GetFormat());
		}

		if (description_->IsDepthEnabled())
		{
			assert(depthStencil_.GetDepth() != nullptr);
			auto texture2DDepth = CheckedSPCast<Texture2D>(depthStencil_.GetDepth()->GetTexture());
			Size<uint32, 2> depthSize = texture2DDepth->GetDescription().GetSize();
			assert(frameBufferSize.X() == depthSize.X() && frameBufferSize.Y() == depthSize.Y());
		}
		if (description_->IsStencilEnabled())
		{
			assert(depthStencil_.GetStencil() != nullptr);
			auto texture2DStencil = CheckedSPCast<Texture2D>(depthStencil_.GetStencil()->GetTexture());
			Size<uint32, 2> stencilSize = texture2DStencil->GetDescription().GetSize();
			assert(frameBufferSize.X() == stencilSize.X() && frameBufferSize.Y() == stencilSize.Y());
		}

	}

}