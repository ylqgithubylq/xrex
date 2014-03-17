
layout (r32ui) uniform uimage2D heads; // should be initialized to have all 0s
layout (rgba32ui) uniform writeonly uimageBuffer nodePool; // no need to initialize
layout (binding = 0, offset = 0) uniform atomic_uint nodeCounter; // should be initialized to 1, 0 is used as null in VoxelGeneration

uniform sampler2D diffuseMap;

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

const float VoxelAlpha = 1;


#ifdef VS

in vec3 position;
in vec3 textureCoordinate0;

out vec3 wPosition;
out	vec2 pixelTextureCoordinate;


void main()
{
	wPosition = XREX_Transform(XREX_ModelTransformation.WorldFromModel, position);
	gl_Position = XREX_TransformToClip(XREX_ModelTransformation.ClipFromModel, position);
	pixelTextureCoordinate = textureCoordinate0.st;
}

#endif

const float OpticalDepthMax = 6; // alpha = 1 - e^(-optical depth)

// #define STORE_OPTICAL_DEPTH // define this to store optical depth instead of alpha



#ifdef FS

/*
 *	Alpha to optical depth, mapped from [0, OpticalDepthMax] to [0, 1]
 */
float AlphaToPackedOpticalDepth(float alpha)
{
	return -log(1 - alpha) / OpticalDepthMax;
}

/*
 *	mapped optical depth to alpha, optical depth mapped from [0, OpticalDepthMax] to [0, 1]
 */
float PackedOpticalDepthToAlpha(float packedOpticalDepth) // used in ConeTracing, listed here as a reference
{
	return 1 - exp(-packedOpticalDepth * OpticalDepthMax);
}

vec4 InsertToLinkedList(layout (r32ui) uimage2D heads, layout (rgba32ui) writeonly uimageBuffer nodePool, atomic_uint nodeCounter, ivec2 coordinate, float depth, int objectID, bool frontFacing, vec3 value)
{
	uint newHead = atomicCounterIncrement(nodeCounter);
	uint oldHead = imageAtomicExchange(heads, coordinate, newHead);
	uint third = objectID | (frontFacing ? (1 << 31) : 0); // frontFacing at highest bit
#ifdef STORE_OPTICAL_DEPTH
	vec4 valueToStore = vec4(value * VoxelAlpha, AlphaToPackedOpticalDepth(VoxelAlpha));
#else
	vec4 valueToStore = vec4(value, 1) * VoxelAlpha;
#endif
	imageStore(nodePool, int(newHead), uvec4(oldHead, floatBitsToUint(depth), third, packSnorm4x8(valueToStore)));
	return vec4(value, 1)/* + vec4(1, 0, 1, 0)*/;
}


in vec3 wPosition;
in vec2 pixelTextureCoordinate;


out vec4 XREX_DefaultFrameBufferOutput;

void main()
{
	vec3 valueToStore = texture(diffuseMap, pixelTextureCoordinate).rgb; // diffuse color
	// vec3 valueToStore = (wPosition - voxelVolumeCenter) / voxelVolumeHalfSize; // position as color
	// gl_FragCoord.xy will round to integer as it has form of (x.5, y.5)
	// gl_fragCoord.z is linear, due to orthogonal projection. range [0, 1]
	XREX_DefaultFrameBufferOutput = InsertToLinkedList(heads, nodePool, nodeCounter, ivec2(gl_FragCoord.xy), gl_FragCoord.z, objectID, gl_FrontFacing, valueToStore);
}

#endif

