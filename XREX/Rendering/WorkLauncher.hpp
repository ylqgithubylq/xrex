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
		void SetLayoutAndProgramConnector(LayoutAndProgramConnectorSP const& connector)
		{
			layoutConnector_ = connector;
		}
		void SetFrameBuffer(FrameBufferSP const& frameBuffer)
		{
			frameBuffer_ = frameBuffer;
		}
		void SetFrameBufferAndProgramConnector()
		{

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
		LayoutAndProgramConnectorSP layoutConnector_;
		FrameBufferSP frameBuffer_;
		RenderingTechniqueSP technique_;
	};

}
