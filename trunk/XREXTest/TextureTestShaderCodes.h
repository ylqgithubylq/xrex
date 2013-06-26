#pragma once
#include <string>
#include <vector>

class TextureTestShaderCodes
{
public:
	TextureTestShaderCodes();
	~TextureTestShaderCodes();

	std::vector<std::string> const& GetShaderCodes();

private:
	std::vector<std::string> codes;
};


