#include "XREX.hpp"

#include "GraphicsBuffer.hpp"
#include "Shader.hpp"
#include "GLUtil.hpp"

#include <CoreGL.hpp>


#include <algorithm>
#include <array>


using std::string;
using std::vector;

namespace XREX
{

	namespace
	{
		uint32 GLUsageFromUsage(GraphicsBuffer::Usage usage)
		{
			switch (usage)
			{
			case GraphicsBuffer::Usage::Static:
				return gl::GL_STATIC_DRAW;
			case GraphicsBuffer::Usage::Dynamic:
				return gl::GL_DYNAMIC_DRAW;
			case GraphicsBuffer::Usage::Stream:
				return gl::GL_STREAM_DRAW;
			case GraphicsBuffer::Usage::UsageCount:
				assert(false);
				return gl::GL_STATIC_DRAW;
				break;
			default:
				assert(false);
				return gl::GL_STATIC_DRAW;
				break;
			}
		}

		uint32 GlAccessTypeFromAccessType(GraphicsBuffer::AccessType type)
		{
			switch (type)
			{
			case GraphicsBuffer::AccessType::ReadOnly:
				return gl::GL_READ_ONLY;
			case GraphicsBuffer::AccessType::WriteOnly:
				return gl::GL_WRITE_ONLY;
			case GraphicsBuffer::AccessType::ReadWrite:
				return gl::GL_READ_WRITE;
			default:
				assert(false);
				return 0;
			}
		}
	}


	GraphicsBuffer::GraphicsBuffer(BufferType type, Usage usage, void const* data, uint32 sizeInBytes)
		: type_(type), usage_(usage), sizeInBytes_(sizeInBytes)
	{
		DoConsctruct(data, sizeInBytes);
	}

	void GraphicsBuffer::DoConsctruct(void const* data, uint32 sizeInBytes)
	{
		gl::GenBuffers(1, &glBufferID_);
		assert(glBufferID_ != 0); // 0 is reserved by GL
		glBindingTarget_ = type_ == BufferType::Vertex ? gl::GL_ARRAY_BUFFER : gl::GL_ELEMENT_ARRAY_BUFFER;
		gl::BindBuffer(glBindingTarget_, glBufferID_);
		gl::BufferData(glBindingTarget_, sizeInBytes, data, GLUsageFromUsage(usage_));
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
		gl::BindBuffer(glBindingTarget_, glBufferID_);
		gl::BufferData(glBindingTarget_, sizeInBytes_, nullptr, GLUsageFromUsage(usage_));
	}


	void GraphicsBuffer::Bind()
	{
		gl::BindBuffer(glBindingTarget_, glBufferID_);
	}

	void GraphicsBuffer::Unbind()
	{
		gl::BindBuffer(glBindingTarget_, 0);
	}

	void* GraphicsBuffer::Map(AccessType accessType)
	{
		uint32 glAccessType = GlAccessTypeFromAccessType(accessType);
		gl::BindBuffer(glBindingTarget_, glBufferID_);
		void* p = gl::MapBuffer(glBindingTarget_, glAccessType);
		assert(p != nullptr);
		return p;
	}

	void GraphicsBuffer::Unmap()
	{
		gl::BindBuffer(glBindingTarget_, glBufferID_);
		bool result = gl::UnmapBuffer(glBindingTarget_) == gl::GL_TRUE;
		assert(result);
	}



	bool VertexBuffer::DataLayout::AddChannelLayout(ElementLayout&& elementLayout)
	{
		channelLayouts_.push_back(std::move(elementLayout));
		// TODO check if they overlaid each other
		return true;
	}

	auto VertexBuffer::DataLayout::GetChannelLayout(string const& channel) const -> ElementLayout const&
	{
		auto found = std::find_if(channelLayouts_.begin(), channelLayouts_.end(), [&channel] (ElementLayout const& elementLayout) 
		{
			return elementLayout.channel == channel;
		});
		assert(found != channelLayouts_.end());
		return *found;
	}


	void VertexBuffer::BindToProgram(ProgramObjectSP const& program)
	{
		// binding relations are saved in VAO in RenderingLayout.
		Bind();
		assert(lastAttributeLocations_.size() == 0); // make sure this buffer is not binding to other programs
		lastAttributeLocations_.resize(layout_.GetChannelLayoutCount());
		for (uint32 i = 0; i < layout_.GetChannelLayoutCount(); ++i)
		{
			DataLayout::ElementLayout const& channelLayout = layout_.GetChannelLayoutAtIndex(i);
			lastAttributeLocations_[i] = program->GetAttributeLocation(channelLayout.channel);
			if (lastAttributeLocations_[i] != -1)
			{
				gl::EnableVertexAttribArray(lastAttributeLocations_[i]);
				gl::VertexAttribPointer(lastAttributeLocations_[i], GetElementPrimitiveCount(channelLayout.elementType), GLTypeFromElementType(GetElementPrimitiveType(channelLayout.elementType)),
					channelLayout.needNormalize, channelLayout.strip, reinterpret_cast<void const*>(channelLayout.start));
			}
		}
	}

	void VertexBuffer::Unbind()
	{
		GraphicsBuffer::Unbind();
		for (uint32 i = 0; i < layout_.GetChannelLayoutCount(); ++i)
		{
			if (lastAttributeLocations_[i] != -1)
			{
				gl::DisableVertexAttribArray(lastAttributeLocations_[i]);
			}
		}
		lastAttributeLocations_.shrink_to_fit();
	}

}
