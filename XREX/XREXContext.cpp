#include "XREX.hpp"

#include "XREXContext.hpp"

#include "GLWindow.hpp"

#include "RenderingFactory.hpp"
#include "RenderingEngine.hpp"
#include "InputCenter.hpp"
#include "ResourceManager.hpp"
#include "LocalResourceLoader.hpp"

namespace XREX
{

	XREXContext::XREXContext()
		: settings_("") // temp root path
	{
	}


	XREXContext::~XREXContext()
	{
		mainWindow_->SetRunning(false);

		resourceLoader_.reset();
		resourceManager_.reset();
		inputCenter_.reset();

		// make these two released last
		renderingFactory_.reset();
		renderingEngine_ = nullptr;
		mainWindow_.reset();
	}


	void XREXContext::Initialize(Settings const& settings)
	{
		settings_ = settings;
		resourceManager_ = MakeUP<ResourceManager>(settings.rootPath);
		renderingFactory_ = MakeUP<RenderingFactory>();
		inputCenter_ = MakeUP<InputCenter>();
		resourceLoader_ = MakeUP<LocalResourceLoader>();

		InitializeMainWindow(settings.windowTitle, settings.renderingSettings);
		renderingFactory_->Initialize();
		renderingEngine_ = &renderingFactory_->GetRenderingEngine();
		renderingEngine_->Initialize();
	}


	void XREXContext::InitializeMainWindow(std::wstring const& name, RenderingSettings const& settings)
	{
		mainWindow_ = std::move(MakeUP<GLWindow>(name, settings));
	}


	void XREXContext::Start()
	{
		timer_.Restart();
		renderingEngine_->Start();
		mainWindow_->StartHandlingMessages();
		// when runs to here, the whole program will stop running.
	}

	void XREXContext::ExecuteALogicFrame()
	{
		inputCenter_->ExecuteAllQueuedActions();
		double currentTime = timer_.Elapsed();
		double delta = currentTime - lastTime_;
		if (logicFunction_)
		{
			if (!logicFunction_(currentTime, delta))
			{
				mainWindow_->SetRunning(false);
			}
		}
		lastTime_ = currentTime;
	}

	void XREXContext::RenderAFrame()
	{
		ExecuteALogicFrame(); // temp, move to somewhere else when using separate thread to do logic.
		renderingEngine_->RenderAFrame();
	}


}