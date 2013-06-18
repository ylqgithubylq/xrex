#pragma once

#include "Declare.hpp"

namespace XREX
{
	class XREX_API TextureImage
		: Noncopyable
	{
	public:
		// TODO temp
		TextureImage(TextureSP const& texture, uint32 level, TexelFormat bindingFormatTEMP);

		TextureImage(TextureSP const& texture, uint32 level);
		~TextureImage();

		TextureSP const& GetTexture() const
		{
			return texture_;
		}

		TexelFormat GetBindingFormat_TEMP() const
		{
			return bindingFormat_TEMP_;
		}
		TexelFormat GetFormat() const;

		void Bind(uint32 index, TexelFormat format);
		void Unbind();

	private:
		TextureSP texture_;
		uint32 level_;

		uint32 lastBindingIndex_;

		TexelFormat bindingFormat_TEMP_; // TODO temp
	};
}


