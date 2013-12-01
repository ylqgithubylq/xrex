#include "XREX.hpp"

#include "SystemTechnique.hpp"

#include "Base/XREXContext.hpp"
#include "Rendering/RenderingFactory.hpp"
#include "Rendering/TechniqueBuilder.hpp"
#include "Rendering/RenderingTechnique.hpp"

#include "Rendering/Camera.hpp"

namespace XREX
{

	TechniqueBuilderSP const& TransformationTechnique::GetTechniqueToInclude() const
	{
		static TechniqueBuilderSP const Builder = [] ()
		{
			std::string code =
				"\n"
				"uniform XREX_ModelTransformation\n"
				"{\n"
				"	mat4 WorldFromModel;\n"
				"	mat4 WorldFromModelNormal;\n"
				"\n"
				"	mat4 ViewFromModel;"
				"	mat4 ClipFromModel;"
				"} XREX_ModelTransformation;\n"
				"\n"
				"vec3 XREX_Transform(mat4 transformationMatrix, vec3 position)\n"
				"{\n"
				"	return (transformationMatrix * vec4(position, 1)).xyz;\n"
				"}\n"
				"\n"
				"vec3 XREX_TransformNormal(mat4 normalTransformationMatrix, vec3 normal)\n"
				"{\n"
				"	return (mat3(normalTransformationMatrix) * normal);\n"
				"}\n"
				"\n"
				"vec4 XREX_TransformToClip(mat4 clipTranformationMatrix, vec3 position)\n"
				"{\n"
				"	return (clipTranformationMatrix * vec4(position, 1));\n"
				"}\n"
				;
			TechniqueBuilderSP builder = MakeSP<TechniqueBuilder>("XREX_ModelTransformation");
			builder->AddCommonCode(MakeSP<std::string>(std::move(code)));

			std::vector<VariableInformation const> modelVariables;
			modelVariables.push_back(VariableInformation("WorldFromModel", ElementType::FloatM44, 0));
			modelVariables.push_back(VariableInformation("WorldFromModelNormal", ElementType::FloatM44, 0));
			modelVariables.push_back(VariableInformation("ViewFromModel", ElementType::FloatM44, 0));
			modelVariables.push_back(VariableInformation("ClipFromModel", ElementType::FloatM44, 0));
			builder->AddUniformBufferInformation(BufferInformation("XREX_ModelTransformation", BufferView::BufferType::Uniform, std::move(modelVariables)));

			return builder;
		} ();
		return Builder;
	}


	TransformationSetter::TransformationSetter(RenderingTechniqueSP const& technique)
		: technique_(technique)
	{
		assert(technique != nullptr);
		modelParameter_ = technique->GetParameterByName("XREX_ModelTransformation");
		parameterBuffer_ = modelParameter_->As<ShaderResourceBufferSP>().GetValue();
		GraphicsBufferSP buffer = XREXContext::GetInstance().GetRenderingFactory().CreateGraphicsBufferWithBufferInformation(
			GraphicsBuffer::Usage::DynamicDraw, parameterBuffer_->GetBufferInformation());
		parameterBuffer_->SetBuffer(buffer);

		auto worldFromModelResult = parameterBuffer_->GetSetter("XREX_ModelTransformation.WorldFromModel");
		assert(worldFromModelResult.first);
		worldFromModel_ = worldFromModelResult.second;
		auto worldFromModelNormalResult = parameterBuffer_->GetSetter("XREX_ModelTransformation.WorldFromModelNormal");
		assert(worldFromModelNormalResult.first);
		worldFromModelNormal_ = worldFromModelNormalResult.second;
		auto viewFromModelResult = parameterBuffer_->GetSetter("XREX_ModelTransformation.ViewFromModel");
		assert(viewFromModelResult.first);
		viewFromModel_ = viewFromModelResult.second;
		auto clipFromModelResult = parameterBuffer_->GetSetter("XREX_ModelTransformation.ClipFromModel");
		assert(clipFromModelResult.first);
		clipFromModel_ = clipFromModelResult.second;
	}

	void TransformationSetter::Connect(CameraSP const& camera)
	{
		assert(camera != nullptr);
		camera_ = camera;
	}


	void TransformationSetter::SetParameter(TransformationSP const& component)
	{
		assert(camera_ != nullptr);
		floatM44 viewMatrix = camera_->GetViewMatrix();
		floatM44 projectionMatrix = camera_->GetProjectionMatrix();

		ShaderResourceBuffer::BufferMapper mapper = parameterBuffer_->GetMapper();
		floatM44 modelMatrix = component->GetModelMatrix();
		floatM44 viewFromModelMatrix = viewMatrix * modelMatrix;
		floatM44 clipFromModelMatrix = projectionMatrix * viewFromModelMatrix;

		worldFromModel_.SetValue(mapper, modelMatrix);
		worldFromModelNormal_.SetValue(mapper, modelMatrix); // TODO should be normal matrix
		viewFromModel_.SetValue(mapper, viewFromModelMatrix);
		clipFromModel_.SetValue(mapper, clipFromModelMatrix);
	}




	TechniqueBuilderSP const& CameraTechnique::GetTechniqueToInclude() const
	{
		static TechniqueBuilderSP const Builder = [] ()
		{
			std::string code =
				"\n"
				"const vec3 XREX_CameraPositionInView = vec3(0, 0, 1);\n"
				"\n"
				"uniform XREX_CameraTransformation\n"
				"{\n"
				"	mat4 ViewFromWorld;\n"
				"	mat4 ClipFromView;\n"
				"	mat4 ClipFromWorld;\n"
				"	vec3 CameraPositionInWorld;\n"
				"} XREX_CameraTransformation;\n"
				"\n"
				;
			TechniqueBuilderSP builder = MakeSP<TechniqueBuilder>("XREX_CameraTransformation");
			builder->AddCommonCode(MakeSP<std::string>(std::move(code)));

			std::vector<VariableInformation const> cameraVariables;
			cameraVariables.push_back(VariableInformation("ViewFromWorld", ElementType::FloatM44, 0));
			cameraVariables.push_back(VariableInformation("ClipFromView", ElementType::FloatM44, 0));
			cameraVariables.push_back(VariableInformation("ClipFromWorld", ElementType::FloatM44, 0));
			cameraVariables.push_back(VariableInformation("CameraPositionInWorld", ElementType::FloatV3, 0));
			builder->AddUniformBufferInformation(BufferInformation("XREX_CameraPositionInView", BufferView::BufferType::Uniform, std::move(cameraVariables)));

			return builder;
		} ();
		return Builder;
	}


	CameraSetter::CameraSetter(RenderingTechniqueSP const& technique)
		: technique_(technique)
	{
		assert(technique_ != nullptr);
		cameraParameter_ = technique->GetParameterByName("XREX_CameraTransformation");
		parameterBuffer_ = cameraParameter_->As<ShaderResourceBufferSP>().GetValue();
		GraphicsBufferSP buffer = XREXContext::GetInstance().GetRenderingFactory().CreateGraphicsBufferWithBufferInformation(
			GraphicsBuffer::Usage::DynamicDraw, parameterBuffer_->GetBufferInformation());
		parameterBuffer_->SetBuffer(buffer);

		auto viewFromeWorldResult = parameterBuffer_->GetSetter("XREX_CameraTransformation.ViewFromWorld");
		assert(viewFromeWorldResult.first);
		viewFromeWorld_ = viewFromeWorldResult.second;
		auto clipFromViewResult = parameterBuffer_->GetSetter("XREX_CameraTransformation.ClipFromView");
		assert(clipFromViewResult.first);
		clipFromView_ = clipFromViewResult.second;
		auto clipFromWorldResult = parameterBuffer_->GetSetter("XREX_CameraTransformation.ClipFromWorld");
		assert(clipFromWorldResult.first);
		clipFromWorld_ = clipFromWorldResult.second;
		auto cameraPositionInWorldResult = parameterBuffer_->GetSetter("XREX_CameraTransformation.CameraPositionInWorld");
		assert(cameraPositionInWorldResult.first);
		cameraPositionInWorld_ = cameraPositionInWorldResult.second;
	}

	void CameraSetter::SetParameter(CameraSP const& component)
	{
		floatM44 viewMatrix = component->GetViewMatrix();
		floatM44 projectionMatrix = component->GetProjectionMatrix();

		ShaderResourceBuffer::BufferMapper mapper = parameterBuffer_->GetMapper();
		floatM44 clipFromWorld = projectionMatrix * viewMatrix;

		viewFromeWorld_.SetValue(mapper, viewMatrix);
		clipFromView_.SetValue(mapper, projectionMatrix);
		clipFromWorld_.SetValue(mapper, clipFromWorld);
		cameraPositionInWorld_.SetValue(mapper, 
			component->GetOwnerSceneObject()->GetComponent<Transformation>()->GetWorldPosition());
	}

}
