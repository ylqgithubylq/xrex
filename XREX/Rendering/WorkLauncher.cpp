#include "XREX.hpp"

#include "WorkLauncher.hpp"

#include "Rendering/RenderingLayout.hpp"
#include "Rendering/Shader.hpp"
#include "Rendering/BufferAndProgramConnector.hpp"

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
		gl::DrawElements(GLDrawModeFromTopologicalType(layout_->GetIndexBuffer()->GetTopologicalType()),
			layout_->GetElementCount(),  GLTypeFromElementType(layout_->GetIndexElementType()), reinterpret_cast<void const*>(0));
	}

}
