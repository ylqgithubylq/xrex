#include "XREX.hpp"

#include "RenderingEngine.hpp"

#include "Base/XREXContext.hpp"
#include "Base/Window.hpp"
#include "Base/Logger.hpp"
#include "Rendering/RenderingFactory.hpp"
#include "Rendering/RenderingPipelineState.hpp"
#include "Rendering/GL/GLUtil.hpp"
#include "Rendering/GraphicsContext.hpp"
#include "Rendering/RenderingProcess.hpp"
#include "Rendering/FrameBuffer.hpp"
#include "Rendering/DefinedShaderName.hpp"
#include "Rendering/SystemTechnique.hpp"

#include <CoreGL.hpp>

#include <map>
#include <string>

using std::vector;

namespace XREX
{
	namespace
	{
		struct DebugCallback
		{
			static void APIENTRY Callback(uint32 source, uint32 type, uint32 id, uint32 severity, int32 length, char const* message, void* userParam)
			{
				RenderingEngine* engine = reinterpret_cast<RenderingEngine*>(userParam);
				
				std::string sourceString;
				switch (source)
				{
				case gl::GL_DEBUG_SOURCE_API:
					sourceString = "DEBUG_SOURCE_API";
					break;
				case gl::GL_DEBUG_SOURCE_WINDOW_SYSTEM:
					sourceString = "DEBUG_SOURCE_WINDOW_SYSTEM";
					break;
				case gl::GL_DEBUG_SOURCE_SHADER_COMPILER:
					sourceString = "DEBUG_SOURCE_SHADER_COMPILER";
					break;
				case gl::GL_DEBUG_SOURCE_THIRD_PARTY:
					sourceString = "DEBUG_SOURCE_THIRD_PARTY";
					break;
				case gl::GL_DEBUG_SOURCE_APPLICATION:
					sourceString = "DEBUG_SOURCE_APPLICATION";
					break;
				case gl::GL_DEBUG_SOURCE_OTHER:
					sourceString = "DEBUG_SOURCE_OTHER";
					break;
				default:
					assert(false);
					break;
				}

				std::string typeString;
				switch (type)
				{
				case gl::GL_DEBUG_TYPE_ERROR:
					typeString = "DEBUG_TYPE_ERROR";
					break;
				case gl::GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
					typeString = "DEBUG_TYPE_DEPRECATED_BEHAVIOR";
					break;
				case gl::GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
					typeString = "DEBUG_TYPE_UNDEFINED_BEHAVIOR";
					break;
				case gl::GL_DEBUG_TYPE_PORTABILITY:
					typeString = "DEBUG_TYPE_PORTABILITY";
					break;
				case gl::GL_DEBUG_TYPE_PERFORMANCE:
					typeString = "DEBUG_TYPE_PERFORMANCE";
					break;
				case gl::GL_DEBUG_TYPE_MARKER:
					typeString = "DEBUG_TYPE_MARKER";
					break;
				case gl::GL_DEBUG_TYPE_PUSH_GROUP:
					typeString = "DEBUG_TYPE_PUSH_GROUP";
					break;
				case gl::GL_DEBUG_TYPE_POP_GROUP:
					typeString = "DEBUG_TYPE_POP_GROUP";
					break;
				case gl::GL_DEBUG_TYPE_OTHER:
					typeString = "DEBUG_TYPE_OTHER";
					break;
				default:
					assert(false);
					break;
				}

				std::string severityString;
				switch (severity)
				{
				case gl::GL_DEBUG_SEVERITY_LOW:
					severityString = "DEBUG_SEVERITY_LOW";
					break;
				case gl::GL_DEBUG_SEVERITY_MEDIUM:
					severityString = "DEBUG_SEVERITY_MEDIUM";
					break;
				case gl::GL_DEBUG_SEVERITY_HIGH:
					severityString = "DEBUG_SEVERITY_HIGH";
					break;
				case gl::GL_DEBUG_SEVERITY_NOTIFICATION:
					severityString = "DEBUG_SEVERITY_NOTIFICATION";
					break;
				default:
					assert(false);
					break;
				}
				XREXContext::GetInstance().GetLogger().Log("from: " + sourceString + ", ").Log("type: " + typeString + ", ")
					.Log("id: ").Log(id).Log(", ").Log("severity: " + severityString + ", ").EndLine()
					.Log("message: ").Log(message).EndLine();
				assert(severity != gl::GL_DEBUG_SEVERITY_HIGH);
			}
		};

		FrameBufferLayoutDescriptionSP MakeDescription(Size<uint32, 2> const& size, TexelFormat colorFormat, TexelFormat depthStencilFormat)
		{
			FrameBufferLayoutDescriptionSP description = MakeSP<FrameBufferLayoutDescription>("XREX_BackBuffer");
			description->SetSize(size);
			description->SetSizeMode(FrameBufferLayoutDescription::SizeMode::Sceen);
			description->AddChannel(FrameBufferLayoutDescription::ChannelDescription(GetOutputAttributeString(DefinedOutputAttribute::DefaultFrameBufferOutput), TexelFormat::RGBA8));
			switch (depthStencilFormat)
			{
			case TexelFormat::Depth16:
			case TexelFormat::Depth24:
			case TexelFormat::Depth32:
				description->SetDepth(depthStencilFormat);
				break;
			case TexelFormat::Depth32F:
				assert(false);
				break;
			case TexelFormat::Depth24Stencil8:
				description->SetDepthStencil(depthStencilFormat);
				break;
			case TexelFormat::Stencil8:
				description->SetStencil(depthStencilFormat);
				break;
			case TexelFormat::TexelFormatCount:
				assert(false);
				break;
			default:
				assert(false);
				break;
			}
			return description;
		}

		class DefaultFrameBuffer
			: public FrameBuffer
		{
		public:
			DefaultFrameBuffer(Size<uint32, 2> const& size, TexelFormat colorFormat, TexelFormat depthStencilFormat)
				: FrameBuffer(MakeDescription(size, colorFormat, depthStencilFormat))
			{
			}
			virtual ~DefaultFrameBuffer() override
			{
			}
		};

	}



	RenderingEngine::RenderingEngine(Window& window, Settings const& settings)
		: defaultBlendColor_(0, 0, 0, 1)
	{
		// initialize the graphics context first
		graphicsContext_ = MakeUP<GraphicsContext>(window, settings);

		// window may not have the size specified in settings, so use window.GetClientRegionSize() here to get the actual size.
		defaultFrameBuffer_ = MakeSP<DefaultFrameBuffer>(window.GetClientRegionSize(), settings.renderingSettings.colorFormat, settings.renderingSettings.depthStencilFormat);

#ifdef XREX_DEBUG
		gl::DebugMessageCallback(&DebugCallback::Callback, this);
		gl::Enable(gl::GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif // XREX_DEBUG


		gl::PixelStorei(gl::GL_UNPACK_ALIGNMENT, 1); // default 4
		gl::PixelStorei(gl::GL_PACK_ALIGNMENT, 1); // default 4


		gl::Enable(gl::GL_SCISSOR_TEST);

		gl::Enable(gl::GL_POLYGON_OFFSET_FILL);
		gl::Enable(gl::GL_POLYGON_OFFSET_POINT);
		gl::Enable(gl::GL_POLYGON_OFFSET_LINE);
		defaultRasterizerState_ = XREXContext::GetInstance().GetRenderingFactory().CreateRasterizerStateObject(RasterizerState());
		defaultDepthStencilState_ = XREXContext::GetInstance().GetRenderingFactory().CreateDepthStencilStateObject(DepthStencilState());
		defaultBlendState_ = XREXContext::GetInstance().GetRenderingFactory().CreateBlendStateObject(BlendState());

		RegisterSystemTechniqueFactory(MakeUP<TransformationTechniqueFactory>());
		RegisterSystemTechniqueFactory(MakeUP<CameraTechniqueFactory>());
	}


	RenderingEngine::~RenderingEngine()
	{
		beforeRenderingFunction_.swap(decltype(beforeRenderingFunction_)());
		afterRenderingFunction_.swap(decltype(afterRenderingFunction_)());
	}

	uint32 RenderingEngine::GetGLError()
	{
		return gl::GetError();
	}

	bool RenderingEngine::RegisterSystemTechniqueFactory(std::unique_ptr<ISystemTechniqueFactory>&& factory)
	{
		auto found = systemTechniqueFactories_.find(factory->GetIndexName());
		if (found == systemTechniqueFactories_.end())
		{
			systemTechniqueFactories_[factory->GetIndexName()] = std::move(factory);
			return true;
		}
		return false;
	}

	ISystemTechniqueFactory* RenderingEngine::GetSystemTechniqueFactory(std::string const& name)
	{
		auto found = systemTechniqueFactories_.find(name);
		if (found != systemTechniqueFactories_.end())
		{
			return found->second.get();
		}
		return nullptr;
	}


	void RenderingEngine::SwapBuffers()
	{
		graphicsContext_->SwapBuffers();
	}

	void RenderingEngine::RenderAFrame()
	{

		// clear frame buffer globally first.
		gl::ClearColor(0, 0, 0, 0);
		gl::Clear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT | gl::GL_STENCIL_BUFFER_BIT);
		double currentTime = timer_.Elapsed();
		double delta = currentTime - lastTime_;
		if (beforeRenderingFunction_ != nullptr)
		{
			beforeRenderingFunction_(currentTime, delta);
		}

#ifdef USE_OPENGL_COMPATIBILITY_PROFILE
		gl::Enable(gl::GL_POLYGON_OFFSET_FILL);
		gl::Enable(gl::GL_POLYGON_OFFSET_POINT);
		gl::Enable(gl::GL_POLYGON_OFFSET_LINE);
#endif

		SceneSP const& scene = XREXContext::GetInstance().GetScene();
		assert(process_);
		process_->RenderScene(scene);


#ifdef USE_OPENGL_COMPATIBILITY_PROFILE
		gl::UseProgram(0);
		gl::ActiveTexture(gl::GL_TEXTURE0);
		gl::BindSampler(0, 0);
		gl::BindBuffer(gl::GL_ARRAY_BUFFER, 0);
		gl::BindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, 0);
		// TODO temp hack for CEGUI.
		defaultRasterizerState_->Bind(0, 0);
		defaultDepthStencilState_->Bind(0, 0);
		defaultBlendState_->Bind(defaultBlendColor_);
		gl::Disable(gl::GL_POLYGON_OFFSET_FILL);
		gl::Disable(gl::GL_POLYGON_OFFSET_POINT);
		gl::Disable(gl::GL_POLYGON_OFFSET_LINE);
		gl::Disable(gl::GL_DEPTH_TEST);
		gl::Disable(gl::GL_CULL_FACE);
#endif

		if (afterRenderingFunction_ != nullptr)
		{
			afterRenderingFunction_(currentTime, delta);
		}

		lastTime_ = currentTime;
	}

}
