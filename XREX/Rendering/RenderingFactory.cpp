#include "XREX.hpp"

#include "RenderingFactory.hpp"

#include "Base/Window.hpp"
#include "Base/XREXContext.hpp"
#include "Rendering/RenderingEngine.hpp"
#include "Rendering/GraphicsContext.hpp"
#include "Rendering/RenderingTechnique.hpp"
#include "Rendering/Viewport.hpp"
#include "Rendering/RenderingLayout.hpp"
#include "Rendering/ProgramConnector.hpp"

namespace XREX
{
	RenderingFactory::RenderingFactory(Window& window, Settings const& settings)
	{
		renderingEngine_ = MakeUP<RenderingEngine>(window, settings);
		GraphicsContext& graphicsContext = renderingEngine_->GetGraphicsContext();
		
		glslVersionString_ = "#version ";
		std::string version = "000";
		assert(graphicsContext.GetMajorVersion() < 10);
		assert(graphicsContext.GetMinorVersion() < 10);
		version[0] = '0' + graphicsContext.GetMajorVersion(); // int to one byte char
		version[1] = '0' + graphicsContext.GetMinorVersion(); // int to one byte char
		glslVersionString_ += version + "\n\n";

		auto depthOrder = std::numeric_limits<decltype(std::declval<Viewport>().GetDepthOrder())>::max();
		defaultViewport_ = CreateViewport(depthOrder, 0.f, 0.f, 1.f, 1.f); // float parameters to use relative mode

		Size<uint32, 1> size1(1);
		Texture::DataDescription<1> description1(TexelFormat::RGBA8, size1);
		std::vector<std::vector<uint32>> data1(1, std::vector<uint32>(1, std::numeric_limits<uint8>::max() << 24)); // ARGB
		blackTexture1D_ = CreateTexture1D(description1, data1, true);

		Size<uint32, 2> size2(1, 1);
		Texture::DataDescription<2> description2(TexelFormat::RGBA8, size2);
		std::vector<std::vector<uint32>> data2(1, std::vector<uint32>(1, std::numeric_limits<uint8>::max() << 24)); // ARGB
		blackTexture2D_ = CreateTexture2D(description2, data2, true);

		Size<uint32, 3> size3(1, 1, 1);
		Texture::DataDescription<3> description3(TexelFormat::RGBA8, size3);
		std::vector<std::vector<uint32>> data3(1, std::vector<uint32>(1, std::numeric_limits<uint8>::max() << 24)); // ARGB
		blackTexture3D_ = CreateTexture3D(description3, data3, true);

		// TODO blackTextureCube

		SamplerState samplerState;
		defaultSampler_ = CreateSampler(samplerState);

	}


	RenderingFactory::~RenderingFactory()
	{
	}

	LayoutAndProgramConnectorSP RenderingFactory::GetConnector(RenderingLayoutSP const& layout, RenderingTechniqueSP const& technique)
	{
		// TODO use weak_ptr, but weak_ptr cannot be hashed
		assert(layout);
		assert(technique);
		auto toFind = std::make_pair(layout, technique);
		auto found = connectors_.find(toFind);
		if (found != connectors_.end())
		{
			return found->second;
		}

		LayoutAndProgramConnectorSP connector = CreateLayoutAndProgramConnector(layout, technique->GetProgram());
		connectors_[toFind] = connector;
		return connector;
	}

	RasterizerStateObjectSP RenderingFactory::CreateRasterizerStateObject(RasterizerState const& rasterizerState)
	{
		return MakeSP<RasterizerStateObject>(rasterizerState);
	}

	DepthStencilStateObjectSP RenderingFactory::CreateDepthStencilStateObject(DepthStencilState const& depthStencilState)
	{
		return MakeSP<DepthStencilStateObject>(depthStencilState);
	}

	BlendStateObjectSP RenderingFactory::CreateBlendStateObject(BlendState const& blendState)
	{
		return MakeSP<BlendStateObject>(blendState);
	}

	ShaderObjectSP RenderingFactory::CreateShaderObject(ShaderObject::ShaderType type)
	{
		return MakeSP<ShaderObject>(type);
	}

	ProgramObjectSP RenderingFactory::CreateProgramObject()
	{
		return MakeSP<ProgramObject>();
	}

	GraphicsBufferSP RenderingFactory::CreateGraphicsBuffer(GraphicsBuffer::Usage usage, uint32 sizeInBytes)
	{
		return MakeSP<GraphicsBuffer>(usage, sizeInBytes);
	}

	GraphicsBufferSP RenderingFactory::CreateGraphicsBuffer(GraphicsBuffer::Usage usage, void const* data, uint32 sizeInBytes)
	{
		return MakeSP<GraphicsBuffer>(usage, data, sizeInBytes);
	}

	GraphicsBufferSP RenderingFactory::CreateGraphicsBuffer(GraphicsBuffer::Usage usage, uint32 sizeInBytes, BufferView::BufferType typeHint)
	{
		return MakeSP<GraphicsBuffer>(usage, sizeInBytes, typeHint);
	}

	GraphicsBufferSP RenderingFactory::CreateGraphicsBuffer(GraphicsBuffer::Usage usage, void const* data, uint32 sizeInBytes, BufferView::BufferType typeHint)
	{
		return MakeSP<GraphicsBuffer>(usage, data, sizeInBytes, typeHint);
	}

	GraphicsBufferSP RenderingFactory::CreateGraphicsBufferWithBufferInformation(GraphicsBuffer::Usage usage, BufferBindingInformation const& information)
	{
		return MakeSP<GraphicsBuffer>(usage, information.GetDataSize(), information.GetBufferType());
	}

	ShaderResourceBufferSP RenderingFactory::CreateShaderResourceBuffer(BufferBindingInformation const& information, bool createBuffer)
	{
		if (createBuffer)
		{
			GraphicsBufferSP buffer = CreateGraphicsBufferWithBufferInformation(GraphicsBuffer::Usage::DynamicDraw, information);
			return MakeSP<ShaderResourceBuffer>(information, std::move(buffer));
		}
		else
		{
			return MakeSP<ShaderResourceBuffer>(information);
		}
	}

	VertexBufferSP RenderingFactory::CreateVertexBuffer(VertexBuffer::DataLayoutDescription&& description)
	{
		return MakeSP<VertexBuffer>(std::move(description));
	}

	VertexBufferSP RenderingFactory::CreateVertexBuffer(VertexBuffer::DataLayoutDescription&& description, GraphicsBufferSP const& buffer)
	{
		return MakeSP<VertexBuffer>(std::move(description), buffer);
	}

	VertexBufferSP RenderingFactory::CreateVertexBuffer(GraphicsBuffer::Usage usage, uint32 elementSizeInBytes, VertexBuffer::DataLayoutDescription&& description)
	{
		GraphicsBufferSP buffer = CreateGraphicsBuffer(usage, elementSizeInBytes);
		return CreateVertexBuffer(std::move(description), std::move(buffer));
	}

	IndexBufferSP RenderingFactory::CreateIndexBuffer(IndexBuffer::TopologicalType topologicalType, ElementType elementType, uint32 elementCount, GraphicsBufferSP const& buffer)
	{
		return MakeSP<IndexBuffer>(topologicalType, elementType, elementCount, buffer);
	}

	IndexBufferSP RenderingFactory::CreateIndexBuffer(IndexBuffer::TopologicalType topologicalType, ElementType elementType, uint32 elementCount)
	{
		return MakeSP<IndexBuffer>(topologicalType, elementType, elementCount);
	}

	RenderingLayoutSP RenderingFactory::CreateRenderingLayout(std::vector<VertexBufferSP> const& buffers, IndexBufferSP const& indexBuffer)
	{
		return MakeSP<RenderingLayout>(buffers, indexBuffer);
	}

	LayoutAndProgramConnectorSP RenderingFactory::CreateLayoutAndProgramConnector(RenderingLayoutSP const& layout, ProgramObjectSP const& program)
	{
		return MakeSP<LayoutAndProgramConnector>(layout, program);
	}

	SamplerSP RenderingFactory::CreateSampler(SamplerState const& samplerState)
	{
		return MakeSP<Sampler>(samplerState);
	}

	Texture1DSP RenderingFactory::CreateTexture1D(Texture::DataDescription<1> const& description, bool generateMipmap)
	{
		return MakeSP<Texture1D>(description, generateMipmap);
	}

	Texture1DSP RenderingFactory::CreateTexture1D(Texture::DataDescription<1> const& description, std::vector<void const*> const& data, bool generateMipmap)
	{
		return MakeSP<Texture1D>(description, data, generateMipmap);
	}

	Texture2DSP RenderingFactory::CreateTexture2D(Texture::DataDescription<2> const& description, bool generateMipmap)
	{
		return MakeSP<Texture2D>(description, generateMipmap);
	}

	Texture2DSP RenderingFactory::CreateTexture2D(Texture::DataDescription<2> const& description, std::vector<void const*> const& data, bool generateMipmap)
	{
		return MakeSP<Texture2D>(description, data, generateMipmap);
	}

	Texture3DSP RenderingFactory::CreateTexture3D(Texture::DataDescription<3> const& description, bool generateMipmap)
	{
		return MakeSP<Texture3D>(description, generateMipmap);
	}

	Texture3DSP RenderingFactory::CreateTexture3D(Texture::DataDescription<3> const& description, std::vector<void const*> const& data, bool generateMipmap)
	{
		return MakeSP<Texture3D>(description, data, generateMipmap);
	}

	TextureCubeSP RenderingFactory::CreateTextureCube(Texture::DataDescription<2> const& description, bool generateMipmap)
	{
		assert(false);
		return nullptr;
	}
	TextureCubeSP RenderingFactory::CreateTextureCube(Texture::DataDescription<2> const& description, std::array<std::vector<void const*>, 6> const& data, bool generateMipmap)
	{
		assert(false);
		return nullptr;
	}

	TextureBufferSP RenderingFactory::CreateTextureBuffer(GraphicsBufferSP const& textureBuffer, TexelFormat format)
	{
		return MakeSP<TextureBuffer>(textureBuffer, format);
	}

	FrameBufferSP RenderingFactory::CreateFrameBuffer(FrameBufferLayoutDescriptionSP description, std::unordered_map<std::string, Texture2DImageSP const>&& colorTextures, FrameBuffer::DepthStencilBinding const& depthStencil)
	{
		return MakeSP<FrameBuffer>(std::move(description), std::move(colorTextures), depthStencil);
	}

	ViewportSP RenderingFactory::CreateViewport(int32 depthOrder, int32 left, int32 bottom, uint32 width, uint32 height)
	{
		return MakeSP<Viewport>(depthOrder, left, bottom, width, height);
	}

	ViewportSP RenderingFactory::CreateViewport(int32 depthOrder, float left, float bottom, float width, float height)
	{
		return MakeSP<Viewport>(depthOrder, left, bottom, width, height);
	}



}
