#pragma once

#include "Declare.hpp"

namespace XREX
{
	class XREX_API LayoutAndProgramConnector
		: Noncopyable
	{
	public:
		LayoutAndProgramConnector(RenderingLayoutSP const& layout, ProgramObjectSP const& program);
		~LayoutAndProgramConnector();

		RenderingLayoutSP const& GetRenderingLayout() const
		{
			return layout_;
		}
		ProgramObjectSP const& GetProgram() const
		{
			return program_;
		}
		void Bind();
		void Unbind();
	private:
		RenderingLayoutSP layout_;
		ProgramObjectSP program_;

		uint32 glVAO_;
	};

}