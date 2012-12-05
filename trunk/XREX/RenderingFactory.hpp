#pragma once

#include "Declare.hpp"

#include "Shader.hpp"
#include "GraphicsBuffer.hpp"
#include "RenderingLayout.hpp"
#include "Texture.hpp"
#include "RenderingPipelineState.hpp"
#include "Sampler.hpp"
#include "Viewport.hpp"

namespace XREX
{

	class XREX_API RenderingFactory
		: Noncopyable
	{
	public:
		RenderingFactory();
		virtual ~RenderingFactory();

		void Initialize();

		ViewportSP const& GetDefaultViewport() const
		{
			return defaultViewport_;
		}

		TextureSP const& GetBlackTexture1D() const
		{
			return blackTexture1D_;
		}
		TextureSP const& GetBlackTexture2D() const
		{
			return blackTexture2D_;
		}
		TextureSP const& GetBlackTexture3D() const
		{
			return blackTexture3D_;
		}
		TextureSP const& GetBlackTextureCube() const
		{
			assert(false); // not implemented
			return blackTextureCube_;
		}

		SamplerSP const& GetDefaultSampler() const
		{
			return defaultSampler_;
		}

		RenderingEngine& GetRenderingEngine();

		RasterizerStateObjectSP CreateRasterizerStateObject(RasterizerState const& rasterizerState)
		{
			return MakeSP<RasterizerStateObject>(rasterizerState);
		}
		DepthStencilStateObjectSP CreateDepthStencilStateObject(DepthStencilState const& depthStencilState)
		{
			return MakeSP<DepthStencilStateObject>(depthStencilState);
		}
		BlendStateObjectSP CreateBlendStateObject(BlendState const& blendState)
		{
			return MakeSP<BlendStateObject>(blendState);
		}
		ShaderObjectSP CreateShaderObject(ShaderObject::ShaderType type, std::string const& source)
		{
			return MakeSP<ShaderObject>(type, source);
		}
		ShaderObjectSP CreateShaderObject(ShaderObject::ShaderType type, std::string&& source)
		{
			return MakeSP<ShaderObject>(type, std::move(source));
		}
		ProgramObjectSP CreateProgramObject()
		{
			return MakeSP<ProgramObject>();
		}
		template <typename T>
		VertexBufferSP CreateVertexBuffer(GraphicsBuffer::Usage usage, std::vector<T> const& data, std::string const& channel)
		{
			return MakeSP<VertexBuffer>(usage, data, channel);
		}
		template <typename T>
		VertexBufferSP CreateVertexBuffer(GraphicsBuffer::Usage usage, std::vector<T> const& data, std::string const& channel, bool normalized)
		{
			return MakeSP<VertexBuffer>(usage, data, channel, normalized);
		}
		template <typename T>
		VertexBufferSP CreateVertexBuffer(GraphicsBuffer::Usage usage, std::vector<T> const& data, VertexBuffer::DataLayout&& description)
		{
			return MakeSP<VertexBuffer>(usage, data, std::move(description));
		}
		template <typename T>
		IndexBufferSP CreateIndexBuffer(GraphicsBuffer::Usage usage, std::vector<T> const& data, IndexBuffer::PrimitiveType primitiveType)
		{
			return MakeSP<IndexBuffer>(usage, data, primitiveType);
		}
		RenderingLayoutSP CreateRenderingLayout(std::vector<VertexBufferSP> const& buffers, IndexBufferSP const& indexBuffer)
		{
			return MakeSP<RenderingLayout>(buffers, indexBuffer);
		}

		SamplerSP CreateSampler(SamplerState const& samplerState)
		{
			return MakeSP<Sampler>(samplerState);
		}
		template <typename T>
		TextureSP CreateTexture1D(Texture::DataDescription<1> const& description, std::vector<std::vector<T>> const& data)
		{
			return MakeSP<Texture1D>(description, data);
		}
		template <typename T>
		TextureSP CreateTexture1D(Texture::DataDescription<1> const& description, std::vector<std::vector<T>> const& data, bool generateMipmap)
		{
			return MakeSP<Texture1D>(description, data, generateMipmap);
		}
		template <typename T>
		TextureSP CreateTexture2D(Texture::DataDescription<2> const& description, std::vector<std::vector<T>> const& data)
		{
			return MakeSP<Texture2D>(description, data);
		}
		template <typename T>
		TextureSP CreateTexture2D(Texture::DataDescription<2> const& description, std::vector<std::vector<T>> const& data, bool generateMipmap)
		{
			return MakeSP<Texture2D>(description, data, generateMipmap);
		}
		template <typename T>
		TextureSP CreateTexture3D(Texture::DataDescription<3> const& description, std::vector<std::vector<T>> const& data)
		{
			return MakeSP<Texture3D>(description, data);
		}
		template <typename T>
		TextureSP CreateTexture3D(Texture::DataDescription<3> const& description, std::vector<std::vector<T>> const& data, bool generateMipmap)
		{
			return MakeSP<Texture3D>(description, data, generateMipmap);
		}
		template <typename T>
		TextureSP CreateTextureCube(Texture::DataDescription<2> const& description, std::array<std::vector<std::vector<T>>, 6> const& data)
		{
			return MakeSP<TextureCube>(description, data);
		}
		template <typename T>
		TextureSP CreateTextureCube(Texture::DataDescription<2> const& description, std::array<std::vector<std::vector<T>>, 6> const& data, bool generateMipmap)
		{
			return MakeSP<TextureCube>(description, data, generateMipmap);
		}

		ViewportSP CreateViewport(int32 depthOrder, int32 left, int32 bottom, uint32 width, uint32 height)
		{
			return MakeSP<Viewport>(depthOrder, left, bottom, width, height);
		}
		ViewportSP CreateViewport(int32 depthOrder, float left, float bottom, float width, float height)
		{
			return MakeSP<Viewport>(depthOrder, left, bottom, width, height);
		}

	private:
		ViewportSP defaultViewport_;
		TextureSP blackTexture1D_;
		TextureSP blackTexture2D_;
		TextureSP blackTexture3D_;
		TextureSP blackTextureCube_;
		SamplerSP defaultSampler_;
		std::unique_ptr<RenderingEngine> renderingEngine_;

	};

}
