#include <string>
#include <array>

namespace XREX
{

	/*
	 *	Using GetUniformString function to get the string representation.
	 */
	enum class DefinedUniform
	{
		ModelMatrix,
		NormalMatrix,
		ViewMatrix,
		ProjectionMatrix,

		CameraPosition,

		DiffuseColor,
		SpecularColor,
		EmissiveColor,
		TransparentColor,
		Opacity,
		Shininess,
		SpecularLevel,

		DiffuseMap,
		SpecularMap,
		EmissiveMap,
		NormalMap,
		ShininessMap,
		OpacityMap,
		DisplacementMap,
		LightMap,
		HeightMap,
		ReflectionMap,

		DefinedUniformCount,
	};

	XREX_API inline std::string const& GetUniformString(DefinedUniform definedUniform)
	{
		static std::array<std::string, static_cast<uint32>(DefinedUniform::DefinedUniformCount)> const mapping = [] ()
		{
			std::array<std::string, static_cast<uint32>(DefinedUniform::DefinedUniformCount)> temp;
			temp[static_cast<uint32>(DefinedUniform::ModelMatrix)] = "modelMatrix";
			temp[static_cast<uint32>(DefinedUniform::NormalMatrix)] = "normalMatrix";
			temp[static_cast<uint32>(DefinedUniform::ViewMatrix)] = "viewMatrix";
			temp[static_cast<uint32>(DefinedUniform::ProjectionMatrix)] = "projectionMatrix";

			temp[static_cast<uint32>(DefinedUniform::CameraPosition)] = "cameraPosition";

			temp[static_cast<uint32>(DefinedUniform::DiffuseColor)] = "diffuseColor";
			temp[static_cast<uint32>(DefinedUniform::SpecularColor)] = "specularColor";
			temp[static_cast<uint32>(DefinedUniform::EmissiveColor)] = "emissiveColor";
			temp[static_cast<uint32>(DefinedUniform::TransparentColor)] = "transparentColor";
			temp[static_cast<uint32>(DefinedUniform::Opacity)] = "opacity";
			temp[static_cast<uint32>(DefinedUniform::Shininess)] = "shininess";
			temp[static_cast<uint32>(DefinedUniform::SpecularLevel)] = "specularLevel";

			temp[static_cast<uint32>(DefinedUniform::DiffuseMap)] = "diffuseMap";
			temp[static_cast<uint32>(DefinedUniform::SpecularMap)] = "specularMap";
			temp[static_cast<uint32>(DefinedUniform::EmissiveMap)] = "emissiveMap";
			temp[static_cast<uint32>(DefinedUniform::NormalMap)] = "normalMap";
			temp[static_cast<uint32>(DefinedUniform::ShininessMap)] = "shininessMap";
			temp[static_cast<uint32>(DefinedUniform::OpacityMap)] = "opacityMap";
			temp[static_cast<uint32>(DefinedUniform::DisplacementMap)] = "displacementMap";
			temp[static_cast<uint32>(DefinedUniform::LightMap)] = "lightMap";
			temp[static_cast<uint32>(DefinedUniform::HeightMap)] = "heightMap";
			temp[static_cast<uint32>(DefinedUniform::ReflectionMap)] = "reflectionMap";
			return temp;
		} ();
		return mapping[static_cast<uint32>(definedUniform)];
	}

	enum class DefinedInputAttribute
	{
		Position,
		Normal,

		TextureCoordinate0,
		TextureCoordinate1,
		TextureCoordinate2,
		TextureCoordinate3,
		TextureCoordinate4,
		TextureCoordinate5,
		TextureCoordinate6,
		TextureCoordinate7,

		Color0,
		Color1,
		Color2,
		Color3,
		Color4,
		Color5,
		Color6,
		Color7,

		DefinedAttributeCount,

		DefinedTextureCoordinateCount = TextureCoordinate7 - TextureCoordinate0 + 1,
		DefinedColorCount = Color7 - Color0 + 1,
	};

	XREX_API inline std::string const& GetInputAttributeString(DefinedInputAttribute definedAttribute)
	{
		static std::array<std::string, static_cast<uint32>(DefinedInputAttribute::DefinedAttributeCount)> const mapping = [] ()
		{
			std::array<std::string, static_cast<uint32>(DefinedInputAttribute::DefinedAttributeCount)> temp;
			temp[static_cast<uint32>(DefinedInputAttribute::Position)] = "position";
			temp[static_cast<uint32>(DefinedInputAttribute::Normal)] = "normal";
			
			temp[static_cast<uint32>(DefinedInputAttribute::TextureCoordinate0)] = "textureCoordinate0";
			temp[static_cast<uint32>(DefinedInputAttribute::TextureCoordinate1)] = "textureCoordinate1";
			temp[static_cast<uint32>(DefinedInputAttribute::TextureCoordinate2)] = "textureCoordinate2";
			temp[static_cast<uint32>(DefinedInputAttribute::TextureCoordinate3)] = "textureCoordinate3";
			temp[static_cast<uint32>(DefinedInputAttribute::TextureCoordinate4)] = "textureCoordinate4";
			temp[static_cast<uint32>(DefinedInputAttribute::TextureCoordinate5)] = "textureCoordinate5";
			temp[static_cast<uint32>(DefinedInputAttribute::TextureCoordinate6)] = "textureCoordinate6";
			temp[static_cast<uint32>(DefinedInputAttribute::TextureCoordinate7)] = "textureCoordinate7";

			temp[static_cast<uint32>(DefinedInputAttribute::Color0)] = "color0";
			temp[static_cast<uint32>(DefinedInputAttribute::Color1)] = "color1";
			temp[static_cast<uint32>(DefinedInputAttribute::Color2)] = "color2";
			temp[static_cast<uint32>(DefinedInputAttribute::Color3)] = "color3";
			temp[static_cast<uint32>(DefinedInputAttribute::Color4)] = "color4";
			temp[static_cast<uint32>(DefinedInputAttribute::Color5)] = "color5";
			temp[static_cast<uint32>(DefinedInputAttribute::Color6)] = "color6";
			temp[static_cast<uint32>(DefinedInputAttribute::Color7)] = "color7";

			return temp;
		} ();
		return mapping[static_cast<uint32>(definedAttribute)];
	}

	enum class DefinedOutputAttribute
	{
		DefaultFrameBufferOutput,

		DefinedAttributeCount,
	};

	XREX_API inline std::string const& GetOutputAttributeString(DefinedOutputAttribute definedAttribute)
	{
		static std::array<std::string, static_cast<uint32>(DefinedOutputAttribute::DefinedAttributeCount)> const mapping = [] ()
		{
			std::array<std::string, static_cast<uint32>(DefinedOutputAttribute::DefinedAttributeCount)> temp;
			temp[static_cast<uint32>(DefinedOutputAttribute::DefaultFrameBufferOutput)] = "XREX_DefaultFrameBufferOutput";
			return temp;
		} ();
		return mapping[static_cast<uint32>(definedAttribute)];
	}
}
