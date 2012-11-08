#include "XREX.hpp"

#include "RenderingFactory.hpp"

#include "RenderingEngine.hpp"

namespace XREX
{

	RenderingFactory::RenderingFactory()
	{
	}


	RenderingFactory::~RenderingFactory()
	{
	}


	void RenderingFactory::Initialize()
	{
		renderingEngine_ = MakeUP<RenderingEngine>();

		std::array<uint32, 1> size1;
		size1[0] = 1;
		Texture::DataDescription<1> description1(Texture::TexelFormat::RGBA8, size1);
		std::vector<std::vector<uint32>> data1(1, std::vector<uint32>(1, 0));
		blackTexture1D_ = MakeSP<Texture1D>(description1, data1, true);

		std::array<uint32, 2> size2;
		size2[0] = 1;
		size2[1] = 1;
		Texture::DataDescription<2> description2(Texture::TexelFormat::RGBA8, size2);
		std::vector<std::vector<uint32>> data2(1, std::vector<uint32>(1, 0));
		blackTexture2D_ = MakeSP<Texture2D>(description2, data2, true);

		std::array<uint32, 3> size3;
		size3[0] = 1;
		size3[1] = 1;
		size3[2] = 1;
		Texture::DataDescription<3> description3(Texture::TexelFormat::RGBA8, size3);
		std::vector<std::vector<uint32>> data3(1, std::vector<uint32>(1, 0));
		blackTexture3D_ = MakeSP<Texture3D>(description3, data3, true);

		// TODO blackTextureCube
	}

	RenderingEngine& RenderingFactory::GetRenderingEngine()
	{
		return *renderingEngine_;
	}

}
