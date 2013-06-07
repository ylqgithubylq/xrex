#pragma once
#include "Declare.hpp"

#include "XREXContext.hpp"
#include "RenderingEngine.hpp"

#include <memory>


namespace XREX
{


	class XREX_API GraphicsContext
	{
	public:
		GraphicsContext(Window& window, Settings const& settings);
		~GraphicsContext();

		void SwapBuffers();

		int32 GetMajorVersion() const
		{
			return majorVersion_;
		}
		int32 GetMinorVersion() const
		{
			return minorVersion_;
		}
	protected:
		void OnMessageIdle();

	private:
		// used to hide windows.h to the cpp file
		struct GLHideWindows_;
		std::unique_ptr<GLHideWindows_> glHideWindows_;

		int32 majorVersion_;
		int32 minorVersion_;

		uint32 colorBits_;
		uint32 depthBits_;
		uint32 stencilBits_;
		uint32 sampleCount_;

		std::string description_;
	};

}
