#include "XREXAll.hpp"
#include "ShaderCodes.h"


ShaderCodes::ShaderCodes()
{
	codes.emplace_back(
	"\n\
	vec3 ReturnSelf(vec3 v)\n\
	{\n\
		return v;\n\
	}\n\
	"
		);

	codes.emplace_back(
	"\n\
	uniform mat4 modelMatrix;\n\
	uniform mat4 normalMatrix;\n\
	uniform mat4 viewMatrix;\n\
	uniform mat4 projectionMatrix;\n\
	uniform vec3 cameraPosition;\n\
	\n\
	layout (rgba32f) uniform image2D testImage0;\n\
	layout (rgba32f) uniform image2D testImage1;\n\
	uniform sampler2D notUsedTexture0;\n\
	uniform sampler2D notUsedTexture1;\n\
	uniform sampler3D test3DTexture;\n\
	\n\
	#ifdef VS\n\
	\n\
	in vec3 position;\n\
	in vec3 normal;\n\
	in vec3 textureCoordinate0;\n\
	out vec3 eNormal;\n\
	out vec2 pixelTextureCoordinate;\n\
	out vec3 mPosition;\n\
	\n\
	void main()\n\
	{\n\
		//position = ReturnSelf(position);\n\
		mPosition = position;\n\
		eNormal = viewMatrix * modelMatrix * vec4(normal, 0);\n\
		gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);\n\
		pixelTextureCoordinate = textureCoordinate0.st;\n\
		//gl_Position += texture(notUsedTexture, pixelTextureCoordinate) * 0.0001;\n\
	}\n\
	\n\
	#endif\n\
	\n\
	#ifdef FS\n\
	\n\
	in vec3 mPosition;\n\
	in vec3 eNormal;\n\
	in vec2 pixelTextureCoordinate;\n\
	\n\
	layout(location = 0) out vec4 finalColor;\n\
	\n\
	void main()\n\
	{\n\
		//ivec2 size = textureSize(bumpTexture, 0); // image has same size with bumpTexture \n\
		ivec2 size = imageSize(testImage0);\n\
		//vec4 loadResult = imageLoad(testImage, ivec2(gl_FragCoord.xy * size));\n\
		vec4 loadResult0 = imageLoad(testImage0, ivec2(pixelTextureCoordinate * size));\n\
		vec4 loadResult1 = imageLoad(testImage1, ivec2(pixelTextureCoordinate * size));\n\
		//vec4 loadResult0 = imageLoad(testImage0, ivec2(0, 0));\n\
		//vec4 loadResult1 = imageLoad(testImage1, ivec2(0, 0));\n\
		vec4 textureResult0 = vec4(ReturnSelf(textureLod(notUsedTexture0, pixelTextureCoordinate, 0).rgb), 1);\n\
		vec4 textureResult1 = vec4(ReturnSelf(textureLod(notUsedTexture1, pixelTextureCoordinate, 0).rgb), 1);\n\
		vec4 texture3DResult = texture(test3DTexture, 0.2 * mPosition);\n\
		float dotResult = dot(eNormal, vec3(0, 0, -1));\n\
		finalColor = vec4(0)\n\
		//+ loadResult0 * 0.4 + loadResult1 * 0.4\n\
		//+ textureResult0 * 0.1 + textureResult1 * 0.1\n\
		+ texture3DResult\n\
		//+ vec4(eNormal, 1)\n\
		;\n\
		//finalColor = finalColor + texture(notUsedTexture, pixelTextureCoordinate) * 0.01;\n\
		//finalColor = vec4(textureLod(bumpTexture, vec2(1, 1), 0).rgb, 1);\n\
		//finalColor = vec4(size, 0, 1);\n\
	}\n\
	\n\
	#endif\n\
	\n\
	"
		);
}


ShaderCodes::~ShaderCodes()
{
}

std::vector<std::string> const& ShaderCodes::GetShaderCodes()
{
	return codes;
}
