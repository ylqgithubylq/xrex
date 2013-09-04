#pragma once

#include "Declare.hpp"

namespace XREX
{
	class XREX_API TextureImage
		: Noncopyable
	{
	public:
		// TODO temp
		TextureImage(TextureSP const& texture, uint32 level, TexelFormat bindingFormat_TEMP);

		TextureImage(TextureSP const& texture, uint32 level);
		~TextureImage();

		TextureSP const& GetTexture() const
		{
			return texture_;
		}

		TexelFormat GetBindingFormat_TEMP() const // shader should provide interface to get image format, not specified by user like this
		{
			return bindingFormat_TEMP_;
		}
		TexelFormat GetFormat() const;

		void Bind(uint32 index, TexelFormat format, AccessType accessType);
		void Unbind();

	private:
		TextureSP texture_;
		uint32 level_;

		uint32 lastBindingIndex_;

		TexelFormat bindingFormat_TEMP_; // TODO temp
	};
}


