#pragma once

#include <cassert>


#include <memory>

struct Settings;
struct RenderingSettings;

class Context;
class LocalResourceLoader;
class Logger;
class RenderingEngine;
class Window;
class GLWindow;

class SceneObject;
typedef std::shared_ptr<SceneObject> SceneObjectSP;
class Component;
typedef std::shared_ptr<Component> ComponentSP;

class Transformation;
typedef std::shared_ptr<Transformation> TransformationSP;

class Renderable;
typedef std::shared_ptr<Renderable> RenderableSP;
class StaticMesh;
typedef std::shared_ptr<StaticMesh> StaticMeshSP;

class ShaderObject;
typedef std::shared_ptr<ShaderObject> ShaderObjectSP;
class ProgramObject;
typedef std::shared_ptr<ProgramObject> ProgramObjectSP;
class EffectParameter;
typedef std::shared_ptr<EffectParameter> EffectParameterSP;
class RenderingEffect;
typedef std::shared_ptr<RenderingEffect> RenderingEffectSP;
class RenderingPass;
typedef std::shared_ptr<RenderingPass> RenderingPassSP;

class RenderingLayout;
typedef std::shared_ptr<RenderingLayout> RenderingLayoutSP;
class GraphicsBuffer;
typedef std::shared_ptr<GraphicsBuffer> GraphicsBufferSP;

class Camera;
typedef std::shared_ptr<Camera> CameraSP;


#pragma warning(disable: 4482) // enum grammar extension: MyEnum::EnumValue

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


