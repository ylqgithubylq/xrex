#pragma once

#include "Declare.hpp"

#include "Shader.hpp"
#include "GraphicsBuffer.hpp"
#include "RenderingLayout.hpp"
#include "Texture.hpp"
#include "RenderingPipelineState.hpp"

class RenderingFactory
	: Noncopyable
{
public:
	RenderingFactory();
	virtual ~RenderingFactory();

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
	GraphicsBufferSP CreateGraphicsVertexBuffer(GraphicsBuffer::Usage usage, std::vector<T> const& data)
	{
		return MakeSP<GraphicsBuffer>(GraphicsBuffer::BufferType::Vertex, usage, data);
	}
	template <typename T>
	GraphicsBufferSP CreateGraphicsVertexBuffer(GraphicsBuffer::Usage usage, std::vector<T> const& data, std::string const& channel)
	{
		return MakeSP<GraphicsBuffer>(GraphicsBuffer::BufferType::Vertex, usage, data, channel);
	}
	template <typename T>
	GraphicsBufferSP CreateGraphicsVertexBuffer(GraphicsBuffer::Usage usage, std::vector<T> const& data, std::string const& channel, bool normalized)
	{
		return MakeSP<GraphicsBuffer>(GraphicsBuffer::BufferType::Vertex, usage, data, channel, normalized);
	}
	template <typename T>
	GraphicsBufferSP CreateGraphicsVertexBuffer(GraphicsBuffer::Usage usage, std::vector<T> const& data, GraphicsBuffer::DataDescription&& description)
	{
		return MakeSP<GraphicsBuffer>(GraphicsBuffer::BufferType::Vertex, usage, data, std::move(description));
	}
	template <typename T>
	GraphicsBufferSP CreateGraphicsIndexBuffer(GraphicsBuffer::Usage usage, std::vector<T> const& data)
	{
		return MakeSP<GraphicsBuffer>(GraphicsBuffer::BufferType::Index, usage, data);
	}
	RenderingLayoutSP CreateRenderingLayout(std::vector<GraphicsBufferSP> const& buffers, GraphicsBufferSP const& indexBuffer, RenderingLayout::DrawingMode mode)
	{
		return MakeSP<RenderingLayout>(buffers, indexBuffer, mode);
	}

	SamplerStateObjectSP CreateSamplerStateObject(SamplerState const& samplerState)
	{
		return MakeSP<SamplerStateObject>(samplerState);
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
};

