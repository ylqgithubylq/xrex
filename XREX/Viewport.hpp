#pragma once

#include "Declare.hpp"


namespace XREX
{
	class XREX_API Viewport
		: Noncopyable
	{
	public:
		/*
		 *	Absolute mode. The unit of parameters are actual pixel on screen.
		 *	@depthOrder: viewport with smaller value will be displayed on top of larger value's one.
		 */
		Viewport(int32 depthOrder, int32 left, int32 bottom, uint32 width, uint32 height);
		/*
		 *	Relative mode. The unit of parameters are relative size to the screen. range: [0, 1]
		 *	@depthOrder: viewport with smaller value will be displayed on top of larger value's one.
		 */
		Viewport(int32 depthOrder, float left, float bottom, float width, float height);
		virtual ~Viewport();

		int32 GetDepthOrder() const
		{
			return depthOrder_;
		}

		bool IsAbsoluteMode() const
		{
			return absolute_;
		}
		/*
		 *	Can only be called in absolute mode.
		 *	@return: tuple<(left), (bottom), (width), (height)>
		 */
		std::tuple<int32, int32, uint32, uint32> GetAbsolute() const
		{
			assert(absolute_);
			return std::make_tuple(data_.absolute.left, data_.absolute.bottom, data_.absolute.width, data_.absolute.height);
		}
		/*
		 *	Can only be called in relative mode.
		 *	@return: tuple<(left), (bottom), (width), (height)>
		 */
		std::tuple<float, float, float, float> GetRelative() const
		{
			assert(!absolute_);
			return std::make_tuple(data_.relative.left, data_.relative.bottom, data_.relative.width, data_.relative.height);
		}
		/*
		 *	If viewport is absolute mode, all parameters are ignored.
		 */
		void Bind(uint32 windowWidth, uint32 windowHeight);

	private:

		union DataUnion
		{
			struct Absolute
			{
				int32 left;
				int32 bottom;
				uint32 width;
				uint32 height;
				static Absolute Make(int32 left, int32 bottom, uint32 width, uint32 height);
			};
			struct Relative
			{
				float left;
				float bottom;
				float width;
				float height;
				static Relative Make(float left, float bottom, float width, float height);
			};
			static_assert(sizeof(Absolute) == sizeof(Relative), "Absolute and Relative must have same size.");
			Absolute absolute;
			Relative relative;

			DataUnion(int32 left, int32 bottom, uint32 width, uint32 height);
			DataUnion(float left, float bottom, float width, float height);
		};

	private:
		int32 depthOrder_;
		DataUnion data_;
		bool absolute_;

	};

}
