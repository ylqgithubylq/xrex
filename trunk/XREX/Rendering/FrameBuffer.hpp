#pragma once

#include "Declare.hpp"

#include "Rendering/GraphicsType.hpp"

namespace XREX
{

	class XREX_API FrameBufferLayoutDescription
	{
	public:
		enum class DepthStencilCombinationState
		{
			None,
			DepthOnly,
			StencilOnly,
			Separate,
			Combined,
		};

		enum class SizeMode
		{
			Fixed,
			Sceen,
			HalfSceen,
		};

		class XREX_API ChannelDescription
		{
		public:
			ChannelDescription(std::string channel, TexelFormat format)
				: channel_(std::move(channel)), format_(format)
			{
			}

			std::string const& GetChannel() const
			{
				return channel_;
			}
			TexelFormat GetFormat() const
			{
				return format_;
			}

		private:
			std::string channel_;
			TexelFormat format_;
		};


	public:
		FrameBufferLayoutDescription(std::string name)
			: name_(std::move(name)), depth_(TexelFormat::TexelFormatCount), stencil_(TexelFormat::TexelFormatCount),
			combined_(DepthStencilCombinationState::None), size_(0, 0), sizeMode_(SizeMode::Sceen)
		{
		}

		void AddChannel(ChannelDescription const& channelInformation);

		uint32 GetChannelCount() const
		{
			return framebufferChannels_.size();
		}
		std::vector<ChannelDescription const> const& GetAllChannels() const
		{
			return framebufferChannels_;
		}

		void SetDepth(TexelFormat format);
		void SetStencil(TexelFormat format);
		void SetDepthStencil(TexelFormat format);

		bool IsDepthEnabled() const
		{
			return combined_ != DepthStencilCombinationState::None && combined_ != DepthStencilCombinationState::StencilOnly;
		}
		bool IsStencilEnabled() const
		{
			return combined_ != DepthStencilCombinationState::None && combined_ != DepthStencilCombinationState::DepthOnly;
		}
		DepthStencilCombinationState GetDepthStencilCombinationState() const
		{
			return combined_;
		}
		TexelFormat GetDepthFormat() const
		{
			return depth_;
		}
		TexelFormat GetStencilFormat() const
		{
			return stencil_;
		}
		TexelFormat GetDepthStencilFormat() const
		{
			assert(combined_ == DepthStencilCombinationState::Combined);
			return depth_;
		}

		void SetSizeMode(SizeMode sizeMode)
		{
			sizeMode_ = sizeMode;
		}
		void SetSize(Size<uint32, 2> const& size)
		{
			size_ = size;
		}
		SizeMode GetSizeMode() const
		{
			return sizeMode_;
		}
		Size<uint32, 2> const& GetSize() const
		{
			return size_;
		}

	private:
		std::string name_;
		std::vector<ChannelDescription const> framebufferChannels_;
		TexelFormat depth_;
		TexelFormat stencil_;
		DepthStencilCombinationState combined_;
		Size<uint32, 2> size_;
		SizeMode sizeMode_;
	};



	class XREX_API FrameBuffer
		: Noncopyable
	{
	public:

		class XREX_API DepthStencilBinding
		{
		public:
			DepthStencilBinding()
				: combined_(FrameBufferLayoutDescription::DepthStencilCombinationState::None)
			{
			}
			DepthStencilBinding(Texture2DImageSP depth, Texture2DImageSP stencil_);
			explicit DepthStencilBinding(Texture2DImageSP const& depthStencil_);

			Texture2DImageSP const& GetDepth() const
			{
				return depth_;
			}
			Texture2DImageSP const& GetStencil() const
			{
				return stencil_;
			}
			Texture2DImageSP const& GetDepthStencil() const
			{
				assert(combined_ == FrameBufferLayoutDescription::DepthStencilCombinationState::Combined);
				return depth_;
			}
			FrameBufferLayoutDescription::DepthStencilCombinationState GetDepthStencilCombinatationState() const
			{
				return combined_;
			}
		private:
			Texture2DImageSP depth_;
			Texture2DImageSP stencil_;
			FrameBufferLayoutDescription::DepthStencilCombinationState combined_;
		};
	protected:
		FrameBuffer(FrameBufferLayoutDescriptionSP description); // for default frame buffer use only.
	public:
		FrameBuffer(FrameBufferLayoutDescriptionSP description, 
			std::unordered_map<std::string, Texture2DImageSP const>&& channelTextures, DepthStencilBinding const& depthStencil);
		virtual ~FrameBuffer();

		FrameBufferLayoutDescriptionSP const& GetLayoutDescription() const
		{
			return description_;
		}

		/*
		 *	@return: nullptr if not found.
		 */
		Texture2DImageSP GetColorAttachment(std::string const& channel);
		std::unordered_map<std::string, Texture2DImageSP const> const& GetAllColorAttachment() const
		{
			return colorTextures_;
		}
		Texture2DImageSP GetDepthAttachment()
		{
			return depthStencil_.GetDepth();
		}
		Texture2DImageSP GetStencilAttachment()
		{
			return depthStencil_.GetStencil();
		}
		Texture2DImageSP GetDepthStencilAttachment()
		{
			return depthStencil_.GetDepthStencil();
		}

		virtual void BindWrite();
		virtual void BindRead();

		enum class ClearMask
		{
			Color = 1 << 0,
			Depth = 1 << 1,
			Stencil = 1 << 2,
			ColorAndDepth = Color | Depth,
			DepthAndStencil = Depth | Stencil,
			All = Color | Depth | Stencil,
		};
		void Clear(ClearMask clearMask, Color const& clearColor, float clearDepth, uint16 clearStencil);

	private:
		void TextureCheck();

	private:
		FrameBufferLayoutDescriptionSP description_;
		std::unordered_map<std::string, Texture2DImageSP const> colorTextures_;
		DepthStencilBinding depthStencil_;

		uint32 glFrameBufferID_;
	};


}
