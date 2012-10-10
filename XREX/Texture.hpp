#pragma once

#include "Declare.hpp"

#include <vector>
#include <array>

class Texture
	: Noncopyable
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

	enum class TexelFormat
	{
		RGB8,
		BGR8,
		RGBA8,
		BGRA8,

		R8,

		// TODO
		NotUsed,
		TexelFormatCount
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
	static uint32 GLBindingTargetFromTextureType(TextureType type);
	struct GLTextureFormat;
	static GLTextureFormat const& GLGLTextureFormatFromTexelFormat(TexelFormat format);

public:
	explicit Texture(TextureType type);
	virtual ~Texture();

	TextureType GetType() const
	{
		return type_;
	}

	void BindTexture(uint32 textureChannel);
	void UnbindTexture();

protected:
	TextureType type_;
	uint32 mipmapCount_;
	uint32 bindingTarget_; // gl binding target
	uint32 textureID_; // gl texture ID
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

	/*
	 *	@generateMipmap: true will generate mipmap, ignore data vector except data at index 0.
	 */
	template <typename T>
	ConcreteTexture(DataDescription<Dimension> const& description, std::vector<std::vector<T>> const& data, bool generateMipmap = true)
		: Texture(TextureDimensionToTextureType<Dimension>::TextureType), description_(description)
	{
		std::vector<void const*> rawData(data.size());
		for (uint32 i = 0; i < data.size(); ++i)
		{
			rawData[i] = data[i].data();
		}
		DoConstructTexture(rawData, description, generateMipmap);
	}

	virtual ~ConcreteTexture() override;

private:
	void DoConstructTexture(std::vector<void const*>& rawData, DataDescription<Dimension> const& description, bool generateMipmap);
	void DoFillTexture(uint32 mipmapLevel, GLTextureFormat const& glFormat, DataDescription<Dimension> const& description, void const* data);

private:
	DataDescription<Dimension> description_;
};

typedef ConcreteTexture<1> Texture1D;
typedef ConcreteTexture<2> Texture2D;
typedef ConcreteTexture<3> Texture3D;


class TextureCube
	: public Texture
{
public:
	TextureCube();
	virtual ~TextureCube() override;
	// TODO not finished

private:
	std::array<DataDescription<2>, static_cast<uint32>(CubeFace::CubeFaceElementCount)> description_;
};
