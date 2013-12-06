#pragma once

#include "Declare.hpp"
#include "Rendering/GraphicsType.hpp"
#include "Rendering/BufferView.hpp"

#include <vector>
#include <unordered_map>

namespace XREX
{

	class XREX_API VertexBuffer
		: public BufferView
	{
	public:
		class XREX_API DataLayoutDescription
			: XREX::Noncopyable
		{
		public:
			struct XREX_API ElementLayoutDescription
			{
				uint32 start;
				uint32 strip;
				ElementType elementType;
				std::string channel;
				bool needNormalize;
				/*
				 *	@elementStrip: 0 indicates no strip between elements.
				 */
				ElementLayoutDescription(uint32 startLocationInBytes, uint32 elementStripInBytes, ElementType type, std::string const& attributeChannel, bool normalize = false)
					: start(startLocationInBytes), strip(elementStripInBytes), elementType(type), channel(attributeChannel), needNormalize(normalize)
				{
				}
			};

		public:
			explicit DataLayoutDescription(uint32 vertexCount)
				: vertexCount_(vertexCount)
			{
			}
			DataLayoutDescription(DataLayoutDescription&& right)
				: vertexCount_(right.vertexCount_), channelLayouts_(std::move(right.channelLayouts_))
			{
			}

			bool AddChannelLayout(ElementLayoutDescription&& elementLayout);
			ElementLayoutDescription const& GetChannelLayout(std::string const& channel) const;
			std::vector<ElementLayoutDescription> const& GetAllLayouts() const
			{
				return channelLayouts_;
			}
			uint32 GetChannelLayoutCount() const
			{
				return channelLayouts_.size();
			}
			ElementLayoutDescription const& GetChannelLayoutAtIndex(uint32 index) const
			{
				return channelLayouts_[index];
			}
			uint32 GetVertexCount() const
			{
				return vertexCount_;
			}

		private:
			uint32 vertexCount_;
			std::vector<ElementLayoutDescription> channelLayouts_;
		};

	public:

		explicit VertexBuffer(DataLayoutDescription&& layoutDescription);
		VertexBuffer(DataLayoutDescription&& layoutDescription, GraphicsBufferSP const& buffer);

		DataLayoutDescription const& GetDataLayoutDescription() const
		{
			return layoutDescription_;
		}

		uint32 GetElementCount() const
		{
			return layoutDescription_.GetVertexCount();
		}

	private:
		DataLayoutDescription layoutDescription_;
	};



	class XREX_API IndexBuffer
		: public BufferView
	{
	public:
		enum class TopologicalType
		{
			Points,
			LineStrip,
			LineLoop,
			Lines,
			TriangleStrip,
			TriangleFan,
			Triangles,

			DrawingModeCount
		};

	public:
		IndexBuffer(TopologicalType topologicalType, ElementType elementType, uint32 elementCount);
		IndexBuffer(TopologicalType topologicalType, ElementType elementType, uint32 elementCount, GraphicsBufferSP const& buffer);

		TopologicalType GetTopologicalType() const
		{
			return topologicalType_;
		}
		uint32 GetElementCount() const
		{
			return elementCount_;
		}
		ElementType GetElementType() const
		{
			return elementType_;
		}

	private:
		virtual bool SetBufferCheck(GraphicsBufferSP const& newBuffer) override;


	private:
		TopologicalType topologicalType_;
		ElementType elementType_;
		uint32 elementCount_;
	};


	class XREX_API RenderingLayout
		: Noncopyable
	{

	public:
		RenderingLayout(std::vector<VertexBufferSP> const& buffers, IndexBufferSP const& indexBuffer);
		~RenderingLayout();

		ElementType GetIndexElementType() const;
		uint32 GetElementCount() const
		{
			return indexBuffer_->GetElementCount();
		}

		std::vector<VertexBufferSP> const& GetVertexBuffers() const
		{
			return buffers_;
		}
		IndexBufferSP const& GetIndexBuffer() const
		{
			return indexBuffer_;
		}


	private:
		std::vector<VertexBufferSP> buffers_;
		IndexBufferSP indexBuffer_;
	};



}
