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
			BufferMapper(BufferMapper&& rhs)
				: buffer_(rhs.buffer_), data_(rhs.data_)
			{
				rhs.data_ = nullptr; // prevent Unmap of rhs in destructor
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
		void Resize(uint32 sizeInBytes);

		void Bind();
		virtual void Unbind();

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
		uint32 glBindingTarget_;
		uint32 glBufferID_;

		uint32 sizeInBytes_;
	};



	class XREX_API VertexBuffer
		: public GraphicsBuffer
	{
	public:
		class XREX_API DataLayout
			: XREX::Noncopyable
		{
		public:
			struct XREX_API ElementLayout
			{
				uint32 start;
				uint32 strip;
				ElementType elementType;
				std::string channel;
				bool needNormalize;
				/*
				 *	@elementStrip: 0 indicates no strip between elements.
				 */
				ElementLayout(uint32 startLocationInBytes, uint32 elementStripInBytes, ElementType type, std::string const& attributeChannel, bool normalize = false)
					: start(startLocationInBytes), strip(elementStripInBytes), elementType(type), channel(attributeChannel), needNormalize(normalize)
				{
				}
			};

		public:
			explicit DataLayout(uint32 vertexCount)
				: vertexCount_(vertexCount)
			{
			}
			DataLayout(DataLayout&& rhs)
				: vertexCount_(rhs.vertexCount_), channelLayouts_(std::move(rhs.channelLayouts_))
			{
			}

			bool AddChannelLayout(ElementLayout&& elementLayout);
			ElementLayout const& GetChannelLayout(std::string const& channel) const;
			uint32 GetChannelLayoutCount() const
			{
				return channelLayouts_.size();
			}
			ElementLayout const& GetChannelLayoutAtIndex(uint32 index) const
			{
				return channelLayouts_[index];
			}
			uint32 GetVertexCount() const
			{
				return vertexCount_;
			}

		private:
			uint32 vertexCount_;
			std::vector<ElementLayout> channelLayouts_;
		};

	public:
		/*
		 *	For single-channel buffer.
		 *	@data: vector of type T, T must be the actual type of element in data.
		 */
		template <typename T>
		VertexBuffer(Usage usage, std::vector<T> const& data, std::string const& channel, bool normalized = false)
			: GraphicsBuffer(BufferType::Vertex, usage, data.data(), data.size() * sizeof(T)), layout_(data.size())
		{
			assert(channel != "");
			layout_.AddChannelLayout(DataLayout::ElementLayout(0, 0, TypeToElementType<T>::Type, channel));
			assert(sizeof(T) == GetElementSizeInBytes(TypeToElementType<T>::Type));
		}

		/*
		 *	For multi-channel buffer. a.k.a.: array of structures.
		 */
		template <typename T>
		VertexBuffer(Usage usage, std::vector<T> const& data, DataLayout&& layout)
			: GraphicsBuffer(BufferType::Vertex, usage, data.data(), data.size() * sizeof(T)), layout_(std::move(layout))
		{
		}

		DataLayout const& GetDataLayout() const
		{
			return layout_;
		}

		uint32 GetElementCount() const
		{
			return layout_.GetVertexCount();
		}

		void BindToProgram(ProgramObjectSP const& program);

		virtual void Unbind() override;

	private:
		DataLayout layout_;
		std::vector<int32> lastAttributeLocations_; // used to store attribute binding location temporarily
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
