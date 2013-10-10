#pragma once

#include "Declare.hpp"

namespace XREX
{
	class XREX_API TextureImage
		: Noncopyable
	{
	public:

		TextureImage(TextureSP const& texture, uint32 level);
		~TextureImage();

		TextureSP const& GetTexture() const
		{
			return texture_;
		}

		TexelFormat GetFormat() const;

		void Bind(uint32 index, TexelFormat format, AccessType accessType);
		void Unbind();

	private:
		TextureSP texture_;
		uint32 level_;

		uint32 lastBindingIndex_;

	};
}


