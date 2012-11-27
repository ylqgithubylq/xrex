#pragma once


#pragma warning(disable: 4482) // enum grammar extension: MyEnum::EnumValue
#pragma warning(error: 4150) // deletion of pointer to incomplete type (destructor will not be called)
#pragma warning(error: 4715) // not all control paths return a value


#include <cassert>

#include <memory>

#include <utility>

#include <limits>

namespace XREX
{
	
	struct Settings;
	struct RenderingSettings;

	class XREXContext;
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
	class OrbitCameraController;
	typedef std::shared_ptr<OrbitCameraController> OrbitCameraControllerSP;
	class FirstPersonCameraController;
	typedef std::shared_ptr<FirstPersonCameraController> FirstPersonCameraControllerSP;

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

	class RasterizerStateObject;
	typedef std::shared_ptr<RasterizerStateObject> RasterizerStateObjectSP;
	class DepthStencilStateObject;
	typedef std::shared_ptr<DepthStencilStateObject> DepthStencilStateObjectSP;
	class BlendStateObject;
	typedef std::shared_ptr<BlendStateObject> BlendStateObjectSP;
	class SamplerStateObject;
	typedef std::shared_ptr<SamplerStateObject> SamplerStateObjectSP;
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

	class Viewport;
	typedef std::shared_ptr<Viewport> ViewportSP;

}


#include "BasicType.hpp"

#include "Math.hpp"
#include "GeometricalMath.hpp"

#include "Color.hpp"

#include "Util.hpp"

