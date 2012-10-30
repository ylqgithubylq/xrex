#include "XREX.hpp"

#include "Window.hpp"
#include "XREXContext.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif
#include <windows.h>


using std::vector;

namespace XREX
{

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
		std::function<void(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)> messageHook_;
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
		if (messageHook_ != nullptr)
		{
			messageHook_(hWnd, message, wParam, lParam);
		}

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
			{
				wParam = VK_LBUTTON;
				window_.OnMouseDown(wParam, LOWORD(lParam), HIWORD(lParam));
			}
			break;
		case WM_RBUTTONDOWN:
			{
				wParam = VK_RBUTTON;
				window_.OnMouseDown(wParam, LOWORD(lParam), HIWORD(lParam));
			}
			break;
		case WM_MBUTTONDOWN:
			{
				wParam = VK_MBUTTON;
				window_.OnMouseDown(wParam, LOWORD(lParam), HIWORD(lParam));
			}
			break;
		case WM_LBUTTONUP:
			{
				wParam = VK_LBUTTON;
				window_.OnMouseUp(wParam, LOWORD(lParam), HIWORD(lParam));
			}
			break;
		case WM_RBUTTONUP:
			{
				wParam = VK_RBUTTON;
				window_.OnMouseUp(wParam, LOWORD(lParam), HIWORD(lParam));
			}
			break;
		case WM_MBUTTONUP:
			{
				wParam = VK_MBUTTON;
				window_.OnMouseUp(wParam, LOWORD(lParam), HIWORD(lParam));
			}
			break;

		case WM_MOUSEWHEEL:
			{
				// wParam buttons should not be relied on
				window_.OnMouseWheel(GET_KEYSTATE_WPARAM(wParam), LOWORD(lParam), HIWORD(lParam), GET_WHEEL_DELTA_WPARAM(wParam));
			}
			break;

		case WM_MOUSEMOVE:
			{
				// wParam buttons should not be relied on
				window_.OnMouseMove(GET_KEYSTATE_WPARAM(wParam), LOWORD(lParam), HIWORD(lParam));
			}
			break;

		case WM_MOUSELEAVE:
			{

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



	void Window::SetRawWindowsMessageHook(void const* hook)
	{
		hideWindows_->messageHook_ = *static_cast<std::function<void(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)> const*>(hook);
	}


	void* Window::GetHWND() const
	{
		return hideWindows_->hWnd_;
	}



	Window::Window(std::wstring const& name, int32 left, int32 top, int32 width, int32 height)
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

	InputCenter::InputSemantic Window::InputSemanticFromWindowsVK(uint32 winKey)
	{
		static std::vector<InputCenter::InputSemantic> mapping = [] ()
		{
			uint32 const WindowsVKCount = 256;
			vector<InputCenter::InputSemantic> mapping(WindowsVKCount, InputCenter::InputSemantic::InputSemanticInvalid);

			mapping[0] = InputCenter::InputSemantic::NullSemantic;

			mapping[VK_LBUTTON] = InputCenter::InputSemantic::M_Button0;
			mapping[VK_RBUTTON] = InputCenter::InputSemantic::M_Button1;
			mapping[VK_CANCEL] = InputCenter::InputSemantic::InputSemanticInvalid;
			mapping[VK_MBUTTON] = InputCenter::InputSemantic::M_Button2;
			mapping[VK_XBUTTON1] = InputCenter::InputSemantic::M_Button3;
			mapping[VK_XBUTTON2] = InputCenter::InputSemantic::M_Button4;

			mapping[VK_BACK] = InputCenter::InputSemantic::K_BackSpace;
			mapping[VK_TAB] = InputCenter::InputSemantic::K_Tab;
			mapping[VK_CLEAR] = InputCenter::InputSemantic::InputSemanticInvalid;
			mapping[VK_RETURN] = InputCenter::InputSemantic::K_Enter;

			mapping[VK_SHIFT] = InputCenter::InputSemantic::Temp_Shift;
			mapping[VK_CONTROL] = InputCenter::InputSemantic::Temp_Ctrl;
			mapping[VK_MENU] = InputCenter::InputSemantic::Temp_Alt;
			mapping[VK_PAUSE] = InputCenter::InputSemantic::K_Pause;
			mapping[VK_CAPITAL] = InputCenter::InputSemantic::K_CapsLock;

			mapping[VK_KANA] = InputCenter::InputSemantic::InputSemanticInvalid;
			mapping[VK_HANGEUL] = InputCenter::InputSemantic::InputSemanticInvalid;
			mapping[VK_HANGUL] = InputCenter::InputSemantic::InputSemanticInvalid;

			mapping[VK_JUNJA] = InputCenter::InputSemantic::InputSemanticInvalid;
			mapping[VK_FINAL] = InputCenter::InputSemantic::InputSemanticInvalid;
			mapping[VK_HANJA] = InputCenter::InputSemantic::InputSemanticInvalid;
			mapping[VK_KANJI] = InputCenter::InputSemantic::InputSemanticInvalid;

			mapping[VK_ESCAPE] = InputCenter::InputSemantic::K_Escape;
			mapping[VK_CONVERT] = InputCenter::InputSemantic::InputSemanticInvalid;
			mapping[VK_NONCONVERT] = InputCenter::InputSemantic::InputSemanticInvalid;
			mapping[VK_ACCEPT] = InputCenter::InputSemantic::InputSemanticInvalid;
			mapping[VK_MODECHANGE] = InputCenter::InputSemantic::InputSemanticInvalid;

			mapping[VK_SPACE] = InputCenter::InputSemantic::K_Space;
			mapping[VK_PRIOR] = InputCenter::InputSemantic::K_PageUp;
			mapping[VK_NEXT] = InputCenter::InputSemantic::K_PageDown;
			mapping[VK_END] = InputCenter::InputSemantic::K_End;
			mapping[VK_HOME] = InputCenter::InputSemantic::K_Home;
			mapping[VK_LEFT] = InputCenter::InputSemantic::K_LeftArrow;
			mapping[VK_UP] = InputCenter::InputSemantic::K_UpArrow;
			mapping[VK_RIGHT] = InputCenter::InputSemantic::K_RightArrow;
			mapping[VK_DOWN] = InputCenter::InputSemantic::K_DownArrow;
			mapping[VK_SELECT] = InputCenter::InputSemantic::InputSemanticInvalid;
			mapping[VK_PRINT] = InputCenter::InputSemantic::InputSemanticInvalid;
			mapping[VK_EXECUTE] = InputCenter::InputSemantic::InputSemanticInvalid;
			mapping[VK_SNAPSHOT] = InputCenter::InputSemantic::InputSemanticInvalid;
			mapping[VK_INSERT] = InputCenter::InputSemantic::K_Insert;
			mapping[VK_DELETE] = InputCenter::InputSemantic::K_Delete;
			mapping[VK_HELP] = InputCenter::InputSemantic::InputSemanticInvalid;

			mapping['0'] = InputCenter::InputSemantic::K_0;
			mapping['1'] = InputCenter::InputSemantic::K_1;
			mapping['2'] = InputCenter::InputSemantic::K_2;
			mapping['3'] = InputCenter::InputSemantic::K_3;
			mapping['4'] = InputCenter::InputSemantic::K_4;
			mapping['5'] = InputCenter::InputSemantic::K_5;
			mapping['6'] = InputCenter::InputSemantic::K_6;
			mapping['7'] = InputCenter::InputSemantic::K_7;
			mapping['8'] = InputCenter::InputSemantic::K_8;
			mapping['9'] = InputCenter::InputSemantic::K_9;

			mapping['A'] = InputCenter::InputSemantic::K_A;
			mapping['B'] = InputCenter::InputSemantic::K_B;
			mapping['C'] = InputCenter::InputSemantic::K_C;
			mapping['D'] = InputCenter::InputSemantic::K_D;
			mapping['E'] = InputCenter::InputSemantic::K_E;
			mapping['F'] = InputCenter::InputSemantic::K_F;
			mapping['G'] = InputCenter::InputSemantic::K_G;

			mapping['H'] = InputCenter::InputSemantic::K_H;
			mapping['I'] = InputCenter::InputSemantic::K_I;
			mapping['J'] = InputCenter::InputSemantic::K_J;
			mapping['K'] = InputCenter::InputSemantic::K_K;
			mapping['L'] = InputCenter::InputSemantic::K_L;
			mapping['M'] = InputCenter::InputSemantic::K_M;
			mapping['N'] = InputCenter::InputSemantic::K_N;

			mapping['O'] = InputCenter::InputSemantic::K_O;
			mapping['P'] = InputCenter::InputSemantic::K_P;
			mapping['Q'] = InputCenter::InputSemantic::K_Q;
			mapping['R'] = InputCenter::InputSemantic::K_R;
			mapping['S'] = InputCenter::InputSemantic::K_S;
			mapping['T'] = InputCenter::InputSemantic::K_T;

			mapping['U'] = InputCenter::InputSemantic::K_U;
			mapping['V'] = InputCenter::InputSemantic::K_V;
			mapping['W'] = InputCenter::InputSemantic::K_W;
			mapping['X'] = InputCenter::InputSemantic::K_X;
			mapping['Y'] = InputCenter::InputSemantic::K_Y;
			mapping['Z'] = InputCenter::InputSemantic::K_Z;

			mapping[VK_LWIN] = InputCenter::InputSemantic::K_LeftWin;
			mapping[VK_RWIN] = InputCenter::InputSemantic::K_RightWin;
			mapping[VK_APPS] = InputCenter::InputSemantic::K_Apps;

			mapping[VK_SLEEP] = InputCenter::InputSemantic::K_Sleep;

			mapping[VK_NUMPAD0] = InputCenter::InputSemantic::K_NumPad0;
			mapping[VK_NUMPAD1] = InputCenter::InputSemantic::K_NumPad1;
			mapping[VK_NUMPAD2] = InputCenter::InputSemantic::K_NumPad2;
			mapping[VK_NUMPAD3] = InputCenter::InputSemantic::K_NumPad3;
			mapping[VK_NUMPAD4] = InputCenter::InputSemantic::K_NumPad4;
			mapping[VK_NUMPAD5] = InputCenter::InputSemantic::K_NumPad5;
			mapping[VK_NUMPAD6] = InputCenter::InputSemantic::K_NumPad6;
			mapping[VK_NUMPAD7] = InputCenter::InputSemantic::K_NumPad7;
			mapping[VK_NUMPAD8] = InputCenter::InputSemantic::K_NumPad8;
			mapping[VK_NUMPAD9] = InputCenter::InputSemantic::K_NumPad9;

			mapping[VK_MULTIPLY] = InputCenter::InputSemantic::K_NumPadAsterisk;
			mapping[VK_ADD] = InputCenter::InputSemantic::K_NumPadPlus;
			mapping[VK_SEPARATOR] = InputCenter::InputSemantic::InputSemanticInvalid;
			mapping[VK_SUBTRACT] = InputCenter::InputSemantic::K_NumPadMinus;
			mapping[VK_DECIMAL] = InputCenter::InputSemantic::K_NumPadPeriod;
			mapping[VK_DIVIDE] = InputCenter::InputSemantic::K_NumPadSlash;

			mapping[VK_F1] = InputCenter::InputSemantic::K_F1;
			mapping[VK_F2] = InputCenter::InputSemantic::K_F2;
			mapping[VK_F3] = InputCenter::InputSemantic::K_F3;
			mapping[VK_F4] = InputCenter::InputSemantic::K_F4;
			mapping[VK_F5] = InputCenter::InputSemantic::K_F5;
			mapping[VK_F6] = InputCenter::InputSemantic::K_F6;
			mapping[VK_F7] = InputCenter::InputSemantic::K_F7;
			mapping[VK_F8] = InputCenter::InputSemantic::K_F8;
			mapping[VK_F9] = InputCenter::InputSemantic::K_F9;
			mapping[VK_F10] = InputCenter::InputSemantic::K_F10;
			mapping[VK_F11] = InputCenter::InputSemantic::K_F11;
			mapping[VK_F12] = InputCenter::InputSemantic::K_F12;
			mapping[VK_F13] = InputCenter::InputSemantic::K_F13;
			mapping[VK_F14] = InputCenter::InputSemantic::K_F14;
			mapping[VK_F15] = InputCenter::InputSemantic::K_F15;
			mapping[VK_F16] = InputCenter::InputSemantic::InputSemanticInvalid;
			mapping[VK_F17] = InputCenter::InputSemantic::InputSemanticInvalid;
			mapping[VK_F18] = InputCenter::InputSemantic::InputSemanticInvalid;
			mapping[VK_F19] = InputCenter::InputSemantic::InputSemanticInvalid;
			mapping[VK_F20] = InputCenter::InputSemantic::InputSemanticInvalid;
			mapping[VK_F21] = InputCenter::InputSemantic::InputSemanticInvalid;
			mapping[VK_F22] = InputCenter::InputSemantic::InputSemanticInvalid;
			mapping[VK_F23] = InputCenter::InputSemantic::InputSemanticInvalid;
			mapping[VK_F24] = InputCenter::InputSemantic::InputSemanticInvalid;

			mapping[VK_NUMLOCK] = InputCenter::InputSemantic::K_NumLock;
			mapping[VK_SCROLL] = InputCenter::InputSemantic::K_ScrollLock;

			mapping[VK_OEM_NEC_EQUAL] = InputCenter::InputSemantic::K_NumPadEquals;


			mapping[VK_LSHIFT] = InputCenter::InputSemantic::K_LeftShift;
			mapping[VK_RSHIFT] = InputCenter::InputSemantic::K_RightShift;
			mapping[VK_LCONTROL] = InputCenter::InputSemantic::K_LeftCtrl;
			mapping[VK_RCONTROL] = InputCenter::InputSemantic::K_RightCtrl;
			mapping[VK_LMENU] = InputCenter::InputSemantic::K_LeftAlt;
			mapping[VK_RMENU] = InputCenter::InputSemantic::K_RightAlt;

			mapping[VK_BROWSER_BACK] = InputCenter::InputSemantic::K_WebBack;
			mapping[VK_BROWSER_FORWARD] = InputCenter::InputSemantic::K_WebForward;
			mapping[VK_BROWSER_REFRESH] = InputCenter::InputSemantic::K_WebRefresh;
			mapping[VK_BROWSER_STOP] = InputCenter::InputSemantic::K_WebStop;
			mapping[VK_BROWSER_SEARCH] = InputCenter::InputSemantic::K_WebSearch;
			mapping[VK_BROWSER_FAVORITES] = InputCenter::InputSemantic::K_WebFavorites;
			mapping[VK_BROWSER_HOME] = InputCenter::InputSemantic::K_WebHome;

			mapping[VK_VOLUME_MUTE] = InputCenter::InputSemantic::K_Mute;
			mapping[VK_VOLUME_DOWN] = InputCenter::InputSemantic::K_VolumeUp;
			mapping[VK_VOLUME_UP] = InputCenter::InputSemantic::K_VolumeDown;
			mapping[VK_MEDIA_NEXT_TRACK] = InputCenter::InputSemantic::K_NextTrack;
			mapping[VK_MEDIA_PREV_TRACK] = InputCenter::InputSemantic::K_PrevTrack;
			mapping[VK_MEDIA_STOP] = InputCenter::InputSemantic::K_MediaStop;
			mapping[VK_MEDIA_PLAY_PAUSE] = InputCenter::InputSemantic::K_PlayPause;
			mapping[VK_LAUNCH_MAIL] = InputCenter::InputSemantic::K_Mail;
			mapping[VK_LAUNCH_MEDIA_SELECT] = InputCenter::InputSemantic::K_MediaSelect;
			mapping[VK_LAUNCH_APP1] = InputCenter::InputSemantic::InputSemanticInvalid;
			mapping[VK_LAUNCH_APP2] = InputCenter::InputSemantic::InputSemanticInvalid;

			mapping[VK_OEM_1] = InputCenter::InputSemantic::K_Semicolon;
			mapping[VK_OEM_PLUS] = InputCenter::InputSemantic::InputSemanticInvalid;
			mapping[VK_OEM_COMMA] = InputCenter::InputSemantic::K_Comma;
			mapping[VK_OEM_MINUS] = InputCenter::InputSemantic::K_Minus;
			mapping[VK_OEM_PERIOD] = InputCenter::InputSemantic::K_Period;
			mapping[VK_OEM_2] = InputCenter::InputSemantic::K_Slash;
			mapping[VK_OEM_3] = InputCenter::InputSemantic::K_Tilde;

			mapping[VK_OEM_4] = InputCenter::InputSemantic::K_LeftBracket;
			mapping[VK_OEM_5] = InputCenter::InputSemantic::K_BackSlash;
			mapping[VK_OEM_6] = InputCenter::InputSemantic::K_RightBracket;
			mapping[VK_OEM_7] = InputCenter::InputSemantic::K_Quote;
			mapping[VK_OEM_8] = InputCenter::InputSemantic::InputSemanticInvalid;

			return mapping;
		} ();

		return mapping[winKey];
	}




	void Window::OnResize(uint32 width, uint32 height)
	{
		// TODO
	}

	void Window::OnKeyDown(uint32 winKey)
	{
		winKey = DistinguishLeftRightShiftCtrlAlt(winKey, true);
		XREXContext::GetInstance().GetInputCenter().InjectKeyDown(InputSemanticFromWindowsVK(winKey));
	}

	void Window::OnKeyUp(uint32 winKey)
	{
		winKey = DistinguishLeftRightShiftCtrlAlt(winKey, false);
		XREXContext::GetInstance().GetInputCenter().InjectKeyUp(InputSemanticFromWindowsVK(winKey));
	}

	void Window::OnMouseDown(uint32 winKey, uint32 x, uint32 y)
	{
		XREXContext::GetInstance().GetInputCenter().InjectMouseDown(InputSemanticFromWindowsVK(winKey), x, height_ - y);
	}

	void Window::OnMouseUp(uint32 winKey, uint32 x, uint32 y)
	{
		XREXContext::GetInstance().GetInputCenter().InjectMouseUp(InputSemanticFromWindowsVK(winKey), x, height_ - y);
	}

	void Window::OnMouseWheel(uint32 winKey, uint32 x, uint32 y, int32 wheelDelta)
	{
		XREXContext::GetInstance().GetInputCenter().InjectMouseWheel(InputCenter::InputSemantic::M_Wheel, x, height_ - y, wheelDelta);
	}

	void Window::OnMouseMove(uint32 winKey, uint32 x, uint32 y)
	{
		XREXContext::GetInstance().GetInputCenter().InjectMouseMove(InputCenter::InputSemantic::M_Move, x, height_ - y);
	}

	uint32 Window::DistinguishLeftRightShiftCtrlAlt(uint32 winKey, bool down)
	{
		if (down)
		{
			switch (winKey)
			{
			case VK_CONTROL:
				{
					if (::GetKeyState(VK_LCONTROL) < 0) // highest bit is 1
					{
						winKey = VK_LCONTROL;
						leftCtrl_ = true;
					}
					else if (::GetKeyState(VK_RCONTROL) < 0) // highest bit is 1
					{
						winKey = VK_RCONTROL;
						rightCtrl_ = true;
					}
				}
				break;
			case VK_SHIFT:
				{
					if (::GetKeyState(VK_LSHIFT) < 0) // highest bit is 1
					{
						winKey = VK_LSHIFT;
						leftShift_ = true;
					}
					else if (::GetKeyState(VK_RSHIFT) < 0) // highest bit is 1
					{
						winKey = VK_RSHIFT;
						rightShift_ = true;
					}
				}
				break;
			case VK_MENU:
				{
					if (::GetKeyState(VK_LMENU) < 0) // highest bit is 1
					{
						winKey = VK_LMENU;
						leftAlt_ = true;
					}
					else if (::GetKeyState(VK_RMENU) < 0) // highest bit is 1
					{
						winKey = VK_RMENU;
						rightAlt_ = true;
					}
				}
				break;
			default:
				break;
			}
		}
		else
		{
			switch (winKey)
			{
			case VK_CONTROL:
				{
					if (leftCtrl_ && !(::GetKeyState(VK_LCONTROL) < 0))
					{
						winKey = VK_LCONTROL;
						leftCtrl_ = false;
					}
					else if (rightCtrl_ && !(::GetKeyState(VK_RCONTROL) < 0))
					{
						winKey = VK_RCONTROL;
						rightCtrl_ = false;
					}
				}
				break;
			case VK_SHIFT:
				{
					if (leftShift_ && !(::GetKeyState(VK_LSHIFT) < 0))
					{
						winKey = VK_LSHIFT;
						leftShift_ = false;
					}
					else if (rightShift_ && !(::GetKeyState(VK_RSHIFT) < 0))
					{
						winKey = VK_RSHIFT;
						rightCtrl_ = false;
					}
				}
				break;
			case VK_MENU:
				{
					if (leftAlt_ && !(::GetKeyState(VK_LMENU) < 0))
					{
						winKey = VK_LMENU;
						leftAlt_ = false;
					}
					else if (rightAlt_ && !(::GetKeyState(VK_RMENU) < 0))
					{
						winKey = VK_RMENU;
						rightAlt_ = false;
					}
				}
				break;
			default:
				break;
			}
		}
		return winKey;
	}

}
