#pragma once

#include "Declare.hpp"
#include "Rendering/GraphicsType.hpp"
#include "Rendering/BufferView.hpp"

#include <string>
#include <vector>

namespace XREX
{

	class XREX_API GraphicsBuffer
		: Noncopyable
	{
	public:

		/*
		 *	Draw: write by CPU and read by GPU.
		 *	Read: write by GPU and read by CPU.
		 *	Copy: write by GPU and read by GPU.
		 */
		enum class Usage
		{
			StaticDraw,
			DynamicDraw,
			StreamDraw,
			StaticRead,
			DynamicRead,
			StreamRead,
			StaticCopy,
			DynamicCopy,
			StreamCopy,

			UsageCount
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
			BufferMapper(GraphicsBuffer& buffer, AccessType type);
		public:
			BufferMapper(BufferMapper&& right);

			~BufferMapper();

			GraphicsBuffer& GetBuffer() const
			{
				return buffer_;
			}

			void Finish();

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
		GraphicsBuffer(Usage usage, uint32 sizeInBytes);
		GraphicsBuffer(Usage usage, void const* data, uint32 sizeInBytes);

		GraphicsBuffer(Usage usage, uint32 sizeInBytes, BufferView::BufferType typeHint);
		GraphicsBuffer(Usage usage, void const* data, uint32 sizeInBytes, BufferView::BufferType typeHint);

		virtual ~GraphicsBuffer();

		uint32 GetID() const
		{
			return glBufferID_;
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

		template <typename T>
		void Clear(T const& data)
		{
			ElementType type = TypeToElementType<T>::Type;
			assert(type != ElementType::ElementTypeCount);
			Clear(type, &data);
		}
		void Clear(ElementType dataType, void const* data);

		/*
		 *	Bind buffer for write from CPU.
		 */
		void BindWrite();
		/*
		 *	Bind buffer for read from CPU.
		 */
		void BindRead();

		void Bind(BufferView::BufferType type);
		void BindIndex(BufferView::BufferType type, uint32 index);
		void Unbind();
		void UnbindIndex();

		BufferMapper GetMapper(AccessType accessType)
		{
			return BufferMapper(*this, accessType);
		}

	private:
		void DoConsctruct(void const* data, uint32 sizeInBytes);
		void DoConsctruct(void const* data, uint32 sizeInBytes, BufferView::BufferType typeHint);

		void* Map(AccessType accessType);
		void Unmap();

	private:
		Usage usage_;
		uint32 sizeInBytes_;
		uint32 glBufferID_;
		uint32 glCurrentBindingTarget_;
		uint32 glCurrentBindingIndex_;
	};

}
