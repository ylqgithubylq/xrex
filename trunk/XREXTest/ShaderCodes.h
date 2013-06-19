#pragma once
#include <string>
#include <vector>

class ShaderCodes
{
public:
	ShaderCodes();
	~ShaderCodes();

	std::vector<std::string> const& GetShaderCodes();

private:
	std::vector<std::string> codes;
};

