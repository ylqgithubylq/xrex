#pragma once

#include "Declare.hpp"

namespace XREX
{

	class XREX_API MeshLoader
		: Noncopyable
	{
	public:
		MeshLoader();
		~MeshLoader();

		/*
		 *	@return nullptr if failed.
		 */
		MeshSP LoadMesh(std::string const& fileName);
	};

}
