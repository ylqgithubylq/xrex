#pragma once
#include "Declare.hpp"

#include "window.hpp"
#include "RenderingEngine.hpp"

#include <memory>




class GLWindow
	: public Window
{
public:
	GLWindow(std::wstring const & name, RenderingSettings const & settings);
	virtual ~GLWindow();

	void Destory();

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
	virtual void OnMessageIdle() override
	{
		RenderingEngine::GetInstance().Update();
		SwapBuffers();
	}

private:
	// used to hide windows.h to the cpp file
	struct GLHideWindows_;
	std::unique_ptr<GLHideWindows_, CheckedDeleter<GLHideWindows_>> glHideWindows_;

	int32 majorVersion_;
	int32 minorVersion_;

	uint32 colorBits_;
	uint32 depthBits_;
	uint32 stencilBits_;
	uint32 sampleCount_;

	std::string description_;
};

