
layout (r32ui) uniform uimage2D heads; // should be initialized to have all 0s
layout (rgba32ui) uniform writeonly uimageBuffer nodePool; // no need to initialize
layout (binding = 0, offset = 0) uniform atomic_uint nodeCounter; // should be initialized to 1, 0 is used as null in VoxelGeneration

uniform PerObject
{
	int objectID; // start from 0
};

uniform PerAxis
{
	int axis;
};

uniform NeverChanged
{
	float voxelVolumeHalfSize;
	vec3 voxelVolumeCenter;
};


const int AxisX = 0;
const int AxisY = 1;
const int AxisZ = 2;


#ifdef VS

in vec3 position;

out vec3 wPosition;

void main()
{
	wPosition = XREX_Transform(XREX_ModelTransformation.WorldFromModel, position);
	gl_Position = XREX_TransformToClip(XREX_ModelTransformation.ClipFromModel, position);
}

#endif




#ifdef FS


vec4 InsertToLinkedList(layout (r32ui) uimage2D heads, layout (rgba32ui) writeonly uimageBuffer nodePool, atomic_uint nodeCounter, ivec2 coordinate, float depth, int objectID, bool frontFacing, vec3 value)
{
	uint newHead = atomicCounterIncrement(nodeCounter);
	uint oldHead = imageAtomicExchange(heads, coordinate, newHead);
	uint third = objectID | (frontFacing ? (1 << 31) : 0); // frontFacing at highest bit
	imageStore(nodePool, int(newHead), uvec4(oldHead, floatBitsToUint(depth), third, packSnorm4x8(vec4(value, 1))));
	return vec4(value, 1)/* + vec4(1, 0, 1, 0)*/;
}


in vec3 wPosition;

out vec4 XREX_DefaultFrameBufferOutput;

void main()
{
	// gl_FragCoord.xy will round to integer as it has form of (x.5, y.5)
	// gl_fragCoord.z is linear, due to orthogonal projection. range [0, 1]
	XREX_DefaultFrameBufferOutput = InsertToLinkedList(heads, nodePool, nodeCounter, ivec2(gl_FragCoord.xy), gl_FragCoord.z, objectID, gl_FrontFacing, (wPosition - voxelVolumeCenter) / voxelVolumeHalfSize);
}

#endif

