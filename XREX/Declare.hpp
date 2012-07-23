#pragma once

#include <cassert>


#include <memory>

struct RenderingSettings;
class LocalResourceLoader;
class Logger;
class RenderingEngine;
class GLWindow;

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

class Renderable;
typedef std::shared_ptr<Renderable> RenderableSP;
class StaticMesh;
typedef std::shared_ptr<StaticMesh> StaticMeshSP;

#include "BasicType.hpp"

#include "Math.hpp"

#include "Util.hpp"

#include <CoreGL.hpp>

uint32 GetGLError()
{
	return gl::GetError();
}

