
layout (r32ui) uniform readonly uimage2D header; // edge size twice larges than intermediateVolume
layout (rgba32ui) uniform readonly uimageBuffer nodePool;
layout (r32ui) uniform writeonly uimage3D intermediateVolume;

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

Iterator First(layout (r32ui) readonly uimage2D header, ivec2 coordinate)
{
	return Iterator(imageLoad(header, coordinate));
}

bool HasNext(Iterator current)
{
	return current.value.x != 0;
}

Iterator Next(layout (rgba32ui) readonly uimageBuffer nodePool, Iterator current)
{
	return Iterator(imageLoad(nodePool, int(current.value.x)));
}

float GetDepth(Iterator i)
{
	return uintBitsToFloat(i.value.y);
}

bool GetFrontFacing(Iterator i)
{
	return bool(i.value.z);
}

int DepthToCoordinate(layout (r32ui) readonly uimage2D header, float depth)
{
	ivec2 size = imageSize(header);
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

void Do(layout (r32ui) writeonly uimage3D intermediateVolume, ivec3 coordinate, bool frontFacing)
{
	// TODO insert into volume
	imageStore(intermediateVolume, coordinate / 2, uvec4(1, 0, 0, 0));
}

void main()
{
	ivec2 coordinate = ivec2(gl_FragCoord.xy);
	Iterator i = First(header, coordinate);
	while (HasNext(i) == true) // write HasNext(i) directly will crash the NV driver
	{
		i = Next(nodePool, i);
		float depth = GetDepth(i);
		bool frontFacing = GetFrontFacing(i);
		ivec3 originalCoordinate = ivec3(coordinate, DepthToCoordinate(header, depth));
		Do(intermediateVolume, TransformToVolumeCoordinate(originalCoordinate, axis), frontFacing);
	}
}

#endif

