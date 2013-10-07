#include "XREX.hpp"

#include "GraphicsBuffer.hpp"

#include "Rendering/ShaderProgram.hpp"
#include "Rendering/GL/GLUtil.hpp"

#include <CoreGL.hpp>


#include <algorithm>
#include <array>


using std::string;
using std::vector;

namespace XREX
{

	GraphicsBuffer::BufferMapper::BufferMapper(GraphicsBuffer& buffer, AccessType type)
		: buffer_(buffer)
	{
		data_ = buffer_.Map(type);
	}

	GraphicsBuffer::BufferMapper::BufferMapper(BufferMapper&& right)
		: buffer_(right.buffer_), data_(right.data_)
	{
		right.data_ = nullptr; // prevent Unmap of right in destructor
	}

	GraphicsBuffer::BufferMapper::~BufferMapper()
	{
		Finish();
	}

	void GraphicsBuffer::BufferMapper::Finish()
	{
		if (data_)
		{
			data_ = nullptr;
			buffer_.Unmap();
		}
	}


	GraphicsBuffer::GraphicsBuffer(Usage usage, uint32 sizeInBytes)
		: usage_(usage), sizeInBytes_(sizeInBytes)
	{
		DoConsctruct(nullptr, sizeInBytes);
	}

	GraphicsBuffer::GraphicsBuffer(Usage usage, void const* data, uint32 sizeInBytes)
		: usage_(usage), sizeInBytes_(sizeInBytes)
	{
		DoConsctruct(data, sizeInBytes);
	}

	GraphicsBuffer::GraphicsBuffer(Usage usage, uint32 sizeInBytes, BufferView::BufferType typeHint)
		: usage_(usage), sizeInBytes_(sizeInBytes)
	{
		DoConsctruct(nullptr, sizeInBytes, typeHint);

	}

	GraphicsBuffer::GraphicsBuffer(Usage usage, void const* data, uint32 sizeInBytes, BufferView::BufferType typeHint)
		: usage_(usage), sizeInBytes_(sizeInBytes)
	{
		DoConsctruct(data, sizeInBytes, typeHint);
	}

	void GraphicsBuffer::DoConsctruct(void const* data, uint32 sizeInBytes)
	{
		assert(sizeInBytes != 0);
		gl::GenBuffers(1, &glBufferID_);
		assert(glBufferID_ != 0); // 0 is reserved by GL
		glCurrentBindingTarget_ = gl::GL_COPY_WRITE_BUFFER;
		glCurrentBindingIndex_ = 0;
		BindWrite();
		gl::BufferData(gl::GL_COPY_WRITE_BUFFER, sizeInBytes, data, GLUsageFromUsage(usage_));
	}

	void GraphicsBuffer::DoConsctruct(void const* data, uint32 sizeInBytes, BufferView::BufferType typeHint)
	{
		assert(sizeInBytes != 0);
		gl::GenBuffers(1, &glBufferID_);
		assert(glBufferID_ != 0); // 0 is reserved by GL
		glCurrentBindingTarget_ = GLBufferTypeFromBufferType(typeHint);
		glCurrentBindingIndex_ = 0;
		Bind(typeHint);
		gl::BufferData(glCurrentBindingTarget_, sizeInBytes, data, GLUsageFromUsage(usage_));
	}

	GraphicsBuffer::~GraphicsBuffer()
	{
		if (glBufferID_ != 0)
		{
			gl::DeleteBuffers(1, &glBufferID_);
			glBufferID_ = 0;
		}
	}


	void GraphicsBuffer::Resize(uint32 sizeInBytes)
	{
		assert(sizeInBytes != 0);
		sizeInBytes_ = sizeInBytes;
		BindWrite();
		gl::BufferData(gl::GL_COPY_WRITE_BUFFER, sizeInBytes_, nullptr, GLUsageFromUsage(usage_));
	}

	void GraphicsBuffer::UpdateData(void const* data)
	{
		BindWrite();
		gl::BufferSubData(gl::GL_COPY_WRITE_BUFFER, 0, sizeInBytes_, data);
	}


	void GraphicsBuffer::Clear(ElementType type, void const* data)
	{
		ElementType primitiveType = GetElementPrimitiveType(type);
		uint32 elementCount = GetElementPrimitiveCount(type);
		BindWrite();
		TexelFormat format = GetCorrespondingTexelFormat(type);
		uint32 componentCount = GetElementPrimitiveCount(type);
		GLTextureFormat const& glFormat = GLTextureFormatFromTexelFormat(format);
		gl::ClearBufferData(gl::GL_COPY_WRITE_BUFFER, glFormat.glInternalFormat, glFormat.glSourceFormat, glFormat.glTextureElementType, data);
	}





	void GraphicsBuffer::BindWrite()
	{
		gl::BindBuffer(gl::GL_COPY_WRITE_BUFFER, glBufferID_);
		glCurrentBindingTarget_ = gl::GL_COPY_WRITE_BUFFER;
	}

	void GraphicsBuffer::BindRead()
	{
		gl::BindBuffer(gl::GL_COPY_READ_BUFFER, glBufferID_);
		glCurrentBindingTarget_ = gl::GL_COPY_READ_BUFFER;
	}

	void GraphicsBuffer::Bind(BufferView::BufferType type)
	{
		glCurrentBindingTarget_ = GLBufferTypeFromBufferType(type);
		gl::BindBuffer(glCurrentBindingTarget_, glBufferID_);
	}

	void GraphicsBuffer::BindIndex(BufferView::BufferType type, uint32 index)
	{
		glCurrentBindingTarget_ = GLBufferTypeFromBufferType(type);
		glCurrentBindingIndex_ = index;
		gl::BindBufferBase(glCurrentBindingTarget_, index, glBufferID_);
	}

	void GraphicsBuffer::Unbind()
	{
		gl::BindBuffer(glCurrentBindingTarget_, 0);
	}

	void GraphicsBuffer::UnbindIndex()
	{
		gl::BindBufferBase(glCurrentBindingTarget_, glCurrentBindingIndex_, 0);
	}

	void* GraphicsBuffer::Map(AccessType accessType)
	{
		uint32 glAccessType = GLAccessTypeFromAccessType(accessType);
		BindWrite();
		void* p = gl::MapBuffer(gl::GL_COPY_WRITE_BUFFER, glAccessType);
		assert(p != nullptr);
		return p;
	}

	void GraphicsBuffer::Unmap()
	{
		BindWrite();
		bool result = gl::UnmapBuffer(gl::GL_COPY_WRITE_BUFFER) == gl::GL_TRUE;
		assert(result);
	}

	bool VertexBuffer::DataLayoutDescription::AddChannelLayout(ElementLayoutDescription&& elementLayout)
	{
		channelLayouts_.push_back(std::move(elementLayout));
		// TODO check if they overlaid each other
		return true;
	}

	auto VertexBuffer::DataLayoutDescription::GetChannelLayout(string const& channel) const -> ElementLayoutDescription const&
	{
		auto found = std::find_if(channelLayouts_.begin(), channelLayouts_.end(), [&channel] (ElementLayoutDescription const& elementLayout) 
		{
			return elementLayout.channel == channel;
		});
		assert(found != channelLayouts_.end());
		return *found;
	}



}
