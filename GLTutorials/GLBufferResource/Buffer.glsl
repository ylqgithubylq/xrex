#version 430 core

// input layout qualifier declaring a 1 (x 1 x 1) local work group size
layout (local_size_x = 1) in;

// define the input and output resources

uniform MyUniformBuffer // this is the name used for CPU side
{
	float someFloat;
	vec4 someVector4;
} UniformBufferNameInShader; // this is the name used here

struct MyStruct
{
	uint structUint;
};

buffer MyShaderStorageBuffer // this is the name used for CPU side
{
	ivec2[2] someIntVector2Array;
	int[2] someIntArray;
	MyStruct[] someMyStructArray;
} ShaderStorageBufferNameInShader; // this is the name used here


void main()
{
	// make sure these buffers are not optimized away by the compiler
	int length = ShaderStorageBufferNameInShader.someMyStructArray.length();
	uint id = gl_GlobalInvocationID.x;
	ShaderStorageBufferNameInShader.someIntArray[0] = length;
	ShaderStorageBufferNameInShader.someIntArray[1] = length;
	ShaderStorageBufferNameInShader.someMyStructArray[id].structUint = uint(UniformBufferNameInShader.someFloat) + id;
	// ShaderStorageBufferNameInShader.someFloatArraySSBO[gl_GlobalInvocationID.x]
	// -> write like this will cause NVidia's compiler fail to compile.
}
