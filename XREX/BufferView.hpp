#pragma once
#include "Declare.hpp"

namespace XREX
{

	class XREX_API BufferView
		: Noncopyable
	{

	public:
		enum class BufferType
		{
			Vertex,
			Index,
			Uniform,
			TransformFeedback,
			Texture,

			TypeCount,
		};

	protected:
		explicit BufferView(BufferType type);
		BufferView(BufferType type, GraphicsBufferSP const& buffer);

	public:
		virtual ~BufferView();

		BufferType GetType() const
		{
			return type_;
		}

		bool HaveBuffer() const
		{
			return buffer_ != nullptr;
		}
		GraphicsBufferSP const& GetBuffer() const
		{
			return buffer_;
		}
		void SetBuffer(GraphicsBufferSP const& buffer)
		{
			buffer_ = buffer;
		}

		uint32 GetBufferSize() const;

		virtual void Bind();
		virtual void BindIndex(uint32 index);

		virtual void Unbind();
		virtual void UnbindIndex();

	private:
		GraphicsBufferSP buffer_;
		BufferType type_;
	};


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
		VertexBuffer(GraphicsBufferSP const& buffer, DataLayoutDescription&& layoutDescription);

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
		IndexBuffer(GraphicsBufferSP const& buffer, TopologicalType topologicalType, ElementType elementType, uint32 elementCount);

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
		TopologicalType topologicalType_;
		ElementType elementType_;
		uint32 elementCount_;
	};
}
