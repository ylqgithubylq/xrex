#pragma once

#include "Declare.hpp"

#include "Settings.hpp"



#include <string>

class Context
	: Noncopyable
{
public:
	static Context& GetInstance()
	{
		static Context context;
		return context;
	}

	RenderingEngine& GetRenderingEngine() const
	{
		return *renderingEngine_;
	}
	Window& GetMainWindow() const
	{
		return *mainWindow_.get();
	}

	void Initialize(Settings const & settings);
	void Start();

private:
	void InitializeMainWindow(std::wstring const & name, RenderingSettings const & settings);

private:
	Context();
	~Context();

private:
	std::unique_ptr<Window> mainWindow_;
	std::unique_ptr<RenderingEngine> renderingEngine_;
};

