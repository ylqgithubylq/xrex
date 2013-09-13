#pragma once

#include "Declare.hpp"

namespace XREX
{
	class XREX_API BufferAndProgramConnector
		: Noncopyable
	{
	public:
		BufferAndProgramConnector(RenderingLayoutSP const& layout, ProgramObjectSP const& program);
		~BufferAndProgramConnector();

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


	// TODO is there better approach to handle connector?

	class XREX_API ConnectorPack
		: Noncopyable
	{
	public:
		ConnectorPack(std::vector<BufferAndProgramConnectorSP> const& connectors)
			: connectors_(connectors)
		{
		}
		ConnectorPack(std::vector<BufferAndProgramConnectorSP>&& connectors)
			: connectors_(std::move(connectors))
		{
		}

		std::vector<BufferAndProgramConnectorSP> const& GetAllConnectors() const
		{
			return connectors_;
		}

	private:
		std::vector<BufferAndProgramConnectorSP> connectors_;
	};

}