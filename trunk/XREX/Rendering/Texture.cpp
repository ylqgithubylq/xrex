#include "XREX.hpp"

#include "Texture.hpp"

#include "Rendering/TextureImage.hpp"
#include "Rendering/GraphicsBuffer.hpp"

#include "Rendering/GL/GLUtil.hpp"

#include <CoreGL.hpp>

#include <algorithm>
#include <array>

using std::vector;

namespace XREX
{

	Texture::Texture(TextureType type)
		: type_(type), glTextureID_(0), mipmapCount_(0), lastBindingIndex_(0)
	{
		glBindingTarget_ = GLTextureTypeFromTextureType(type_);
		gl::GenTextures(1, &glTextureID_);
		assert(glTextureID_ != 0);
	}

	Texture::~Texture()
	{
		if (glTextureID_ != 0)
		{
			gl::DeleteTextures(1, &glTextureID_);
			glTextureID_ = 0;
		}
	}

	void Texture::Bind(uint32 index)
	{
		lastBindingIndex_ = index;
		gl::ActiveTexture(gl::GL_TEXTURE0 + lastBindingIndex_);
		gl::BindTexture(glBindingTarget_, glTextureID_);
	}

	void Texture::Unbind()
	{
		gl::ActiveTexture(gl::GL_TEXTURE0 + lastBindingIndex_);
		gl::BindTexture(glBindingTarget_, 0);
	}

	void Texture::RecreateMipmap()
	{
		Bind(lastBindingIndex_);
		gl::GenerateMipmap(glBindingTarget_);
	}






	template <>
	void DimensionalTexture<1>::DoFillTexture(DataDescription<1> const& description, uint32 mipmapLevel, void const* data)
	{
		GLTextureFormat const& glFormat = GLTextureFormatFromTexelFormat(description.GetFormat());
// 		gl::TexStorage1D(glBindingTarget_, mipmapLevel, glFormat.glInternalFormat, description.GetSize()[0]);
// 		gl::TexSubImage1D(glBindingTarget_, mipmapLevel, 0, description.GetSize()[0], glFormat.glSourceFormat, glFormat.glTextureElementType, data);
		gl::TexImage1D(glBindingTarget_, mipmapLevel, glFormat.glInternalFormat, description.GetSize().X(), 0, glFormat.glSourceFormat, glFormat.glTextureElementType, data);
	}
	template <>
	void DimensionalTexture<2>::DoFillTexture(DataDescription<2> const& description, uint32 mipmapLevel, void const* data)
	{
		GLTextureFormat const& glFormat = GLTextureFormatFromTexelFormat(description.GetFormat());
// 		gl::TexStorage2D(glBindingTarget_, mipmapLevel, glFormat.glInternalFormat, description.GetSize()[0], description.GetSize()[1]);
// 		gl::TexSubImage2D(glBindingTarget_, mipmapLevel, 0, 0, description.GetSize()[0], description.GetSize()[1], glFormat.glSourceFormat, glFormat.glTextureElementType, data);
		gl::TexImage2D(glBindingTarget_, mipmapLevel, glFormat.glInternalFormat, description.GetSize().X(), description.GetSize().Y(), 0, glFormat.glSourceFormat, glFormat.glTextureElementType, data);
	}
	template <>
	void DimensionalTexture<3>::DoFillTexture(DataDescription<3> const& description, uint32 mipmapLevel, void const* data)
	{
		GLTextureFormat const& glFormat = GLTextureFormatFromTexelFormat(description.GetFormat());
// 		gl::TexStorage3D(glBindingTarget_, mipmapLevel, glFormat.glInternalFormat, description.GetSize()[0], description.GetSize()[1], description.GetSize()[2]);
// 		gl::TexSubImage3D(glBindingTarget_, mipmapLevel, 0, 0, 0, description.GetSize()[0], description.GetSize()[1], description.GetSize()[2], glFormat.glSourceFormat, glFormat.glTextureElementType, data);
		gl::TexImage3D(glBindingTarget_, mipmapLevel, glFormat.glInternalFormat, description.GetSize().X(), description.GetSize().Y(), description.GetSize().Z(), 0, glFormat.glSourceFormat, glFormat.glTextureElementType, data);
	}


	template <uint32 N>
	struct TextureDimensionToTextureType
	{
		static_assert(N >= 1 && N <= 3, "Texture only have 1, 2, 3 dimensions.");
		static Texture::TextureType const TextureType = Texture::TextureType::TextureCube;
	};
	template <>
	struct TextureDimensionToTextureType<1>
	{
		static Texture::TextureType const TextureType = Texture::TextureType::Texture1D;
	};
	template <>
	struct TextureDimensionToTextureType<2>
	{
		static Texture::TextureType const TextureType = Texture::TextureType::Texture2D;
	};
	template <>
	struct TextureDimensionToTextureType<3>
	{
		static Texture::TextureType const TextureType = Texture::TextureType::Texture3D;
	};

	template <uint32 Dimension>
	DimensionalTexture<Dimension>::DimensionalTexture(DataDescription<Dimension> const& description, bool generateMipmap)
		: Texture(TextureDimensionToTextureType<Dimension>::TextureType), description_(description)
	{
		Bind(0);
		DoFillTexture(description, 0, nullptr);
		mipmapCount_ = 1;

		if (generateMipmap)
		{
			Size<uint32, Dimension> size = description.GetSize();
			mipmapCount_ = 1;
			while (*std::max_element(size.data.begin(), size.data.end()) > 1)
			{
				for (uint32 i = 0; i < Dimension; ++i)
				{
					size[i] = std::max(size[i] / 2, 1u);
				}
				++mipmapCount_;
			}
			RecreateMipmap();
		}
	}

	template <uint32 Dimension>
	DimensionalTexture<Dimension>::DimensionalTexture(DataDescription<Dimension> const& description, std::vector<void const*> const& data, bool generateMipmap)
		: Texture(TextureDimensionToTextureType<Dimension>::TextureType), description_(description)
	{
		assert(data.size() > 0);
		Bind(0);

		if (!generateMipmap)
		{
			DataDescription<Dimension> descriptionOfALevel = description;
			Size<uint32, Dimension> size = description.GetSize();
			for (uint32 mipmapLevel = 0; mipmapLevel < data.size(); ++mipmapLevel)
			{
				void const* dataOfALevel = data[mipmapLevel];
				DoFillTexture(descriptionOfALevel, mipmapLevel, dataOfALevel);
				for (uint32 i = 0; i < Dimension; ++i)
				{
					size[i] = std::max(size[i] / 2, 1u);
				}
				descriptionOfALevel = DataDescription<Dimension>(description.GetFormat(), size);
			}
		}
		else
		{
			DoFillTexture(description, 0, data[0]);
		}

		if (generateMipmap)
		{
			Size<uint32, Dimension> size = description.GetSize();
			mipmapCount_ = 1;
			while (*std::max_element(size.data.begin(), size.data.end()) > 1)
			{
				for (uint32 i = 0; i < Dimension; ++i)
				{
					size[i] = std::max(size[i] / 2, 1u);
				}
				++mipmapCount_;
			}
			RecreateMipmap();
		}
		else
		{
			mipmapCount_ = data.size();
		}
	}


	// instantiate 1, 2, 3 Dimensional Texture specialization
	template class XREX_API DimensionalTexture<1>;
	template class XREX_API DimensionalTexture<2>;
	template class XREX_API DimensionalTexture<3>;

	Texture1D::Texture1D(DataDescription<1> const& description, bool generateMipmap)
		: DimensionalTexture(description, generateMipmap)
	{
	}

	Texture1D::Texture1D(DataDescription<1> const& description, std::vector<void const*> const& data, bool generateMipmap)
		: DimensionalTexture(description, data, generateMipmap)
	{
	}

	XREX::Texture1DImageSP Texture1D::GetImage(uint32 level)
	{
		assert(level < mipmapCount_);
		return MakeSP<Texture1DImage>(CheckedSPCast<Texture1D>(shared_from_this()), level);
	}

	Texture2D::Texture2D(DataDescription<2> const& description, bool generateMipmap)
		: DimensionalTexture(description, generateMipmap)
	{
	}

	Texture2D::Texture2D(DataDescription<2> const& description, std::vector<void const*> const& data, bool generateMipmap)
		: DimensionalTexture(description, data, generateMipmap)
	{
	}

	XREX::Texture2DImageSP Texture2D::GetImage(uint32 level)
	{
		assert(level < mipmapCount_);
		return MakeSP<Texture2DImage>(CheckedSPCast<Texture2D>(shared_from_this()), level);

	}

	Texture3D::Texture3D(DataDescription<3> const& description, bool generateMipmap)
		: DimensionalTexture(description, generateMipmap)
	{
	}

	Texture3D::Texture3D(DataDescription<3> const& description, std::vector<void const*> const& data, bool generateMipmap)
		: DimensionalTexture(description, data, generateMipmap)
	{
	}

	XREX::Texture3DImageSP Texture3D::GetImage(uint32 level)
	{
		assert(level < mipmapCount_);
		return MakeSP<Texture3DImage>(CheckedSPCast<Texture3D>(shared_from_this()), level);
	}

	Texture2DImageSP Texture3D::GetLayerImage(uint32 layer, uint32 level)
	{
		assert(layer < GetDescription().GetSize().Z());
		assert(level < GetMipmapCount());
		return MakeSP<Texture3DLayerImage>(CheckedSPCast<Texture3D>(shared_from_this()), layer, level);
	}




	Texture2DImageSP TextureCube::GetFaceImage(CubeFace face, uint32 level)
	{
		assert(level < mipmapCount_);
		return MakeSP<TextureCubeFaceImage>(CheckedSPCast<TextureCube>(shared_from_this()), face, level);
	}




	TextureBuffer::TextureBuffer(GraphicsBufferSP const& buffer, TexelFormat format)
		: Texture(TextureType::TextureBuffer), buffer_(buffer), format_(format)
	{
		assert(buffer_ != nullptr);
		mipmapCount_ = 1;
		Bind(0);
		gl::TexBuffer(gl::GL_TEXTURE_BUFFER, GLTextureFormatFromTexelFormat(format_).glInternalFormat, buffer_->GetID());
	}

	TextureBuffer::~TextureBuffer()
	{
	}

	Size<uint32, 1> TextureBuffer::GetSize() const
	{
		return Size<uint32, 1>(buffer_->GetSize() / GetTexelSizeInBytes(format_));
	}
	
	TextureBufferImageSP TextureBuffer::GetImage()
	{
		return MakeSP<TextureBufferImage>(CheckedSPCast<TextureBuffer>(shared_from_this()));
	}


}
