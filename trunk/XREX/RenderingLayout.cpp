#include "XREX.hpp"

#include "RenderingLayout.hpp"
#include "GraphicsBuffer.hpp"
#include "GLUtil.hpp"

#include <CoreGL.hpp>


using std::vector;



std::vector<uint32> RenderingLayout::InitializeGLDrawTypeMapping()
{
	vector<uint32> mapping(DrawingMode::DrawingModeCount);
	mapping[static_cast<uint32>(DrawingMode::Points)] = gl::GL_POINTS;
	mapping[static_cast<uint32>(DrawingMode::LineStrip)] = gl::GL_LINE_STRIP;
	mapping[static_cast<uint32>(DrawingMode::LineLoop)] = gl::GL_LINE_LOOP;
	mapping[static_cast<uint32>(DrawingMode::Lines)] = gl::GL_LINES;
	mapping[static_cast<uint32>(DrawingMode::TriangleStrip)] = gl::GL_TRIANGLE_STRIP;
	mapping[static_cast<uint32>(DrawingMode::TriangleFan)] = gl::GL_TRIANGLE_FAN;
	mapping[static_cast<uint32>(DrawingMode::Triangles)] = gl::GL_TRIANGLES;
	return move(mapping);
}

std::vector<uint32> const RenderingLayout::DrawTypeToGLDrawType = RenderingLayout::InitializeGLDrawTypeMapping();






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


void RenderingLayout::Draw()
{
	gl::DrawElements(DrawTypeToGLDrawType[static_cast<uint32>(GetDrawingMode())], GetElementCount(), GetGLType(GetIndexElementType()), reinterpret_cast<void const *>(0));
}
