

uniform vec3 centerPosition;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;


#ifdef VS

in vec3 position;
out vec4 color;
out vec3 textureCoordinate;

void main()
{
	textureCoordinate = position;
	gl_Position = 
		projectionMatrix *
		viewMatrix *
		modelMatrix *
		vec4(position, 1.0);
	color = vec4(((modelMatrix * vec4(position, 1)).xyz) / length(centerPosition) / 2, 0.5);
}

#endif


#ifdef FS

layout(location = 0) out vec4 finalColor;

in vec4 color;
in vec3 textureCoordinate;
const vec4 colorA = vec4(0.5, 0.5, 0, 1);
const vec4 colorB = vec4(0, 0.5, 0.5, 1);
const vec3 colorASize = vec3(0.7, 0.7, 0.7);

void main()
{
	vec3 texturePosition = fract(textureCoordinate * 8);
	vec3 useColorA = step(texturePosition, colorASize);
	vec4 mixedColor = mix(colorA, colorB, useColorA.x * useColorA.y * useColorA.z);
	finalColor = color + mixedColor;
}

#endif


	
