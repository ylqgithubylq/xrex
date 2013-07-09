#include "XREX.hpp"
#include "BufferView.hpp"
#include "GraphicsBuffer.hpp"

#include "GLUtil.hpp"


namespace XREX
{
	BufferView::BufferView(BufferType type)
		: type_(type)
	{
	}

	BufferView::BufferView(BufferType type, GraphicsBufferSP const& buffer)
		: type_(type), buffer_(buffer)
	{
	}

	BufferView::~BufferView()
	{
	}


	void BufferView::SetBuffer(GraphicsBufferSP const& buffer)
	{
		assert(SetBufferCheck(buffer));
		buffer_ = buffer;
	}


	uint32 BufferView::GetBufferSize() const
	{
		assert(HaveBuffer());
		return buffer_->GetSize();
	}


	void BufferView::Bind()
	{
		assert(HaveBuffer());
		buffer_->Bind(type_);
	}

	void BufferView::BindIndex(uint32 index)
	{
		assert(HaveBuffer());
		buffer_->BindIndex(type_, index);
	}

	void BufferView::Unbind()
	{
		assert(HaveBuffer());
		buffer_->Unbind();
	}

	void BufferView::UnbindIndex()
	{
		assert(HaveBuffer());
		buffer_->UnbindIndex();
	}

	bool BufferView::SetBufferCheck(GraphicsBufferSP const& newBuffer)
	{
		return true;
	}



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


}
