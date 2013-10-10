#include "XREX.hpp"

#include "TextureImage.hpp"

#include "Rendering/Texture.hpp"

#include "Rendering/GL/GLUtil.hpp"

#include <CoreGL.hpp>

namespace XREX
{

	TextureImage::TextureImage(TextureSP const& texture, uint32 level)
		: texture_(texture), level_(level), lastBindingIndex_(0)
	{
	}

	TextureImage::~TextureImage()
	{
	}

	XREX::TexelFormat TextureImage::GetFormat() const
	{
		return texture_->GetFormat();
	}

	void TextureImage::Bind(uint32 index, TexelFormat format, AccessType accessType)
	{
		lastBindingIndex_ = index;
		gl::BindImageTexture(lastBindingIndex_, texture_->GetID(), level_, true, 0, GLAccessTypeFromAccessType(accessType), GLTextureFormatFromTexelFormat(format).glInternalFormat);
	}

	void TextureImage::Unbind()
	{
		gl::BindImageTexture(lastBindingIndex_, 0, level_, true, 0, gl::GL_READ_WRITE, gl::GL_RGBA32F);
	}

}
