#pragma once
#include "Declare.hpp"
#include "Rendering/GraphicsType.hpp"

#include <vector>

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
			AtomicCounter,
			ShaderStorage,

			TypeCount,
		};

	protected:
		explicit BufferView(BufferType type);
		BufferView(BufferType type, GraphicsBufferSP const& buffer);

	public:
		virtual ~BufferView();

		BufferType GetType() const // TODO make this virtual, and remove BufferType type_;
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
		void SetBuffer(GraphicsBufferSP const& buffer);

		uint32 GetBufferSize() const;

		virtual void Bind();
		virtual void BindIndex(uint32 index);

		virtual void Unbind();
		virtual void UnbindIndex();

	protected:
		virtual bool SetBufferCheck(GraphicsBufferSP const& newBuffer);

	private:
		GraphicsBufferSP buffer_;
		BufferType type_;
	};


}
