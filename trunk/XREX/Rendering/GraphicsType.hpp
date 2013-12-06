#pragma once

#include "Declare.hpp"

namespace XREX
{
	
	enum class ElementType
	{
		Void, // for special use

		Structure, // for special use

		Bool,
	
		Uint8,
		Uint16,
		Uint32,
		Int8,
		Int16,
		Int32,

		IntV2,
		IntV3,
		IntV4,

		UintV2,
		UintV3,
		UintV4,

		Float,
		FloatV2,
		FloatV3,
		FloatV4,
		FloatM44,

		Double,
		DoubleV2,
		DoubleV3,
		DoubleV4,
		DoubleM44,

		Shadow, // Shadow texture sampler

		Sampler, // for TechniqueParameter use

		Sampler1D,
		IntSampler1D,
		UintSampler1D,
		Sampler2D,
		IntSampler2D,
		UintSampler2D,
		Sampler3D,
		IntSampler3D,
		UintSampler3D,
		SamplerCube,
		IntSamplerCube,
		UintSamplerCube,
		SamplerBuffer,
		IntSamplerBuffer,
		UintSamplerBuffer,

		Image, // for TechniqueParameter use

		Image1D,
		IntImage1D,
		UintImage1D,
		Image2D,
		IntImage2D,
		UintImage2D,
		Image3D,
		IntImage3D,
		UintImage3D,
		ImageCube,
		IntImageCube,
		UintImageCube,
		ImageBuffer,
		IntImageBuffer,
		UintImageBuffer,

		Buffer, // for TechniqueParameter use

		AtomicUint32Counter,

		ElementTypeCount,
	};

	XREX_API bool IsSamplerType(ElementType type);
	XREX_API bool IsImageType(ElementType type);
	XREX_API bool IsAtomicBufferType(ElementType type);

	/*
	 *	@return: how many primitive element in a type.
	 */
	XREX_API uint32 GetElementPrimitiveCount(ElementType type);

	XREX_API ElementType GetElementPrimitiveType(ElementType type);

	XREX_API uint32 GetElementSizeInBytes(ElementType type);

	template <typename T>
	struct TypeToElementType
	{
		static ElementType const Type = ElementType::ElementTypeCount;
	};
	template <>
	struct TypeToElementType<bool>
	{
		static ElementType const Type = ElementType::Bool;
	};
	template <>
	struct TypeToElementType<uint8>
	{
		static ElementType const Type = ElementType::Uint8;
	};
	template <>
	struct TypeToElementType<uint16>
	{
		static ElementType const Type = ElementType::Uint16;
	};
	template <>
	struct TypeToElementType<uint32>
	{
		static ElementType const Type = ElementType::Uint32;
	};
	template <>
	struct TypeToElementType<int8>
	{
		static ElementType const Type = ElementType::Int8;
	};
	template <>
	struct TypeToElementType<int16>
	{
		static ElementType const Type = ElementType::Int16;
	};
	template <>
	struct TypeToElementType<int32>
	{
		static ElementType const Type = ElementType::Int32;
	};
	template <>
	struct TypeToElementType<float>
	{
		static ElementType const Type = ElementType::Float;
	};
	template <>
	struct TypeToElementType<floatV2>
	{
		static ElementType const Type = ElementType::FloatV2;
	};
	template <>
	struct TypeToElementType<floatV3>
	{
		static ElementType const Type = ElementType::FloatV3;
	};
	template <>
	struct TypeToElementType<floatV4>
	{
		static ElementType const Type = ElementType::FloatV4;
	};
	template <>
	struct TypeToElementType<floatM44>
	{
		static ElementType const Type = ElementType::FloatM44;
	};
	template <>
	struct TypeToElementType<double>
	{
		static ElementType const Type = ElementType::Double;
	};
	template <>
	struct TypeToElementType<doubleV2>
	{
		static ElementType const Type = ElementType::DoubleV2;
	};
	template <>
	struct TypeToElementType<doubleV3>
	{
		static ElementType const Type = ElementType::DoubleV3;
	};
	template <>
	struct TypeToElementType<doubleV4>
	{
		static ElementType const Type = ElementType::DoubleV4;
	};
	template <>
	struct TypeToElementType<doubleM44>
	{
		static ElementType const Type = ElementType::DoubleM44;
	};



	enum class TexelFormat
	{
		R8,
		RG8,
		RGB8,
		RGBA8,

		R8I,
		RG8I,
		RGB8I,
		RGBA8I,

		R8UI,
		RG8UI,
		RGB8UI,
		RGBA8UI,

		R16I,
		RG16I,
		RGB16I,
		RGBA16I,

		R16UI,
		RG16UI,
		RGB16UI,
		RGBA16UI,

		R16F,
		RG16F,
		RGB16F,
		RGBA16F,

		R32I,
		RG32I,
		RGB32I,
		RGBA32I,

		R32UI,
		RG32UI,
		RGB32UI,
		RGBA32UI,

		R32F,
		RG32F,
		RGB32F,
		RGBA32F,

		BGR8,
		BGRA8,
		BGR16F,
		BGRA16F,
		BGR32F,
		BGRA32F,

		Depth16,
		Depth24,
		Depth32,
		Depth32F,
		Depth24Stencil8,
		Stencil8,

		TexelFormatCount
	};

	XREX_API ElementType GetCorrespondingElementType(TexelFormat format);

	XREX_API ElementType GetTexelType(TexelFormat format);

	XREX_API uint32 GetTexelSizeInBytes(TexelFormat format);

	XREX_API TexelFormat GetCorrespondingTexelFormat(ElementType type);

	enum class AccessType
	{
		ReadOnly,
		WriteOnly,
		ReadWrite,
	};
}