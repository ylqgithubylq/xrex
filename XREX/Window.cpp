#include "XREX.hpp"

#include "Window.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif
#include <windows.h>


struct Window::HideWindows_
{
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	LRESULT InstanceWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	HideWindows_(Window& window)
		: window_(window)
	{
	}

	Window& window_;
	HWND hWnd_;

};

//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
LRESULT CALLBACK Window::HideWindows_::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Window* thiz = reinterpret_cast<Window*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));

	if (thiz)
	{
		return thiz->hideWindows_->InstanceWndProc(hWnd, message, wParam, lParam);
	}
	else
	{
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

LRESULT Window::HideWindows_::InstanceWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		// Add all windows message handling here
	case WM_KEYDOWN:
		{
			window_.OnKeyDown(wParam);
			break;
		}

	case WM_KEYUP:
		{
			window_.OnKeyUp(wParam);
			break;
		}
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		{
			window_.OnMouseDown(wParam, LOWORD(lParam), HIWORD(lParam));
		}
		break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		{
			window_.OnMouseUp(wParam, LOWORD(lParam), HIWORD(lParam));
		}
		break;


	case WM_MOUSEWHEEL:
		{
			window_.OnMouseWheel(GET_KEYSTATE_WPARAM(wParam), LOWORD(lParam), HIWORD(lParam), GET_WHEEL_DELTA_WPARAM(wParam));
		}
		break;

	case WM_MOUSEMOVE:
		{
			window_.OnMouseMove(GET_KEYSTATE_WPARAM(wParam), LOWORD(lParam), HIWORD(lParam));
		}
		break;


	case WM_ACTIVATE:
		{
			if (!HIWORD(wParam))
			{
				window_.active_ = true;
			}
			else
			{
				window_.active_ = false;
			}
			break;
		}

	case WM_SIZE:
		{
			window_.OnResize(LOWORD(lParam), HIWORD(lParam)); // LoWord = width, HiWord = height
		}
		break;


	case WM_ERASEBKGND:
		{
			return 1;
		}
		break;
	case WM_CLOSE:
		{
			PostQuitMessage(0);
		}
		break;

	default:
		{
		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}




void* Window::GetHWND() const
{
	return hideWindows_->hWnd_;
}



Window::Window(std::wstring const & name, int32 left, int32 top, int32 width, int32 height)
	: name_(name), left_(left), top_(top), active_(false), running_(false), rendering_(false)
{
	hideWindows_ = MakeUP<HideWindows_>(*this);

	HINSTANCE hInstance = ::GetModuleHandle(NULL);

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= HideWindows_::WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= static_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH));
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= name_.c_str();
	wcex.hIconSm		= NULL;

	::RegisterClassEx(&wcex);



	RECT windowRect = {left, top, left + width, top + height};

	DWORD style;
	style = WS_OVERLAPPEDWINDOW;

	::AdjustWindowRect(&windowRect, style, false);

	hideWindows_->hWnd_ = ::CreateWindow(name_.c_str(), name_.c_str(), style,
		windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, NULL, NULL, hInstance, NULL);

	
	::GetClientRect(hideWindows_->hWnd_, &windowRect);

	left_ = windowRect.left;
	top_ = windowRect.top;
	width_ = windowRect.right - windowRect.left;
	height_ = windowRect.bottom - windowRect.top;

	::SetWindowLongPtr(hideWindows_->hWnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	::ShowWindow(hideWindows_->hWnd_, SW_SHOWNORMAL);
	::UpdateWindow(hideWindows_->hWnd_);

}


Window::~Window()
{
	if (hideWindows_->hWnd_ != NULL)
	{
		::DestroyWindow(hideWindows_->hWnd_);
		hideWindows_->hWnd_ = NULL;
	}
}

void Window::Recreate()
{
	HINSTANCE hInstance = ::GetModuleHandle(NULL);

	DWORD style = static_cast<DWORD>(::GetWindowLongPtr(hideWindows_->hWnd_, GWL_STYLE));
	RECT windowRect = { left_, top_, width_, height_ };

	::AdjustWindowRect(&windowRect, style, false);
	::DestroyWindow(hideWindows_->hWnd_);

	hideWindows_->hWnd_ = ::CreateWindow(name_.c_str(), name_.c_str(), style,
		windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, 0, 0, hInstance, NULL);

	::GetClientRect(hideWindows_->hWnd_, &windowRect);
	left_ = windowRect.left;
	top_ = windowRect.top;
	width_ = windowRect.right - windowRect.left;
	height_ = windowRect.bottom - windowRect.top;

	::SetWindowLongPtr(hideWindows_->hWnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	::SetWindowLongPtr(hideWindows_->hWnd_, GWL_STYLE, style);

	::ShowWindow(hideWindows_->hWnd_, SW_SHOWNORMAL);
	::UpdateWindow(hideWindows_->hWnd_);
}


void Window::StartHandlingMessages()
{
	MSG msg;
	BOOL hasMessage;
	// Main message loop:
	running_ = true;
	rendering_ = true;
	while(running_)
	{
		if (active_ && rendering_)
		{
			hasMessage = ::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
		}
		else
		{
			hasMessage = ::GetMessage(&msg, NULL, 0, 0);
		}

		if (hasMessage)
		{
			if (msg.message == WM_QUIT)
			{
				running_ = false;
			}
			else
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}
		else
		{
			OnMessageIdle();
		}
	}
}

void Window::OnResize(uint32 width, uint32 height)
{
	
}

void Window::OnKeyDown(uint32 winKey)
{
	
}

void Window::OnKeyUp(uint32 winKey)
{
	
}

void Window::OnMouseDown(uint32 buttons, uint32 x, uint32 y)
{

}

void Window::OnMouseUp(uint32 buttons, uint32 x, uint32 y)
{

}

void Window::OnMouseWheel(uint32 buttons, uint32 x, uint32 y, uint32 wheelDelta)
{

}

void Window::OnMouseMove(uint32 buttons, uint32 x, uint32 y)
{

}
