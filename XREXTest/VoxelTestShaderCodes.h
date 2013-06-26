#pragma once
#include <string>
#include <vector>

class VoxelTestShaderCodes
{
public:
	VoxelTestShaderCodes();

	std::vector<std::string> const& GetShaderCodes();

private:
	std::vector<std::string> codes;

};