#include "XREX.hpp"

#include "Util.hpp"

#include <vector>
#include <utility>

using std::vector;

namespace
{
	vector<uint32> InitializeElementPrimitiveCountMap()
	{
		vector<uint32> temp(static_cast<uint32>(ElementType::ParameterTypeCount));
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
		return std::move(temp);
	}

	vector<uint32> InitializeElementSizeMap()
	{
		vector<uint32> temp(static_cast<uint32>(ElementType::ParameterTypeCount));
		temp[static_cast<uint32>(ElementType::Void)] = 0;
		temp[static_cast<uint32>(ElementType::Bool)] = sizeof(bool);
		temp[static_cast<uint32>(ElementType::Uint8)] = sizeof(uint8);
		temp[static_cast<uint32>(ElementType::Uint16)] = sizeof(uint16);
		temp[static_cast<uint32>(ElementType::Uint32)] = sizeof(uint32);
		temp[static_cast<uint32>(ElementType::Int8)] = sizeof(int8);
		temp[static_cast<uint32>(ElementType::Int16)] = sizeof(int16);
		temp[static_cast<uint32>(ElementType::Int32)] = sizeof(int32);
		temp[static_cast<uint32>(ElementType::Float)] = sizeof(float);
		temp[static_cast<uint32>(ElementType::FloatV2)] = sizeof(floatV2);
		temp[static_cast<uint32>(ElementType::FloatV3)] = sizeof(floatV3);
		temp[static_cast<uint32>(ElementType::FloatV4)] = sizeof(floatV4);
		temp[static_cast<uint32>(ElementType::FloatM44)] = sizeof(floatM44);
		temp[static_cast<uint32>(ElementType::Sampler1D)] = sizeof(int32);
		temp[static_cast<uint32>(ElementType::Sampler2D)] = sizeof(int32);
		temp[static_cast<uint32>(ElementType::Sampler3D)] = sizeof(int32);
		temp[static_cast<uint32>(ElementType::SamplerCube)] = sizeof(int32);
		return std::move(temp);
	}

	vector<ElementType> InitializeElementPrimitiveMap()
	{
		vector<ElementType> temp(static_cast<uint32>(ElementType::ParameterTypeCount));
		temp[static_cast<ElementType>(ElementType::Void)] = ElementType::Void;
		temp[static_cast<ElementType>(ElementType::Bool)] = ElementType::Bool;
		temp[static_cast<ElementType>(ElementType::Uint8)] = ElementType::Uint8;
		temp[static_cast<ElementType>(ElementType::Uint16)] = ElementType::Uint16;
		temp[static_cast<ElementType>(ElementType::Uint32)] = ElementType::Uint32;
		temp[static_cast<ElementType>(ElementType::Int8)] = ElementType::Int8;
		temp[static_cast<ElementType>(ElementType::Int16)] = ElementType::Int16;
		temp[static_cast<ElementType>(ElementType::Int32)] = ElementType::Int32;
		temp[static_cast<ElementType>(ElementType::Float)] = ElementType::Float;
		temp[static_cast<ElementType>(ElementType::FloatV2)] = ElementType::Float;
		temp[static_cast<ElementType>(ElementType::FloatV3)] = ElementType::Float;
		temp[static_cast<ElementType>(ElementType::FloatV4)] = ElementType::Float;
		temp[static_cast<ElementType>(ElementType::FloatM44)] = ElementType::Float;
		temp[static_cast<ElementType>(ElementType::Sampler1D)] = ElementType::Void;
		temp[static_cast<ElementType>(ElementType::Sampler2D)] = ElementType::Void;
		temp[static_cast<ElementType>(ElementType::Sampler3D)] = ElementType::Void;
		temp[static_cast<ElementType>(ElementType::SamplerCube)] = ElementType::Void;
		return std::move(temp);
	}
}

uint32 GetElementPrimitiveCount(ElementType type)
{
	static vector<uint32> counts = InitializeElementPrimitiveCountMap();
	assert(type != ElementType::Void);
	return counts[static_cast<uint32>(type)];
}

uint32 GetElementSizeInByte(ElementType type)
{
	static vector<uint32> sizes = InitializeElementSizeMap();
	assert(type != ElementType::Void);
	return sizes[static_cast<uint32>(type)];
}

ElementType GetElementPrimitiveType(ElementType type)
{
	static vector<ElementType> primitiveTypes = InitializeElementPrimitiveMap();
	ElementType primitiveType = primitiveTypes[static_cast<uint32>(type)];
	assert(primitiveType != ElementType::Void);
	return primitiveType;
}
