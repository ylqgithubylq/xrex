#pragma once

#include "Declare.hpp"

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

			TextureTypeCount
		};
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



		template <uint32 Dimension>
		class DataDescription
		{
		public:
			DataDescription(TexelFormat format, std::array<uint32, Dimension> const& sizes)
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
			std::array<uint32, Dimension> const& GetSizes() const
			{
				return sizes_;
			}

		private:
			static_assert(Dimension <= 3, "Dimension must <= 3");
			TexelFormat format_;
			std::array<uint32, Dimension> sizes_;
		};

	protected:
		explicit Texture(TextureType type);
	public:
		virtual ~Texture();

		TextureType GetType() const
		{
			return type_;
		}

		uint32 GetID() const
		{
			return glTextureID_;
		}

		virtual TexelFormat GetFormat() const = 0;

		void Bind(uint32 index);
		void Unbind();

		TextureImageSP GetImage_TEMP(uint32 level, TexelFormat format);
		TextureImageSP GetImage(uint32 level);

	protected:
		TextureType type_;
		uint32 mipmapCount_;
		uint32 glBindingTarget_;
		uint32 glTextureID_;

		uint32 lastBindingIndex_;
	};




	template <uint32 N>
	struct TextureDimensionToTextureType
	{
		static_assert(N >= 1 && N <= 3, "Texture only have 1, 2, 3 dimensions.");
		static Texture::TextureType const TextureType = Texture::TextureType::TextureCube;
	};
	template <>
	struct TextureDimensionToTextureType<1>
	{
		static Texture::TextureType const TextureType = Texture::TextureType::Texture1D;
	};
	template <>
	struct TextureDimensionToTextureType<2>
	{
		static Texture::TextureType const TextureType = Texture::TextureType::Texture2D;
	};
	template <>
	struct TextureDimensionToTextureType<3>
	{
		static Texture::TextureType const TextureType = Texture::TextureType::Texture3D;
	};

	template <uint32 Dimension>
	class ConcreteTexture
		: public Texture
	{
		static_assert(Dimension >= 0 && Dimension <= 3, "Dimension must >= 0 && <= 3");

	public:

		explicit ConcreteTexture(DataDescription<Dimension> const& description);
		/*
		 *	@generateMipmap: true will generate mipmap, ignore data vector except data at index 0.
		 */
		ConcreteTexture(DataDescription<Dimension> const& description, std::vector<void const*> const& data, bool generateMipmap = true);
		virtual ~ConcreteTexture() override;

		virtual TexelFormat GetFormat() const override
		{
			return description_.GetFormat();
		}

	private:
		void DoFillTexture(DataDescription<Dimension> const& description, uint32 mipmapLevel, void const* data);

	private:
		DataDescription<Dimension> description_;
	};

	typedef ConcreteTexture<1> Texture1D;
	typedef ConcreteTexture<2> Texture2D;
	typedef ConcreteTexture<3> Texture3D;


	class XREX_API TextureCube
		: public Texture
	{
	public:
		TextureCube();
		virtual ~TextureCube() override;
		// TODO not finished

	private:
		std::array<DataDescription<2>, static_cast<uint32>(CubeFace::CubeFaceElementCount)> description_;
	};

}

