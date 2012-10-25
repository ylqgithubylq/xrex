#pragma once

#include "Declare.hpp"




struct RenderingPipelineState
{
	enum class PolygonMode
	{
		Point,
		Line,
		Fill,
	};

	enum class CullMode
	{
		None,
		Front,
		Back,
	};

	enum class BlendOperation
	{
		Add,
		Sub,
		ReverseSub,
		Min,
		Max,
	};

	enum class AlphaBlendFactor
	{
		Zero,
		One,
		SourceAlpha,
		DestinationAlpha,
		OneMinusSourceAlpha,
		OneMinusDestinationAlpha,
		SourceColor,
		DestinationColor,
		OneMinusSourceColor,
		OneMinusDestinationColor,
		SourceAlphaSaturate,
	};

	enum class CompareFunction
	{
		AlwaysFail,
		AlwaysPass,
		Less,
		LessEqual,
		Equal,
		NotEqual,
		Greater,
		GreaterEqual,
	};

	enum class StencilOperation
	{
		Keep,
		Zero,
		Replace,
		Increase,
		Decrease,
		Invert,
		IncreaseWarp,
		DecreaseWarp,
	};

	enum class TextureAddressingMode
	{
		Wrap,
		Mirror,
		Clamp,
		Border,
	};

	enum class TextureFilterOperation
	{
		Temp,
	};
};

struct RasterizerState
	: public RenderingPipelineState
{
	PolygonMode polygonMode;
	CullMode cullMode;
	float polygonOffsetFactor;
	float polygonOffsetUnits;
	bool frontFaceCCW;
	bool scissorEnable;
	bool multisampleEnable;

	RasterizerState();
};

struct DepthStencilState
	: public RenderingPipelineState
{
	bool depthEnable;
	bool depthWriteMask;
	CompareFunction depthFunction;

	bool stencilEnable;

	CompareFunction frontStencilFunction;
	uint16 frontStencilReadMask;
	uint16 frontStencilWriteMask;
	StencilOperation frontStencilFail;
	StencilOperation frontStencilDepthFail;
	StencilOperation frontStencilPass;

	CompareFunction backStencilFunction;
	uint16 backStencilReadMask;
	uint16 backStencilWriteMask;
	StencilOperation backStencilFail;
	StencilOperation backStencilDepthFail;
	StencilOperation backStencilPass;

	DepthStencilState();
};

struct BlendState
	: public RenderingPipelineState
{
	bool alphaToCoverageEnable;

	bool blendEnable;
	BlendOperation blendOperation;
	AlphaBlendFactor sourceBlend;
	AlphaBlendFactor destinationBlend;
	BlendOperation blendOperationAlpha;
	AlphaBlendFactor sourceBlendAlpha;
	AlphaBlendFactor destinationBlendAlpha;
	bool redMask;
	bool greenMask;
	bool blueMask;
	bool alphaMask;

	BlendState();
};

struct SamplerState
	: public RenderingPipelineState
{
	Color borderColor;
	TextureAddressingMode addressingModeU;
	TextureAddressingMode addressingModeV;
	TextureAddressingMode addressingModeW;
	TextureFilterOperation filterOperation;
	uint8 maxAnisotropy;
	float minLOD;
	float maxLOD;
	float mipmapLODBias;
	CompareFunction compareFunction;

	SamplerState();
};



class RasterizerStateObject
	: Noncopyable
{
public:
	explicit RasterizerStateObject(RasterizerState const& state);
	void Bind();
private:
	RasterizerState state_;

	uint32 glPolygonMode_;
	uint32 glFrontFace_;
	uint32 glCullFace_;
	bool glCullFaceEnable_;
};


class DepthStencilStateObject
	: Noncopyable
{
public:
	explicit DepthStencilStateObject(DepthStencilState const& state);
	void Bind(uint16 frontStencilReference, uint16 backStencilReference);
private:
	DepthStencilState state_;

	uint32 glDepthFunction_;
	uint32 glFrontStencilFunction_;
	uint32 glFrontStencilFail_;
	uint32 glFrontStencilDepthFail_;
	uint32 glFrontStencilPass_;
	uint32 glBackStencilFunction_;
	uint32 glBackStencilFail_;
	uint32 glBackStencilDepthFail_;
	uint32 glBackStencilPass_;
};


class BlendStateObject
	: Noncopyable
{
public:
	explicit BlendStateObject(BlendState const& state);
	void Bind(Color const& blendFactor);
private:
	BlendState state_;

	uint32 glBlendOperation_;
	uint32 glBlendOperationAlpha_;
	uint32 glSourceBlend_;
	uint32 glDestinationBlend_;
	uint32 glSourceBlendAlpha_;
	uint32 glDestinationBlendAlpha_;
};


class SamplerStateObject
	: Noncopyable
{
public:
	explicit SamplerStateObject(SamplerState const& state);
	void Bind(); // TODO parameter?
private:
	SamplerState state_;
};
