#include "XREX.hpp"

#include "Texture.hpp"
#include "TextureImage.hpp"
#include "GraphicsBuffer.hpp"

#include "GLUtil.hpp"

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

	XREX::TextureImageSP Texture::GetImage(uint32 level)
	{
		assert(level < mipmapCount_);
		return MakeSP<TextureImage>(shared_from_this(), level);
	}

	XREX::TextureImageSP Texture::GetImage_TEMP(uint32 level, TexelFormat format)
	{
		assert(level < mipmapCount_);
		return MakeSP<TextureImage>(shared_from_this(), level, format);
	}

	void Texture::RecreateMipmap()
	{
		Bind(lastBindingIndex_);
		gl::GenerateMipmap(glBindingTarget_);
	}






	template <>
	void ConcreteTexture<1>::DoFillTexture(DataDescription<1> const& description, uint32 mipmapLevel, void const* data)
	{
		GLTextureFormat const& glFormat = GLTextureFormatFromTexelFormat(description.GetFormat());
// 		gl::TexStorage1D(glBindingTarget_, mipmapLevel, glFormat.glInternalFormat, description.GetSizes()[0]);
// 		gl::TexSubImage1D(glBindingTarget_, mipmapLevel, 0, description.GetSizes()[0], glFormat.glSourceFormat, glFormat.glTextureElementType, data);
		gl::TexImage1D(glBindingTarget_, mipmapLevel, glFormat.glInternalFormat, description.GetSizes()[0], 0, glFormat.glSourceFormat, glFormat.glTextureElementType, data);
	}
	template <>
	void ConcreteTexture<2>::DoFillTexture(DataDescription<2> const& description, uint32 mipmapLevel, void const* data)
	{
		GLTextureFormat const& glFormat = GLTextureFormatFromTexelFormat(description.GetFormat());
// 		gl::TexStorage2D(glBindingTarget_, mipmapLevel, glFormat.glInternalFormat, description.GetSizes()[0], description.GetSizes()[1]);
// 		gl::TexSubImage2D(glBindingTarget_, mipmapLevel, 0, 0, description.GetSizes()[0], description.GetSizes()[1], glFormat.glSourceFormat, glFormat.glTextureElementType, data);
		gl::TexImage2D(glBindingTarget_, mipmapLevel, glFormat.glInternalFormat, description.GetSizes()[0], description.GetSizes()[1], 0, glFormat.glSourceFormat, glFormat.glTextureElementType, data);
	}
	template <>
	void ConcreteTexture<3>::DoFillTexture(DataDescription<3> const& description, uint32 mipmapLevel, void const* data)
	{
		GLTextureFormat const& glFormat = GLTextureFormatFromTexelFormat(description.GetFormat());
// 		gl::TexStorage3D(glBindingTarget_, mipmapLevel, glFormat.glInternalFormat, description.GetSizes()[0], description.GetSizes()[1], description.GetSizes()[2]);
// 		gl::TexSubImage3D(glBindingTarget_, mipmapLevel, 0, 0, 0, description.GetSizes()[0], description.GetSizes()[1], description.GetSizes()[2], glFormat.glSourceFormat, glFormat.glTextureElementType, data);
		gl::TexImage3D(glBindingTarget_, mipmapLevel, glFormat.glInternalFormat, description.GetSizes()[0], description.GetSizes()[1], description.GetSizes()[2], 0, glFormat.glSourceFormat, glFormat.glTextureElementType, data);
	}


	template <uint32 Dimension>
	XREX::ConcreteTexture<Dimension>::ConcreteTexture(DataDescription<Dimension> const& description, bool generateMipmap)
		: Texture(TextureDimensionToTextureType<Dimension>::TextureType), description_(description)
	{
		Bind(0);
		DoFillTexture(description, 0, nullptr);
		mipmapCount_ = 1;

		if (generateMipmap)
		{
			std::array<uint32, Dimension> sizes = description.GetSizes();
			mipmapCount_ = 1;
			while (*std::max_element(sizes.begin(), sizes.end()) > 1)
			{
				for (uint32 i = 0; i < Dimension; ++i)
				{
					sizes[i] = std::max(sizes[i] / 2, 1u);
				}
				++mipmapCount_;
			}
			RecreateMipmap();
		}
	}

	template <uint32 Dimension>
	ConcreteTexture<Dimension>::ConcreteTexture(DataDescription<Dimension> const& description, std::vector<void const*> const& data, bool generateMipmap)
		: Texture(TextureDimensionToTextureType<Dimension>::TextureType), description_(description)
	{
		assert(data.size() > 0);
		Bind(0);

		if (!generateMipmap)
		{
			DataDescription<Dimension> descriptionOfALevel = description;
			std::array<uint32, Dimension> sizes = description.GetSizes();
			for (uint32 mipmapLevel = 0; mipmapLevel < data.size(); ++mipmapLevel)
			{
				void const* dataOfALevel = data[mipmapLevel];
				DoFillTexture(descriptionOfALevel, mipmapLevel, dataOfALevel);
				for (uint32 i = 0; i < Dimension; ++i)
				{
					sizes[i] = std::max(sizes[i] / 2, 1u);
				}
				descriptionOfALevel = DataDescription<Dimension>(description.GetFormat(), sizes);
			}
		}
		else
		{
			DoFillTexture(description, 0, data[0]);
		}

		if (generateMipmap)
		{
			std::array<uint32, Dimension> sizes = description.GetSizes();
			mipmapCount_ = 1;
			while (*std::max_element(sizes.begin(), sizes.end()) > 1)
			{
				for (uint32 i = 0; i < Dimension; ++i)
				{
					sizes[i] = std::max(sizes[i] / 2, 1u);
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

	template <uint32 Dimension>
	ConcreteTexture<Dimension>::~ConcreteTexture()
	{
	}



	// instantiate 1, 2, 3 Dimensional Texture specialization
	template class XREX_API ConcreteTexture<1>;
	template class XREX_API ConcreteTexture<2>;
	template class XREX_API ConcreteTexture<3>;



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

}
