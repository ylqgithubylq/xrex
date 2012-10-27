#include "XREX.hpp"

#include "RenderingLayout.hpp"
#include "GraphicsBuffer.hpp"
#include "GLUtil.hpp"

#include <CoreGL.hpp>

#include <array>

using std::vector;


namespace XREX
{
	namespace
	{
		uint32 GLDrawModeFromDrawMode(RenderingLayout::DrawingMode mode)
		{
			static std::array<uint32, static_cast<uint32>(RenderingLayout::DrawingMode::DrawingModeCount)> const mapping = [] ()
			{
				std::array<uint32, static_cast<uint32>(RenderingLayout::DrawingMode::DrawingModeCount)> mapping;
				mapping[static_cast<uint32>(RenderingLayout::DrawingMode::Points)] = gl::GL_POINTS;
				mapping[static_cast<uint32>(RenderingLayout::DrawingMode::LineStrip)] = gl::GL_LINE_STRIP;
				mapping[static_cast<uint32>(RenderingLayout::DrawingMode::LineLoop)] = gl::GL_LINE_LOOP;
				mapping[static_cast<uint32>(RenderingLayout::DrawingMode::Lines)] = gl::GL_LINES;
				mapping[static_cast<uint32>(RenderingLayout::DrawingMode::TriangleStrip)] = gl::GL_TRIANGLE_STRIP;
				mapping[static_cast<uint32>(RenderingLayout::DrawingMode::TriangleFan)] = gl::GL_TRIANGLE_FAN;
				mapping[static_cast<uint32>(RenderingLayout::DrawingMode::Triangles)] = gl::GL_TRIANGLES;
				return mapping;
			} ();
			return mapping[static_cast<uint32>(mode)];
		};
	}


	RenderingLayout::RenderingLayout(vector<GraphicsBufferSP> const& buffers, GraphicsBufferSP const& indexBuffer, DrawingMode mode)
		: buffers_(buffers), indexBuffer_(indexBuffer), mode_(mode),
		glDrawingMode_(GLDrawModeFromDrawMode(mode))
	{
		 glIndexBufferElementType_ = GLTypeFromElementType(GetIndexElementType());

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
		for (auto& vao : programBindingVAOCache_)
		{
			gl::DeleteVertexArrays(1, &vao.second);
		}
	}

	void RenderingLayout::BindToProgram(ProgramObjectSP const& program)
	{
		auto found = programBindingVAOCache_.find(program);
		if (found == programBindingVAOCache_.end()) // initialize vao to the new program
		{
			uint32 vao;
			gl::GenVertexArrays(1, &vao);
			assert(vao != 0);
			programBindingVAOCache_[program] = vao;
			gl::BindVertexArray(vao);

			for (auto& buffer : buffers_)
			{
				buffer->BindToProgram(program);
			}
			indexBuffer_->Bind();

			gl::BindVertexArray(0);
			for (auto& buffer : buffers_)
			{
				buffer->Unbind();
			}
			indexBuffer_->Unbind();
		}

		gl::BindVertexArray(programBindingVAOCache_[program]);

	}

	void RenderingLayout::Unbind()
	{
		gl::BindVertexArray(0);
	}


	ElementType RenderingLayout::GetIndexElementType() const
	{
		return indexBuffer_->GetDataDescription().GetChannelLayout("").elementType;
	}


	void RenderingLayout::Draw()
	{
		gl::DrawElements(glDrawingMode_, GetElementCount(), glIndexBufferElementType_, reinterpret_cast<void const*>(0));
	}

}
