#pragma once

#include "Declare.hpp"

namespace XREX
{
	template <typename Type>
	class LoadingResult
		: Noncopyable
	{
	public:

		virtual ~LoadingResult()
		{
		}

		virtual bool Succeeded() const = 0;

		/*
		 *	Create object from data.
		 *	Cache the texture when first called.
		 *	@return: will be nullptr if loading failed.
		 */
		virtual std::shared_ptr<Type> Create() = 0;
		/*
		 *	Create object from data.
		 *	Cache the texture when first called.
		 *	@return: will be nullptr if loading failed.
		 */
		std::shared_ptr<Type> operator ()()
		{
			return Create();
		}
	};

	typedef LoadingResult<Texture> TextureLoadingResult;
	typedef std::shared_ptr<TextureLoadingResult> TextureLoadingResultSP;

	typedef LoadingResult<Mesh> MeshLoadingResult;
	typedef std::shared_ptr<MeshLoadingResult> MeshLoadingResultSP;

	typedef LoadingResult<RenderingTechnique> TechniqueLoadingResult;
	typedef std::shared_ptr<TechniqueLoadingResult> TechniqueLoadingResultSP;

	typedef LoadingResult<FrameBuffer> FrameBufferLoadingResult;
	typedef std::shared_ptr<FrameBufferLoadingResult> FrameBufferLoadingResultSP;

}
