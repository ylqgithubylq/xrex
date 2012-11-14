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
			static std::array<uint32, static_cast<uint32>(GraphicsBuffer::Usage::UsageCount)> const mapping = [] ()
			{
				std::array<uint32, static_cast<uint32>(GraphicsBuffer::Usage::UsageCount)> temp;
				temp[static_cast<uint32>(GraphicsBuffer::Usage::Static)] = gl::GL_STATIC_DRAW;
				temp[static_cast<uint32>(GraphicsBuffer::Usage::Dynamic)] = gl::GL_DYNAMIC_DRAW;
				temp[static_cast<uint32>(GraphicsBuffer::Usage::Stream)] = gl::GL_STREAM_DRAW;
				return temp;
			} ();
			return mapping[static_cast<uint32>(usage)];
		}
	}

	bool GraphicsBuffer::DataLayout::AddChannelLayout(ElementLayout&& elementLayout)
	{
		channelLayouts_.push_back(std::move(elementLayout));
		// TODO check if they overlaid each other
		return true;
	}

	auto GraphicsBuffer::DataLayout::GetChannelLayout(string const& channel) const -> ElementLayout const&
	{
		auto found = std::find_if(channelLayouts_.begin(), channelLayouts_.end(), [&channel] (ElementLayout const& elementLayout) 
		{
			return elementLayout.channel == channel;
		});
		assert(found != channelLayouts_.end());
		return *found;
	}


	void GraphicsBuffer::DoConsctruct(void const* data, uint32 dataSize)
	{
		gl::GenBuffers(1, &glBufferID_);
		assert(glBufferID_ != 0); // 0 is reserved by GL
		glBindingTarget_ = type_ == BufferType::Vertex ? gl::GL_ARRAY_BUFFER : gl::GL_ELEMENT_ARRAY_BUFFER;
		gl::BindBuffer(glBindingTarget_, glBufferID_);
		gl::BufferData(glBindingTarget_, dataSize, data, GLUsageFromUsage(usage_));
	}

	GraphicsBuffer::~GraphicsBuffer()
	{
		gl::DeleteBuffers(1, &glBufferID_);
	}

	void GraphicsBuffer::Bind()
	{
		gl::BindBuffer(glBindingTarget_, glBufferID_);
	}

	void GraphicsBuffer::BindToProgram(ProgramObjectSP const& program)
	{
		// binding relations are saved in VAO in RenderingLayout.
		Bind();
		if (type_ == BufferType::Vertex)
		{
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
	}

	void GraphicsBuffer::Unbind()
	{
		if (type_ == BufferType::Vertex)
		{
			for (uint32 i = 0; i < layout_.GetChannelLayoutCount(); ++i)
			{
				if (lastAttributeLocations_[i] != -1)
				{
					gl::DisableVertexAttribArray(lastAttributeLocations_[i]);
				}
			}
			lastAttributeLocations_.clear();
		}

		gl::BindBuffer(glBindingTarget_, 0);

	}

}