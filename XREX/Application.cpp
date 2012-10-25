#include "XREX.hpp"

#include "Application.hpp"

#include "GLWindow.hpp"

#include "RenderingFactory.hpp"
#include "RenderingEngine.hpp"
#include "InputCenter.hpp"
#include "ResourceManager.hpp"
#include "LocalResourceLoader.hpp"


Application::Application()
	: renderingFactory_(MakeUP<RenderingFactory>()), renderingEngine_(MakeUP<RenderingEngine>()), inputCenter_(MakeUP<InputCenter>()), resourceManager_(MakeUP<ResourceManager>()), resourceLoader_(MakeUP<LocalResourceLoader>())
{

}


Application::~Application()
{
	resourceLoader_.reset();
	resourceManager_.reset();
	inputCenter_.reset();
	renderingFactory_.reset();

	// make these two released last
	renderingEngine_.reset();
	mainWindow_.reset();
}


void Application::Initialize(Settings const& settings)
{
	settings_ = settings;
	InitializeMainWindow(settings.windowTitle, settings.renderingSettings);
	renderingEngine_->Initialize();
}


void Application::InitializeMainWindow(std::wstring const& name, RenderingSettings const& settings)
{
	mainWindow_ = std::move(MakeUP<GLWindow>(name, settings));
}


void Application::Start()
{
	timer_.Restart();
	renderingEngine_->Start();
	mainWindow_->StartHandlingMessages();
}

void Application::ExecuteALogicFrame()
{
	inputCenter_->ExecuteAllQueuedActions();
}

void Application::RenderAFrame()
{
	ExecuteALogicFrame(); // temp, remove when using separate thread to do logic.
	renderingEngine_->RenderAFrame();
}
