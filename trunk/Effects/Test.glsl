

const vec3 lightColor = vec3(50, 50, 60);
uniform vec3 centerPosition;

uniform mat4 modelMatrix;
uniform mat4 normalMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec3 cameraPosition;

uniform float opacity;
uniform float specularLevel;
uniform float shininess;
// specularLevel * pow(LdotHV, shininess)

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
// uniform sampler2D normalMap;
uniform sampler2D shininessMap;
uniform sampler2D opacityMap;

#ifdef VS

in vec3 position;
in vec3 normal;
in vec3 textureCoordinate0;
out vec3 wNormal;
out vec2 pixelTextureCoordinate;
out vec3 wPositionToLight;

void main()
{
	vec3 temp = (modelMatrix * vec4(position, 1.0)).xyz - centerPosition;
	wNormal = mat3(normalMatrix) * normal;

	vec4 wPosition4 = modelMatrix * vec4(position, 1.0);
	vec3 lightPosition = cameraPosition; // assume light on camera
	wPositionToLight = lightPosition - wPosition4.xyz;

	gl_Position = 
		projectionMatrix *
		viewMatrix *
		wPosition4;

	pixelTextureCoordinate = textureCoordinate0.st;
}

#endif


#ifdef FS

in vec3 wNormal;
in vec2 pixelTextureCoordinate;
in vec3 wPositionToLight;

layout(location = 0) out vec4 finalColor;

void main()
{
	vec3 normalToShow = normalize(wNormal) * 0.5 + 0.5;

	float distanceToLight = length(wPositionToLight);
	vec3 lightDirection = wPositionToLight / distanceToLight;

	vec3 halfVector = normalize(lightDirection + wNormal);
	float lDotN = dot(lightDirection, wNormal);
	float lDotHV = dot(halfVector, lightDirection);
	float lightIntensity = 1 / ((distanceToLight + 10)/* * (distanceToLight + 10)*/);

	vec3 diffuseColor = lightColor * lightIntensity * max(lDotN, 0) * texture(diffuseMap, pixelTextureCoordinate).rgb;

	float specularLevelFetched = texture(specularMap, pixelTextureCoordinate);
	float specularColor = lightColor * lightIntensity * specularLevelFetched * pow(lDotHV, specularLevelFetched);

	// TODO assimp do not load specularMap correctly, so default black texture used here.
	// the problem is caused by the sponza.mtl, which do not have the specular map in it at all.
	finalColor = //vec4(texture(specularMap, pixelTextureCoordinate).rgb, 1);
		//vec4(normalToShow, 1.0) * 0.1 +
		//texture(diffuseMap, pixelTextureCoordinate) + texture(specularMap, pixelTextureCoordinate);
		vec4(diffuseColor + specularColor, 1);
}

#endif


	
