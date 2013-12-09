#pragma once

#include "Declare.hpp"


namespace XREX
{

	struct XREX_API RenderingPipelineState
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

	};

	struct XREX_API RasterizerState
		: public RenderingPipelineState
	{
		PolygonMode polygonMode;
		CullMode cullMode;
		bool frontFaceCCW;
		bool multisampleEnable;

		RasterizerState();
	};

	struct XREX_API DepthStencilState
		: public RenderingPipelineState
	{
		bool depthTestEnable;
		bool depthWriteMask;
		CompareFunction depthFunction;

		bool stencilTestEnable;

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

		bool IsDepthReadEnabled() const;
		bool IsDepthWriteEnabled() const;
		bool IsStencilReadEnabled() const;
		bool IsStencilWriteEnabled() const;
	};

	struct XREX_API BlendState
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



	class XREX_API RasterizerStateObject
		: Noncopyable
	{
	public:
		explicit RasterizerStateObject(RasterizerState const& state);
		void Bind(float polygonOffsetFactor, float polygonOffsetUnits);

		RasterizerState const& GetState() const
		{
			return state_;
		}
	private:
		RasterizerState state_;

		uint32 glPolygonMode_;
		uint32 glFrontFace_;
		uint32 glCullFace_;
		bool glCullFaceEnable_;
	};


	class XREX_API DepthStencilStateObject
		: Noncopyable
	{
	public:
		explicit DepthStencilStateObject(DepthStencilState const& state);
		void Bind(uint16 frontStencilReference, uint16 backStencilReference);

		DepthStencilState const& GetState() const
		{
			return state_;
		}
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


	class XREX_API BlendStateObject
		: Noncopyable
	{
	public:
		explicit BlendStateObject(BlendState const& state);
		void Bind(Color const& blendFactor);

		BlendState const& GetState() const
		{
			return state_;
		}
	private:
		BlendState state_;

		uint32 glBlendOperation_;
		uint32 glBlendOperationAlpha_;
		uint32 glSourceBlend_;
		uint32 glDestinationBlend_;
		uint32 glSourceBlendAlpha_;
		uint32 glDestinationBlendAlpha_;
	};



}
