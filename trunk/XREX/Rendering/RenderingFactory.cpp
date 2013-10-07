#include "XREX.hpp"

#include "RenderingFactory.hpp"

#include "Base/Window.hpp"
#include "Base/XREXContext.hpp"
#include "Rendering/RenderingEngine.hpp"
#include "Rendering/GraphicsContext.hpp"
#include "Rendering/RenderingTechnique.hpp"

#include "Rendering/Viewport.hpp"

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
		defaultViewport_ = CreateViewport(depthOrder, 0, 0, window.GetClientRegionSize().x, window.GetClientRegionSize().y);

		std::array<uint32, 1> size1;
		size1[0] = 1;
		Texture::DataDescription<1> description1(TexelFormat::RGBA8, size1);
		std::vector<std::vector<uint32>> data1(1, std::vector<uint32>(1, std::numeric_limits<uint8>::max() << 24)); // ARGB
		blackTexture1D_ = CreateTexture1D(description1, data1, true);

		std::array<uint32, 2> size2;
		size2[0] = 1;
		size2[1] = 1;
		Texture::DataDescription<2> description2(TexelFormat::RGBA8, size2);
		std::vector<std::vector<uint32>> data2(1, std::vector<uint32>(1, std::numeric_limits<uint8>::max() << 24)); // ARGB
		blackTexture2D_ = CreateTexture2D(description2, data2, true);

		std::array<uint32, 3> size3;
		size3[0] = 1;
		size3[1] = 1;
		size3[2] = 1;
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

	BufferAndProgramConnectorSP RenderingFactory::GetConnector(RenderingLayoutSP const& layout, RenderingTechniqueSP const& technique)
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

		BufferAndProgramConnectorSP connector = CreateBufferAndProgramConnector(layout, technique->GetProgram());
		connectors_[toFind] = connector;
		return connector;
	}



}
