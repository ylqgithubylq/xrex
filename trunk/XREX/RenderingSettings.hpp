#pragma once
#include "Declare.hpp"

struct RenderingSettings
{
public:
	RenderingSettings();
	~RenderingSettings();

	bool fullScreen;

	int32 left;
	int32 top;
	int32 width;
	int32 height;

	uint32 colorBits;
	uint32 depthBits;
	uint32 stencilBits;

	uint32 sampleCount;

};

