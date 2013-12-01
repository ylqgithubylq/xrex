#pragma once

#include "Declare.hpp"


namespace XREX
{

	class XREX_API FrameBufferLayoutDescription
	{
	public:
		class XREX_API ColorChannelDescription
		{
		public:
			ColorChannelDescription()
				: format_(TexelFormat::TexelFormatCount)
			{
			}
			ColorChannelDescription(std::string const& channel, TexelFormat format)
				: channel_(channel), format_(format)
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
		FrameBufferLayoutDescription()
			: sizes_(0, 0), depthEnabled_(false), stencilEnabled_(false)
		{
		}
		explicit FrameBufferLayoutDescription(Size<uint32, 2> const& sizes, bool depthEnabled, bool stencilEnabled)
			: sizes_(sizes), depthEnabled_(depthEnabled), stencilEnabled_(stencilEnabled)
		{
		}

		Size<uint32, 2> const& GetSizes() const
		{
			return sizes_;
		}

		uint32 GetColorChannelCount() const
		{
			return colorChannels_.size();
		}
		std::vector<ColorChannelDescription> const& GetAllColorChannels() const
		{
			return colorChannels_;
		}
		void AddColorChannel(ColorChannelDescription const& description);

		bool GetDepthEnabled() const
		{
			return depthEnabled_;
		}
		bool GetStencilEnabled() const
		{
			return stencilEnabled_;
		}
	private:
		Size<uint32, 2> sizes_;
		std::vector<ColorChannelDescription> colorChannels_;
		bool depthEnabled_;
		bool stencilEnabled_;
	};



	class XREX_API FrameBuffer
		: Noncopyable
	{
	public:
		enum class DepthStencilCombinatationState
		{
			Saperate,
			Combinated,
			None,
		};
		class XREX_API DepthStencilBinding
		{
		public:
			DepthStencilBinding()
				: combinated_(DepthStencilCombinatationState::None)
			{
			}
			DepthStencilBinding(TextureImageSP const& depth, TextureImageSP const& stencil_)
				: depth_(depth), stencil_(stencil_), combinated_(DepthStencilCombinatationState::Saperate)
			{
			}
			explicit DepthStencilBinding(TextureImageSP const& depthStencil_)
				: depth_(depthStencil_), stencil_(depthStencil_), combinated_(DepthStencilCombinatationState::Combinated)
			{
			}
			TextureImageSP const& GetDepth() const
			{
				return depth_;
			}
			TextureImageSP const& GetStencil() const
			{
				return stencil_;
			}
			TextureImageSP const& GetDepthStencil() const
			{
				return depth_;
			}
			DepthStencilCombinatationState GetDepthStencilCombinatationState() const
			{
				return combinated_;
			}
		private:
			TextureImageSP depth_;
			TextureImageSP stencil_;
			DepthStencilCombinatationState combinated_;
		};
	protected:
		FrameBuffer(FrameBufferLayoutDescription const& description); // for default frame buffer use only.
	public:
		FrameBuffer(FrameBufferLayoutDescription const& description, 
			std::unordered_map<std::string, TextureImageSP>&& colorTextures, DepthStencilBinding const& depthStencil);
		virtual ~FrameBuffer();

		FrameBufferLayoutDescription const& GetLayoutDescription() const
		{
			return description_;
		}

		TextureImageSP GetColorAttachement(std::string const& channel);
		TextureImageSP GetDepthAttachement();
		TextureImageSP GetStencilAttachement();
		TextureImageSP GetDepthStencilAttachement();

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
		FrameBufferLayoutDescription description_;
		std::unordered_map<std::string, TextureImageSP> colorTextures_;
		DepthStencilBinding depthStencil_;

		uint32 glFrameBufferID_;
	};


}
