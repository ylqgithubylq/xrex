#include "XREX.hpp"
#include "Viewport.hpp"

#include <CoreGL.hpp>

namespace XREX
{

	Viewport::Viewport(int32 depthOrder, int32 left, int32 bottom, uint32 width, uint32 height)
		: depthOrder_(depthOrder), absolute_(true), data_(left, bottom, width, height)
	{
	}

	Viewport::Viewport(int32 depthOrder, float left, float bottom, float width, float height)
		: depthOrder_(depthOrder), absolute_(false), data_(left, bottom, width, height)
	{
		assert(width > 0 && height > 0);
	}


	Viewport::~Viewport()
	{
	}

	void Viewport::Bind(uint32 windowWidth, uint32 windowHeight)
	{
		if (absolute_)
		{
			DataUnion::Absolute& absolute = data_.absolute;
			gl::Viewport(absolute.left, absolute.bottom, absolute.width, absolute.height);
			gl::Scissor(absolute.left, absolute.bottom, absolute.width, absolute.height);
		}
		else
		{
			DataUnion::Relative& relative = data_.relative;
			gl::Viewport(static_cast<int32>(relative.left * windowWidth), static_cast<int32>(relative.bottom * windowHeight),
				static_cast<uint32>(relative.width * windowWidth), static_cast<uint32>(relative.height * windowHeight));
			gl::Scissor(static_cast<int32>(relative.left * windowWidth), static_cast<int32>(relative.bottom * windowHeight),
				static_cast<uint32>(relative.width * windowWidth), static_cast<uint32>(relative.height * windowHeight));
		}
	}


	auto Viewport::DataUnion::Absolute::Make(int32 left, int32 bottom, uint32 width, uint32 height) -> Absolute
	{
		Absolute absolute;
		absolute.left = left;
		absolute.bottom = bottom;
		absolute.width = width;
		absolute.height = height;
		return absolute;
	}


	auto Viewport::DataUnion::Relative::Make(float left, float bottom, float width, float height) -> Relative
	{
		Relative relative;
		relative.left = left;
		relative.bottom = bottom;
		relative.width = width;
		relative.height = height;
		return relative;
	}


	Viewport::DataUnion::DataUnion(int32 left, int32 bottom, uint32 width, uint32 height)
		: absolute(Absolute::Make(left, bottom, width, height))
	{
	}

	Viewport::DataUnion::DataUnion(float left, float bottom, float width, float height)
		: relative(Relative::Make(left, bottom, width, height))
	{
	}

}

