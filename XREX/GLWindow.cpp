#include "XREX.hpp"

#include "GLWindow.hpp"

#include "Settings.hpp"

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



struct GLWindow::GLHideWindows_
{
	GLHideWindows_(GLWindow& window)
		: window_(window)
	{
	}

	HWND GetHWND()
	{
		return static_cast<HWND>(window_.GetHWND());
	}

	GLWindow& window_;
	HDC hDC_;
	HGLRC hRC_;
};


GLWindow::GLWindow(wstring const& name, RenderingSettings const& settings)
	: Window(name, settings.left, settings.top, settings.width, settings.height), majorVersion_(0), minorVersion_(0)
{
	glHideWindows_ = MakeUP<GLHideWindows_>(*this);

	colorBits_ = settings.colorBits;
	depthBits_ = settings.depthBits;
	stencilBits_ = settings.stencilBits;
	sampleCount_ = settings.sampleCount;

	glHideWindows_->hDC_ = ::GetDC(glHideWindows_->GetHWND());

	uint32 style;
	style = WS_OVERLAPPEDWINDOW;


	RECT windowRect = { settings.left, settings.top, settings.left + settings.width, settings.top + settings.height };
	::AdjustWindowRect(&windowRect, style, false);

	::SetWindowLongPtr(glHideWindows_->GetHWND(), GWL_STYLE, style);

	::SetWindowPos(glHideWindows_->GetHWND(), NULL, windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
		SWP_SHOWWINDOW | SWP_NOZORDER);

	RECT clientRect;
	::GetClientRect(glHideWindows_->GetHWND(), &clientRect);

	width_ = clientRect.right - clientRect.left;
	height_ = clientRect.bottom - clientRect.top;


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
		float float_attrs[] = { 0, 0 };
		BOOL valid = FALSE;
		do
		{
			int32 int_attrs[] =
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

			valid = wglChoosePixelFormatARB(glHideWindows_->hDC_, int_attrs, float_attrs, 1, &pixelFormat, &numberFormats);
			if (!valid || (numberFormats < 1))
			{
				--sampleCount_;
			}
		}
		while((sampleCount_ > 1) && (!valid || (numberFormats < 1)));



		if (valid && (sampleCount_ > 1))
		{
			::wglMakeCurrent(glHideWindows_->hDC_, NULL);
			::wglDeleteContext(glHideWindows_->hRC_);
			::ReleaseDC(glHideWindows_->GetHWND(), glHideWindows_->hDC_);

			Recreate(); // this will lead to WM_QUIT message in the main message loop, why?

			glHideWindows_->hDC_ = ::GetDC(glHideWindows_->GetHWND());

			::SetWindowPos(glHideWindows_->GetHWND(), NULL, windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
				SWP_SHOWWINDOW | SWP_NOZORDER);

			::SetPixelFormat(glHideWindows_->hDC_, pixelFormat, &pixelFormatDescriptor);

			glHideWindows_->hRC_ = ::wglCreateContext(glHideWindows_->hDC_);
			::wglMakeCurrent(glHideWindows_->hDC_, glHideWindows_->hRC_);


		}
	}





	uint32 flags = 0;
	flags = WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB; // no deprecated GL API
#ifdef XREX_DEBUG
	flags |= WGL_CONTEXT_DEBUG_BIT_ARB;
#endif

	int32 versions[3][2] =
	{
		{ 4, 2 },
		{ 4, 1 },
		{ 4, 0 },
	};

	int32 attribs[] = { WGL_CONTEXT_MAJOR_VERSION_ARB, versions[0][0], WGL_CONTEXT_MINOR_VERSION_ARB, versions[0][1], WGL_CONTEXT_FLAGS_ARB, flags,
		WGL_CONTEXT_PROFILE_MASK_ARB, /*WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB*/WGL_CONTEXT_CORE_PROFILE_BIT_ARB, 0 };
	for (int32 i = 0; i < 3; ++ i)
	{
		attribs[1] = versions[i][0];
		attribs[3] = versions[i][1];
		HGLRC newRC = wglCreateContextAttribsARB(glHideWindows_->hDC_, NULL, attribs);
		if (newRC != NULL)
		{
			::wglMakeCurrent(glHideWindows_->hDC_, NULL);
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

	//gl::PixelStorei(gl::GL_PACK_ALIGNMENT, 1); // default: 4
	//gl::PixelStorei(gl::GL_UNPACK_ALIGNMENT, 1); // default: 4


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

	active_ = true;

	cout << description_ << endl;

	uint32 glError = gl::GetError();
	if (glError != gl::GL_NO_ERROR)
	{
		cout << "GL error: " << glError << endl;
	}
}


GLWindow::~GLWindow()
{
	Destory();
}

void GLWindow::Destory()
{
	if (glHideWindows_->GetHWND() != NULL)
	{
		if (glHideWindows_->hDC_ != NULL)
		{
			::wglMakeCurrent(glHideWindows_->hDC_, NULL);
			if (glHideWindows_->hRC_ != NULL)
			{
				::wglDeleteContext(glHideWindows_->hRC_);
				glHideWindows_->hRC_ = NULL;
			}
			::ReleaseDC(glHideWindows_->GetHWND(), glHideWindows_->hDC_);
			glHideWindows_->hDC_ = NULL;
		}

		::ChangeDisplaySettings(NULL, 0);
		ShowCursor(TRUE);
	}
}

void GLWindow::SwapBuffers()
{
	::SwapBuffers(glHideWindows_->hDC_);
}