#pragma once
#include "Declare.hpp"
#include "Rendering/RenderingProcess.hpp"

namespace XREX
{
	class DefaultRenderingProcess
		: public RenderingProcess
	{
	public:
		DefaultRenderingProcess();
		virtual ~DefaultRenderingProcess() override;

		virtual void RenderScene(SceneSP const& scene) override;

	private:
		void RenderACamera(SceneSP const& scene, SceneObjectSP const& cameraObject);

	};
}
