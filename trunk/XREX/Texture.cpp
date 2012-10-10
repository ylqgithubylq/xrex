#include "XREX.hpp"

#include "Texture.hpp"

#include <CoreGL.hpp>

#include <algorithm>

using std::vector;

uint32 Texture::GLBindingTargetFromTextureType(TextureType type)
{
	static vector<uint32> const mapping = [] ()
	{
		vector<uint32> temp(static_cast<uint32>(TextureType::TextureTypeCount));
		temp[static_cast<uint32>(TextureType::Texture1D)] = gl::GL_TEXTURE_1D;
		temp[static_cast<uint32>(TextureType::Texture2D)] = gl::GL_TEXTURE_2D;
		temp[static_cast<uint32>(TextureType::Texture3D)] = gl::GL_TEXTURE_3D;
		return temp;
	} ();
	return mapping[static_cast<uint32>(type)];
}

struct Texture::GLTextureFormat
{
	uint32 internalFormat;
	uint32 sourceFormat;
	uint32 textureElementType;
	/*
	 *	Do not use this constructor.
	 */
	GLTextureFormat()
	{
	}
	GLTextureFormat(uint32 theInternalFormat, uint32 theSourceFormat, uint32 theTextureElementType)
		: internalFormat(theInternalFormat), sourceFormat(theSourceFormat), textureElementType(theTextureElementType)
	{
	}
};

Texture::GLTextureFormat const& Texture::GLGLTextureFormatFromTexelFormat(TexelFormat format)
{
	static vector<GLTextureFormat> const mapping = [] ()
	{
		vector<GLTextureFormat> temp(static_cast<uint32>(TexelFormat::TexelFormatCount));
		temp[static_cast<uint32>(TexelFormat::RGB8)] = GLTextureFormat(gl::GL_RGBA8, gl::GL_RGB, gl::GL_UNSIGNED_BYTE);
		temp[static_cast<uint32>(TexelFormat::BGR8)] = GLTextureFormat(gl::GL_RGBA8, gl::GL_BGR, gl::GL_UNSIGNED_BYTE);
		temp[static_cast<uint32>(TexelFormat::RGBA8)] = GLTextureFormat(gl::GL_RGBA8, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE);
		temp[static_cast<uint32>(TexelFormat::BGRA8)] = GLTextureFormat(gl::GL_RGBA8, gl::GL_BGRA, gl::GL_UNSIGNED_BYTE);
		return temp;
	} ();
	return mapping[static_cast<uint32>(format)];
}



Texture::Texture(TextureType type)
	: type_(type), textureID_(0), mipmapCount_(0)
{
}

Texture::~Texture()
{
	if (textureID_ != 0)
	{
		gl::DeleteTextures(1, &textureID_);
		textureID_ = 0;
	}
}

void Texture::BindTexture(uint32 textureChannel)
{
	gl::ActiveTexture(gl::GL_TEXTURE0 + textureChannel);
	gl::BindTexture(bindingTarget_, textureID_);
}

void Texture::UnbindTexture()
{
	gl::BindTexture(bindingTarget_, 0);
}





template <>
void ConcreteTexture<1>::DoFillTexture(uint32 mipmapLevel, GLTextureFormat const& glFormat, DataDescription<1> const& description, void const* data)
{
	gl::TexImage1D(bindingTarget_, mipmapLevel, glFormat.internalFormat, description.GetSizes()[0], 0, glFormat.sourceFormat, glFormat.textureElementType, data);
}
template <>
void ConcreteTexture<2>::DoFillTexture(uint32 mipmapLevel, GLTextureFormat const& glFormat, DataDescription<2> const& description, void const* data)
{
	gl::TexImage2D(bindingTarget_, mipmapLevel, glFormat.internalFormat, description.GetSizes()[0], description.GetSizes()[1], 0, glFormat.sourceFormat, glFormat.textureElementType, data);
}
template <>
void ConcreteTexture<3>::DoFillTexture(uint32 mipmapLevel, GLTextureFormat const& glFormat, DataDescription<3> const& description, void const* data)
{
	gl::TexImage3D(bindingTarget_, mipmapLevel, glFormat.internalFormat, description.GetSizes()[0], description.GetSizes()[1], description.GetSizes()[2], 0, glFormat.sourceFormat, glFormat.textureElementType, data);
}



template <uint32 Dimension>
ConcreteTexture<Dimension>::~ConcreteTexture()
{

}

template <uint32 Dimension>
void ConcreteTexture<Dimension>::DoConstructTexture(std::vector<void const*>& rawData, DataDescription<Dimension> const& description, bool generateMipmap)
{
	assert(rawData.size() > 0);
	bindingTarget_ = GLBindingTargetFromTextureType(type_);
	gl::GenTextures(1, &textureID_);
	assert(textureID_ != 0);
	gl::BindTexture(bindingTarget_, textureID_);


	GLTextureFormat const& glFormat = GLGLTextureFormatFromTexelFormat(description.GetFormat());
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
		gl::GenerateMipmap(bindingTarget_);
	}
	else
	{
		mipmapCount_ = rawData.size();
	}

	gl::BindTexture(bindingTarget_, 0);
}


// instantiate 1, 2, 3 Dimensional Texture specialization
template ConcreteTexture<1>;
template ConcreteTexture<2>;
template ConcreteTexture<3>;
