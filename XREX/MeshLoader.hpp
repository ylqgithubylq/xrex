#pragma once

#include "Declare.hpp"

class MeshLoader
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

