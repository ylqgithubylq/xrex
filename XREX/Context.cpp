#include "XREX.hpp"

#include "Context.hpp"

#include "GLWindow.hpp"


Context::Context()
	: renderingEngine_(MakeUP<RenderingEngine>())
{

}


Context::~Context()
{
}


void Context::Initialize(Settings const & settings)
{
	settings_ = settings;
	InitializeMainWindow(settings.windowTitle, settings.renderingSettings);
}


void Context::InitializeMainWindow(std::wstring const & name, RenderingSettings const & settings)
{
	mainWindow_ = std::move(MakeUP<GLWindow>(name, settings));
}

void Context::Start()
{
	mainWindow_->StartHandlingMessages();
}
