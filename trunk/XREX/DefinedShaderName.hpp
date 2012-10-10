#include <string>

/*
 *	Using GetUniformString function to get the string representation.
 */
enum class DefinedUniform
{
	ModelMatrix,
	ViewMatrix,
	ProjectionMatrix,

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

inline std::string const& GetUniformString(DefinedUniform definedUniform)
{
	static std::vector<std::string> const mapping = [] ()
	{
		std::vector<std::string> temp(static_cast<uint32>(DefinedUniform::DefinedUniformCount));
		temp[static_cast<uint32>(DefinedUniform::ModelMatrix)] = "modelMatrix";
		temp[static_cast<uint32>(DefinedUniform::ViewMatrix)] = "viewMatrix";
		temp[static_cast<uint32>(DefinedUniform::ProjectionMatrix)] = "projectionMatrix";

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

enum class DefinedAttribute // not used
{
	Position,
	Normal,
};