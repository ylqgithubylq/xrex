#include "XREX.hpp"

#include "Application.hpp"
#include "RenderingFactory.hpp"
#include "TextureLoader.hpp"
#include "Texture.hpp"

#include <FreeImage.h>

#include <unordered_map>

#include <vector>

namespace
{

	Texture::TexelFormat TexelFormatFromFreeImageFormat(FREE_IMAGE_FORMAT freeImageFormat, FIBITMAP* bitmap)
	{
		FREE_IMAGE_TYPE imageType = FreeImage_GetImageType(bitmap);
		FREE_IMAGE_COLOR_TYPE colorType = FreeImage_GetColorType(bitmap);

		FREE_IMAGE_COLOR_TYPE::FIC_MINISBLACK;
		FREE_IMAGE_COLOR_TYPE::FIC_RGB;
		FREE_IMAGE_COLOR_TYPE::FIC_RGBALPHA;
		
		Texture::TexelFormat texelFormat;
		if (imageType == FREE_IMAGE_TYPE::FIT_BITMAP)
		{
			static std::unordered_map<FREE_IMAGE_COLOR_TYPE, Texture::TexelFormat> const colorTypeMapping = [] ()
			{
				std::unordered_map<FREE_IMAGE_COLOR_TYPE, Texture::TexelFormat> temp;
				temp[FREE_IMAGE_COLOR_TYPE::FIC_MINISBLACK] = Texture::TexelFormat::R8;
				temp[FREE_IMAGE_COLOR_TYPE::FIC_RGB] = Texture::TexelFormat::RGB8;
				temp[FREE_IMAGE_COLOR_TYPE::FIC_RGBALPHA] = Texture::TexelFormat::RGBA8;
				return temp;
			} ();
			texelFormat = colorTypeMapping.at(colorType);
		}
		else
		{
			static std::unordered_map<FREE_IMAGE_TYPE, Texture::TexelFormat> const imageTypeMapping = [] ()
			{
				std::unordered_map<FREE_IMAGE_TYPE, Texture::TexelFormat> temp;
				temp[FREE_IMAGE_TYPE::FIT_BITMAP] = Texture::TexelFormat::NotUsed;
				temp[FREE_IMAGE_TYPE::FIT_COMPLEX] = Texture::TexelFormat::NotUsed;
				temp[FREE_IMAGE_TYPE::FIT_DOUBLE] = Texture::TexelFormat::NotUsed;
				temp[FREE_IMAGE_TYPE::FIT_FLOAT] = Texture::TexelFormat::NotUsed;
				temp[FREE_IMAGE_TYPE::FIT_INT16] = Texture::TexelFormat::NotUsed;
				temp[FREE_IMAGE_TYPE::FIT_INT32] = Texture::TexelFormat::NotUsed;
				temp[FREE_IMAGE_TYPE::FIT_RGB16] = Texture::TexelFormat::NotUsed;
				temp[FREE_IMAGE_TYPE::FIT_RGBA16] = Texture::TexelFormat::NotUsed;
				temp[FREE_IMAGE_TYPE::FIT_RGBF] = Texture::TexelFormat::NotUsed;
				temp[FREE_IMAGE_TYPE::FIT_RGBAF] = Texture::TexelFormat::NotUsed;
				temp[FREE_IMAGE_TYPE::FIT_UINT16] = Texture::TexelFormat::NotUsed;
				temp[FREE_IMAGE_TYPE::FIT_UINT32] = Texture::TexelFormat::NotUsed;
				temp[FREE_IMAGE_TYPE::FIT_UNKNOWN] = Texture::TexelFormat::NotUsed;
				return temp;
			} ();
			texelFormat = imageTypeMapping.at(imageType);
			assert(texelFormat != Texture::TexelFormat::NotUsed); // above imageTypeMapping not implement yet
		}
		return texelFormat;
	}

}


TextureLoader::TextureLoader()
{
}


TextureLoader::~TextureLoader()
{
}

namespace
{
	template <typename TextureType>
	struct TextureHandler
	{
		TextureSP& texture;
		TextureHandler(TextureSP& theTexture)
			: texture(theTexture)
		{
		}
		void operator ()(uint32 width, uint32 height, uint32 size, Texture::TexelFormat format, uint8 const* data);
	};

	template <>
	void TextureHandler<Texture1D>::operator ()(uint32 width, uint32 height, uint32 size, Texture::TexelFormat format, uint8 const* data)
	{
		std::array<uint32, 1> sizes;
		sizes[0] = width;
		Texture::DataDescription<1> description(format, sizes);
		std::vector<std::vector<uint8>> dataContainer;
		std::vector<uint8> topLevelData(size);
		memcpy_s(&topLevelData[0], topLevelData.size(), data, size);
		dataContainer.emplace_back(std::move(topLevelData));
		texture = Application::GetInstance().GetRenderingFactory().CreateTexture1D(description, dataContainer);
	}
	template <>
	void TextureHandler<Texture2D>::operator ()(uint32 width, uint32 height, uint32 size, Texture::TexelFormat format, uint8 const* data)
	{
		std::array<uint32, 2> sizes;
		sizes[0] = width;
		sizes[1] = height;
		Texture::DataDescription<2> description(format, sizes);
		std::vector<std::vector<uint8>> dataContainer;
		std::vector<uint8> topLevelData(size);
		memcpy_s(&topLevelData[0], topLevelData.size(), data, size);
		dataContainer.emplace_back(std::move(topLevelData));
		texture = Application::GetInstance().GetRenderingFactory().CreateTexture2D(description, dataContainer);
	}
	template <>
	void TextureHandler<Texture3D>::operator ()(uint32 width, uint32 height, uint32 size, Texture::TexelFormat format, uint8 const* data)
	{
		assert(false); // TODO not finished, how to load 3D texture?
		std::array<uint32, 3> sizes;
		sizes[0] = width;
		sizes[1] = width;
		sizes[3] = height / width;
		Texture::DataDescription<3> description(format, sizes);
		std::vector<std::vector<uint8>> dataContainer;
		std::vector<uint8> topLevelData(size);
		memcpy_s(&topLevelData[0], topLevelData.size(), data, size);
		dataContainer.emplace_back(std::move(topLevelData));
		texture = Application::GetInstance().GetRenderingFactory().CreateTexture3D(description, dataContainer);
	}
	template <>
	void TextureHandler<TextureCube>::operator ()(uint32 width, uint32 height, uint32 size, Texture::TexelFormat format, uint8 const* data)
	{
		assert(false); // TODO not finished, how to load cube texture?
		std::array<uint32, 3> sizes;
		sizes[0] = width;
		sizes[1] = width;
		sizes[3] = height / width;
		Texture::DataDescription<3> description(format, sizes);
		std::vector<std::vector<uint8>> dataContainer;
		std::vector<uint8> topLevelData(size);
		memcpy_s(&topLevelData[0], topLevelData.size(), data, size);
		dataContainer.emplace_back(std::move(topLevelData));
		texture = Application::GetInstance().GetRenderingFactory().CreateTexture3D(description, dataContainer);
	}
}


TextureSP TextureLoader::LoadTexture1D(std::string const& fileName)
{
	TextureSP texture;
	TextureHandler<Texture1D> handler(texture);
	LoadBits(fileName, handler); // TextureHandler will create texture if succeed
	return texture;
}

TextureSP TextureLoader::LoadTexture2D(std::string const& fileName)
{
	TextureSP texture;
	TextureHandler<Texture2D> handler(texture);
	LoadBits(fileName, handler); // TextureHandler will create texture if succeed
	return texture;
}

TextureSP TextureLoader::LoadTexture3D(std::string const& fileName)
{
	TextureSP texture;
	TextureHandler<Texture3D> handler(texture);
	LoadBits(fileName, handler); // TextureHandler will create texture if succeed
	return texture;
}

TextureSP TextureLoader::LoadTextureCube(std::string const& fileName)
{
	TextureSP texture;
	TextureHandler<TextureCube> handler(texture);
	LoadBits(fileName, handler); // TextureHandler will create texture if succeed
	return texture;
}

bool TextureLoader::LoadBits(std::string const &fileName, std::function<void(uint32 width, uint32 height, uint32 size, Texture::TexelFormat format, uint8 const* data)> const& handler)
{
	FREE_IMAGE_FORMAT imageFormat = FIF_UNKNOWN;

	//check the file signature and deduce its format
	imageFormat = FreeImage_GetFileType(fileName.c_str(), 0);
	//if still unknown, try to guess the file format from the file extension
	if(imageFormat == FIF_UNKNOWN)
	{
		imageFormat = FreeImage_GetFIFFromFilename(fileName.c_str());
	}
	//if still unknown, return failure
	if(imageFormat == FIF_UNKNOWN)
	{
		return false;
	}

	//pointer to the image, once loaded
	FIBITMAP* bitmap = nullptr;

	//check that the plugin has reading capabilities and load the file
	if(FreeImage_FIFSupportsReading(imageFormat))
	{
		bitmap = FreeImage_Load(imageFormat, fileName.c_str());
	}
	//if the image failed to load, return failure
	if(!bitmap)
	{
		return false;
	} // after this, make sure to call FreeImage_Unload(FIBITMAP*)


	//retrieve the image data
	uint8* bits = FreeImage_GetBits(bitmap);

	uint32 width = FreeImage_GetWidth(bitmap);
	uint32 height = FreeImage_GetHeight(bitmap);
	//if this somehow one of these failed (they shouldn't), return failure
	if(!bits || width == 0 || height == 0)
	{
		return false;
	}


	uint32 bpp = FreeImage_GetBPP(bitmap);
	handler(height, width, bpp / 8 * height * width, TexelFormatFromFreeImageFormat(imageFormat, bitmap), bits);

	//Free FreeImage's copy of the data
	FreeImage_Unload(bitmap);

	return true;
}
