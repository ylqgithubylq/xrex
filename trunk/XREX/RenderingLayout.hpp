#pragma once

#include "Declare.hpp"
#include "GraphicsBuffer.hpp"

#include <vector>

class RenderingLayout
	: Noncopyable
{
public:
	enum DrawingMode
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
	RenderingLayout(std::vector<GraphicsBufferSP> const & buffers, GraphicsBufferSP& indexBuffer, DrawingMode mode);
	~RenderingLayout();



	void BindToProgram(ProgramObject const & program);
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
	static std::vector<uint32> InitializeGLDrawTypeMapping();
	static std::vector<uint32> const DrawTypeToGLDrawType;

private:
	std::vector<GraphicsBufferSP> buffers_;
	GraphicsBufferSP indexBuffer_;
	DrawingMode mode_;
};

