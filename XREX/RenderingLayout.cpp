#include "XREX.hpp"

#include "RenderingLayout.hpp"
#include "GraphicsBuffer.hpp"

#include <CoreGL.hpp>

using std::vector;

RenderingLayout::RenderingLayout(vector<GraphicsBufferSP> const & buffers, GraphicsBufferSP& indexBuffer, DrawingMode mode)
	: buffers_(buffers), indexBuffer_(indexBuffer), mode_(mode)
{
#ifdef XREX_DEBUG
	int32 elementCount = -1;
	for (auto i = buffers_.begin(); i != buffers_.end(); ++i)
	{
		if (elementCount == -1)
		{
			elementCount = (*i)->GetElementCount();
		}
		else
		{
			assert(elementCount == (*i)->GetElementCount());
		}
	}
#endif
}


RenderingLayout::~RenderingLayout()
{
}

void RenderingLayout::BindToProgram(ProgramObject const & program)
{
	// TODO build VAO
	for (auto i = buffers_.begin(); i != buffers_.end(); ++i)
	{
		(*i)->BindToProgram(program);
	}
	indexBuffer_->Bind();
}

void RenderingLayout::Unbind()
{
	for (auto i = buffers_.begin(); i != buffers_.end(); ++i)
	{
		(*i)->Unbind();
	}
}


ElementType RenderingLayout::GetIndexElementType() const
{
	return indexBuffer_->GetDataDescription().GetChannelLayout("").elementType;
}
