#include "XREX.hpp"

#include "ShaderProgramInterface.hpp"

namespace XREX
{


	TextureInformation::TextureInformation(std::string const& channel, Texture::TextureType textureType, Texture::TexelType texelType, std::string const& samplerName)
		: channel_(channel), textureType_(textureType), texelType_(texelType), samplerName_(samplerName)
	{
	}

}

