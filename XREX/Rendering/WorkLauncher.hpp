#pragma once

#include "Declare.hpp"

namespace XREX
{
	struct XREX_API IWorkLauncher
	{
		virtual ~IWorkLauncher();

		virtual void Launch() = 0;
	};


	class XREX_API IndexedDrawer
		: public IWorkLauncher, Noncopyable
	{
	public:
		void SetRenderingLayout(RenderingLayoutSP const& layout)
		{
			layout_ = layout;
		}

		virtual void Launch() override;

	private:
		RenderingLayoutSP layout_;
	};

}
