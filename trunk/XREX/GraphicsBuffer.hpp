#pragma once

#include "Declare.hpp"

#include <string>
#include <vector>

namespace XREX
{

	class XREX_API GraphicsBuffer
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

		enum class Usage
		{
			Static,
			Dynamic,
			Stream,

			UsageCount
		};

		enum class AccessType
		{
			ReadOnly,
			WriteOnly,
			ReadWrite,
		};

	public:
		/*
		 *	Wrapper object to Unmap the buffer when destructed.
		 */
		class XREX_API BufferMapper
			: Noncopyable
		{
			friend class GraphicsBuffer;
		private:
			BufferMapper(GraphicsBuffer& buffer, AccessType type)
				: buffer_(buffer)
			{
				data_ = buffer_.Map(type);
			}
		public:
			BufferMapper(BufferMapper&& right)
				: buffer_(right.buffer_), data_(right.data_)
			{
				right.data_ = nullptr; // prevent Unmap of right in destructor
			}

			~BufferMapper()
			{
				if (data_)
				{
					data_ = nullptr;
					buffer_.Unmap();
				}
			}

			template <typename T>
			T* GetPointer()
			{
				return static_cast<T*>(data_);
			}

		private:
			GraphicsBuffer& buffer_;
			void* data_;
		};

	public:
		GraphicsBuffer(BufferType type, Usage usage, uint32 sizeInBytes);

		GraphicsBuffer(BufferType type, Usage usage, void const* data, uint32 sizeInBytes);

		virtual ~GraphicsBuffer();

		BufferType GetType() const
		{
			return type_;
		}
		Usage GetUsage() const
		{
			return usage_;
		}

		/*
		 *	@return: size in bytes.
		 */
		uint32 GetSize() const
		{
			return sizeInBytes_;
		}
		/*
		 *	This will invalidate the content of the buffer.
		 */
		void Resize(uint32 sizeInBytes);

		void UpdateData(void const* data);

		void BindWrite();
		void BindRead();

		void Bind();
		void BindIndex(uint32 index);
		void Unbind();
		void UnbindIndex();

		BufferMapper GetMapper(AccessType accessType)
		{
			return BufferMapper(*this, accessType);
		}

	private:
		void DoConsctruct(void const* data, uint32 sizeInBytes);

		void* Map(AccessType accessType);
		void Unmap();

	private:
		BufferType type_;
		Usage usage_;
		uint32 sizeInBytes_;
		uint32 glBindingTarget_;
		uint32 glBufferID_;
		uint32 glCurrentBindingTarget_;
		uint32 glCurrentBindingIndex_;
	};



	class XREX_API VertexBuffer
		: public GraphicsBuffer
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
		/*
		 *	Short cut for single-channel buffer.
		 *	@data: vector of type T, T must be the actual type of element in data.
		 */
		template <typename T>
		VertexBuffer(Usage usage, std::vector<T> const& data, std::string const& channel, bool normalized = false)
			: GraphicsBuffer(BufferType::Vertex, usage, data.data(), data.size() * sizeof(T)), layout_(data.size())
		{
			assert(channel != "");
			layout_.AddChannelLayout(DataLayoutDescription::ElementLayoutDescription(0, 0, TypeToElementType<T>::Type, channel));
			assert(sizeof(T) == GetElementSizeInBytes(TypeToElementType<T>::Type));
		}

		/*
		 *	For multi-channel buffer. a.k.a.: array of structures.
		 */
		template <typename T>
		VertexBuffer(Usage usage, std::vector<T> const& data, DataLayoutDescription&& layout)
			: GraphicsBuffer(BufferType::Vertex, usage, data.data(), data.size() * sizeof(T)), layout_(std::move(layout))
		{
		}

		/*
		 *	Create a buffer without initial data.
		 *	@elementSizeInBytes: element size in bytes of one vertex.
		 */
		VertexBuffer(Usage usage, uint32 elementSizeInBytes, DataLayoutDescription&& layout)
			: GraphicsBuffer(BufferType::Vertex, usage, elementSizeInBytes * layout.GetVertexCount()), layout_(std::move(layout))
		{
		}

		DataLayoutDescription const& GetDataLayoutDescription() const
		{
			return layout_;
		}

		uint32 GetElementCount() const
		{
			return layout_.GetVertexCount();
		}



	private:
		DataLayoutDescription layout_;
	};



	class XREX_API IndexBuffer
		: public GraphicsBuffer
	{
	public:
		enum class PrimitiveType
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
		/*
		 *	@data: vector of type T, T must be the actual type of element in data.
		 */
		template <typename T>
		IndexBuffer(Usage usage, std::vector<T> const& data, PrimitiveType primitiveType)
			: GraphicsBuffer(BufferType::Index, usage, data.data(), data.size() * sizeof(T)), primitiveType_(primitiveType), elementCount_(data.size())
		{
			elementType_ = TypeToElementType<T>::Type;
		}

		PrimitiveType GetPrimitiveType() const
		{
			return primitiveType_;
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
		PrimitiveType primitiveType_;
		ElementType elementType_;
		uint32 elementCount_;
	};



}
