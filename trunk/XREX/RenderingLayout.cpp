#include "XREX.hpp"

#include "RenderingLayout.hpp"
#include "GraphicsBuffer.hpp"
#include "Shader.hpp"
#include "GLUtil.hpp"

#include <CoreGL.hpp>

#include <array>

using std::vector;


namespace XREX
{
	namespace
	{
		uint32 GLDrawModeFromTopologicalType(IndexBuffer::TopologicalType primitiveType)
		{
			switch (primitiveType)
			{
			case IndexBuffer::TopologicalType::Points:
				return gl::GL_POINTS;
			case IndexBuffer::TopologicalType::LineStrip:
				return gl::GL_LINE_STRIP;
			case IndexBuffer::TopologicalType::LineLoop:
				return gl::GL_LINE_LOOP;
			case IndexBuffer::TopologicalType::Lines:
				return gl::GL_LINES;
			case IndexBuffer::TopologicalType::TriangleStrip:
				return gl::GL_TRIANGLE_STRIP;
			case IndexBuffer::TopologicalType::TriangleFan:
				return gl::GL_TRIANGLE_FAN;
			case IndexBuffer::TopologicalType::Triangles:
				return gl::GL_TRIANGLES;
			case IndexBuffer::TopologicalType::DrawingModeCount:
				assert(false);
				return 0;
			default:
				assert(false);
				return 0;
			}
		};
	}



	RenderingLayout::RenderingLayout(vector<VertexBufferSP> const& buffers, IndexBufferSP const& indexBuffer)
		: buffers_(buffers), indexBuffer_(indexBuffer)
	{
		glDrawingMode_ = GLDrawModeFromTopologicalType(indexBuffer_->GetTopologicalType());
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

			std::vector<std::vector<uint32>> lastAttributeLocations(buffers_.size());

			for (uint32 bufferIndex = 0; bufferIndex < buffers_.size(); ++bufferIndex)
			{
				VertexBufferSP const& buffer = buffers_[bufferIndex];
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
			indexBuffer_->Bind();

			gl::BindVertexArray(0);
			for (uint32 bufferIndex = 0; bufferIndex < buffers_.size(); ++bufferIndex)
			{
				VertexBufferSP const& buffer = buffers_[bufferIndex];
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
