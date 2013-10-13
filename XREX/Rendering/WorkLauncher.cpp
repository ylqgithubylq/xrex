#include "XREX.hpp"

#include "WorkLauncher.hpp"

#include "Rendering/RenderingLayout.hpp"
#include "Rendering/ShaderProgram.hpp"
#include "Rendering/ProgramConnector.hpp"
#include "Rendering/RenderingTechnique.hpp"
#include "Rendering/GL/GLUtil.hpp"

#include <CoreGL.hpp>


namespace XREX
{
	IWorkLauncher::~IWorkLauncher()
	{
	}

	void IndexedDrawer::Launch()
	{
		assert(layout_->GetIndexBuffer() != nullptr);
		assert(layoutConnector_ != nullptr);
		assert(technique_ != nullptr);
		technique_->Use();
		layoutConnector_->Bind();
		CoreLaunch();
		layoutConnector_->Unbind();
	}

	void IndexedDrawer::CoreLaunch()
	{
		assert(layout_ != nullptr);
		assert(layout_->GetIndexBuffer() != nullptr);
		gl::DrawElements(GLDrawModeFromTopologicalType(layout_->GetIndexBuffer()->GetTopologicalType()),
			layout_->GetElementCount(),  GLTypeFromElementType(layout_->GetIndexElementType()), reinterpret_cast<void const*>(0));
	}

}
