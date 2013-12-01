#pragma once

#include "Declare.hpp"

#include "Rendering/TextureImage.hpp"

#include <vector>
#include <array>

namespace XREX
{

	class XREX_API Texture
		: public std::enable_shared_from_this<Texture>, Noncopyable
	{
	public:
		enum class TextureType
		{
			Texture1D,
			Texture2D,
			Texture3D,
			TextureCube,
			TextureBuffer,

			TextureTypeCount
		};

		template <uint32 Dimension>
		class DataDescription
		{
		public:
			DataDescription(TexelFormat format, Size<uint32, Dimension> const& sizes)
				: format_(format), sizes_(sizes)
			{
			}

			uint32 GetDimension() const
			{
				return Dimension;
			}
			TexelFormat GetFormat() const
			{
				return format_;
			}
			Size<uint32, Dimension> const& GetSizes() const
			{
				return sizes_;
			}

		private:
			static_assert(Dimension <= 3, "Dimension must <= 3");
			TexelFormat format_;
			Size<uint32, Dimension> sizes_;
		};

	protected:
		explicit Texture(TextureType type);
	public:
		virtual ~Texture();

		TextureType GetType() const // TODO make this virtual, and remove TextureType type_;
		{
			return type_;
		}

		uint32 GetID() const
		{
			return glTextureID_;
		}

		virtual TexelFormat GetFormat() const = 0;

		ElementType GetTexelElementType() const
		{
			return GetCorrespondingElementType(GetFormat());
		}

		uint32 GetMipmapCount() const
		{
			return mipmapCount_;
		}

		void Bind(uint32 index);
		void Unbind();

		void RecreateMipmap();

	protected:
		TextureType type_;
		uint32 mipmapCount_;
		uint32 glBindingTarget_;
		uint32 glTextureID_;

		uint32 lastBindingIndex_;
	};



	template <uint32 Dimension>
	class DimensionalTexture
		: public Texture
	{
		static_assert(Dimension >= 0 && Dimension <= 3, "Dimension must >= 0 && <= 3");

	public:

		DimensionalTexture(DataDescription<Dimension> const& description, bool generateMipmap);
		/*
		 *	@generateMipmap: true will generate mipmap, ignore data vector except data at index 0.
		 */
		DimensionalTexture(DataDescription<Dimension> const& description, std::vector<void const*> const& data, bool generateMipmap);
		virtual ~DimensionalTexture() override;

		virtual TexelFormat GetFormat() const override
		{
			return description_.GetFormat();
		}

		DataDescription<Dimension> const& GetDescription() const
		{
			return description_;
		}

		Size<uint32, Dimension> const& GetSize() const
		{
			return description_.GetSizes();
		}

		std::shared_ptr<DimensionalTextureImage<Dimension>> GetImage(uint32 level);


	private:
		void DoFillTexture(DataDescription<Dimension> const& description, uint32 mipmapLevel, void const* data);

	private:
		DataDescription<Dimension> description_;
	};

	class XREX_API Texture3D
		: public DimensionalTexture<3>
	{
	public:
		Texture3D(DataDescription<3> const& description, bool generateMipmap);
		/*
		 *	@generateMipmap: true will generate mipmap, ignore data vector except data at index 0.
		 */
		Texture3D(DataDescription<3> const& description, std::vector<void const*> const& data, bool generateMipmap);

		std::shared_ptr<Texture2DImage> GetLayerImage(uint32 layer, uint32 level);
	};

	typedef DimensionalTexture<1> Texture1D;
	typedef DimensionalTexture<2> Texture2D;



	enum class CubeFace
	{
		PositiveX,
		NegativeX,
		PositiveY,
		NegativeY,
		PositiveZ,
		NegativeZ,

		CubeFaceElementCount,
	};

	class XREX_API TextureCube
		: public Texture
	{
	public:
		TextureCube();
		virtual ~TextureCube() override;
		// TODO not finished

		

		Size<uint32, 2> const& GetSize() const
		{
			return description_[0].GetSizes();
		}

		virtual TexelFormat GetFormat() const override
		{
			return description_[0].GetFormat();
		}

		std::shared_ptr<TextureCubeImage> GetImage(CubeFace face, uint32 level);

	private:
		std::array<DataDescription<2>, static_cast<uint32>(CubeFace::CubeFaceElementCount)> description_;
	};


	class XREX_API TextureBuffer
		: public Texture
	{
	public:
		TextureBuffer(GraphicsBufferSP const& buffer, TexelFormat format);
		virtual ~TextureBuffer() override;

		virtual TexelFormat GetFormat() const override
		{
			return format_;
		}

		Size<uint32, 1> GetSize() const;

		std::shared_ptr<TextureBufferImage> GetImage();
	private:
		GraphicsBufferSP buffer_;
		TexelFormat format_;
	};

}

