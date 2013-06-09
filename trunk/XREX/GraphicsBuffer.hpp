#pragma once

#include "Declare.hpp"
#include "BufferView.hpp"

#include <string>
#include <vector>

namespace XREX
{

	class XREX_API GraphicsBuffer
		: Noncopyable
	{
	public:

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
		GraphicsBuffer(Usage usage, uint32 sizeInBytes);
		GraphicsBuffer(Usage usage, void const* data, uint32 sizeInBytes);

		virtual ~GraphicsBuffer();

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
