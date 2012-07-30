#include "XREX.hpp"

#include "RenderingEngine.hpp"



RenderingEngine::RenderingEngine()
{
}


RenderingEngine::~RenderingEngine()
{
}


void RenderingEngine::Update()
{
	if (renderingFunction_ != nullptr)
	{
		renderingFunction_(0.0);
	}
}
