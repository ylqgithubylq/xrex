#pragma once

#include "Declare.hpp"
#include "Rendering/GraphicsBuffer.hpp"

#include <vector>
#include <unordered_map>

namespace XREX
{

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
