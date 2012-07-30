#pragma once

#include "Declare.hpp"

#include <functional>
#include <vector>

class RenderingEngine
	: Noncopyable
{

public:
	RenderingEngine();
	~RenderingEngine();

	void Update();

	// Temp function for convenience
	void SetRenderingFunction(std::function<void(double delta)>& renderingFunction)
	{
		renderingFunction_ = renderingFunction;
	}


private:
	std::function<void(double delta)> renderingFunction_;
	std::vector<CameraSP> cameras_;
	CameraSP currentCamera_;
};

