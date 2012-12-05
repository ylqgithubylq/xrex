#pragma once

#include "Declare.hpp"
#include "InputCenter.hpp"

#include <string>
#include <memory>
#include <vector>


namespace XREX
{

	class XREX_API Window
		: Noncopyable
	{
	public:
		Window(std::wstring const& name, int32 left, int32 top, int32 width, int32 height);
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

		std::wstring GetTitleText() const;
		void SetTitleText(std::wstring const& text);

		/*
		 *	@hook: pointer is type: std::function<void(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)>*
		 *		the pointer will be cast to the type above and make a copy of the function object.
		 */
		void SetRawWindowsMessageHook(void const* hook);

	protected:
		void Recreate();

		virtual void OnMessageIdle()
		{
		}

		/*
		 *	@return: actual type is HWND, return void* to remove windows.h dependency from hpp.
		 */
		void* GetHWND() const;


	private:
		static InputCenter::InputSemantic InputSemanticFromWindowsVK(uint32 winKey);

	private:

		void OnResize(uint32 width, uint32 height);

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

		uint32 height_;
		uint32 width_;

		uint32 left_;
		uint32 top_;

		std::wstring name_;

	};

}