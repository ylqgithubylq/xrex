

uniform vec3 color;
uniform vec3 centerPosition;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
// uniform sampler2D normalMap;
// uniform sampler2D shininessMap;
// uniform sampler2D opacityMap;

#ifdef VS

in vec3 position;
in vec3 normal;
in vec3 textureCoordinate0;
out vec3 wNormal;
out vec2 pixelTextureCoordinate;

void main()
{
	vec3 temp = (modelMatrix * vec4(position, 1.0)).xyz - centerPosition;
	//vertexColor = normalize(temp) * 1.1 + color / 16;
	//vertexColor = (position - centerPosition) + 1.0 + color / 16;
	wNormal = (modelMatrix * vec4(normal, 0.0)).xyz;
	gl_Position = 
		projectionMatrix *
		viewMatrix *
		modelMatrix *
		vec4(position, 1.0);
	pixelTextureCoordinate = textureCoordinate0.st;
}

#endif


#ifdef FS

in vec3 wNormal;
in vec2 pixelTextureCoordinate;

layout(location = 0) out vec4 finalColor;

void main()
{
	vec3 normal = normalize(wNormal) * 0.5 + 0.5;
	finalColor = 
		//vec4(normal, 1.0) * 0.1 +
		texture(diffuseMap, pixelTextureCoordinate) + texture(specularMap, pixelTextureCoordinate);
}

#endif


	
