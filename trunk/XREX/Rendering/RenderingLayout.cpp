#include "XREX.hpp"

#include "RenderingLayout.hpp"

#include "Rendering/GraphicsBuffer.hpp"
#include "Rendering/ShaderProgram.hpp"
#include "Rendering/GL/GLUtil.hpp"

#include <CoreGL.hpp>

#include <array>

using std::vector;


namespace XREX
{

	RenderingLayout::RenderingLayout(vector<VertexBufferSP> const& buffers, IndexBufferSP const& indexBuffer)
		: buffers_(buffers), indexBuffer_(indexBuffer)
	{

	#ifdef XREX_DEBUG
		int32 elementCount = -1;
		for (auto& buffer : buffers_)
		{
			if (elementCount == -1)
			{
				elementCount = buffer->GetElementCount();
			}
			else
			{
				assert(elementCount == buffer->GetElementCount());
			}
		}
	#endif
	}


	RenderingLayout::~RenderingLayout()
	{
	}

	ElementType RenderingLayout::GetIndexElementType() const
	{
		return indexBuffer_->GetElementType();
	}
}
