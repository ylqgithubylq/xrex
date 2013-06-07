#include "XREX.hpp"

#include "GraphicsContext.hpp"

#include "Settings.hpp"
#include "Window.hpp"

#include <sstream>
#include <iostream>

#include <glload/gll.hpp>
#include <glload/wgl_exts.h>
#include <CoreGL.hpp>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif
#include <WinGDI.h>


using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::wstring;

namespace XREX
{

	struct GraphicsContext::GLHideWindows_
	{
		GLHideWindows_(Window& window)
			: window_(window)
		{
		}

		HWND GetHWND()
		{
			return static_cast<HWND>(window_.GetHWND());
		}

		Window& window_;
		HDC hDC_;
		HGLRC hRC_;
	};


	GraphicsContext::GraphicsContext(Window& window, Settings const& settings)
	{

		glHideWindows_ = MakeUP<GLHideWindows_>(window);

		RenderingSettings const& renderingSettings = settings.renderingSettings;

		colorBits_ = renderingSettings.colorBits;
		depthBits_ = renderingSettings.depthBits;
		stencilBits_ = renderingSettings.stencilBits;
		sampleCount_ = renderingSettings.sampleCount;

		glHideWindows_->hDC_ = ::GetDC(glHideWindows_->GetHWND());

		PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
		memset(&pixelFormatDescriptor, 0, sizeof(pixelFormatDescriptor));
		pixelFormatDescriptor.nSize			= sizeof(pixelFormatDescriptor);
		pixelFormatDescriptor.nVersion		= 1;
		pixelFormatDescriptor.dwFlags		= PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pixelFormatDescriptor.iPixelType	= PFD_TYPE_RGBA;
		pixelFormatDescriptor.cColorBits	= static_cast<BYTE>(colorBits_);
		pixelFormatDescriptor.cDepthBits	= static_cast<BYTE>(depthBits_);
		pixelFormatDescriptor.cStencilBits	= static_cast<BYTE>(stencilBits_);
		pixelFormatDescriptor.iLayerType	= PFD_MAIN_PLANE;

		int32 pixelFormat = ::ChoosePixelFormat(glHideWindows_->hDC_, &pixelFormatDescriptor);
		assert(pixelFormat != 0);

		::SetPixelFormat(glHideWindows_->hDC_, pixelFormat, &pixelFormatDescriptor);

		glHideWindows_->hRC_ = ::wglCreateContext(glHideWindows_->hDC_);
		::wglMakeCurrent(glHideWindows_->hDC_, glHideWindows_->hRC_);



		if (glload::LoadWinFunctions(glHideWindows_->hDC_) == glload::LS_LOAD_FAILED)
		{
			cerr << "wgl initialize failed." << endl;
		}



		if (sampleCount_ > 1)
		{
			uint32 numberFormats;
			float floatAttributes[] = { 0, 0 };
			BOOL valid = FALSE;
			do
			{
				int32 intAttributes[] =
				{
					WGL_DRAW_TO_WINDOW_ARB, gl::GL_TRUE,
					WGL_SUPPORT_OPENGL_ARB, gl::GL_TRUE,
					WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
					WGL_COLOR_BITS_ARB, colorBits_,
					WGL_DEPTH_BITS_ARB, depthBits_,
					WGL_STENCIL_BITS_ARB, stencilBits_,
					WGL_DOUBLE_BUFFER_ARB, gl::GL_TRUE,
					WGL_SAMPLE_BUFFERS_ARB, gl::GL_TRUE,
					WGL_SAMPLES_ARB, sampleCount_,
					0, 0
				};

				valid = wglChoosePixelFormatARB(glHideWindows_->hDC_, intAttributes, floatAttributes, 1, &pixelFormat, &numberFormats);
				if (!valid || (numberFormats < 1))
				{
					--sampleCount_;
				}
			}
			while((sampleCount_ > 1) && (!valid || (numberFormats < 1)));



			if (valid && (sampleCount_ > 1))
			{
				::wglMakeCurrent(glHideWindows_->hDC_, nullptr);
				::wglDeleteContext(glHideWindows_->hRC_);
				::ReleaseDC(glHideWindows_->GetHWND(), glHideWindows_->hDC_);

				glHideWindows_->window_.Recreate();

				glHideWindows_->hDC_ = ::GetDC(glHideWindows_->GetHWND());
				::SetPixelFormat(glHideWindows_->hDC_, pixelFormat, &pixelFormatDescriptor);

				glHideWindows_->hRC_ = ::wglCreateContext(glHideWindows_->hDC_);
				::wglMakeCurrent(glHideWindows_->hDC_, glHideWindows_->hRC_);
			}
		}





		uint32 flags = 0;
#ifndef USE_OPENGL_COMPATIBILITY_PROFILE
		flags = WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB; // no deprecated GL API
#endif
#ifdef XREX_DEBUG
		flags |= WGL_CONTEXT_DEBUG_BIT_ARB;
#endif

		int32 versions[][2] =
		{
			{ 4, 3 },
			{ 4, 2 },
			{ 4, 1 },
			{ 4, 0 },
		};

		int32 attribs[] = { WGL_CONTEXT_MAJOR_VERSION_ARB, versions[0][0], WGL_CONTEXT_MINOR_VERSION_ARB, versions[0][1], WGL_CONTEXT_FLAGS_ARB, flags,
			WGL_CONTEXT_PROFILE_MASK_ARB,
#ifdef USE_OPENGL_COMPATIBILITY_PROFILE
			WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
#else
			WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#endif
			0 };
		for (int32 i = 0; i < 3; ++i)
		{
			attribs[1] = versions[i][0];
			attribs[3] = versions[i][1];
			HGLRC newRC = wglCreateContextAttribsARB(glHideWindows_->hDC_, nullptr, attribs);
			if (newRC != nullptr)
			{
				::wglMakeCurrent(glHideWindows_->hDC_, nullptr);
				::wglDeleteContext(glHideWindows_->hRC_);
				glHideWindows_->hRC_ = newRC;

				::wglMakeCurrent(glHideWindows_->hDC_, glHideWindows_->hRC_);

				break;
			}
		}

		if (glload::LoadFunctions() != glload::LS_LOAD_FUNCTIONS_ALL)
		{
			cout << "not all GL function are loaded." << endl;
		}

		gl::GetIntegerv(gl::GL_MAJOR_VERSION, &majorVersion_);
		gl::GetIntegerv(gl::GL_MINOR_VERSION,&minorVersion_);
		cout << "OpenGL version: " << majorVersion_ << "." << minorVersion_ << endl;


		string vendor, renderer, glVersion, glslVersion;
		vendor = reinterpret_cast<char const*>(gl::GetString(gl::GL_VENDOR));
		renderer = reinterpret_cast<char const*>(gl::GetString(gl::GL_RENDERER));
		glVersion = reinterpret_cast<char const*>(gl::GetString(gl::GL_VERSION));
		glslVersion = reinterpret_cast<char const*>(gl::GetString(gl::GL_SHADING_LANGUAGE_VERSION));
		std::stringstream oss;
		oss << vendor << " " << renderer << endl;
		oss << glVersion << ", GLSL version: " << glslVersion;
		if (sampleCount_ > 1)
		{
			oss << " (" << sampleCount_ << "x AA)";
		}
		description_ = oss.str();

		cout << description_ << endl;

		uint32 glError = gl::GetError();
		if (glError != gl::GL_NO_ERROR)
		{
			cout << "GL error: " << glError << endl;
		}
	}


	GraphicsContext::~GraphicsContext()
	{
		if (glHideWindows_->GetHWND() != nullptr)
		{
			if (glHideWindows_->hDC_ != nullptr)
			{
				::wglMakeCurrent(glHideWindows_->hDC_, nullptr);
				if (glHideWindows_->hRC_ != nullptr)
				{
					::wglDeleteContext(glHideWindows_->hRC_);
					glHideWindows_->hRC_ = nullptr;
				}
				::ReleaseDC(glHideWindows_->GetHWND(), glHideWindows_->hDC_);
				glHideWindows_->hDC_ = nullptr;
			}
		}
	}

	void GraphicsContext::OnMessageIdle()
	{
		XREXContext::GetInstance().RenderAFrame();
		SwapBuffers();
	}

	void GraphicsContext::SwapBuffers()
	{
		::SwapBuffers(glHideWindows_->hDC_);
	}

}