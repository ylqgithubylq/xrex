/*
 *	@r: sample size over voxel size.
 */
vec4 AdjustResult(vec4 value, float r)
{
	if (value.a < 0.0001)
	{
		return value;
	}
 	float adjustedAlpha = 1 - pow(1 - value.a, r);
 	return value * adjustedAlpha / value.a;
}

const float ReciprocalLog2 = 1 / log(2);

/*
 *	@vexelVolume: voxels range from [-1, 1].
 *	@normalizedSamplePoint: should be normalized to [-1, 1].
 *	@normalizedSampleRadius: should be normalized to [0, 1].
 */
vec4 Sample(sampler3D vexelVolume, float voxelSize, vec3 normalizedSamplePoint, float normalizedSampleRadius)
{
	vec3 textureCoordinate = normalizedSamplePoint + vec3(1, 1, 1) / 2;
	float sampleSize = normalizedSampleRadius / 2 * 2; // sampleSize is diameter
	float sampleLevel = log(sampleSize / voxelSize) * ReciprocalLog2;
	vec4 result = textureLod(vexelVolume, textureCoordinate, max(0, sampleLevel));
	result = AdjustResult(result, sampleSize / voxelSize); // adjust result based on size difference
	return result;
}

/*
 *	Front to back accumulation.
 */
vec4 AccumulateColor(vec4 currentColor, vec4 newColor)
{
	return currentColor + (1 - currentColor.a) * newColor;
}

/*
 *	@vexelVolume: color should alpha pre-multiplied.
 *	@return: alpha pre-multiplied color.
 */
vec4 ConeTrace(sampler3D vexelVolume, vec3 volumeCenter, float volumeHalfSize, vec3 startPoint, vec3 viewDirection, float coneAperture, float alphaThreshold)
{
	const ivec3 gridCounts = textureSize(vexelVolume, 0); // x,y,z should be the same.
	const int gridCount = gridCounts.x;
	const float maxTracingDistance = volumeHalfSize * 4; // TODO change to proper value
	const float smallestGridSize = 2 * volumeHalfSize / gridCount;
	const float sinHalfAperture = sin(coneAperture / 2);

	vec4 finalColor = vec4(0, 0, 0, 0); // alpha pre-multiplied color

	// the volume of first sample point should include the startPoint
	float currentSampleDistance = smallestGridSize / 2;
	float currentSampleRadius = currentSampleDistance;

	while (currentSampleDistance + currentSampleRadius < maxTracingDistance)
	{
		vec3 currentSamplePoint = startPoint - volumeCenter + viewDirection * currentSampleDistance;
		vec4 color = Sample(vexelVolume, float(1) / gridCount, currentSamplePoint / volumeHalfSize, currentSampleRadius / volumeHalfSize); // normalized to [-1, 1]
		//vec4 modifiedColor = mix(vec4(0, 1, 0, color.a), vec4(0, 1, 1, color.a), currentSampleDistance / maxTracingDistance); // TODO temp
		//finalColor = AccumulateColor(finalColor, modifiedColor);
		finalColor = AccumulateColor(finalColor, color);
		if (finalColor.a > alphaThreshold)
		{
			break;
		}
		// next sphere inside the cone next to the current sample sphere.
		// (currentSampleDistance + currentSampleRadius + nextSampleRadius) * sin(sinHalfAperture) = nextSampleRadius
		float nextSampleRadius = (currentSampleDistance + currentSampleRadius) * sinHalfAperture / (1 - sinHalfAperture);
		float tracingStep = currentSampleRadius + nextSampleRadius;
		if (tracingStep < smallestGridSize) // make the tracingStep always larger than a voxel
		{
			tracingStep = smallestGridSize / 2;
			nextSampleRadius = tracingStep / 2;
		}
		currentSampleDistance = currentSampleDistance + tracingStep;
		currentSampleRadius = nextSampleRadius;
	}
	return finalColor;
}

uniform mat4 modelMatrix;
uniform mat4 normalMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec3 cameraPosition;

uniform sampler3D voxels;
uniform vec3 voxelVolumeCenter;
uniform float voxelVolumeHalfSize;

uniform float aperture;

#ifdef VS

in vec3 position; // [-1, 1], should be the back face of the bounding box.
out vec3 wPosition;

void main()
{
	wPosition = (modelMatrix * vec4(position, 1.0)).xyz;
	gl_Position = projectionMatrix * viewMatrix * vec4(wPosition, 1);
}

#endif

const float alphaThreshold = 0.99;

#ifdef FS

in vec3 wPosition;

layout(location = 0) out vec4 finalColor;

void main()
{
	vec3 direction = normalize(wPosition - cameraPosition);
	vec4 color = ConeTrace(voxels, voxelVolumeCenter, voxelVolumeHalfSize, cameraPosition, direction, aperture, alphaThreshold);
	finalColor = color;
}

#endif