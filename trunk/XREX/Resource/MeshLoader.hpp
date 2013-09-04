#pragma once

#include "Declare.hpp"
#include "LoadingResult.hpp"

namespace XREX
{

	class XREX_API MeshLoader
		: Noncopyable
	{
	public:
		MeshLoader();
		~MeshLoader();

		/*
		 *	@return: mesh and texture data ready to create mesh.
		 */
		MeshLoadingResultSP LoadMesh(std::string const& fileName);
	};

}
