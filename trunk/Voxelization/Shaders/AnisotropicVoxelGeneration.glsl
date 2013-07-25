
layout (r32ui) uniform readonly uimage2D heads; // edge size twice larges than intermediateVolume
layout (rgba32ui) uniform readonly uimageBuffer nodePool;
layout (r32ui) uniform uimage3D volume;

uniform int axis;

const int AxisX = 0;
const int AxisY = 1;
const int AxisZ = 2;

#ifdef VS

in vec2 position;

void main()
{
	gl_Position = vec4(position, 0, 1);
}

#endif

#ifdef FS




struct Iterator
{
	uvec4 value;
};


float GetDepth(Iterator i)
{
	return uintBitsToFloat(i.value.y);
}


bool Compare(uvec4 left, uvec4 right)
{
	return GetDepth(Iterator(left)) < GetDepth(Iterator(right));
}


Iterator First(layout (r32ui) readonly uimage2D heads, ivec2 coordinate)
{
	return Iterator(imageLoad(heads, coordinate));
}

bool HasNext(Iterator current)
{
	return current.value.x != 0;
}

Iterator Next(layout (rgba32ui) readonly uimageBuffer nodePool, Iterator current)
{
	return Iterator(imageLoad(nodePool, int(current.value.x)));
}

Iterator Current(layout (rgba32ui) readonly uimageBuffer nodePool, uint current)
{
	return Iterator(imageLoad(nodePool, int(current)));
}

uint NextAddress(Iterator current)
{
	return current.value.x;
}

struct Cache
{
	uint memoryIndex;
	uint cacheIndex;
};

const uint InsertionSortThresholdCount = 64;

void InsertionSort(inout uvec4 toSort[InsertionSortThresholdCount], uint start, uint end)
{
	for (uint j = start + 1; j < end; ++j)
	{
		uvec4 value = toSort[j];
		int i = int(j - 1);
		while (i >= 0 && Compare(value, toSort[i]))
		{
			toSort[i + 1] = toSort[i];
			i -= 1;
		}
		toSort[i + 1] = value;
	}
}

uint InsertionSortPass(layout (r32ui) readonly uimage2D heads, layout (rgba32ui) readonly uimageBuffer nodePool, ivec2 coordinate, inout uvec4 nodeCache[InsertionSortThresholdCount])
{
	uint totalCount = 0;

	bool firstWrite = true;

	Cache lastIndex = Cache(0, 0);
	uvec4 lastNode = uvec4(0, 0, 0, 0);

	Iterator current = First(heads, coordinate);

	uint countInPass = 0;
	for (; HasNext(current) && countInPass < InsertionSortThresholdCount; ++countInPass)
	{
		current = Next(nodePool, current);
		nodeCache[countInPass] = current.value;
		totalCount += 1;
	}
	while (HasNext(current) == true)
	{ // just count the remaining
		current = Next(nodePool, current);
		totalCount += 1;
	}

	InsertionSort(nodeCache, 0, countInPass);

	return totalCount;
}

uint GetObjectIDAndFrontFacing(Iterator i)
{
	return i.value.z;
}

uint GetPackedValue(Iterator i)
{
	return i.value.w;
}

bool GetFrontFacing(Iterator i)
{
	return bool(i.value.z & (1 << 31)); // frontFacing at highest bit
}
bool GetFrontFacing(uint value)
{
	return bool(value & (1 << 31)); // frontFacing at highest bit
}

int GetObjectID(Iterator i)
{
	return int(i.value.z & (0x7fffffff)); // first bit is frontFacing
}
int GetObjectID(uint value)
{
	return int(value & (0x7fffffff)); // first bit is frontFacing
}

int DepthToCoordinate(layout (r32ui) readonly uimage2D heads, float depth)
{
	ivec2 size = imageSize(heads);
	return int((1 - depth) * size.x);
}

ivec3 TransformToVolumeCoordinate(ivec3 originalCoordinate, int axis)
{ // TODO extract this out?
	switch (axis)
	{
	case AxisX: // from +x, +z is up
		return originalCoordinate.zxy;
	case AxisY: // from +y, +x is up
		return originalCoordinate.yzx;
	case AxisZ: // from +z, +y is up
		return originalCoordinate.xyz;
	}
}

vec4 RGBAFromRGBA8(uint value)
{
	return vec4(float((value & 0x000000FF) >> 0U), float((value & 0x0000FF00) >> 8U), float((value & 0x00FF0000) >> 16U), float((value & 0xFF000000) >> 24U)) / 255;
}

uint RGBA8FromRGBA(vec4 value)
{
	value *= 255;
	return (uint(value.w) & 0x000000FF) << 24U | (uint(value.z) & 0x000000FF) << 16U | (uint(value.y) & 0x000000FF) << 8U | (uint(value.x) & 0x000000FF);
}

void StoreFinal(layout (r32ui) uimage3D volume, ivec3 coordinate, int axis, float value)
{
	uint uintValue = uint(value * 255) << 8 * axis;
	imageAtomicOr(volume, coordinate, uintValue);
}

const int ObjectUnpairedMaxCount = 32;

struct Unpaired
{
	uint objectIDAndFrontFacing;
	float depth;
	uint packedValue;
};


/*
 *	@return: depth -1 indicates unpaired.
 */
Unpaired Pairing(inout Unpaired objectUnpairedBuffer[ObjectUnpairedMaxCount], inout int size, uint objectIDAndFrontFacing, float depth, uint packedValue)
{
	Unpaired paired = Unpaired(0, -1, 0);

	int objectID = GetObjectID(objectIDAndFrontFacing);
	bool frontFacing = GetFrontFacing(objectIDAndFrontFacing);
	// find
	int pairedIndex = -1;
	if (frontFacing == false) // only back facing fragment need to find pair
	{
		for (int i = size - 1; i >= 0; --i)
		{
			//if (GetFrontFacing(objectUnpairedBuffer[i].objectIDAndFrontFacing) != frontFacing && GetObjectID(objectUnpairedBuffer[i].objectIDAndFrontFacing) == objectID)
			if (GetFrontFacing(objectUnpairedBuffer[i].objectIDAndFrontFacing) != frontFacing)
			//if (GetObjectID(objectUnpairedBuffer[i].objectIDAndFrontFacing) == objectID)
			{ // found
				pairedIndex = i;
				break;
			}
		}
	}

	if (pairedIndex != -1)
	{ // found, remove paired one if frontFacing is not equal
		paired = objectUnpairedBuffer[pairedIndex];
		for (int i = pairedIndex; i < size - 1; ++i)
		{
			objectUnpairedBuffer[i] = objectUnpairedBuffer[i + 1];
		}
		size -= 1;
	}
	else if (size < ObjectUnpairedMaxCount)
	{ // insert
		objectUnpairedBuffer[size] = Unpaired(objectIDAndFrontFacing, depth, packedValue);
		size += 1;
	} // else simply discarded

	return paired;
}


#define SOLID

void main()
{
	ivec2 coordinate = ivec2(gl_FragCoord.xy);
	Iterator i = First(heads, coordinate);
	
	Unpaired objectUnpairedBuffer[ObjectUnpairedMaxCount];
	int unpairedCount = 0;

	uvec4 nodeCache[InsertionSortThresholdCount];
	uint totalCount = InsertionSortPass(heads, nodePool, coordinate, nodeCache);


	for (int i = 0; i < min(totalCount, InsertionSortThresholdCount); ++i)
	{
		Iterator current = Iterator(nodeCache[i]);
		float depth = GetDepth(current);
		uint objectIDAndFrontFacing = GetObjectIDAndFrontFacing(current);
		bool frontFacing = GetFrontFacing(current);
		int objectID = GetObjectID(current);
		uint packedValue = GetPackedValue(current);
		vec4 unpackedValue = unpackSnorm4x8(packedValue);
		int currentLocation = DepthToCoordinate(heads, depth);
		ivec3 originalCoordinate = ivec3(coordinate, currentLocation);
#ifdef SOLID
		Unpaired pairingResult = Pairing(objectUnpairedBuffer, unpairedCount, objectIDAndFrontFacing, depth, packedValue);
		if (pairingResult.depth != -1)
		{
			int pairedGrid = DepthToCoordinate(heads, pairingResult.depth);
			vec4 pairedUnpackedValue = unpackSnorm4x8(pairingResult.packedValue);
			for (int j = currentLocation; j <= pairedGrid; ++j)
			{
				// linear interpolate from paired grid to current grid
				vec4 finalValue = mix(pairedUnpackedValue, unpackedValue, float(j - currentLocation) / (pairedGrid - currentLocation));
				StoreFinal(volume, TransformToVolumeCoordinate(ivec3(coordinate, j), axis), axis, 1);
			}
		}
#else
		StoreFinal(volume, TransformToVolumeCoordinate(ivec3(coordinate, currentLocation), axis), axis, 1);
#endif
	}

	// surface voxelize all unpaired fragments
	for (int i = 0; i < unpairedCount; ++i)
	{
		int location = DepthToCoordinate(heads, objectUnpairedBuffer[i].depth);
		vec4 unpacked = unpackSnorm4x8(objectUnpairedBuffer[i].packedValue);
		StoreFinal(volume, TransformToVolumeCoordinate(ivec3(coordinate, location), axis), axis, 1);
	}


}

#endif

