#pragma once


#pragma warning(disable: 4482) // enum grammar extension: MyEnum::EnumValue
#pragma warning(error: 4150) // deletion of pointer to incomplete type (destructor will not be called)
#pragma warning(error: 4715) // not all control paths return a value


#include <cassert>

#include <memory>

#include <utility>

#include <limits>



struct Settings;
struct RenderingSettings;

class Application;
class LocalResourceLoader;
class MeshLoader;
class TextureLoader;
class ResourceManager;
class RenderingFactory;
class RenderingEngine;
class InputCenter;
class Window;
class GLWindow;

class InputHandler;
typedef std::shared_ptr<InputHandler> InputHandlerSP;
class FreeRoamCameraController;
typedef std::shared_ptr<FreeRoamCameraController> FreeRoamCameraControllerSP;

class Scene;
typedef std::shared_ptr<Scene> SceneSP;
class NaiveManagedScene;
typedef std::shared_ptr<NaiveManagedScene> NaiveManagedSceneSP;

class SceneObject;
typedef std::shared_ptr<SceneObject> SceneObjectSP;
class Component;
typedef std::shared_ptr<Component> ComponentSP;

class Transformation;
typedef std::shared_ptr<Transformation> TransformationSP;

class Renderable;
typedef std::shared_ptr<Renderable> RenderableSP;
class Mesh;
typedef std::shared_ptr<Mesh> MeshSP;
class SubMesh;
typedef std::shared_ptr<SubMesh> SubMeshSP;

class ShaderObject;
typedef std::shared_ptr<ShaderObject> ShaderObjectSP;
class ProgramObject;
typedef std::shared_ptr<ProgramObject> ProgramObjectSP;
class EffectParameter;
typedef std::shared_ptr<EffectParameter> EffectParameterSP;
class RenderingEffect;
typedef std::shared_ptr<RenderingEffect> RenderingEffectSP;
class RenderingTechnique;
typedef std::shared_ptr<RenderingTechnique> RenderingTechniqueSP;
class RenderingPass;
typedef std::shared_ptr<RenderingPass> RenderingPassSP;
class Material;
typedef std::shared_ptr<Material> MaterialSP;

class RenderingLayout;
typedef std::shared_ptr<RenderingLayout> RenderingLayoutSP;
class GraphicsBuffer;
typedef std::shared_ptr<GraphicsBuffer> GraphicsBufferSP;
class Texture;
typedef std::shared_ptr<Texture> TextureSP;

class Camera;
typedef std::shared_ptr<Camera> CameraSP;




#include "BasicType.hpp"

#include "Math.hpp"

#include "Color.hpp"

#include "Util.hpp"


// temp
#include <CoreGL.hpp>

uint32 GetGLError()
{
	return gl::GetError();
}


