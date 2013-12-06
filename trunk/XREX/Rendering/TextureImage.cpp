#include "XREX.hpp"

#include "TextureImage.hpp"

#include "Rendering/Texture.hpp"

#include "Rendering/GL/GLUtil.hpp"

#include <CoreGL.hpp>

namespace XREX
{

	TextureImage::TextureImage(ImageType type, TextureSP const& texture, uint32 level)
		: type_(type), texture_(texture), level_(level), lastBindingIndex_(0)
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

	template <uint32 N>
	struct ImageDimensionToImageType
	{
		static_assert(N >= 1 && N <= 3, "Image only have 1, 2, 3 dimensions.");
		static TextureImage::ImageType const ImageType = TextureImage::ImageType::ImageTypeCount;
	};
	template <>
	struct ImageDimensionToImageType<1>
	{
		static TextureImage::ImageType const TextureType = TextureImage::ImageType::Image1D;
	};
	template <>
	struct ImageDimensionToImageType<2>
	{
		static TextureImage::ImageType const TextureType = TextureImage::ImageType::Image2D;
	};
	template <>
	struct ImageDimensionToImageType<3>
	{
		static TextureImage::ImageType const TextureType = TextureImage::ImageType::Image3D;
	};

	template <uint32 Dimension>
	DimensionalTextureImage<Dimension>::DimensionalTextureImage(TextureSP const& texture, uint32 level)
		: TextureImage(ImageDimensionToImageType<Dimension>::TextureType, texture, level), size_(CalculateImageSize(CheckedSPCast<DimensionalTexture<Dimension>>(texture)->GetSize(), level))
	{
	}

	template class XREX_API DimensionalTextureImage<1>;
	template class XREX_API DimensionalTextureImage<2>;
	template class XREX_API DimensionalTextureImage<3>;


	Texture1DImage::Texture1DImage(Texture1DSP const& texture, uint32 level)
		: DimensionalTextureImage(texture, level)
	{
	}

	Texture2DImage::Texture2DImage(TextureSP const& texture, uint32 level)
		: DimensionalTextureImage(texture, level)
	{
	}
	Texture2DImage::Texture2DImage(Texture2DSP const& texture, uint32 level)
		: DimensionalTextureImage(texture, level)
	{
	}

	Texture3DImage::Texture3DImage(Texture3DSP const& texture, uint32 level)
		: DimensionalTextureImage(texture, level)
	{
	}

	Texture3DLayerImage::Texture3DLayerImage(Texture3DSP const& texture, uint32 layer, uint32 level)
		: Texture2DImage(texture, level), layer_(layer)
	{
	}



	TextureCubeFaceImage::TextureCubeFaceImage(TextureCubeSP const& texture, CubeFace face, uint32 level)
		: Texture2DImage(texture, level), face_(face)
	{
	}




	TextureBufferImage::TextureBufferImage(TextureSP const& texture)
		: TextureImage(ImageType::ImageBuffer, texture, 0)
	{
	}

	Size<uint32, 1> TextureBufferImage::GetSize() const
	{
		return CheckedSPCast<TextureBuffer>(GetTexture())->GetSize();
	}




}
