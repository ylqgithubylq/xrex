#pragma once

#include "Declare.hpp"

namespace XREX
{
	class XREX_API TextureImage
		: Noncopyable
	{
	public:

		TextureImage(TextureSP const& texture, uint32 level);
		virtual ~TextureImage();

		TextureSP const& GetTexture() const
		{
			return texture_;
		}

		TexelFormat GetFormat() const;
		
		uint32 GetLevel() const
		{
			return level_;
		}

		void Bind(uint32 index, TexelFormat format, AccessType accessType);
		void Unbind();

	private:
		TextureSP texture_;
		uint32 level_;

		uint32 lastBindingIndex_;
	};

	template <uint32 Dimension>
	class DimensionalTextureImage
		: public TextureImage
	{
	public:
		DimensionalTextureImage(TextureSP const& texture, uint32 level);

		Size<uint32, Dimension> const& GetSize() const
		{
			return size_;
		}
	private:
		Size<uint32, Dimension> size_;
	};

	typedef DimensionalTextureImage<1> Texture1DImage;
	typedef DimensionalTextureImage<2> Texture2DImage;
	typedef DimensionalTextureImage<3> Texture3DImage;


	class XREX_API Texture3DLayerImage
		: public Texture2DImage
	{
	public:
		Texture3DLayerImage(TextureSP const& texture, uint32 layer, uint32 level);

		uint32 GetLayer() const
		{
			return layer_;
		}
	private:
		uint32 layer_;
	};


	enum class CubeFace;

	class XREX_API TextureCubeImage
		: public Texture2DImage
	{
	public:
		TextureCubeImage(TextureSP const& texture, CubeFace face, uint32 level);

		CubeFace GetFace() const
		{
			return face_;
		}
	private:
		CubeFace face_;
	};



	class XREX_API TextureBufferImage
		: public TextureImage
	{
	public:
		TextureBufferImage(TextureSP const& texture);

		Size<uint32, 1> GetSize() const;
	};
}


