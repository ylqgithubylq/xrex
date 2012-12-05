#pragma once

#include "Declare.hpp"
#include "LoadingResult.hpp"

#include <functional>

namespace XREX
{

	class XREX_API TextureLoader
		: Noncopyable
	{
	public:
		TextureLoader();
		~TextureLoader();

		/*
		 *	@return: texture data ready to create texture.
		 */
		TextureLoadingResultSP LoadTexture1D(std::string const& fileName, bool generateMipmap = true);
		/*
		 *	@return: texture data ready to create texture.
		 */
		TextureLoadingResultSP LoadTexture2D(std::string const& fileName, bool generateMipmap = true);
		/*
		 *	@return: texture data ready to create texture.
		 */
		TextureLoadingResultSP LoadTexture3D(std::string const& fileName, bool generateMipmap = true);
		/*
		 *	@return: texture data ready to create texture.
		 */
		TextureLoadingResultSP LoadTextureCube(std::string const& fileName, bool generateMipmap = true);

	};

}
