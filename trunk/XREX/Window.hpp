#pragma once

#include "Declare.hpp"

#include <string>
#include <memory>




class Window
	: Noncopyable
{
public:
	Window(std::wstring const & name, int32 left, int32 top, int32 width, int32 height);
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


	// TODO add event register unregister, fire event

	void OnResize(uint32 width, uint32 height);

	void OnKeyDown(uint32 winKey);
	void OnKeyUp(uint32 winKey);
	void OnMouseDown(uint32 buttons, uint32 x, uint32 y);
	void OnMouseUp(uint32 buttons, uint32 x, uint32 y);
	void OnMouseWheel(uint32 buttons, uint32 x, uint32 y, uint32 wheelDelta);
	void OnMouseMove(uint32 buttons, uint32 x, uint32 y);

protected:
	 // used to hide windows.h to the cpp file
	struct HideWindows_;
	std::unique_ptr<HideWindows_> hideWindows_;

	bool active_;
	bool running_;
	bool rendering_;

	uint32 height_;
	uint32 width_;

	uint32 left_;
	uint32 top_;

	std::wstring name_;

};

