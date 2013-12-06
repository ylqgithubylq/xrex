#pragma once

#include "Declare.hpp"

#include "Rendering/GraphicsType.hpp"

namespace XREX
{
	class XREX_API TextureImage
		: Noncopyable
	{
	public:
		enum class ImageType
		{
			Image1D,
			Image2D,
			Image3D,
			ImageCube,
			ImageBuffer,

			ImageTypeCount
		};
	protected:
		TextureImage(ImageType type, TextureSP const& texture, uint32 level);
	public:
		virtual ~TextureImage();

		ImageType GetType() const
		{
			return type_;
		}

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
		ImageType type_;
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

	class XREX_API Texture1DImage
		: public DimensionalTextureImage<1>
	{
	public:
		Texture1DImage(Texture1DSP const& texture, uint32 level);
	};

	class XREX_API Texture2DImage
		: public DimensionalTextureImage<2>
	{
	protected:
		Texture2DImage(TextureSP const& texture, uint32 level); // for Texture3DLayerImage and TextureCubeImage
	public:
		Texture2DImage(Texture2DSP const& texture, uint32 level);
	};

	class XREX_API Texture3DImage
		: public DimensionalTextureImage<3>
	{
	public:
		Texture3DImage(Texture3DSP const& texture, uint32 level);
	};

	class XREX_API Texture3DLayerImage
		: public Texture2DImage
	{
	public:
		Texture3DLayerImage(Texture3DSP const& texture, uint32 layer, uint32 level);

		uint32 GetLayer() const
		{
			return layer_;
		}
	private:
		uint32 layer_;
	};


	enum class CubeFace;

	class XREX_API TextureCubeFaceImage
		: public Texture2DImage
	{
	public:
		TextureCubeFaceImage(TextureCubeSP const& texture, CubeFace face, uint32 level);

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


