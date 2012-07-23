#pragma once

#include "Declare.hpp"

#include <functional>

class RenderingEngine
	: Noncopyable
{
public:
	static RenderingEngine& GetInstance();

public:
	void Update();

	// Temp function for convenience
	void SetRenderingFunction(std::function<void(double delta)>& renderingFunction)
	{
		renderingFunction_ = renderingFunction;
	}

private:
	RenderingEngine();
	~RenderingEngine();

private:
	std::function<void(double delta)> renderingFunction_;

};

