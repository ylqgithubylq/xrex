#pragma once

#include "Declare.hpp"
#include "InputCenter.hpp"

#include <string>
#include <vector>


namespace XREX
{

	class XREX_API Window
		: Noncopyable
	{
	public:
		/*
		 *	Size and position of created window may not equal to the values in settings.
		 */
		Window(Settings const& settings);
		virtual ~Window();

		void StartHandlingMessages();

		bool IsRendering() const
		{
			return rendering_;
		}
		void SetRendering(bool rendering)
		{
			rendering_ = rendering;
		}
		bool IsRunning() const
		{
			return running_;
		}
		void SetRunning(bool running)
		{
			running_ = running;
		}

		bool IsActive() const
		{
			return active_;
		}
		void SetActive(bool active)
		{
			active_ = active;
		}

		/*
		 *	Client region size, not the window size.
		 */
		Size<uint32> GetClientRegionSize() const
		{
			return Size<uint32>(width_, height_);
		}

		/*
		 *	Upper left corner of the window.
		 */
		Size<int32> GetWindowPosition() const
		{
			return Size<int32>(windowLeft_, windowTop_);
		}

		std::wstring GetTitleText() const;
		void SetTitleText(std::wstring const& text);

		/*
		 *	@hook: pointer is type: std::function<void(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)>*
		 *		the pointer will be cast to the type above and make a copy of the function object.
		 */
		void SetRawWindowsMessageHook(void const* hook);

		/*
		 *	@return: actual type is HWND, return void* to remove windows.h dependency from hpp.
		 */
		void* GetHWND() const;

		void Recreate();

		void SetMessageIdle(std::function<void()> const& messageIdle)
		{
			messageIdle_ = messageIdle;
		}

	protected:

		void OnMessageIdle();

	private:
		static InputCenter::InputSemantic InputSemanticFromWindowsVK(uint32 winKey);

	private:

		// void OnResize(uint32 width, uint32 height);

		void OnKeyDown(uint32 winKey);
		void OnKeyUp(uint32 winKey);
		void OnMouseDown(uint32 winKey, uint32 x, uint32 y);
		void OnMouseUp(uint32 winKey, uint32 x, uint32 y);
		void OnMouseWheel(uint32 winKey, uint32 x, uint32 y, int32 wheelDelta);
		void OnMouseMove(uint32 winKey, uint32 x, uint32 y);

		uint32 DistinguishLeftRightShiftCtrlAlt(uint32 winKey, bool down);

	protected:
		 // used to hide windows.h to the cpp file
		struct HideWindows_;
		std::unique_ptr<HideWindows_> hideWindows_;

		bool leftShift_;
		bool rightShift_;
		bool leftCtrl_;
		bool rightCtrl_;
		bool leftAlt_;
		bool rightAlt_;

		bool active_;
		bool running_;
		bool rendering_;

		bool fullScreen_;

		uint32 height_;
		uint32 width_;

		int32 left_;
		int32 top_;

		int32 windowLeft_;
		int32 windowTop_;

		std::wstring name_;

		std::function<void()> messageIdle_;
	};

}
