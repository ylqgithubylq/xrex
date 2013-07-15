
uniform mat4 modelMatrix;
uniform mat4 normalMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec3 cameraPosition;

layout (r32ui) uniform uimage2D header; // should be initialized to have all 0s
layout (rgba32ui) uniform writeonly uimageBuffer nodePool; // no need to initialize
layout (binding = 0, offset = 0) uniform atomic_uint nodeCounter; // should be initialized to 1, 0 is used as null in VoxelGeneration


#ifdef VS

in vec3 position;

void main()
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1);
}

#endif


void InsertToLinkedList(layout (r32ui) uimage2D header, layout (rgba32ui) writeonly uimageBuffer nodePool, atomic_uint nodeCounter, ivec2 coordinate, float depth, bool frontFacing)
{
	uint newHead = atomicCounterIncrement(nodeCounter);
	uint oldHead = imageAtomicExchange(header, coordinate, newHead);
	imageStore(nodePool, int(newHead), uvec4(oldHead, floatBitsToUint(depth), uint(frontFacing), 0));
}


#ifdef FS

void main()
{
	// gl_FragCoord.xy will round to integer as it has form of (x.5, y.5)
	// gl_fragCoord.z is linear, due to orthogonal projection. range [0, 1]
	InsertToLinkedList(header, nodePool, nodeCounter, ivec2(gl_FragCoord.xy), gl_FragCoord.z, gl_FrontFacing);
}

#endif

