

const vec3 lightColor = vec3(150, 150, 160);
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
out vec3 eNormal;
out vec2 pixelTextureCoordinate;
out vec3 wPositionToLight;
out float vDepth;

void main()
{
	vec3 temp = (modelMatrix * vec4(position, 1.0)).xyz - centerPosition;
	wNormal = mat3(normalMatrix) * normal;
	eNormal = mat3(viewMatrix) * wNormal;
	vec4 wPosition4 = modelMatrix * vec4(position, 1.0);
	vec3 lightPosition = cameraPosition; // assume light on camera
	wPositionToLight = lightPosition - wPosition4.xyz;

	vec4 vPosition4 = viewMatrix * wPosition4;
	gl_Position = projectionMatrix * vPosition4;
	vDepth = vPosition4.z / 10000;

	pixelTextureCoordinate = textureCoordinate0.st;
}

#endif


#ifdef FS

in vec3 wNormal;
in vec3 eNormal;
in vec2 pixelTextureCoordinate;
in vec3 wPositionToLight;
in float vDepth;

out vec4 colorOutput;
out vec4 normalOutput;
out float depthInColorOutput;

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

	float specularLevelFetched = texture(specularMap, pixelTextureCoordinate).x;
	vec3 specularColor = lightColor * lightIntensity * specularLevelFetched * pow(lDotHV, specularLevelFetched);

	colorOutput = vec4(diffuseColor + specularColor, 1);
	normalOutput = vec4(eNormal, 1);
	depthInColorOutput = vDepth;
}

#endif


	
