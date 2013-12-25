#pragma once

#include "Declare.hpp"
#include "Resource/LoadingResult.hpp"

namespace XREX
{
	class XREX_API TechniqueLoader
	{
	public:
		TechniqueLoader();
		~TechniqueLoader();
		TechniqueLoadingResultSP LoadTechnique(std::string const& fullPath, std::vector<std::pair<std::string, std::string>> macros);
		FrameBufferLoadingResultSP LoadFrameBuffer(std::string const& fullPath);


	private:
		struct CacheDetail;
		std::unique_ptr<CacheDetail> cache_;
	};

}

