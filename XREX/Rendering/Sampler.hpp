#pragma once
#include "Declare.hpp"

#include "Rendering/RenderingPipelineState.hpp"

namespace XREX
{

	struct XREX_API SamplerState
		: public RenderingPipelineState
	{

		enum class TextureAddressingMode
		{
			Repeat,
			MirroredRepeat,
			ClampToEdge,
			ClampToBorder,
		};

		enum class TextureFilterMode
		{
			Nearest,
			Linear,
			NearestMipmapNearest,
			LinearMipmapNearest,
			NearestMipmapLinear,
			LinearMipmapLinear,
			Anisotropic,
		};

		Color borderColor;
		TextureAddressingMode addressingModeS;
		TextureAddressingMode addressingModeT;
		TextureAddressingMode addressingModeR;
		TextureFilterMode minFilterMode;
		TextureFilterMode magFilterMode;
		uint8 maxAnisotropy;
		float minLOD;
		float maxLOD;
		float mipmapLODBias;
		bool compareEnable;
		CompareFunction compareFunction;

		SamplerState();
	};

	class XREX_API Sampler
		: Noncopyable
	{
	public:
		Sampler(SamplerState const& state);
		virtual ~Sampler();

		void Bind(uint32 textureChannel);
	private:
		SamplerState state_;

		uint32 glAddressingModeS_;
		uint32 glAddressingModeT_;
		uint32 glAddressingModeR_;
		uint32 glMinFilter_;
		uint32 glMagFilter_;

		uint32 glCompareFunction_;

		uint32 glSamplerID_;
	};

}


