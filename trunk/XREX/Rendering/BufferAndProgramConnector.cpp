#include "XREX.hpp"

#include "BufferAndProgramConnector.hpp"

#include "Rendering/RenderingLayout.hpp"
#include "Rendering/BufferView.hpp"
#include "Rendering/Shader.hpp"
#include "Rendering/GL/GLUtil.hpp"

#include <CoreGL.hpp>

namespace XREX
{


	BufferAndProgramConnector::BufferAndProgramConnector(RenderingLayoutSP const& layout, ProgramObjectSP const& program)
		: layout_(layout), program_(program), glVAO_(0)
	{
		gl::GenVertexArrays(1, &glVAO_);
		assert(glVAO_ != 0);
		gl::BindVertexArray(glVAO_);

		std::vector<VertexBufferSP> const& vertexBuffers = layout_->GetVertexBuffers();

		std::vector<std::vector<uint32>> lastAttributeLocations(vertexBuffers.size());

		for (uint32 bufferIndex = 0; bufferIndex < vertexBuffers.size(); ++bufferIndex)
		{
			VertexBufferSP const& buffer = vertexBuffers[bufferIndex];
			buffer->Bind();
			VertexBuffer::DataLayoutDescription const& dataLayout = buffer->GetDataLayoutDescription();
			lastAttributeLocations[bufferIndex].resize(dataLayout.GetChannelLayoutCount());
			for (uint32 i = 0; i < dataLayout.GetChannelLayoutCount(); ++i)
			{
				VertexBuffer::DataLayoutDescription::ElementLayoutDescription const& channelLayout = dataLayout.GetChannelLayoutAtIndex(i);
				std::pair<bool, ProgramObject::AttributeInformation> attributeInformation = program->GetAttributeInformation(channelLayout.channel);
				if (attributeInformation.first)
				{
					lastAttributeLocations[bufferIndex][i] = attributeInformation.second.GetLocation();
					ElementType channelElementType = attributeInformation.second.GetElementType();

					gl::EnableVertexAttribArray(lastAttributeLocations[bufferIndex][i]);
					switch (channelElementType)
					{
					case ElementType::Int32:
					case ElementType::Uint32:
					case ElementType::IntV2:
					case ElementType::IntV3:
					case ElementType::IntV4:
					case ElementType::UintV2:
					case ElementType::UintV3:
					case ElementType::UintV4:
						gl::VertexAttribIPointer(lastAttributeLocations[bufferIndex][i], GetElementPrimitiveCount(channelLayout.elementType), GLTypeFromElementType(GetElementPrimitiveType(channelLayout.elementType)),
							channelLayout.strip, reinterpret_cast<void const*>(channelLayout.start));
						break;
					case ElementType::Float:
					case ElementType::FloatV2:
					case ElementType::FloatV3:
					case ElementType::FloatV4:
					case ElementType::FloatM44:
						gl::VertexAttribPointer(lastAttributeLocations[bufferIndex][i], GetElementPrimitiveCount(channelLayout.elementType), GLTypeFromElementType(GetElementPrimitiveType(channelLayout.elementType)),
							channelLayout.needNormalize, channelLayout.strip, reinterpret_cast<void const*>(channelLayout.start));
						break;
					case ElementType::Double:
					case ElementType::DoubleV2:
					case ElementType::DoubleV3:
					case ElementType::DoubleV4:
					case ElementType::DoubleM44:
						gl::VertexAttribLPointer(lastAttributeLocations[bufferIndex][i], GetElementPrimitiveCount(channelLayout.elementType), GLTypeFromElementType(GetElementPrimitiveType(channelLayout.elementType)),
							channelLayout.strip, reinterpret_cast<void const*>(channelLayout.start));
						break;
					case ElementType::Void:
					case ElementType::Bool:
					case ElementType::Uint8:
					case ElementType::Uint16:
					case ElementType::Int8:
					case ElementType::Int16:
						assert(false); // impossible
						break;
					case ElementType::ParameterTypeCount:
						assert(false); // impossible
						break;
					default:
						assert(false); // impossible
						break;
					}
				}
			}
		}
		layout_->GetIndexBuffer()->Bind();

		gl::BindVertexArray(0);
		for (uint32 bufferIndex = 0; bufferIndex < vertexBuffers.size(); ++bufferIndex)
		{
			VertexBufferSP const& buffer = vertexBuffers[bufferIndex];
			buffer->Unbind();
			VertexBuffer::DataLayoutDescription const& dataLayout = buffer->GetDataLayoutDescription();
			for (uint32 i = 0; i < dataLayout.GetChannelLayoutCount(); ++i)
			{
				if (lastAttributeLocations[bufferIndex][i] != -1)
				{
					gl::DisableVertexAttribArray(lastAttributeLocations[bufferIndex][i]);
				}
			}
		}
		layout_->GetIndexBuffer()->Unbind();
	}

	BufferAndProgramConnector::~BufferAndProgramConnector()
	{
		if (glVAO_ != 0)
		{
			gl::DeleteVertexArrays(1, &glVAO_);
			glVAO_ = 0;
		}
	}

	void BufferAndProgramConnector::Bind()
	{
		gl::BindVertexArray(glVAO_);
	}

	void BufferAndProgramConnector::Unbind()
	{
		gl::BindVertexArray(0);
	}

}
