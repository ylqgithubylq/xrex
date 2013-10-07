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
		void SetBufferAndProgramConnector(BufferAndProgramConnectorSP const& connector)
		{
			connector_ = connector;
		}
		void SetTechnique(RenderingTechniqueSP const& technique)
		{
			technique_ = technique;
		}

		virtual void Launch() override;

		/*
		 *	Only RenderingLayout need to be set.
		 */
		void CoreLaunch();

	private:
		RenderingLayoutSP layout_;
		BufferAndProgramConnectorSP connector_;
		RenderingTechniqueSP technique_;
	};

}
