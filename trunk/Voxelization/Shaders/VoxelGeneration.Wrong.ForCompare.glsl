
layout (r32ui) uniform readonly uimage2D heads;
layout (rgba32ui) uniform readonly uimageBuffer nodePool;
layout (rgba8) uniform writeonly image3D volume;

uniform PerAxis
{
	int axis;
};

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


void StoreFinal(layout (rgba8) writeonly image3D volume, ivec3 coordinate, vec4 unpackedValue)
{
	imageStore(volume, coordinate, unpackedValue);
}


#define SOLID

void main()
{
	ivec2 coordinate = ivec2(gl_FragCoord.xy);
	
	uvec4 nodeCache[InsertionSortThresholdCount];
	uint totalCount = InsertionSortPass(heads, nodePool, coordinate, nodeCache);

	const int countToCalculate = int(min(totalCount, InsertionSortThresholdCount));
	// insert voxels for each pair
	for (int i = 0; i < countToCalculate - 1; ++i, ++i)
	{
		Iterator current = Iterator(nodeCache[i]);
		float currentDepth = GetDepth(current);
		uint currentPackedValue = GetPackedValue(current);
		vec4 currentUnpackedValue = unpackSnorm4x8(currentPackedValue);
		int currentLocation = DepthToCoordinate(heads, currentDepth);

		Iterator next = Iterator(nodeCache[i + 1]);
		float nextDepth = GetDepth(next);
		uint nextPackedValue = GetPackedValue(next);
		vec4 nextUnpackedValue = unpackSnorm4x8(nextPackedValue);
		int nextLocation = DepthToCoordinate(heads, nextDepth);

		// note: nextLocation < currentLocation
		for (int j = nextLocation; j <= currentLocation; ++j)
		{
			// linear interpolate from current grid to next grid
			vec4 mixedValue = mix(currentUnpackedValue, nextUnpackedValue, float(j - nextLocation) / (currentLocation - nextLocation));
			StoreFinal(volume, TransformToVolumeCoordinate(ivec3(coordinate, j), axis), mixedValue);
		}
	}

	// last unpaired one if exist
	if (countToCalculate % 2 != 0)
	{
		ivec2 volumeSize = imageSize(heads);

		Iterator last = Iterator(nodeCache[countToCalculate - 1]);
		float lastDepth = GetDepth(last);
		uint lastPackedValue = GetPackedValue(last);
		vec4 lastUnpackedValue = unpackSnorm4x8(lastPackedValue);
		int lastLocation = DepthToCoordinate(heads, lastDepth);

		for (int i = lastLocation; i < volumeSize.x; ++i)
		{
			StoreFinal(volume, TransformToVolumeCoordinate(ivec3(coordinate, i), axis), lastUnpackedValue);
		}
	}



}

#endif

