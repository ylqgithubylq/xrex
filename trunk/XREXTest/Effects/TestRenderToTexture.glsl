

const vec3 lightColor = vec3(150, 150, 160);

uniform Material
{
	float opacity;
	float specularLevel;
	float shininess;
} material;

// specularLevel * pow(LdotHV, shininess)

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
// uniform sampler2D normalMap;
uniform sampler2D shininessMap;
uniform sampler2D opacityMap;

struct VSToFS
{
	vec3 wNormal;
	vec3 vNormal;
	vec2 pixelTextureCoordinate;
	vec3 wPositionToLight;
	float vDepth;
};

#ifdef VS

in vec3 position;
in vec3 normal;
in vec3 textureCoordinate0;

out	vec3 wNormal;
out	vec3 vNormal;
out	vec2 pixelTextureCoordinate;
out	vec3 wPositionToLight;
out	float vDepth;
//out VSToFS vsOut;

void main()
{
	/*vsOut.*/wNormal = XREX_TransformNormal(XREX_ModelTransformation.WorldFromModelNormal, normal);
	/*vsOut.*/vNormal = XREX_TransformNormal(XREX_CameraTransformation.ViewFromWorld, /*vsOut.*/wNormal);
	vec3 wPosition = XREX_Transform(XREX_ModelTransformation.WorldFromModel, position);
	vec3 lightPosition = XREX_CameraTransformation.CameraPositionInWorld; // assume light on camera
	/*vsOut.*/wPositionToLight = lightPosition - wPosition;

	vec4 vPosition4 = XREX_TransformToClip(XREX_ModelTransformation.ClipFromModel, position);
	gl_Position = vPosition4;
	/*vsOut.*/vDepth = vPosition4.z / 10000;

	/*vsOut.*/pixelTextureCoordinate = textureCoordinate0.st;
}

#endif


#ifdef FS

in	vec3 wNormal;
in	vec3 vNormal;
in	vec2 pixelTextureCoordinate;
in	vec3 wPositionToLight;
in	float vDepth;
//in VSToFS fsIn;

out vec4 colorOutput;
out vec4 normalOutput;
out vec4 depthInColorOutput;

out vec4 XREX_DefaultFrameBufferOutput;

void main()
{
	vec3 normalToShow = normalize(/*fsIn.*/wNormal) * 0.5 + 0.5;

	float distanceToLight = length(/*fsIn.*/wPositionToLight);
	vec3 lightDirection = /*fsIn.*/wPositionToLight / distanceToLight;

	vec3 halfVector = normalize(lightDirection + /*fsIn.*/wNormal);
	float lDotN = dot(lightDirection, /*fsIn.*/wNormal);
	float lDotHV = dot(halfVector, lightDirection);
	float lightIntensity = 1 / ((distanceToLight + 10)/* * (distanceToLight + 10)*/);

	vec3 diffuseColor = lightColor * lightIntensity * max(lDotN, 0) * texture(diffuseMap, /*fsIn.*/pixelTextureCoordinate).rgb;

	float specularLevelFetched = texture(specularMap, /*fsIn.*/pixelTextureCoordinate).x;
	vec3 specularColor = lightColor * lightIntensity * specularLevelFetched * pow(lDotHV, specularLevelFetched);

	colorOutput = vec4(diffuseColor + specularColor, 1);
	normalOutput = vec4(/*fsIn.*/vNormal, 1);
	depthInColorOutput = /*fsIn.*/vec4(vDepth, 0, 0, 1);

	XREX_DefaultFrameBufferOutput = colorOutput;
}

#endif


	
