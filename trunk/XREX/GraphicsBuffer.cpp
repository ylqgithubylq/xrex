#include "XREX.hpp"

#include "GraphicsBuffer.hpp"
#include "Shader.hpp"
#include "GLUtil.hpp"

#include <CoreGL.hpp>


#include <utility>
#include <algorithm>


using std::string;
using std::vector;

std::vector<uint32> const GraphicsBuffer::USAGE_TO_GL_USAGE = GraphicsBuffer::InitializeUsage();

vector<uint32> GraphicsBuffer:: InitializeUsage()
{
	vector<uint32> temp(static_cast<uint32>(GraphicsBuffer::Usage::UsageCount));
	temp[static_cast<uint32>(GraphicsBuffer::Usage::Static)] = gl::GL_STATIC_DRAW;
	temp[static_cast<uint32>(GraphicsBuffer::Usage::Dynamic)] = gl::GL_DYNAMIC_DRAW;
	temp[static_cast<uint32>(GraphicsBuffer::Usage::Stream)] = gl::GL_STREAM_DRAW;
	return std::move(temp);
}



bool GraphicsBuffer::DataDescription::AddChannelLayout(ElementLayoutDescription&& elementLayout)
{
	channelLayouts_.push_back(std::move(elementLayout));
	// TODO check if they overlaid each other
	return true;
}

auto GraphicsBuffer::DataDescription::GetChannelLayout(string const & channel) const -> ElementLayoutDescription const & 
{
	auto found = std::find_if(channelLayouts_.begin(), channelLayouts_.end(), [&channel] (ElementLayoutDescription const & elementLayout) 
	{
		return elementLayout.channel == channel;
	});
	assert(found != channelLayouts_.end());
	return *found;
}


void GraphicsBuffer::DoConsctruct(void const * data, uint32 dataSize)
{
	lastAttributeLocations_.resize(description_.channelLayouts_.size());

	gl::GenBuffers(1, &bufferID_);
	assert(bufferID_ != 0); // 0 is reserved by GL
	target_ = type_ == BufferType::Vertex ? gl::GL_ARRAY_BUFFER : gl::GL_ELEMENT_ARRAY_BUFFER;
	gl::BindBuffer(target_, bufferID_);
	gl::BufferData(target_, dataSize, data, USAGE_TO_GL_USAGE[static_cast<uint32>(usage_)]);
}

GraphicsBuffer::~GraphicsBuffer()
{
	gl::DeleteBuffers(1, &bufferID_);
}

void GraphicsBuffer::Bind()
{
	gl::BindBuffer(target_, bufferID_);
}

void GraphicsBuffer::BindToProgram(ProgramObject const & program)
{
	Bind();
	for (int i = 0; i < description_.channelLayouts_.size(); ++i)
	{
		DataDescription::ElementLayoutDescription& channelLayout = description_.channelLayouts_[i];
		lastAttributeLocations_[i] = program.GetAttributeLocation(channelLayout.channel);
		if (lastAttributeLocations_[i] != -1)
		{
			gl::EnableVertexAttribArray(lastAttributeLocations_[i]);
			gl::VertexAttribPointer(lastAttributeLocations_[i], GetElementPrimitiveCount(channelLayout.elementType), GetGLType(GetElementPrimitiveType(channelLayout.elementType)),
				channelLayout.needNormalize, channelLayout.strip, reinterpret_cast<void const *>(channelLayout.start));
		}
	}
}

void GraphicsBuffer::Unbind()
{
	for (int i = 0; i < description_.channelLayouts_.size(); ++i)
	{
		gl::DisableVertexAttribArray(lastAttributeLocations_[i]);
	}
	gl::BindBuffer(target_, 0);
}

