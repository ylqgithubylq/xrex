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


	uint32 XREX::BufferView::GetBufferSize() const
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


	VertexBuffer::VertexBuffer(DataLayoutDescription&& layoutDescription)
		: BufferView(BufferType::Vertex), layoutDescription_(std::move(layoutDescription))
	{

	}

	VertexBuffer::VertexBuffer(GraphicsBufferSP const& buffer, DataLayoutDescription&& layoutDescription)
		: BufferView(BufferType::Vertex, buffer), layoutDescription_(std::move(layoutDescription))
	{
	}



	IndexBuffer::IndexBuffer(TopologicalType topologicalType, ElementType elementType, uint32 elementCount)
		: BufferView(BufferType::Index)
	{
	}

	IndexBuffer::IndexBuffer(GraphicsBufferSP const& buffer, TopologicalType topologicalType, ElementType elementType, uint32 elementCount)
		: BufferView(BufferType::Index, buffer), topologicalType_(topologicalType), elementType_(elementType), elementCount_(elementCount)
	{
		assert(GetElementSizeInBytes(elementType) * elementCount_ == GetBuffer()->GetSize());
	}


}
