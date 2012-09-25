#pragma once

#include "Declare.hpp"
#include "GraphicsBuffer.hpp"

#include <vector>
#include <unordered_map>

class RenderingLayout
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
	RenderingLayout(std::vector<GraphicsBufferSP> const& buffers, GraphicsBufferSP& indexBuffer, DrawingMode mode);
	~RenderingLayout();



	void BindToProgram(ProgramObject const& program);
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
	static uint32 GLDrawModeFromDrawMode(DrawingMode mode);

private:
	std::vector<GraphicsBufferSP> buffers_;
	GraphicsBufferSP indexBuffer_;
	DrawingMode mode_;
	std::unordered_map<ProgramObject const*, uint32> programBindingVAOCache_;
};

