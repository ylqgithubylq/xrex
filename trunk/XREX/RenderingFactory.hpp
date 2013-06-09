#pragma once

#include "Declare.hpp"

#include "Shader.hpp"
#include "GraphicsBuffer.hpp"
#include "BufferView.hpp"
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
		RenderingFactory(Window& window, Settings const& settings);
		virtual ~RenderingFactory();

		std::string const& GetGLSLVersionString() const
		{
			return glslVersionString_;
		}

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
		ShaderObjectSP CreateShaderObject(ShaderObject::ShaderType type)
		{
			return MakeSP<ShaderObject>(type);
		}
		ProgramObjectSP CreateProgramObject()
		{
			return MakeSP<ProgramObject>();
		}
		GraphicsBufferSP CreateGraphicsBuffer(GraphicsBuffer::Usage usage, uint32 sizeInBytes)
		{
			return MakeSP<GraphicsBuffer>(usage, sizeInBytes);
		}
		GraphicsBufferSP CreateGraphicsBuffer(GraphicsBuffer::Usage usage, void const* data, uint32 sizeInBytes)
		{
			return MakeSP<GraphicsBuffer>(usage, data, sizeInBytes);
		}
		VertexBufferSP CreateVertexBuffer(VertexBuffer::DataLayoutDescription&& description)
		{
			return MakeSP<VertexBuffer>(std::move(description));
		}
		template <typename T>
		VertexBufferSP CreateVertexBuffer(GraphicsBuffer::Usage usage, std::vector<T> const& data, std::string const& channel)
		{
			GraphicsBufferSP buffer = MakeSP<GraphicsBuffer>(usage, data.data(), data.size() * sizeof(T));
			VertexBuffer::DataLayoutDescription description(data.size());
			description.AddChannelLayout(VertexBuffer::DataLayoutDescription::ElementLayoutDescription(0, 0, TypeToElementType<T>::Type, channel));
			return MakeSP<VertexBuffer>(std::move(buffer), std::move(description));
		}
		template <typename T>
		VertexBufferSP CreateVertexBuffer(GraphicsBuffer::Usage usage, std::vector<T> const& data, std::string const& channel, bool normalized)
		{
			GraphicsBufferSP buffer = MakeSP<GraphicsBuffer>(usage, data.data(), data.size() * sizeof(T));
			VertexBuffer::DataLayoutDescription description(data.size());
			description.AddChannelLayout(VertexBuffer::DataLayoutDescription::ElementLayoutDescription(0, 0, TypeToElementType<T>::Type, channel, normalized));
			return MakeSP<VertexBuffer>(std::move(buffer), std::move(description));
		}
		template <typename T>
		VertexBufferSP CreateVertexBuffer(GraphicsBuffer::Usage usage, std::vector<T> const& data, VertexBuffer::DataLayoutDescription&& description)
		{
			GraphicsBufferSP buffer = MakeSP<GraphicsBuffer>(usage, data.data(), data.size() * sizeof(T));
			return MakeSP<VertexBuffer>(std::move(buffer), std::move(description));
		}
		VertexBufferSP CreateVertexBuffer(GraphicsBuffer::Usage usage, uint32 elementSizeInBytes, VertexBuffer::DataLayoutDescription&& description)
		{
			GraphicsBufferSP buffer = MakeSP<GraphicsBuffer>(usage, elementSizeInBytes);
			return MakeSP<VertexBuffer>(std::move(buffer), std::move(description));
		}
		IndexBufferSP CreateIndexBuffer(IndexBuffer::TopologicalType topologicalType, ElementType elementType, uint32 elementCount)
		{
			return MakeSP<IndexBuffer>(topologicalType, elementType, elementCount);
		}
		template <typename T>
		IndexBufferSP CreateIndexBuffer(GraphicsBuffer::Usage usage, std::vector<T> const& data, IndexBuffer::TopologicalType topologicalType)
		{
			GraphicsBufferSP buffer = MakeSP<GraphicsBuffer>(usage, data.data(), data.size() * sizeof(T));
			return MakeSP<IndexBuffer>(std::move(buffer), topologicalType, TypeToElementType<T>::Type, data.size());
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
		std::string glslVersionString_;
		ViewportSP defaultViewport_;
		TextureSP blackTexture1D_;
		TextureSP blackTexture2D_;
		TextureSP blackTexture3D_;
		TextureSP blackTextureCube_;
		SamplerSP defaultSampler_;
		std::unique_ptr<RenderingEngine> renderingEngine_;

	};

}
