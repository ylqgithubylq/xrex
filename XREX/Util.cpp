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
		case ElementType::Sampler2D:
		case ElementType::Sampler3D:
		case ElementType::SamplerCube:
			return true;
		case ElementType::ParameterTypeCount:
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
		case ElementType::Image2D:
		case ElementType::Image3D:
		case ElementType::ImageCube:
			return true;
		case ElementType::ParameterTypeCount:
			assert(false);
			return false;
		default:
			return false;
		}
	}

	XREX_API uint32 GetElementPrimitiveCount(ElementType type)
	{
		static array<uint32, static_cast<uint32>(ElementType::ParameterTypeCount)> const counts = [] ()
		{
			array<uint32, static_cast<uint32>(ElementType::ParameterTypeCount)> temp;
			temp[static_cast<uint32>(ElementType::Void)] = 0;
			temp[static_cast<uint32>(ElementType::Bool)] = 1;
			temp[static_cast<uint32>(ElementType::Uint8)] = 1;
			temp[static_cast<uint32>(ElementType::Uint16)] = 1;
			temp[static_cast<uint32>(ElementType::Uint32)] = 1;
			temp[static_cast<uint32>(ElementType::Int8)] = 1;
			temp[static_cast<uint32>(ElementType::Int16)] = 1;
			temp[static_cast<uint32>(ElementType::Int32)] = 1;
			temp[static_cast<uint32>(ElementType::Float)] = 1;
			temp[static_cast<uint32>(ElementType::FloatV2)] = 2;
			temp[static_cast<uint32>(ElementType::FloatV3)] = 3;
			temp[static_cast<uint32>(ElementType::FloatV4)] = 4;
			temp[static_cast<uint32>(ElementType::FloatM44)] = 16;
			temp[static_cast<uint32>(ElementType::Sampler1D)] = 1;
			temp[static_cast<uint32>(ElementType::Sampler2D)] = 1;
			temp[static_cast<uint32>(ElementType::Sampler3D)] = 1;
			temp[static_cast<uint32>(ElementType::SamplerCube)] = 1;
			return temp;
		} ();
		assert(type != ElementType::Void);
		return counts[static_cast<uint32>(type)];
	}

	XREX_API uint32 GetElementSizeInBytes(ElementType type)
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
		case ElementType::Sampler1D:
		case ElementType::Sampler2D:
		case ElementType::Sampler3D:
		case ElementType::SamplerCube:
		case ElementType::Image1D:
		case ElementType::Image2D:
		case ElementType::Image3D:
		case ElementType::ImageCube:
			return sizeof(int32);
		case ElementType::ParameterTypeCount:
			assert(false);
			return 0;
		default:
			assert(false);
			return 0;
		}
	}

	XREX_API ElementType GetElementPrimitiveType(ElementType type)
	{
		static array<ElementType, static_cast<uint32>(ElementType::ParameterTypeCount)> const primitiveTypes = [] ()
		{
			array<ElementType, static_cast<uint32>(ElementType::ParameterTypeCount)> temp;
			temp[static_cast<uint32>(ElementType::Void)] = ElementType::Void;
			temp[static_cast<uint32>(ElementType::Bool)] = ElementType::Bool;
			temp[static_cast<uint32>(ElementType::Uint8)] = ElementType::Uint8;
			temp[static_cast<uint32>(ElementType::Uint16)] = ElementType::Uint16;
			temp[static_cast<uint32>(ElementType::Uint32)] = ElementType::Uint32;
			temp[static_cast<uint32>(ElementType::Int8)] = ElementType::Int8;
			temp[static_cast<uint32>(ElementType::Int16)] = ElementType::Int16;
			temp[static_cast<uint32>(ElementType::Int32)] = ElementType::Int32;
			temp[static_cast<uint32>(ElementType::Float)] = ElementType::Float;
			temp[static_cast<uint32>(ElementType::FloatV2)] = ElementType::Float;
			temp[static_cast<uint32>(ElementType::FloatV3)] = ElementType::Float;
			temp[static_cast<uint32>(ElementType::FloatV4)] = ElementType::Float;
			temp[static_cast<uint32>(ElementType::FloatM44)] = ElementType::Float;
			temp[static_cast<uint32>(ElementType::Sampler1D)] = ElementType::Void;
			temp[static_cast<uint32>(ElementType::Sampler2D)] = ElementType::Void;
			temp[static_cast<uint32>(ElementType::Sampler3D)] = ElementType::Void;
			temp[static_cast<uint32>(ElementType::SamplerCube)] = ElementType::Void;
			return temp;
		} ();
		ElementType primitiveType = primitiveTypes[static_cast<uint32>(type)];
		assert(primitiveType != ElementType::Void);
		return primitiveType;
	}

	uint32 GetTexelSizeInBytes(TexelFormat format)
	{
		switch (format)
		{
		case XREX::TexelFormat::R8:
			return 1;
		case XREX::TexelFormat::RG8:
			return 2;
		case XREX::TexelFormat::RGB8:
			return 3;
		case XREX::TexelFormat::RGBA8:
			return 4;
		case XREX::TexelFormat::R16I:
			return 2;
		case XREX::TexelFormat::RG16I:
			return 4;
		case XREX::TexelFormat::RGB16I:
			return 6;
		case XREX::TexelFormat::RGBA16I:
			return 8;
		case XREX::TexelFormat::R16UI:
			return 2;
		case XREX::TexelFormat::RG16UI:
			return 4;
		case XREX::TexelFormat::RGB16UI:
			return 6;
		case XREX::TexelFormat::RGBA16UI:
			return 8;
		case XREX::TexelFormat::R16F:
			return 2;
		case XREX::TexelFormat::RG16F:
			return 4;
		case XREX::TexelFormat::RGB16F:
			return 6;
		case XREX::TexelFormat::RGBA16F:
			return 8;
		case XREX::TexelFormat::R32I:
			return 4;
		case XREX::TexelFormat::RG32I:
			return 8;
		case XREX::TexelFormat::RGB32I:
			return 12;
		case XREX::TexelFormat::RGBA32I:
			return 16;
		case XREX::TexelFormat::R32UI:
			return 4;
		case XREX::TexelFormat::RG32UI:
			return 8;
		case XREX::TexelFormat::RGB32UI:
			return 12;
		case XREX::TexelFormat::RGBA32UI:
			return 16;
		case XREX::TexelFormat::R32F:
			return 4;
		case XREX::TexelFormat::RG32F:
			return 8;
		case XREX::TexelFormat::RGB32F:
			return 12;
		case XREX::TexelFormat::RGBA32F:
			return 16;
		case XREX::TexelFormat::BGR8:
			return 3;
		case XREX::TexelFormat::BGRA8:
			return 4;
		case XREX::TexelFormat::BGR16F:
			return 6;
		case XREX::TexelFormat::BGRA16F:
			return 8;
		case XREX::TexelFormat::BGR32F:
			return 12;
		case XREX::TexelFormat::BGRA32F:
			return 16;
		case XREX::TexelFormat::NotUsed:
			assert(false);
			return 0;
		case XREX::TexelFormat::TexelFormatCount:
			assert(false);
			return 0;
		default:
			assert(false);
			return 0;
		}
	}

}
