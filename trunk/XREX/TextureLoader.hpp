#pragma once

#include "Declare.hpp"
#include "Texture.hpp"

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
		 *	@return: nullptr if failed.
		 */
		TextureSP LoadTexture1D(std::string const& fileName, bool generateMipmap = true);
		/*
		 *	@return: nullptr if failed.
		 */
		TextureSP LoadTexture2D(std::string const& fileName, bool generateMipmap = true);
		/*
		 *	@return: nullptr if failed.
		 */
		TextureSP LoadTexture3D(std::string const& fileName, bool generateMipmap = true);
		/*
		 *	@return: nullptr if failed.
		 */
		TextureSP LoadTextureCube(std::string const& fileName, bool generateMipmap = true);

	private:
		bool LoadBits(std::string const &fileName, std::function<void(uint32 width, uint32 height, uint32 size, Texture::TexelFormat format, uint8 const* data)> const& handler);
	};

}
