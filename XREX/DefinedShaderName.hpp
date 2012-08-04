#include <string>

struct DefinedUniform
{
	static std::string const ModelMatrix;
	static std::string const ViewMatrix;
	static std::string const ProjectionMatrix;
};

std::string const DefinedUniform::ModelMatrix = "modelMatrix";
std::string const DefinedUniform::ViewMatrix = "viewMatrix";
std::string const DefinedUniform::ProjectionMatrix = "projectionMatrix";


struct DefinedAttribute // not used
{
	static std::string const Position;
	static std::string const Normal;
};