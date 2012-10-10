#pragma once

#include "Declare.hpp"

#include "Settings.hpp"
#include "Timer.hpp"

#include <string>


class Application
	: Noncopyable
{
public:
	static Application& GetInstance()
	{
		static Application context;
		return context;
	}
	RenderingFactory& GetRenderingFactory() const
	{
		return *renderingFactory_;
	}
	RenderingEngine& GetRenderingEngine() const
	{
		return *renderingEngine_;
	}
	Window& GetMainWindow() const
	{
		return *mainWindow_;
	}
	InputCenter& GetInputCenter() const
	{
		return *inputCenter_;
	}
	ResourceManager& GetResourceManager() const
	{
		return *resourceManager_;
	}
	LocalResourceLoader& GetResourceLoader() const
	{
		return *resourceLoader_;
	}

	Settings const& GetSettings() const
	{
		return settings_;
	}

	double GetElapsedTime() const
	{
		return timer_.Elapsed();
	}

	void Initialize(Settings const& settings);
	/*
	 *	Block until this program stop running.
	 */
	void Start();

	void ExecuteALogicFrame();

	void RenderAFrame();

private:
	void InitializeMainWindow(std::wstring const& name, RenderingSettings const& settings);

private:
	Application();
	~Application();

private:
	std::unique_ptr<Window> mainWindow_;
	std::unique_ptr<RenderingFactory> renderingFactory_;
	std::unique_ptr<RenderingEngine> renderingEngine_;
	std::unique_ptr<InputCenter> inputCenter_;
	std::unique_ptr<ResourceManager> resourceManager_;
	std::unique_ptr<LocalResourceLoader> resourceLoader_;
	Settings settings_;
	Timer timer_;
};

