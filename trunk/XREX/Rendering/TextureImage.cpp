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

	TexelFormat TextureImage::GetFormat() const
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


	template <uint32 Dimension>
	Size<uint32, Dimension> CalculateImageSize(Size<uint32, Dimension> const& originalSize, uint32 level)
	{
		std::array<uint32, Dimension> data;
		for (uint32 i = 0; i < Dimension; ++i)
		{
			data[i] = originalSize[i] >> level;
			assert(data[i] > 0);
		}
		
		return Size<uint32, Dimension>(data);
	}

	template <uint32 Dimension>
	DimensionalTextureImage<Dimension>::DimensionalTextureImage(TextureSP const& texture, uint32 level)
		: TextureImage(texture, level), size_(CalculateImageSize(CheckedSPCast<DimensionalTexture<Dimension>>(texture)->GetSize(), level))
	{
	}

	template class XREX_API DimensionalTextureImage<1>;
	template class XREX_API DimensionalTextureImage<2>;
	template class XREX_API DimensionalTextureImage<3>;


	Texture3DLayerImage::Texture3DLayerImage(TextureSP const& texture, uint32 layer, uint32 level)
		: Texture2DImage(texture, level), layer_(layer)
	{
	}



	TextureCubeImage::TextureCubeImage(TextureSP const& texture, CubeFace face, uint32 level)
		: Texture2DImage(texture, level), face_(face)
	{
	}




	TextureBufferImage::TextureBufferImage(TextureSP const& texture)
		: TextureImage(texture, 0)
	{
	}

	Size<uint32, 1> TextureBufferImage::GetSize() const
	{
		return CheckedSPCast<TextureBuffer>(GetTexture())->GetSize();
	}



}
