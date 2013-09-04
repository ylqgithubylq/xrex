#pragma once

#include "Declare.hpp"

namespace XREX
{
	class RenderingProcess
		: Noncopyable
	{
	public:
		RenderingProcess();
		virtual ~RenderingProcess();

		virtual void RenderScene(SceneSP const& scene) = 0;
	};
}


