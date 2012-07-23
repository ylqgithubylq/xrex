#include "XREX.hpp"

#include "RenderingEngine.hpp"



RenderingEngine::RenderingEngine()
{
}


RenderingEngine::~RenderingEngine()
{
}

RenderingEngine& RenderingEngine::GetInstance()
{
	static RenderingEngine engine;
	return engine;
}

void RenderingEngine::Update()
{
	if (renderingFunction_ != nullptr)
	{
		renderingFunction_(0.0);
	}
}
