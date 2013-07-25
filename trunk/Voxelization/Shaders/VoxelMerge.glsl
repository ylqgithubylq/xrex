
layout (r32ui) uniform readonly uimage3D intermediateVolume;
layout (rgba32f) uniform writeonly image3D volume;



// #ifdef VS
// 
// in vec2 position;
// 
// void main()
// {
// 	gl_Position = vec4(position, 0, 1);
// }
// 
// #endif

// #ifdef FS
// 
// void main()
// {
// 	int volumeSize = imageSize(intermediateVolume).x;
// 	ivec2 coordinate = ivec2(gl_FragCoord.xy);
// 	for (int i = 0; i < volumeSize; ++i)
// 	{
// 		ivec3 coordinate3D = ivec3(coordinate, i);
// 		uvec4 intermediateVoxel = imageLoad(intermediateVolume, coordinate3D);
// 
// 		imageStore(volume, coordinate3D, intermediateVoxel.xxxx);
// 	}
// }
// 
// #endif


#ifdef CS

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void main()
{
	int volumeSize = imageSize(intermediateVolume).x;
	ivec2 coordinate = ivec3(gl_GlobalInvocationID).xy;
	for (int i = 0; i < volumeSize; ++i)
	{
		ivec3 coordinate3D = ivec3(coordinate, i);
		uvec4 intermediateVoxel = imageLoad(intermediateVolume, coordinate3D);
		vec4 unpacked = unpackUnorm4x8(intermediateVoxel.x);
		imageStore(volume, coordinate3D, vec4(unpacked.xyz, intermediateVoxel.w));
	}
}

#endif

