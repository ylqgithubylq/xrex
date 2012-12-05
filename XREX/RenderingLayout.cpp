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
		uint32 GLDrawModeFromDrawMode(IndexBuffer::PrimitiveType primitiveType)
		{
			switch (primitiveType)
			{
			case IndexBuffer::PrimitiveType::Points:
				return gl::GL_POINTS;
			case IndexBuffer::PrimitiveType::LineStrip:
				return gl::GL_LINE_STRIP;
			case IndexBuffer::PrimitiveType::LineLoop:
				return gl::GL_LINE_LOOP;
			case IndexBuffer::PrimitiveType::Lines:
				return gl::GL_LINES;
			case IndexBuffer::PrimitiveType::TriangleStrip:
				return gl::GL_TRIANGLE_STRIP;
			case IndexBuffer::PrimitiveType::TriangleFan:
				return gl::GL_TRIANGLE_FAN;
			case IndexBuffer::PrimitiveType::Triangles:
				return gl::GL_TRIANGLES;
			case IndexBuffer::PrimitiveType::DrawingModeCount:
				assert(false);
				return gl::GL_TRIANGLES;
			default:
				assert(false);
				return gl::GL_TRIANGLES;
			}
		};
	}



	RenderingLayout::RenderingLayout(vector<VertexBufferSP> const& buffers, IndexBufferSP const& indexBuffer)
		: buffers_(buffers), indexBuffer_(indexBuffer)
	{
		glDrawingMode_ = GLDrawModeFromDrawMode(indexBuffer_->GetPrimitiveType());
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
			if (vao.second != 0)
			{
				gl::DeleteVertexArrays(1, &vao.second);
				vao.second = 0;
			}
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
		return indexBuffer_->GetElementType();
	}


	void RenderingLayout::Draw()
	{
		gl::DrawElements(glDrawingMode_, GetElementCount(), glIndexBufferElementType_, reinterpret_cast<void const*>(0));
	}

}
