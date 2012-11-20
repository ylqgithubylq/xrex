#include "XREX.hpp"

#include "Texture.hpp"

#include <CoreGL.hpp>

#include <algorithm>
#include <array>

using std::vector;

namespace XREX
{

	namespace
	{
		uint32 GLBindingTargetFromTextureType(Texture::TextureType type)
		{
			static std::array<uint32, static_cast<uint32>(Texture::TextureType::TextureTypeCount)> const mapping = [] ()
			{
				std::remove_const<decltype(mapping)>::type temp;
				temp[static_cast<uint32>(Texture::TextureType::Texture1D)] = gl::GL_TEXTURE_1D;
				temp[static_cast<uint32>(Texture::TextureType::Texture2D)] = gl::GL_TEXTURE_2D;
				temp[static_cast<uint32>(Texture::TextureType::Texture3D)] = gl::GL_TEXTURE_3D;
				return temp;
			} ();
			return mapping[static_cast<uint32>(type)];
		}
	}


	struct Texture::GLTextureFormat
	{
		uint32 glInternalFormat;
		uint32 glSourceFormat;
		uint32 glTextureElementType;
		/*
		 *	For std::array initialization, do not use this constructor.
		 */
		GLTextureFormat()
			: glInternalFormat(0), glSourceFormat(0), glTextureElementType(0)
		{
		}
		GLTextureFormat(uint32 internalFormat, uint32 sourceFormat, uint32 textureElementType)
			: glInternalFormat(internalFormat), glSourceFormat(sourceFormat), glTextureElementType(textureElementType)
		{
		}
	};

	Texture::GLTextureFormat const& Texture::GLTextureFormatFromTexelFormat(TexelFormat format)
	{
		switch (format)
		{
		case TexelFormat::RGB8:
			{
				// TODO why BGR?
				//static GLTextureFormat const Format(gl::GL_RGBA8, gl::GL_RGB, gl::GL_UNSIGNED_BYTE);
				static GLTextureFormat const Format(gl::GL_RGBA8, gl::GL_BGR, gl::GL_UNSIGNED_BYTE);
				return Format;
			}
		case TexelFormat::BGR8:
			{
				static GLTextureFormat const Format(gl::GL_RGBA8, gl::GL_BGR, gl::GL_UNSIGNED_BYTE);
				return Format;
			}
		case TexelFormat::RGBA8:
			{
				// TODO why BGRA?
				//static GLTextureFormat const Format(gl::GL_RGBA8, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE);
				static GLTextureFormat const Format(gl::GL_RGBA8, gl::GL_BGRA, gl::GL_UNSIGNED_BYTE);
				return Format;
			}
		case TexelFormat::BGRA8:
			{
				static GLTextureFormat const Format(gl::GL_RGBA8, gl::GL_BGRA, gl::GL_UNSIGNED_BYTE);
				return Format;
			}
		case TexelFormat::R8:
			{
				static GLTextureFormat const Format(gl::GL_R8, gl::GL_RED, gl::GL_UNSIGNED_BYTE);
				return Format;
			}
		default:
			{
				static GLTextureFormat const Format;
				assert(false);
				return Format;
			}
		}
	}



	Texture::Texture(TextureType type)
		: type_(type), glTextureID_(0), mipmapCount_(0)
	{
	}

	Texture::~Texture()
	{
		if (glTextureID_ != 0)
		{
			gl::DeleteTextures(1, &glTextureID_);
			glTextureID_ = 0;
		}
	}

	void Texture::BindTexture(uint32 textureChannel)
	{
		gl::ActiveTexture(gl::GL_TEXTURE0 + textureChannel);
		gl::BindTexture(glBindingTarget_, glTextureID_);
	}

	void Texture::UnbindTexture()
	{
		gl::BindTexture(glBindingTarget_, 0);
	}





	template <>
	void ConcreteTexture<1>::DoFillTexture(uint32 mipmapLevel, GLTextureFormat const& glFormat, DataDescription<1> const& description, void const* data)
	{
		gl::TexImage1D(glBindingTarget_, mipmapLevel, glFormat.glInternalFormat, description.GetSizes()[0], 0, glFormat.glSourceFormat, glFormat.glTextureElementType, data);
	}
	template <>
	void ConcreteTexture<2>::DoFillTexture(uint32 mipmapLevel, GLTextureFormat const& glFormat, DataDescription<2> const& description, void const* data)
	{
		gl::TexImage2D(glBindingTarget_, mipmapLevel, glFormat.glInternalFormat, description.GetSizes()[0], description.GetSizes()[1], 0, glFormat.glSourceFormat, glFormat.glTextureElementType, data);
	}
	template <>
	void ConcreteTexture<3>::DoFillTexture(uint32 mipmapLevel, GLTextureFormat const& glFormat, DataDescription<3> const& description, void const* data)
	{
		gl::TexImage3D(glBindingTarget_, mipmapLevel, glFormat.glInternalFormat, description.GetSizes()[0], description.GetSizes()[1], description.GetSizes()[2], 0, glFormat.glSourceFormat, glFormat.glTextureElementType, data);
	}



	template <uint32 Dimension>
	ConcreteTexture<Dimension>::~ConcreteTexture()
	{

	}

	template <uint32 Dimension>
	void ConcreteTexture<Dimension>::DoConstructTexture(std::vector<void const*>& rawData, DataDescription<Dimension> const& description, bool generateMipmap)
	{
		assert(rawData.size() > 0);
		glBindingTarget_ = GLBindingTargetFromTextureType(type_);
		gl::GenTextures(1, &glTextureID_);
		assert(glTextureID_ != 0);
		gl::BindTexture(glBindingTarget_, glTextureID_);

		GLTextureFormat const& glFormat = GLTextureFormatFromTexelFormat(description.GetFormat());
		if (!generateMipmap)
		{
			DataDescription<Dimension> descriptionOfALevel = description;
			std::array<uint32, Dimension> sizes = description.GetSizes();
			for (uint32 mipmapLevel = 0; mipmapLevel < rawData.size(); ++mipmapLevel)
			{
				void const* dataOfALevel = rawData[mipmapLevel];
				DoFillTexture(mipmapLevel, glFormat, descriptionOfALevel, dataOfALevel);
				for (uint32 i = 0; i < Dimension; ++i)
				{
					sizes[i] = std::max(sizes[i] / 2, 1u);
				}
				descriptionOfALevel = DataDescription<Dimension>(description.GetFormat(), sizes);
			}
		}
		else
		{
			DoFillTexture(0, glFormat, description, rawData[0]);
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
			gl::GenerateMipmap(glBindingTarget_);
		}
		else
		{
			mipmapCount_ = rawData.size();
		}
		gl::BindTexture(glBindingTarget_, 0);
	}


	// instantiate 1, 2, 3 Dimensional Texture specialization
	template class XREX_API ConcreteTexture<1>;
	template class XREX_API ConcreteTexture<2>;
	template class XREX_API ConcreteTexture<3>;

}
