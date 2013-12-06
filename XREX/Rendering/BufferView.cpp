#include "XREX.hpp"

#include "BufferView.hpp"

#include "Rendering/GraphicsBuffer.hpp"

#include "Rendering/GL/GLUtil.hpp"


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

}
