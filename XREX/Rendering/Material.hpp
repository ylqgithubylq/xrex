#pragma once

#include "Declare.hpp"

#include "Rendering/RenderingTechnique.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <type_traits>

namespace XREX
{

	class XREX_API Material
		: Noncopyable
	{

	public:
		Material(std::string const& name);
		~Material();

		std::string const& GetName() const
		{
			return name_;
		}

	private:
		template <typename T>
		struct ResolveType
		{
			typedef T Type;
		};
		template <typename T>
		struct ResolveType<std::shared_ptr<T>>
		{
			// if shared_ptr<T> is shared_ptr</*some TextureImage type*/> then Type is TextureImageSP,
			// if shared_ptr<T> is shared_ptr</*some Texture type*/> then Type is TextureSP,
			// else Type is std::shared_ptr<T>.
			typedef std::shared_ptr<T> OriginalType;
			typedef
				typename std::conditional<std::is_convertible<OriginalType, TextureSP>::value,
					TextureSP,
					typename std::conditional<std::is_convertible<OriginalType, TextureImageSP>::value,
						TextureImageSP,
						OriginalType
					>::type
				>::type Type;
		};
		template <typename T>
		TechniqueParameterSP MakeParameter(std::string const& parameterName)
		{
			// if T is shared_ptr</*some TextureImage type*/> then create SimpleImageParameter,
			// if T is shared_ptr</*some Texture type*/> then create SimpleTextureParameter,
			// else create ConcreteTechniqueParameter<T>
			return MakeSP<
				typename std::conditional<std::is_convertible<T, TextureSP>::value,
					SimpleTextureParameter,
					typename std::conditional<std::is_convertible<T, TextureImageSP>::value,
						SimpleImageParameter,
						ConcreteTechniqueParameter<T>
					>::type
				>::type>(parameterName);
		}

	public:
		template <typename T>
		void SetParameter(std::string const& parameterName, T const& value)
		{
			auto found = parameters_.find(parameterName);
			if (found == parameters_.end())
			{
				TechniqueParameterSP parameter = MakeParameter<T>(parameterName);
				parameter->As<ResolveType<T>::Type>().SetValue(value);
				parameters_[parameterName] = std::move(parameter);
				cacheDirty_ = true;
			}
			else
			{
				found->second->As<ResolveType<T>::Type>().SetValue(value);
			}
		}


		TechniqueParameterSP const& GetParameter(std::string const& parameterName);


		void SetPolygonOffset(float factor, float units);
		/*
		 *	@return: tuple<(use default), (factor), (units)>
		 */
		std::tuple<bool, float, float> GetPolygonOffset() const;
		void RemovePolygonOffset();

		void SetStencilReference(uint16 front, uint16 back);
		/*
		 *	@return: tuple<(use default), (front), (back)>
		 */
		std::tuple<bool, uint16, uint16> GetStencilReference() const;
		void RemoveStencilReference();

		void SetBlendFactor(Color value);
		/*
		 *	@return: tuple<(success), (factor)>
		 */
		std::tuple<bool, Color> GetBlendFactor() const;
		void RemoveBlendFactor();


		void BindToTechnique(RenderingTechniqueSP const& technique);

		void SetAllTechniqueParameterValues();

	private:
		void UpdateBindingMapping();

	private:
		// when use default values, material will not set corresponding parameters.
		struct TechniquePipelineParameterSettings
		{
			TechniquePipelineParameters parameters;
			bool useDefaultPolygonOffset;
			bool useDefaultStencilReference;
			bool useDefaultBlendFactor;
			TechniquePipelineParameterSettings();
		};
	private:
		std::string name_;

		std::unordered_map<std::string, TechniqueParameterSP> parameters_;
		TechniquePipelineParameterSettings pipelineParameter_;

		std::weak_ptr<RenderingTechnique> boundTechnique_;
		std::vector<std::pair<TechniqueParameterSP, std::weak_ptr<TechniqueParameter>>> parameterMappingCache_;
		
		bool cacheDirty_;
	};

}
