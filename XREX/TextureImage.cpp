#include "XREX.hpp"
#include "TextureImage.hpp"
#include "Texture.hpp"

#include "GLUtil.hpp"

#include <CoreGL.hpp>

namespace XREX
{

	TextureImage::TextureImage(TextureSP const& texture, uint32 level, TexelFormat bindingFormat)
		: texture_(texture), level_(level), lastBindingIndex_(0), bindingFormat_TEMP_(bindingFormat)
	{
	}

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

	void TextureImage::Bind(uint32 index, TexelFormat format)
	{
		lastBindingIndex_ = index;
//		texture_->Bind(lastBindingIndex_);
// 		gl::ActiveTexture(gl::GL_TEXTURE0 + lastBindingIndex_);
// 		gl::BindTexture(gl::GL_TEXTURE_2D, texture_->GetID());
		gl::BindImageTexture(lastBindingIndex_, texture_->GetID(), level_, false, 0, gl::GL_READ_ONLY, GLTextureFormatFromTexelFormat(format).glInternalFormat);
	}

	void TextureImage::Unbind()
	{
		gl::BindImageTexture(lastBindingIndex_, 0, level_, false, 0, gl::GL_READ_WRITE, gl::GL_RGBA32F);
	}

}
