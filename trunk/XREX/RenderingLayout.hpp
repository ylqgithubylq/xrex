#pragma once

#include "Declare.hpp"
#include "GraphicsBuffer.hpp"

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



		void BindToProgram(ProgramObjectSP const& program);
		void Unbind();

		ElementType GetIndexElementType() const;
		uint32 GetElementCount() const
		{
			return indexBuffer_->GetElementCount();
		}


		void Draw();

	private:
		std::vector<VertexBufferSP> buffers_;
		IndexBufferSP indexBuffer_;
		uint32 glDrawingMode_;
		uint32 glIndexBufferElementType_;
		std::unordered_map<ProgramObjectSP, uint32> programBindingVAOCache_;
	};

}
