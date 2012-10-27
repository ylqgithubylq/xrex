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
		enum class DrawingMode
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
		RenderingLayout(std::vector<GraphicsBufferSP> const& buffers, GraphicsBufferSP const& indexBuffer, DrawingMode mode);
		~RenderingLayout();



		void BindToProgram(ProgramObjectSP const& program);
		void Unbind();

		DrawingMode GetDrawingMode() const
		{
			return mode_;
		}

		ElementType GetIndexElementType() const;
		uint32 GetElementCount() const
		{
			return indexBuffer_->GetElementCount();
		}


		void Draw();

	private:
		std::vector<GraphicsBufferSP> buffers_;
		GraphicsBufferSP indexBuffer_;
		DrawingMode mode_;
		uint32 glDrawingMode_;
		uint32 glIndexBufferElementType_;
		std::unordered_map<ProgramObjectSP, uint32> programBindingVAOCache_;
	};

}
