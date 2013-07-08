#include "XREX.hpp"

#include "XREXContext.hpp"
#include "RenderingFactory.hpp"
#include "TextureLoader.hpp"
#include "Texture.hpp"

#include <FreeImage.h>

#include <unordered_map>
#include <vector>

namespace XREX
{
	struct NullTextureLoadingResult
		: LoadingResult<Texture>
	{
		virtual bool Succeeded() const override
		{
			return false;
		}
		virtual TextureSP Create() override
		{
			return nullptr;
		}
	};

	template <uint32 N>
	struct TextureLoadingResultDetail
		: LoadingResult<Texture>
	{

		struct DataDetail
			: private XREX::Noncopyable
		{
			Texture::DataDescription<N> description;
			std::vector<std::vector<uint8>> data;
			TextureSP loadedTexture;
			bool generateMipmap;

			DataDetail(Texture::DataDescription<N> const& theDescription, std::vector<std::vector<uint8>>&& theData, bool needGenerateMipmap)
				: description(theDescription), data(std::move(theData)), generateMipmap(needGenerateMipmap)
			{
			}
			~DataDetail()
			{
			}
			TextureSP DoCreateTexture()
			{
				if (loadedTexture == nullptr)
				{
					loadedTexture = DoLoad(generateMipmap);
				}
				return loadedTexture;
			}

			TextureSP DoLoad(bool generateMipmap);

		};

		TextureLoadingResultDetail()
		{
		}

		TextureLoadingResultDetail(Texture::DataDescription<N> const& theDescription, std::vector<std::vector<uint8>>&& theData, bool generateMipmap)
		{
			data_ = MakeUP<DataDetail>(theDescription, std::move(theData), generateMipmap);
		}

		virtual bool Succeeded() const override
		{
			return data_ != nullptr;
		}

		virtual TextureSP Create() override
		{
			if (Succeeded())
			{
				return data_->DoCreateTexture();
			}
			return nullptr;
		}


		std::unique_ptr<DataDetail> data_;

	};

	template <>
	TextureSP TextureLoadingResultDetail<1>::DataDetail::DoLoad(bool generateMipmap)
	{
		return XREXContext::GetInstance().GetRenderingFactory().CreateTexture1D(description, data, generateMipmap);
	}
	template <>
	TextureSP TextureLoadingResultDetail<2>::DataDetail::DoLoad(bool generateMipmap)
	{
		return XREXContext::GetInstance().GetRenderingFactory().CreateTexture2D(description, data, generateMipmap);
	}
	template <>
	TextureSP TextureLoadingResultDetail<3>::DataDetail::DoLoad(bool generateMipmap)
	{
		return XREXContext::GetInstance().GetRenderingFactory().CreateTexture3D(description, data, generateMipmap);
	}




	namespace
	{

		TexelFormat TexelFormatFromFreeImageFormat(FREE_IMAGE_FORMAT freeImageFormat, FIBITMAP* bitmap)
		{
			FREE_IMAGE_TYPE imageType = FreeImage_GetImageType(bitmap);
			FREE_IMAGE_COLOR_TYPE colorType = FreeImage_GetColorType(bitmap);

			FREE_IMAGE_COLOR_TYPE::FIC_MINISBLACK;
			FREE_IMAGE_COLOR_TYPE::FIC_RGB;
			FREE_IMAGE_COLOR_TYPE::FIC_RGBALPHA;
		
			TexelFormat texelFormat;
			if (imageType == FREE_IMAGE_TYPE::FIT_BITMAP)
			{
				static std::unordered_map<std::pair<FREE_IMAGE_COLOR_TYPE, bool>, TexelFormat, STLPairHasher<FREE_IMAGE_COLOR_TYPE, bool>> const colorTypeMapping = [] ()
				{
					std::remove_const<decltype(colorTypeMapping)>::type temp;
					temp[std::make_pair(FREE_IMAGE_COLOR_TYPE::FIC_MINISBLACK, false)] = TexelFormat::R8;
					temp[std::make_pair(FREE_IMAGE_COLOR_TYPE::FIC_RGB, false)] = TexelFormat::RGB8;
					temp[std::make_pair(FREE_IMAGE_COLOR_TYPE::FIC_RGBALPHA, false)] = TexelFormat::RGBA8;
					temp[std::make_pair(FREE_IMAGE_COLOR_TYPE::FIC_MINISBLACK, true)] = TexelFormat::R8;
					temp[std::make_pair(FREE_IMAGE_COLOR_TYPE::FIC_RGB, true)] = TexelFormat::BGR8;
					temp[std::make_pair(FREE_IMAGE_COLOR_TYPE::FIC_RGBALPHA, true)] = TexelFormat::BGRA8;
					return temp;
				} ();
				uint32 blueMask = FreeImage_GetBlueMask(bitmap);
				uint32 redMask = FreeImage_GetRedMask(bitmap);
				texelFormat = colorTypeMapping.at(std::make_pair(colorType, blueMask < redMask));
			}
			else
			{
				static std::unordered_map<FREE_IMAGE_TYPE, TexelFormat> const imageTypeMapping = [] ()
				{
					std::remove_const<decltype(imageTypeMapping)>::type temp;
					temp[FREE_IMAGE_TYPE::FIT_BITMAP] = TexelFormat::TexelFormatCount;
					temp[FREE_IMAGE_TYPE::FIT_COMPLEX] = TexelFormat::TexelFormatCount;
					temp[FREE_IMAGE_TYPE::FIT_DOUBLE] = TexelFormat::TexelFormatCount;
					temp[FREE_IMAGE_TYPE::FIT_FLOAT] = TexelFormat::TexelFormatCount;
					temp[FREE_IMAGE_TYPE::FIT_INT16] = TexelFormat::TexelFormatCount;
					temp[FREE_IMAGE_TYPE::FIT_INT32] = TexelFormat::TexelFormatCount;
					temp[FREE_IMAGE_TYPE::FIT_RGB16] = TexelFormat::TexelFormatCount;
					temp[FREE_IMAGE_TYPE::FIT_RGBA16] = TexelFormat::TexelFormatCount;
					temp[FREE_IMAGE_TYPE::FIT_RGBF] = TexelFormat::TexelFormatCount;
					temp[FREE_IMAGE_TYPE::FIT_RGBAF] = TexelFormat::TexelFormatCount;
					temp[FREE_IMAGE_TYPE::FIT_UINT16] = TexelFormat::TexelFormatCount;
					temp[FREE_IMAGE_TYPE::FIT_UINT32] = TexelFormat::TexelFormatCount;
					temp[FREE_IMAGE_TYPE::FIT_UNKNOWN] = TexelFormat::TexelFormatCount;
					return temp;
				} ();
				texelFormat = imageTypeMapping.at(imageType);
				assert(texelFormat != TexelFormat::TexelFormatCount); // above imageTypeMapping not implement yet
			}
			return texelFormat;
		}



		template <typename TextureType>
		struct TextureHandler
		{
			bool generateMipmap;
			std::string fileName;
			TextureLoadingResultSP result;

			TextureHandler(std::string const &theFileName, bool ifGenerateMipmap = true)
				: fileName(theFileName), generateMipmap(ifGenerateMipmap), result(MakeSP<NullTextureLoadingResult>())
			{
			}

			TextureLoadingResultSP ExtractResult()
			{
				return std::move(result);
			}

			bool Load()
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
				BuildResult(width, height, bpp / 8 * height * width, TexelFormatFromFreeImageFormat(imageFormat, bitmap), bits);

				//Free FreeImage's copy of the data
				FreeImage_Unload(bitmap);

				return true;
			}

			void BuildResult(uint32 width, uint32 height, uint32 size, TexelFormat format, uint8 const* data);
		};

		template <>
		void TextureHandler<Texture1D>::BuildResult(uint32 width, uint32 height, uint32 size, TexelFormat format, uint8 const* data)
		{
			std::array<uint32, 1> sizes;
			sizes[0] = width;
			Texture::DataDescription<1> description(format, sizes);
			std::vector<std::vector<uint8>> dataContainer;
			std::vector<uint8> topLevelData(size);
			memcpy_s(&topLevelData[0], topLevelData.size(), data, size);
			dataContainer.emplace_back(std::move(topLevelData));
			result = MakeSP<TextureLoadingResultDetail<1>>(description, std::move(dataContainer), generateMipmap);
		}
		template <>
		void TextureHandler<Texture2D>::BuildResult(uint32 width, uint32 height, uint32 size, TexelFormat format, uint8 const* data)
		{
			std::array<uint32, 2> sizes;
			sizes[0] = width;
			sizes[1] = height;
			Texture::DataDescription<2> description(format, sizes);
			std::vector<std::vector<uint8>> dataContainer;
			std::vector<uint8> topLevelData(size);
			memcpy_s(&topLevelData[0], topLevelData.size(), data, size);
			dataContainer.emplace_back(std::move(topLevelData));
			result = MakeSP<TextureLoadingResultDetail<2>>(description, std::move(dataContainer), generateMipmap);
		}
		template <>
		void TextureHandler<Texture3D>::BuildResult(uint32 width, uint32 height, uint32 size, TexelFormat format, uint8 const* data)
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
			result = MakeSP<TextureLoadingResultDetail<3>>(description, std::move(dataContainer), generateMipmap);
		}
		template <>
		void TextureHandler<TextureCube>::BuildResult(uint32 width, uint32 height, uint32 size, TexelFormat format, uint8 const* data)
		{
			assert(false); // TODO not finished, how to load cube texture?
			std::array<uint32, 2> sizes;
			sizes[0] = width;
			sizes[1] = width;
			sizes[3] = height / width;
			Texture::DataDescription<2> description(format, sizes);
			std::vector<std::vector<uint8>> dataContainer;
			std::vector<uint8> topLevelData(size);
			memcpy_s(&topLevelData[0], topLevelData.size(), data, size);
			dataContainer.emplace_back(std::move(topLevelData));
			// TODO fill faces
			std::array<decltype(dataContainer), 6> faces;
		}


	}


	TextureLoader::TextureLoader()
	{
	}


	TextureLoader::~TextureLoader()
	{
	}

	TextureLoadingResultSP TextureLoader::LoadTexture1D(std::string const& fileName, bool generateMipmap)
	{
		TextureHandler<Texture1D> handler(fileName, generateMipmap);
		handler.Load();
		return handler.ExtractResult();
	}

	TextureLoadingResultSP TextureLoader::LoadTexture2D(std::string const& fileName, bool generateMipmap)
	{
		TextureHandler<Texture2D> handler(fileName, generateMipmap);
		handler.Load();
		return handler.ExtractResult();
	}

	TextureLoadingResultSP TextureLoader::LoadTexture3D(std::string const& fileName, bool generateMipmap)
	{
		TextureHandler<Texture3D> handler(fileName, generateMipmap);
		handler.Load();
		return handler.ExtractResult();
	}

	TextureLoadingResultSP TextureLoader::LoadTextureCube(std::string const& fileName, bool generateMipmap)
	{
		TextureHandler<TextureCube> handler(fileName, generateMipmap);
		handler.Load();
		return handler.ExtractResult();
	}



}
