#pragma once

#include "Declare.hpp"
#include "Rendering/GraphicsBuffer.hpp"
#include "Rendering/RenderingLayout.hpp"
#include "Rendering/Texture.hpp"
#include "Rendering/Sampler.hpp"
#include "Rendering/RenderingPipelineState.hpp"
#include "Rendering/ShaderProgram.hpp"

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

		RenderingEngine& GetRenderingEngine()
		{
			return *renderingEngine_;
		}

		RasterizerStateObjectSP CreateRasterizerStateObject(RasterizerState const& rasterizerState);
		DepthStencilStateObjectSP CreateDepthStencilStateObject(DepthStencilState const& depthStencilState);
		BlendStateObjectSP CreateBlendStateObject(BlendState const& blendState);

		ShaderObjectSP CreateShaderObject(ShaderObject::ShaderType type);
		ProgramObjectSP CreateProgramObject();

		GraphicsBufferSP CreateGraphicsBuffer(GraphicsBuffer::Usage usage, uint32 sizeInBytes);
		GraphicsBufferSP CreateGraphicsBuffer(GraphicsBuffer::Usage usage, void const* data, uint32 sizeInBytes);
		template <typename T>
		GraphicsBufferSP CreateGraphicsBuffer(GraphicsBuffer::Usage usage, std::vector<T> const& data)
		{
			return CreateGraphicsBuffer(usage, data.data(), data.size() * sizeof(T));
		}
		GraphicsBufferSP CreateGraphicsBuffer(GraphicsBuffer::Usage usage, uint32 sizeInBytes, BufferView::BufferType typeHint);
		GraphicsBufferSP CreateGraphicsBuffer(GraphicsBuffer::Usage usage, void const* data, uint32 sizeInBytes, BufferView::BufferType typeHint);
		template <typename T>
		GraphicsBufferSP CreateGraphicsBuffer(GraphicsBuffer::Usage usage, std::vector<T> const& data, BufferView::BufferType typeHint)
		{
			return CreateGraphicsBuffer(usage, data.data(), data.size() * sizeof(T), typeHint);
		}

		GraphicsBufferSP CreateGraphicsBufferWithBufferInformation(GraphicsBuffer::Usage usage, BufferBindingInformation const& information);
		/*
		 *	Uniform, ShaderStorage, AtomicCounter.
		 */
		ShaderResourceBufferSP CreateShaderResourceBuffer(BufferBindingInformation const& information, bool createBuffer);
		VertexBufferSP CreateVertexBuffer(VertexBuffer::DataLayoutDescription&& description);
		template <typename T>
		VertexBufferSP CreateVertexBuffer(GraphicsBuffer::Usage usage, std::vector<T> const& data, std::string const& channel)
		{
			GraphicsBufferSP buffer = CreateGraphicsBuffer(usage, data.data(), data.size() * sizeof(T));
			VertexBuffer::DataLayoutDescription description(data.size());
			description.AddChannelLayout(VertexBuffer::DataLayoutDescription::ElementLayoutDescription(0, 0, TypeToElementType<T>::Type, channel));
			return CreateVertexBuffer(std::move(description), std::move(buffer));
		}
		template <typename T>
		VertexBufferSP CreateVertexBuffer(GraphicsBuffer::Usage usage, std::vector<T> const& data, std::string const& channel, bool normalized)
		{
			GraphicsBufferSP buffer = CreateGraphicsBuffer(usage, data.data(), data.size() * sizeof(T));
			VertexBuffer::DataLayoutDescription description(data.size());
			description.AddChannelLayout(VertexBuffer::DataLayoutDescription::ElementLayoutDescription(0, 0, TypeToElementType<T>::Type, channel, normalized));
			return CreateVertexBuffer(std::move(description), std::move(buffer));
		}

		VertexBufferSP CreateVertexBuffer(VertexBuffer::DataLayoutDescription&& description, GraphicsBufferSP const& buffer);
		VertexBufferSP CreateVertexBuffer(GraphicsBuffer::Usage usage, uint32 elementSizeInBytes, VertexBuffer::DataLayoutDescription&& description);
		template <typename T>
		VertexBufferSP CreateVertexBuffer(GraphicsBuffer::Usage usage, std::vector<T> const& data, VertexBuffer::DataLayoutDescription&& description)
		{
			GraphicsBufferSP buffer = CreateGraphicsBuffer(usage, data.data(), data.size() * sizeof(T));
			return CreateVertexBuffer(std::move(description), std::move(buffer));
		}
		IndexBufferSP CreateIndexBuffer(IndexBuffer::TopologicalType topologicalType, ElementType elementType, uint32 elementCount, GraphicsBufferSP const& buffer);
		IndexBufferSP CreateIndexBuffer(IndexBuffer::TopologicalType topologicalType, ElementType elementType, uint32 elementCount);
		template <typename T>
		IndexBufferSP CreateIndexBuffer(GraphicsBuffer::Usage usage, std::vector<T> const& data, IndexBuffer::TopologicalType topologicalType)
		{
			GraphicsBufferSP buffer = CreateGraphicsBuffer(usage, data.data(), data.size() * sizeof(T));
			return CreateIndexBuffer(topologicalType, TypeToElementType<T>::Type, data.size(), std::move(buffer));
		}

		RenderingLayoutSP CreateRenderingLayout(std::vector<VertexBufferSP> const& buffers, IndexBufferSP const& indexBuffer);
		LayoutAndProgramConnectorSP CreateLayoutAndProgramConnector(RenderingLayoutSP const& layout, ProgramObjectSP const& program);

		SamplerSP CreateSampler(SamplerState const& samplerState);

		Texture1DSP CreateTexture1D(Texture::DataDescription<1> const& description, bool generateMipmap);
		Texture1DSP CreateTexture1D(Texture::DataDescription<1> const& description, std::vector<void const*> const& data, bool generateMipmap);
		template <typename T>
		Texture1DSP CreateTexture1D(Texture::DataDescription<1> const& description, std::vector<std::vector<T>> const& data, bool generateMipmap)
		{
			std::vector<void const*> rawData(data.size());
			for (uint32 i = 0; i < data.size(); ++i)
			{
				rawData[i] = data[i].data();
			}
			return CreateTexture1D(description, rawData, generateMipmap);
		}
		Texture2DSP CreateTexture2D(Texture::DataDescription<2> const& description, bool generateMipmap);
		Texture2DSP CreateTexture2D(Texture::DataDescription<2> const& description, std::vector<void const*> const& data, bool generateMipmap);
		template <typename T>
		Texture2DSP CreateTexture2D(Texture::DataDescription<2> const& description, std::vector<std::vector<T>> const& data, bool generateMipmap)
		{
			std::vector<void const*> rawData(data.size());
			for (uint32 i = 0; i < data.size(); ++i)
			{
				rawData[i] = data[i].data();
			}
			return CreateTexture2D(description, rawData, generateMipmap);
		}
		Texture3DSP CreateTexture3D(Texture::DataDescription<3> const& description, bool generateMipmap);
		Texture3DSP CreateTexture3D(Texture::DataDescription<3> const& description, std::vector<void const*> const& data, bool generateMipmap);
		template <typename T>
		Texture3DSP CreateTexture3D(Texture::DataDescription<3> const& description, std::vector<std::vector<T>> const& data, bool generateMipmap)
		{
			std::vector<void const*> rawData(data.size());
			for (uint32 i = 0; i < data.size(); ++i)
			{
				rawData[i] = data[i].data();
			}
			return CreateTexture3D(description, rawData, generateMipmap);
		}
		TextureCubeSP CreateTextureCube(Texture::DataDescription<2> const& description, bool generateMipmap);
		TextureCubeSP CreateTextureCube(Texture::DataDescription<2> const& description, std::array<std::vector<void const*>, 6> const& data, bool generateMipmap);
		template <typename T>
		TextureCubeSP CreateTextureCube(Texture::DataDescription<2> const& description, std::array<std::vector<std::vector<T>>, 6> const& data, bool generateMipmap)
		{
			return CreateTextureCube(description, data, generateMipmap);
		}
		TextureBufferSP CreateTextureBuffer(GraphicsBufferSP const& textureBuffer, TexelFormat format);

		FrameBufferSP CreateFrameBuffer(FrameBufferLayoutDescriptionSP description, std::unordered_map<std::string, Texture2DImageSP const>&& colorTextures, FrameBuffer::DepthStencilBinding const& depthStencil);

		ViewportSP CreateViewport(int32 depthOrder, int32 left, int32 bottom, uint32 width, uint32 height);
		ViewportSP CreateViewport(int32 depthOrder, float left, float bottom, float width, float height);

		LayoutAndProgramConnectorSP GetConnector(RenderingLayoutSP const& layout, RenderingTechniqueSP const& technique);

	private:
		std::string glslVersionString_;
		ViewportSP defaultViewport_;
		TextureSP blackTexture1D_;
		TextureSP blackTexture2D_;
		TextureSP blackTexture3D_;
		TextureSP blackTextureCube_;
		SamplerSP defaultSampler_;
		std::unique_ptr<RenderingEngine> renderingEngine_;

		std::unordered_map<std::pair<RenderingLayoutSP, RenderingTechniqueSP>, LayoutAndProgramConnectorSP, STLPairHasher<RenderingLayoutSP, RenderingTechniqueSP>> connectors_;
	};

}
