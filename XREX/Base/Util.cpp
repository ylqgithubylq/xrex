#include "XREX.hpp"

#include "Util.hpp"

#include <array>

using std::array;

namespace XREX
{

	bool IsSamplerType(ElementType type)
	{
		switch (type)
		{
		case ElementType::Sampler1D:
		case ElementType::IntSampler1D:
		case ElementType::UintSampler1D:
		case ElementType::Sampler2D:
		case ElementType::IntSampler2D:
		case ElementType::UintSampler2D:
		case ElementType::Sampler3D:
		case ElementType::IntSampler3D:
		case ElementType::UintSampler3D:
		case ElementType::SamplerCube:
		case ElementType::IntSamplerCube:
		case ElementType::UintSamplerCube:
		case ElementType::SamplerBuffer:
		case ElementType::IntSamplerBuffer:
		case ElementType::UintSamplerBuffer:
			return true;
		case ElementType::Sampler: // special case
			return true;
		case ElementType::ElementTypeCount:
			assert(false);
			return false;
		default:
			return false;
		}
	}

	bool IsImageType(ElementType type)
	{
		switch (type)
		{
		case ElementType::Image1D:
		case ElementType::IntImage1D:
		case ElementType::UintImage1D:
		case ElementType::Image2D:
		case ElementType::IntImage2D:
		case ElementType::UintImage2D:
		case ElementType::Image3D:
		case ElementType::IntImage3D:
		case ElementType::UintImage3D:
		case ElementType::ImageCube:
		case ElementType::IntImageCube:
		case ElementType::UintImageCube:
		case ElementType::ImageBuffer:
		case ElementType::IntImageBuffer:
		case ElementType::UintImageBuffer:
			return true;
		case ElementType::Image: // special case
			return true;
		case ElementType::ElementTypeCount:
			assert(false);
			return false;
		default:
			return false;
		}
	}

	bool IsAtomicBufferType(ElementType type)
	{
		switch (type)
		{
		case ElementType::AtomicUint32Counter:
			return true;
			break;
		case ElementType::ElementTypeCount:
			assert(false);
			return false;
			break;
		default:
			return false;
			break;
		}
	}

	uint32 GetElementPrimitiveCount(ElementType type)
	{
		switch (type)
		{
		case ElementType::Void:
			assert(false);
			return 0;
		case ElementType::Bool:
			return 1;
		case ElementType::Uint8:
			return 1;
		case ElementType::Uint16:
			return 1;
		case ElementType::Uint32:
			return 1;
		case ElementType::Int8:
			return 1;
		case ElementType::Int16:
			return 1;
		case ElementType::Int32:
			return 1;
		case ElementType::IntV2:
			return 2;
		case ElementType::IntV3:
			return 3;
		case ElementType::IntV4:
			return 4;
		case ElementType::UintV2:
			return 2;
		case ElementType::UintV3:
			return 3;
		case ElementType::UintV4:
			return 4;
		case ElementType::Float:
			return 1;
		case ElementType::FloatV2:
			return 2;
		case ElementType::FloatV3:
			return 3;
		case ElementType::FloatV4:
			return 4;
		case ElementType::FloatM44:
			return 16;
		case ElementType::Double:
			return 1;
		case ElementType::DoubleV2:
			return 2;
		case ElementType::DoubleV3:
			return 3;
		case ElementType::DoubleV4:
			return 4;
		case ElementType::DoubleM44:
			return 16;

		case ElementType::ElementTypeCount:
			assert(false);
			return 0;
		default:
			assert(false);
			return 0;
		}
	}

	uint32 GetElementSizeInBytes(ElementType type)
	{
		switch (type)
		{
		case ElementType::Void:
			assert(false);
			return 0;
		case ElementType::Bool:
			return sizeof(bool);
		case ElementType::Uint8:
			return sizeof(uint8);
		case ElementType::Uint16:
			return sizeof(uint16);
		case ElementType::Uint32:
			return sizeof(uint32);
		case ElementType::Int8:
			return sizeof(int8);
		case ElementType::Int16:
			return sizeof(int16);
		case ElementType::Int32:
			return sizeof(int32);
		case ElementType::IntV2:
			return sizeof(intV2);
		case ElementType::IntV3:
			return sizeof(intV3);
		case ElementType::IntV4:
			return sizeof(intV4);
		case ElementType::UintV2:
			return sizeof(uintV2);
		case ElementType::UintV3:
			return sizeof(uintV3);
		case ElementType::UintV4:
			return sizeof(uintV4);
		case ElementType::Float:
			return sizeof(float);
		case ElementType::FloatV2:
			return sizeof(floatV2);
		case ElementType::FloatV3:
			return sizeof(floatV3);
		case ElementType::FloatV4:
			return sizeof(floatV4);
		case ElementType::FloatM44:
			return sizeof(floatM44);
		case ElementType::Double:
			return sizeof(double);
		case ElementType::DoubleV2:
			return sizeof(doubleV2);
		case ElementType::DoubleV3:
			return sizeof(doubleV3);
		case ElementType::DoubleV4:
			return sizeof(doubleV4);
		case ElementType::DoubleM44:
			return sizeof(doubleM44);

		case ElementType::ElementTypeCount:
			assert(false);
			return 0;
		default:
			assert(false);
			return 0;
		}
	}

	ElementType GetElementPrimitiveType(ElementType type)
	{
		switch (type)
		{
		case ElementType::Void:
			return ElementType::Void;
		case ElementType::Bool:
			return ElementType::Bool;
		case ElementType::Uint8:
			return ElementType::Uint8;
		case ElementType::Uint16:
			return ElementType::Uint16;
		case ElementType::Uint32:
			return ElementType::Uint32;
		case ElementType::Int8:
			return ElementType::Int8;
		case ElementType::Int16:
			return ElementType::Int16;
		case ElementType::Int32:
			return ElementType::Int32;
		case ElementType::IntV2:
			return ElementType::Int32;
		case ElementType::IntV3:
			return ElementType::Int32;
		case ElementType::IntV4:
			return ElementType::Int32;
		case ElementType::UintV2:
			return ElementType::Uint32;
		case ElementType::UintV3:
			return ElementType::Uint32;
		case ElementType::UintV4:
			return ElementType::Uint32;
		case ElementType::Float:
			return ElementType::Float;
		case ElementType::FloatV2:
			return ElementType::Float;
		case ElementType::FloatV3:
			return ElementType::Float;
		case ElementType::FloatV4:
			return ElementType::Float;
		case ElementType::FloatM44:
			return ElementType::Float;
		case ElementType::Double:
			return ElementType::Double;
		case ElementType::DoubleV2:
			return ElementType::Double;
		case ElementType::DoubleV3:
			return ElementType::Double;
		case ElementType::DoubleV4:
			return ElementType::Double;
		case ElementType::DoubleM44:
			return ElementType::Double;

		case ElementType::ElementTypeCount:
			assert(false);
			return ElementType::Void;
		default:
			assert(false);
			return ElementType::Void;
		}
	}

	ElementType GetCorrespondingElementType(TexelFormat format)
	{
		switch (format)
		{
		case TexelFormat::R8:
			return ElementType::Float;
		case TexelFormat::RG8:
			return ElementType::FloatV2;
		case TexelFormat::RGB8:
			return ElementType::FloatV3;
		case TexelFormat::RGBA8:
			return ElementType::FloatV4;
		case TexelFormat::R8I:
			return ElementType::Int32;
		case TexelFormat::RG8I:
			return ElementType::IntV2;
		case TexelFormat::RGB8I:
			return ElementType::IntV3;
		case TexelFormat::RGBA8I:
			return ElementType::IntV4;
		case TexelFormat::R8UI:
			return ElementType::Uint32;
		case TexelFormat::RG8UI:
			return ElementType::UintV2;
		case TexelFormat::RGB8UI:
			return ElementType::UintV3;
		case TexelFormat::RGBA8UI:
			return ElementType::UintV4;
		case TexelFormat::R16I:
			return ElementType::Int32;
		case TexelFormat::RG16I:
			return ElementType::IntV2;
		case TexelFormat::RGB16I:
			return ElementType::IntV3;
		case TexelFormat::RGBA16I:
			return ElementType::IntV4;
		case TexelFormat::R16UI:
			return ElementType::Uint32;
		case TexelFormat::RG16UI:
			return ElementType::UintV2;
		case TexelFormat::RGB16UI:
			return ElementType::UintV3;
		case TexelFormat::RGBA16UI:
			return ElementType::UintV4;
		case TexelFormat::R16F:
			return ElementType::Float;
		case TexelFormat::RG16F:
			return ElementType::FloatV2;
		case TexelFormat::RGB16F:
			return ElementType::FloatV3;
		case TexelFormat::RGBA16F:
			return ElementType::FloatV4;
		case TexelFormat::R32I:
			return ElementType::Int32;
		case TexelFormat::RG32I:
			return ElementType::IntV2;
		case TexelFormat::RGB32I:
			return ElementType::IntV3;
		case TexelFormat::RGBA32I:
			return ElementType::IntV4;
		case TexelFormat::R32UI:
			return ElementType::Uint32;
		case TexelFormat::RG32UI:
			return ElementType::UintV2;
		case TexelFormat::RGB32UI:
			return ElementType::UintV3;
		case TexelFormat::RGBA32UI:
			return ElementType::UintV4;
		case TexelFormat::R32F:
			return ElementType::Float;
		case TexelFormat::RG32F:
			return ElementType::FloatV2;
		case TexelFormat::RGB32F:
			return ElementType::FloatV3;
		case TexelFormat::RGBA32F:
			return ElementType::FloatV4;
		case TexelFormat::BGR8:
			return ElementType::FloatV3;
		case TexelFormat::BGRA8:
			return ElementType::FloatV4;
		case TexelFormat::BGR16F:
			return ElementType::FloatV3;
		case TexelFormat::BGRA16F:
			return ElementType::FloatV4;
		case TexelFormat::BGR32F:
			return ElementType::FloatV3;
		case TexelFormat::BGRA32F:
			return ElementType::FloatV4;
		case TexelFormat::Depth16:
			return ElementType::Shadow;
		case TexelFormat::Depth24:
			return ElementType::Shadow;
		case TexelFormat::Depth32:
			return ElementType::Shadow;
		case TexelFormat::Depth32F:
			return ElementType::Shadow;
		case TexelFormat::Depth24Stencil8:
			return ElementType::Shadow;
		case TexelFormat::Stencil8:
			assert(false);
			return ElementType::ElementTypeCount;
		case TexelFormat::TexelFormatCount:
			assert(false);
			return ElementType::ElementTypeCount;
		default:
			assert(false);
			return ElementType::ElementTypeCount;
		}
	}

	uint32 GetTexelSizeInBytes(TexelFormat format)
	{
		switch (format)
		{
		case TexelFormat::R8:
			return 1;
		case TexelFormat::RG8:
			return 2;
		case TexelFormat::RGB8:
			return 3;
		case TexelFormat::RGBA8:
			return 4;
		case TexelFormat::R8I:
			return 1;
		case TexelFormat::RG8I:
			return 2;
		case TexelFormat::RGB8I:
			return 3;
		case TexelFormat::RGBA8I:
			return 4;
		case TexelFormat::R8UI:
			return 1;
		case TexelFormat::RG8UI:
			return 2;
		case TexelFormat::RGB8UI:
			return 3;
		case TexelFormat::RGBA8UI:
			return 4;
		case TexelFormat::R16I:
			return 2;
		case TexelFormat::RG16I:
			return 4;
		case TexelFormat::RGB16I:
			return 6;
		case TexelFormat::RGBA16I:
			return 8;
		case TexelFormat::R16UI:
			return 2;
		case TexelFormat::RG16UI:
			return 4;
		case TexelFormat::RGB16UI:
			return 6;
		case TexelFormat::RGBA16UI:
			return 8;
		case TexelFormat::R16F:
			return 2;
		case TexelFormat::RG16F:
			return 4;
		case TexelFormat::RGB16F:
			return 6;
		case TexelFormat::RGBA16F:
			return 8;
		case TexelFormat::R32I:
			return 4;
		case TexelFormat::RG32I:
			return 8;
		case TexelFormat::RGB32I:
			return 12;
		case TexelFormat::RGBA32I:
			return 16;
		case TexelFormat::R32UI:
			return 4;
		case TexelFormat::RG32UI:
			return 8;
		case TexelFormat::RGB32UI:
			return 12;
		case TexelFormat::RGBA32UI:
			return 16;
		case TexelFormat::R32F:
			return 4;
		case TexelFormat::RG32F:
			return 8;
		case TexelFormat::RGB32F:
			return 12;
		case TexelFormat::RGBA32F:
			return 16;
		case TexelFormat::BGR8:
			return 3;
		case TexelFormat::BGRA8:
			return 4;
		case TexelFormat::BGR16F:
			return 6;
		case TexelFormat::BGRA16F:
			return 8;
		case TexelFormat::BGR32F:
			return 12;
		case TexelFormat::BGRA32F:
			return 16;
		case TexelFormat::Depth16:
			return 2;
		case TexelFormat::Depth24:
			return 3;
		case TexelFormat::Depth32:
			return 4;
		case TexelFormat::Depth32F:
			return 4;
		case TexelFormat::Depth24Stencil8:
			return 4;
		case TexelFormat::Stencil8:
			return 1;
		case TexelFormat::TexelFormatCount:
			assert(false);
			return 0;
		default:
			assert(false);
			return 0;
		}
	}

	XREX_API TexelFormat GetCorrespondingTexelFormat(ElementType type)
	{
		switch (type)
		{
		case ElementType::Uint8:
			return TexelFormat::R8UI;
		case ElementType::Uint16:
			return TexelFormat::R16UI;
		case ElementType::Uint32:
			return TexelFormat::R32UI;
		case ElementType::Int8:
			return TexelFormat::R8I;
		case ElementType::Int16:
			return TexelFormat::R16I;
		case ElementType::Int32:
			return TexelFormat::R32I;
		case ElementType::IntV2:
			return TexelFormat::RG32I;
		case ElementType::IntV3:
			return TexelFormat::RGB32I;
		case ElementType::IntV4:
			return TexelFormat::RGBA32I;
		case ElementType::UintV2:
			return TexelFormat::RG32UI;
		case ElementType::UintV3:
			return TexelFormat::RGB32UI;
		case ElementType::UintV4:
			return TexelFormat::RGBA32UI;
		case ElementType::Float:
			return TexelFormat::R32F;
		case ElementType::FloatV2:
			return TexelFormat::RG32F;
		case ElementType::FloatV3:
			return TexelFormat::RGB32F;
		case ElementType::FloatV4:
			return TexelFormat::RGBA32F;


		case ElementType::ElementTypeCount:
			assert(false);
			return TexelFormat::TexelFormatCount;
		default:
			assert(false);
			return TexelFormat::TexelFormatCount;
		}
	}

}
