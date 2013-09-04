#pragma once

#include "Declare.hpp"

#include <string>

namespace XREX
{

	struct XREX_API RenderingSettings
	{

		bool fullScreen;

		int32 left;
		int32 top;
		int32 width;
		int32 height;

		uint32 colorBits;
		uint32 depthBits;
		uint32 stencilBits;

		uint32 sampleCount;

		RenderingSettings();
	};

	struct XREX_API Settings
	{
		RenderingSettings renderingSettings;
		std::wstring windowTitle;
		std::string rootPath;
		Settings(std::string const& theRootPath)
			: rootPath(theRootPath)
		{
		}
	};

}
