#pragma once

#include "Declare.hpp"

#include "Rendering/ShaderProgram.hpp"

namespace XREX
{
	
	
	/*
	 *	Only used as annotation.
	 */
	struct NUpdateFrequency
	{
	};
	struct NUpdatePerFrame
		: NUpdateFrequency
	{
	};
	struct NUpdatePerObject
		: NUpdateFrequency
	{
	};

	// TODO how to make a template counter? Counter<class T> {uint32 Value;}; Counter<A>::Value == 0; Counter<B>::Value == 1; ...


	struct XREX_API IComponentParameterSetter
	{
		virtual ~IComponentParameterSetter() = 0
		{
		};
	};

	struct XREX_API ComponentParameterSetterBase
		: IComponentParameterSetter
	{
		ComponentParameterSetterBase(RenderingTechniqueSP technique)
			: technique_(std::move(technique))
		{
			assert(technique_ != nullptr);
		}
		RenderingTechniqueSP const& GetTechnique() const
		{
			return technique_;
		}
	private:
		RenderingTechniqueSP technique_;
	};

	template <typename ComponentType>
	struct ComponentParameterSetter
		: ComponentParameterSetterBase
	{
		ComponentParameterSetter(RenderingTechniqueSP technique)
			: ComponentParameterSetterBase(std::move(technique))
		{
		}
		virtual void SetParameter(std::shared_ptr<ComponentType> const& component) = 0;
	};

	struct XREX_API ISystemTechniqueFactory
	{
		virtual ~ISystemTechniqueFactory() = 0
		{
		}
		virtual std::string const& GetIndexName() const = 0;
		/*
		 *	Notice: Every call should return the same instance.
		 */
		virtual TechniqueBuildingInformationSP const& GetTechniqueInformationToInclude() const = 0;
	};




	template <typename RequiredComponentType>
	struct IParameterSetterDepend
	{
		virtual void Connect(std::shared_ptr<RequiredComponentType> const& component) = 0;
	};






	struct XREX_API TransformationTechniqueFactory
		: ISystemTechniqueFactory
	{
		virtual std::string const& GetIndexName() const override
		{
			static std::string const IndexName = "Transformation";
			return IndexName;
		}
		virtual TechniqueBuildingInformationSP const& GetTechniqueInformationToInclude() const override;
	};

	struct XREX_API TransformationSetter
		: ComponentParameterSetter<Transformation>, IParameterSetterDepend<Camera>, NUpdatePerObject
	{
		explicit TransformationSetter(RenderingTechniqueSP technique);

		virtual void Connect(CameraSP const& component) override;

		virtual void SetParameter(TransformationSP const& component) override;

	private:
		CameraSP camera_;
		TechniqueParameterSP modelParameter_;
		ShaderResourceBufferSP parameterBuffer_;

		ShaderResourceBuffer::VariableSetter worldFromModel_;
		ShaderResourceBuffer::VariableSetter worldFromModelNormal_;
		ShaderResourceBuffer::VariableSetter viewFromModel_;
		ShaderResourceBuffer::VariableSetter clipFromModel_;
	};




	struct XREX_API CameraTechniqueFactory
		: ISystemTechniqueFactory
	{
		virtual std::string const& GetIndexName() const override
		{
			static std::string const IndexName = "Camera";
			return IndexName;
		}
		virtual TechniqueBuildingInformationSP const& GetTechniqueInformationToInclude() const override;
	};

	struct XREX_API CameraSetter
		: ComponentParameterSetter<Camera>, NUpdatePerFrame
	{
		explicit CameraSetter(RenderingTechniqueSP technique);

		virtual void SetParameter(CameraSP const& component) override;

	private:
		TechniqueParameterSP cameraParameter_;
		ShaderResourceBufferSP parameterBuffer_;

		ShaderResourceBuffer::VariableSetter viewFromeWorld_;
		ShaderResourceBuffer::VariableSetter clipFromView_;
		ShaderResourceBuffer::VariableSetter clipFromWorld_;
		ShaderResourceBuffer::VariableSetter cameraPositionInWorld_;
	};
}

