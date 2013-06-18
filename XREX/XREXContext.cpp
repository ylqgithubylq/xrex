#include "XREX.hpp"

#include "XREXContext.hpp"

#include "Window.hpp"
#include "GraphicsContext.hpp"

#include "RenderingFactory.hpp"
#include "RenderingEngine.hpp"
#include "InputCenter.hpp"
#include "ResourceManager.hpp"
#include "LocalResourceLoader.hpp"

#include "NaiveManagedScene.hpp"
#include "DefaultRenderingProcess.hpp"


namespace XREX
{

	XREXContext::XREXContext()
		: scene_(MakeSP<NaiveManagedScene>()), settings_("") // temp root path
	{
	}


	XREXContext::~XREXContext()
	{
		mainWindow_->SetRunning(false);

		resourceLoader_.reset();
		resourceManager_.reset();
		inputCenter_.reset();

		scene_.reset();

		// make these two released last
		renderingFactory_.reset();
		renderingEngine_ = nullptr;
		mainWindow_.reset();
	}


	void XREXContext::Initialize(Settings const& settings)
	{
		settings_ = settings;
		resourceManager_ = MakeUP<ResourceManager>(settings_.rootPath);
		inputCenter_ = MakeUP<InputCenter>();
		resourceLoader_ = MakeUP<LocalResourceLoader>();

		InitializeMainWindow();
		InitializeGraphicsContext();

		mainWindow_->SetMessageIdle([this] () // using GUI thread do the rendering
		{
			RenderAFrame();
		});
	}


	void XREXContext::InitializeMainWindow()
	{
		mainWindow_ = MakeUP<Window>(settings_);
		// window size and position will change while creating window, set those values after creating window.
		Size<uint32> clientRegionSize = mainWindow_->GetClientRegionSize();
		settings_.renderingSettings.width = clientRegionSize.x;
		settings_.renderingSettings.height = clientRegionSize.y;
		Size<int32> windowPosition = mainWindow_->GetWindowPosition();
		settings_.renderingSettings.left = windowPosition.x;
		settings_.renderingSettings.top = windowPosition.y;
	}


	void XREXContext::InitializeGraphicsContext()
	{
		renderingFactory_ = MakeUP<RenderingFactory>(*mainWindow_, settings_);
		renderingEngine_ = &renderingFactory_->GetRenderingEngine();
		renderingEngine_->SetRenderingProcess(MakeSP<DefaultRenderingProcess>());
	}



	void XREXContext::Start()
	{
		timer_.Restart();
		mainWindow_->SetActive(true);
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
		renderingEngine_->SwapBuffers();
		
	}


}