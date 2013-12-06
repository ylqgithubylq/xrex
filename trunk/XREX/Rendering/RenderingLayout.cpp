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

	VertexBuffer::VertexBuffer(DataLayoutDescription&& layoutDescription)
		: BufferView(BufferType::Vertex), layoutDescription_(std::move(layoutDescription))
	{
	}

	VertexBuffer::VertexBuffer(DataLayoutDescription&& layoutDescription, GraphicsBufferSP const& buffer)
		: BufferView(BufferType::Vertex, buffer), layoutDescription_(std::move(layoutDescription))
	{
	}



	IndexBuffer::IndexBuffer(TopologicalType topologicalType, ElementType elementType, uint32 elementCount)
		: BufferView(BufferType::Index), topologicalType_(topologicalType), elementType_(elementType), elementCount_(elementCount)
	{
	}

	IndexBuffer::IndexBuffer(TopologicalType topologicalType, ElementType elementType, uint32 elementCount, GraphicsBufferSP const& buffer)
		: BufferView(BufferType::Index, buffer), topologicalType_(topologicalType), elementType_(elementType), elementCount_(elementCount)
	{
		assert(SetBufferCheck(buffer));
	}

	bool IndexBuffer::SetBufferCheck(GraphicsBufferSP const& newBuffer)
	{
		return GetElementSizeInBytes(elementType_) * elementCount_ == newBuffer->GetSize();
	}



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
