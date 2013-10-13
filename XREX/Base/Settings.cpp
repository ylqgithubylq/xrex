#include "XREX.hpp"

#include "Settings.hpp"

namespace XREX
{

	RenderingSettings::RenderingSettings()
		: fullScreen(false), left(200), top(200), width(800), height(600),
		colorFormat(TexelFormat::RGBA8), depthStencilFormat(TexelFormat::Depth24Stencil8), sampleCount(1)
	{
	}

}
