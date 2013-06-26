#include "XREXAll.hpp"
#include "VoxelTestShaderCodes.h"

using namespace XREX;
using namespace std;

#pragma region TestConeTracingCode

floatV4 textureLod(array<array<array<floatV4, 2>, 2>, 2> const& vexelVolume, floatV3 textureCoordinate, float sampleLevel)
{
	return floatV4(0, 0, 0, 1);
}
/*
 *	@vexelVolume: voxels range from [-1, 1].
 *	@normalizedSamplePoint: should be normalized to [-1, 1].
 *	@normalizedSampleRadius: should be normalized to [0, 1].
 */
floatV4 Sample(array<array<array<floatV4, 2>, 2>, 2> const& vexelVolume, uint32 gridCount, floatV3 normalizedSamplePoint, float normalizedSampleRadius)
{
	float const voxelSize = 1.0f / gridCount;
	floatV3 textureCoordinate = normalizedSamplePoint + floatV3(1, 1, 1) / 2;
	float sampleSize = normalizedSampleRadius / 2;
	float sampleLevel = log(sampleSize / voxelSize) / log(2.f);
	sampleLevel = max(0.f, sampleLevel);
	return textureLod(vexelVolume, textureCoordinate, sampleLevel);
}

/*
 *	Front to back accumulation.
 */
floatV4 AccumulateColor(floatV4 currentColor, floatV4 newColor)
{
	return currentColor + (1 - currentColor.W()) * newColor;
}

/*
 *	@vexelVolume: color should alpha pre-multiplied.
 *	@return: alpha pre-multiplied color.
 */
floatV4 ConeTrace(array<array<array<floatV4, 2>, 2>, 2> const& vexelVolume, floatV3 volumeCenter, float volumeHalfSize, floatV3 startPoint, floatV3 viewDirection, float coneAperture, float alphaThreshold)
{
	uint32 const gridCount = 4; // textureSize
	float const maxTracingDistance = volumeHalfSize * 4; // TODO change to proper value
	float const smallestGridSize = 2 * volumeHalfSize / gridCount;
	float const sinHalfAperture = sin(coneAperture / 2);

	floatV4 finalColor = floatV4(0, 0, 0, 0); // alpha pre-multiplied color

	// the volume of first sample point should include the startPoint
	float currentSampleDistance = smallestGridSize / 2;
	float currentSampleRadius = currentSampleDistance;

	while (currentSampleDistance + currentSampleRadius < maxTracingDistance)
	{
		floatV3 currentSamplePoint = startPoint - volumeCenter + viewDirection * currentSampleDistance;
		floatV4 color = Sample(vexelVolume, gridCount, currentSamplePoint / volumeHalfSize, currentSampleRadius / volumeHalfSize); // normalized to [-1, 1]
		finalColor = AccumulateColor(finalColor, color);
		
		if (finalColor.W() > alphaThreshold)
		{
			break;
		}
		// next sphere inside the cone next to the current sample sphere.
		// (currentSampleDistance + currentSampleRadius + nextSampleRadius) * sin(sinHalfAperture) = nextSampleRadius
		float nextSampleRadius = (currentSampleDistance + currentSampleRadius) * sinHalfAperture / (1 - sinHalfAperture);
		float step = currentSampleRadius + nextSampleRadius;
		if (step < smallestGridSize) // make the step always larger than the a voxel
		{
			step = smallestGridSize;
			nextSampleRadius = smallestGridSize / 2;
		}
		currentSampleDistance = currentSampleDistance + step;
		currentSampleRadius = nextSampleRadius;
	}
	return finalColor;
}
#pragma endregion

VoxelTestShaderCodes::VoxelTestShaderCodes()
{
	ProgramObjectSP program = XREXContext::GetInstance().GetRenderingFactory().CreateProgramObject();
	string shaderString;
	string shaderFile = "../../Effects/ConeTracing.glsl";
	if (!XREXContext::GetInstance().GetResourceLoader().LoadString(shaderFile, &shaderString))
	{
		XREXContext::GetInstance().GetLogger().LogLine("file not found. file: " + shaderFile);
	}

	codes.emplace_back(std::move(shaderString));
}

std::vector<std::string> const& VoxelTestShaderCodes::GetShaderCodes()
{
	return codes;
}