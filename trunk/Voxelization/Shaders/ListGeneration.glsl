
uniform mat4 modelMatrix;
uniform mat4 normalMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec3 cameraPosition;

layout (r32ui) uniform uimage2D heads; // should be initialized to have all 0s
layout (rgba32ui) uniform writeonly uimageBuffer nodePool; // no need to initialize
layout (binding = 0, offset = 0) uniform atomic_uint nodeCounter; // should be initialized to 1, 0 is used as null in VoxelGeneration

uniform int objectID; // start from 0
uniform float voxelVolumeHalfSize;

uniform int axis;

const int AxisX = 0;
const int AxisY = 1;
const int AxisZ = 2;


#ifdef VS

in vec3 position;

out vec3 vPosition;

void main()
{
	vPosition = (viewMatrix * modelMatrix * vec4(position, 1)).xyz;
	gl_Position = projectionMatrix * vec4(vPosition, 1);
}

#endif




#ifdef FS

vec3 TransformByAxis(vec3 value, int axis)
{ // TODO extract this out?
	switch (axis)
	{
	case AxisX: // from +x, +z is up
		return value.zxy;
	case AxisY: // from +y, +x is up
		return value.yzx;
	case AxisZ: // from +z, +y is up
		return value.xyz;
	}
}


void InsertToLinkedList(layout (r32ui) uimage2D heads, layout (rgba32ui) writeonly uimageBuffer nodePool, atomic_uint nodeCounter, ivec2 coordinate, float depth, int objectID, bool frontFacing, vec3 value)
{
	uint newHead = atomicCounterIncrement(nodeCounter);
	uint oldHead = imageAtomicExchange(heads, coordinate, newHead);
	uint third = objectID | (frontFacing ? (1 << 31) : 0); // frontFacing at highest bit
	imageStore(nodePool, int(newHead), uvec4(oldHead, floatBitsToUint(depth), third, packSnorm4x8(vec4(value, 1))));
}


in vec3 vPosition;

void main()
{
	// gl_FragCoord.xy will round to integer as it has form of (x.5, y.5)
	// gl_fragCoord.z is linear, due to orthogonal projection. range [0, 1]
	InsertToLinkedList(heads, nodePool, nodeCounter, ivec2(gl_FragCoord.xy), gl_FragCoord.z, objectID, gl_FrontFacing, TransformByAxis(vPosition / voxelVolumeHalfSize, axis));
}

#endif

