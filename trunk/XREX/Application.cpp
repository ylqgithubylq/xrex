#include "XREX.hpp"

#include "Application.hpp"

#include "GLWindow.hpp"

#include "RenderingEngine.hpp"
#include "InputCenter.hpp"


Application::Application()
	: renderingEngine_(MakeUP<RenderingEngine>()), inputCenter_(MakeUP<InputCenter>())
{

}


Application::~Application()
{
}


void Application::Initialize(Settings const & settings)
{
	settings_ = settings;
	InitializeMainWindow(settings.windowTitle, settings.renderingSettings);
	renderingEngine_->Initialize();
}


void Application::InitializeMainWindow(std::wstring const & name, RenderingSettings const & settings)
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
	ExecuteALogicFrame(); // temp, remove when using seperate thread to do logic.
	renderingEngine_->Update();
}
