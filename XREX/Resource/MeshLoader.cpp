#include "XREX.hpp"

#include "MeshLoader.hpp"

#include "Base/XREXContext.hpp"
#include "Base/Logger.hpp"
#include "Rendering/DefinedShaderName.hpp"
#include "Rendering/Mesh.hpp"
#include "Rendering/RenderingLayout.hpp"
#include "Rendering/RenderingEffect.hpp"
#include "Rendering/GraphicsBuffer.hpp"
#include "Base/XREXContext.hpp"
#include "Rendering/RenderingFactory.hpp"
#include "Resource/ResourceManager.hpp"
#include "Rendering/Material.hpp"
#include "Rendering/Sampler.hpp"
#include "Resource/TextureLoader.hpp"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output vertex structure
#include <assimp/postprocess.h>     // Post processing flags

#include <vector>
#include <sstream>
#include <array>
#include <iostream>

#include <filesystem>

using std::vector;

namespace XREX
{
	namespace
	{
		struct NullModelLoadingResult
			: LoadingResult<Mesh>
		{
			virtual bool Succeeded() const override
			{
				return false;
			}
			virtual MeshSP Create() override
			{
				return nullptr;
			}
		};

		struct ModelLoadingResultDetail
			: LoadingResult<Mesh>
		{

			struct DataDetail
				: private XREX::Noncopyable
			{
				struct LayoutData
					: private XREX::Noncopyable
				{
					VertexBuffer::DataLayoutDescription description;
					std::vector<uint8> vertex;
					IndexBuffer::TopologicalType primitiveType;
					ElementType indexType;
					std::vector<uint16> index16;
					std::vector<uint32> index32;

					LayoutData(VertexBuffer::DataLayoutDescription&& theDescription, std::vector<uint8>&& theVertex, IndexBuffer::TopologicalType thePrimitiveType, std::vector<uint16>&& theIndex)
						: description(std::move(theDescription)), vertex(std::move(theVertex)), primitiveType(thePrimitiveType), index16(std::move(theIndex)), indexType(ElementType::Uint16)
					{
					}
					LayoutData(VertexBuffer::DataLayoutDescription&& theDescription, std::vector<uint8>&& theVertex, IndexBuffer::TopologicalType thePrimitiveType, std::vector<uint32>&& theIndex)
						: description(std::move(theDescription)), vertex(std::move(theVertex)), primitiveType(thePrimitiveType), index32(std::move(theIndex)), indexType(ElementType::Uint32)
					{
					}
					LayoutData(LayoutData&& right)
						: description(std::move(right.description)), vertex(std::move(right.vertex)), index16(std::move(right.index16)), index32(std::move(right.index32)),
						primitiveType(right.primitiveType), indexType(right.indexType)
					{
					}
				};

				struct MaterialData
					: private XREX::Noncopyable
				{
					struct TextureData
						: private XREX::Noncopyable
					{
						std::string name;
						SamplerState samplerState;
						TextureLoadingResultSP loadingResult;

						TextureData(std::string const& theName, SamplerState const& theSamplerState, TextureLoadingResultSP&& theLoadingResult)
							: name(theName), samplerState(theSamplerState), loadingResult(std::move(theLoadingResult))
						{
						}
						TextureData(TextureData&& right)
							: name(std::move(right.name)), loadingResult(std::move(right.loadingResult)), samplerState(right.samplerState)
						{
						}
					};

					MaterialSP material;
					std::vector<TextureData> textures;

					MaterialData(MaterialSP&& theMaterial, std::vector<TextureData>&& theTextures)
						: material(std::move(theMaterial)), textures(std::move(theTextures))
					{
					}
					MaterialData(MaterialData&& right)
						: material(std::move(right.material)), textures(std::move(right.textures))
					{
					}
				};

				std::string name;

				std::vector<LayoutData> layouts;
				std::vector<MaterialData> materials;
				std::vector<std::tuple<std::string, uint32, uint32>> subMeshes; // tuple<(name), (material index), (mesh index)>

				std::vector<RenderingLayoutSP> createdLayouts;
				std::vector<MaterialSP> createdMaterials;
				MeshSP loadedMesh;

				DataDetail(std::string const& theName)
					: name(theName)
				{
				}
				~DataDetail()
				{
				}

				void AddSubMeshData(LayoutData&& subMeshData)
				{
					layouts.push_back(std::move(subMeshData));
				}
				void AddMaterialData(MaterialData&& materialData)
				{
					materials.push_back(std::move(materialData));
				}
				// tuple<(name), (material index), (mesh index)>
				void AddSubMesh(std::tuple<std::string, uint32, uint32> meshIndex)
				{
					subMeshes.push_back(meshIndex);
				}

				MeshSP DoCreateMesh()
				{
					if (loadedMesh == nullptr)
					{
						loadedMesh = DoLoad();
					}
					return loadedMesh;
				}

				MeshSP DoLoad()
				{
					createdLayouts.reserve(layouts.size());
					for (auto& layoutToCreate : layouts)
					{
						VertexBufferSP vertices = XREXContext::GetInstance().GetRenderingFactory().CreateVertexBuffer(
							GraphicsBuffer::Usage::StaticDraw, layoutToCreate.vertex, std::move(layoutToCreate.description));
						IndexBufferSP indices = (layoutToCreate.indexType == ElementType::Uint16)
							? XREXContext::GetInstance().GetRenderingFactory().CreateIndexBuffer(
								GraphicsBuffer::Usage::StaticDraw, layoutToCreate.index16, layoutToCreate.primitiveType)
							: XREXContext::GetInstance().GetRenderingFactory().CreateIndexBuffer(
								GraphicsBuffer::Usage::StaticDraw, layoutToCreate.index32, layoutToCreate.primitiveType);

						vector<VertexBufferSP> vertexBuffers(1);
						vertexBuffers[0] = vertices;
						RenderingLayoutSP layout = XREXContext::GetInstance().GetRenderingFactory().CreateRenderingLayout(vertexBuffers, indices);
						createdLayouts.push_back(layout);
					}

					createdMaterials.reserve(materials.size());
					for (auto& materialToFinish : materials)
					{
						for (auto& textureToCreate : materialToFinish.textures)
						{
							SamplerSP sampler = XREXContext::GetInstance().GetRenderingFactory().CreateSampler(textureToCreate.samplerState);
							TextureSP texture = textureToCreate.loadingResult->Create();
							materialToFinish.material->SetParameter(textureToCreate.name, std::make_pair(texture, sampler));
						}
						createdMaterials.push_back(std::move(materialToFinish.material));
					}

					MeshSP createdMesh = MakeSP<Mesh>(name);

					for (auto& meshIndex : subMeshes)
					{
						createdMesh->CreateSubMesh(std::get<0>(meshIndex), createdLayouts[std::get<2>(meshIndex)], createdMaterials[std::get<1>(meshIndex)], nullptr);
					}

					return createdMesh;
				}

			};

			ModelLoadingResultDetail(std::string const& name)
				: data_(MakeUP<DataDetail>(name))
			{
			}
			void AddSubMeshData(DataDetail::LayoutData&& subMeshData)
			{
				data_->AddSubMeshData(std::move(subMeshData));
			}
			void AddMaterialData(DataDetail::MaterialData&& materialData)
			{
				data_->AddMaterialData(std::move(materialData));
			}
			// tuple<(name), (material index), (mesh index)>
			void AddSubMesh(std::tuple<std::string, uint32, uint32> meshIndex)
			{
				data_->AddSubMesh(meshIndex);
			}

			virtual bool Succeeded() const override
			{
				return data_ != nullptr;
			}

			virtual MeshSP Create() override
			{
				if (Succeeded())
				{
					return data_->DoCreateMesh();
				}
				return nullptr;
			}

			std::unique_ptr<DataDetail> data_;
		};



		struct SceneProcessor
		{
			aiScene const& scene_;
			std::string directoryPath_;

			std::shared_ptr<ModelLoadingResultDetail> result_;

			SceneProcessor(aiScene const& theScene, std::string const& filePath)
				: scene_(theScene)
			{
				std::tr2::sys::path scenePath(filePath);
				directoryPath_ = scenePath.parent_path().string() + "/";
				result_ = MakeSP<ModelLoadingResultDetail>(scenePath.string());
				ProcessScene();
			}

			void ProcessScene()
			{
	// 			aiAnimation** animations = scene_.mAnimations;
	// 			for (uint32 i = 0; i < scene_.mNumAnimations; ++i)
	// 			{
	// 				aiAnimation* animation = animations[i];
	// 			}
	// 
	// 			aiCamera** cameras = scene_.mCameras;
	// 			for (uint32 i = 0; i < scene_.mNumCameras; ++i)
	// 			{
	// 				aiCamera* camera = cameras[i];
	// 			}
	// 
	// 			aiLight** lights = scene_.mLights;
	// 			for (uint32 i = 0; i < scene_.mNumLights; ++i)
	// 			{
	// 				aiLight* light = lights[i];
	// 			}

				ProcessMaterial();
				ProcessMesh();

	// 			aiTexture** textures = scene_.mTextures;
	// 			for (uint32 i = 0; i < scene_.mNumTextures; ++i)
	// 			{
	// 				aiTexture* texture = textures[i];
	// 				texture->pcData;
	// 				texture->mHeight;
	// 				texture->mWidth;
	// 			}


				aiNode* rootNode = scene_.mRootNode;
				ProcessNode(*rootNode);
			}

			void ProcessMaterial()
			{
				aiMaterial** materials = scene_.mMaterials;
				for (uint32 i = 0; i < scene_.mNumMaterials; ++i)
				{
					aiMaterial* loaderMaterial = materials[i];
					// TODO iterate over all the loaderMaterial parameters
					aiString name;
					if (AI_SUCCESS != loaderMaterial->Get(AI_MATKEY_NAME, name))
					{
						assert(false);
					}
					MaterialSP material = MakeSP<Material>(name.C_Str());
					static_assert(sizeof(aiColor3D) == sizeof(floatV3), "size not match.");
					aiColor3D aiColor;
					if (AI_SUCCESS == loaderMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor))
					{
						floatV3 color;
						memcpy_s(&color, sizeof(floatV3), &aiColor, sizeof(aiColor3D));
						material->SetParameter(GetUniformString(DefinedUniform::DiffuseColor), color);
					}
					if (AI_SUCCESS == loaderMaterial->Get(AI_MATKEY_COLOR_SPECULAR, aiColor))
					{
						floatV3 color;
						memcpy_s(&color, sizeof(floatV3), &aiColor, sizeof(aiColor3D));
						material->SetParameter(GetUniformString(DefinedUniform::SpecularColor), color);
					}
					if (AI_SUCCESS == loaderMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, aiColor))
					{
						floatV3 color;
						memcpy_s(&color, sizeof(floatV3), &aiColor, sizeof(aiColor3D));
						material->SetParameter(GetUniformString(DefinedUniform::EmissiveColor), color);
					}
					if (AI_SUCCESS == loaderMaterial->Get(AI_MATKEY_COLOR_TRANSPARENT, aiColor))
					{
						floatV3 color;
						memcpy_s(&color, sizeof(floatV3), &aiColor, sizeof(aiColor3D));
						material->SetParameter(GetUniformString(DefinedUniform::TransparentColor), color);
					}
					float value;
					if (AI_SUCCESS == loaderMaterial->Get(AI_MATKEY_OPACITY, value))
					{
						material->SetParameter(GetUniformString(DefinedUniform::Opacity), value);
					}
					if (AI_SUCCESS == loaderMaterial->Get(AI_MATKEY_SHININESS, value))
					{
						material->SetParameter(GetUniformString(DefinedUniform::Shininess), value);
					}
					if (AI_SUCCESS == loaderMaterial->Get(AI_MATKEY_SHININESS_STRENGTH, value))
					{
						material->SetParameter(GetUniformString(DefinedUniform::SpecularLevel), value);
					}


					static std::pair<aiTextureType, std::string> TextureTypes[] = 
					{
						std::make_pair(aiTextureType::aiTextureType_DIFFUSE, GetUniformString(DefinedUniform::DiffuseMap)),
						std::make_pair(aiTextureType::aiTextureType_SPECULAR, GetUniformString(DefinedUniform::SpecularMap)),
						//std::make_pair(aiTextureType::aiTextureType_AMBIENT, GetUniformString(DefinedUniform::DiffuseMap)), // same as diffuse
						std::make_pair(aiTextureType::aiTextureType_EMISSIVE, GetUniformString(DefinedUniform::EmissiveMap)),
						std::make_pair(aiTextureType::aiTextureType_HEIGHT, GetUniformString(DefinedUniform::HeightMap)),
						std::make_pair(aiTextureType::aiTextureType_NORMALS, GetUniformString(DefinedUniform::NormalMap)),
						std::make_pair(aiTextureType::aiTextureType_SHININESS, GetUniformString(DefinedUniform::ShininessMap)),
						std::make_pair(aiTextureType::aiTextureType_OPACITY, GetUniformString(DefinedUniform::OpacityMap)),
						std::make_pair(aiTextureType::aiTextureType_DISPLACEMENT, GetUniformString(DefinedUniform::DisplacementMap)),
						std::make_pair(aiTextureType::aiTextureType_LIGHTMAP, GetUniformString(DefinedUniform::LightMap)),
						std::make_pair(aiTextureType::aiTextureType_REFLECTION, GetUniformString(DefinedUniform::ReflectionMap)),
						std::make_pair(aiTextureType::aiTextureType_UNKNOWN, ""),
					};

					uint32 textureCount = 0;
					aiString path;
					aiTextureMapping textureMapping;
					uint32 uvIndex = 0;
					float blend = 0.f;
					aiTextureOp textureOp = _aiTextureOp_Force32Bit;
					std::array<aiTextureMapMode, 3> textureMapModes;
					textureMapModes.fill(_aiTextureMapMode_Force32Bit);

					std::vector<ModelLoadingResultDetail::DataDetail::MaterialData::TextureData> texturesToLoad;

					for (auto& textureType : TextureTypes)
					{
						textureCount = loaderMaterial->GetTextureCount(textureType.first);
						assert(textureCount <= 1); // larger are not support

						for (uint32 j = 0; j < textureCount; ++j)
						{
							if (AI_SUCCESS == loaderMaterial->GetTexture(textureType.first, j, &path, &textureMapping, &uvIndex, &blend, &textureOp, textureMapModes.data()))
							{
								TextureLoadingResultSP texureLoadingResult = XREXContext::GetInstance().GetResourceManager().LoadTexture2D(directoryPath_ + path.C_Str());
								if (!texureLoadingResult->Succeeded())
								{
									texureLoadingResult = XREXContext::GetInstance().GetResourceManager().LoadTexture2D(path.C_Str());
								}

								assert(textureType.second != ""); // TODO log failure rather than assert

								SamplerState samplerState;
								std::array<SamplerState::TextureAddressingMode, 3> addressingModes;
								for (uint32 i = 0; i < 3; ++i)
								{
									switch (textureMapModes[i])
									{
									case aiTextureMapMode_Wrap:
										addressingModes[i] = SamplerState::TextureAddressingMode::Repeat;
										break;
									case aiTextureMapMode_Clamp:
										addressingModes[i] = SamplerState::TextureAddressingMode::ClampToEdge;
										break;
									case aiTextureMapMode_Decal:
										addressingModes[i] = SamplerState::TextureAddressingMode::ClampToBorder;
										break;
									case aiTextureMapMode_Mirror:
										addressingModes[i] = SamplerState::TextureAddressingMode::MirroredRepeat;
										break;
									case _aiTextureMapMode_Force32Bit: // default value
										addressingModes[i] = SamplerState::TextureAddressingMode::Repeat;
										break;
									default:
										assert(false);
										break;
									}
								}
								samplerState.addressingModeS = addressingModes[0];
								samplerState.addressingModeT = addressingModes[1];
								samplerState.addressingModeR = addressingModes[2];
								samplerState.magFilterOperation = SamplerState::TextureFilterOperation::Linear;
								samplerState.minFilterOperation = SamplerState::TextureFilterOperation::NearestMipmapLinear;

								texturesToLoad.push_back(ModelLoadingResultDetail::DataDetail::MaterialData::TextureData(textureType.second, samplerState, std::move(texureLoadingResult)));
							}
						}
					}
					result_->AddMaterialData(ModelLoadingResultDetail::DataDetail::MaterialData(std::move(material), std::move(texturesToLoad)));
				}
			}

			void ProcessMesh()
			{
				aiMesh** meshes = scene_.mMeshes;
				for (uint32 i = 0; i < scene_.mNumMeshes; ++i)
				{
					aiMesh* mesh = scene_.mMeshes[i];

					// not handled
					// 				mesh->mNumAnimMeshes;
					// 				mesh->mAnimMeshes;
					// 
					// 				mesh->mNumBones;
					// 				mesh->mBones;


					uint32 totalLengthPerElement = 0;
					uint32 textureCoordinateCount = mesh->GetNumUVChannels();
					uint32 vertexColorCount = mesh->GetNumColorChannels();
					if (mesh->HasPositions())
					{
						totalLengthPerElement += sizeof(*mesh->mVertices);
					}
					if (mesh->HasNormals())
					{
						totalLengthPerElement += sizeof(*mesh->mNormals);
					}
					totalLengthPerElement += textureCoordinateCount * sizeof(*mesh->mTextureCoords[0]);
					totalLengthPerElement += vertexColorCount * sizeof(*mesh->mColors[0]);

					int startLocation = 0;
					VertexBuffer::DataLayoutDescription dataDescription = VertexBuffer::DataLayoutDescription(mesh->mNumVertices);
					if (mesh->HasPositions())
					{
						dataDescription.AddChannelLayout(VertexBuffer::DataLayoutDescription::ElementLayoutDescription(startLocation, totalLengthPerElement, ElementType::FloatV3, GetAttributeString(DefinedAttribute::Position)));
						startLocation += sizeof(*mesh->mVertices);
					}
					if (mesh->HasNormals())
					{
						dataDescription.AddChannelLayout(VertexBuffer::DataLayoutDescription::ElementLayoutDescription(startLocation, totalLengthPerElement, ElementType::FloatV3, GetAttributeString(DefinedAttribute::Normal)));
						startLocation += sizeof(*mesh->mNormals);
					}
					for (uint32 j = 0; j < textureCoordinateCount; ++j)
					{
						dataDescription.AddChannelLayout(VertexBuffer::DataLayoutDescription::ElementLayoutDescription(startLocation, totalLengthPerElement, ElementType::FloatV3,
							GetAttributeString(static_cast<DefinedAttribute>(static_cast<uint32>(DefinedAttribute::TextureCoordinate0) + j))));
						startLocation += sizeof(*mesh->mTextureCoords[0]);
					}
					for (uint32 j = 0; j < vertexColorCount; ++j)
					{
						dataDescription.AddChannelLayout(VertexBuffer::DataLayoutDescription::ElementLayoutDescription(startLocation, totalLengthPerElement, ElementType::FloatV4,
							GetAttributeString(static_cast<DefinedAttribute>(static_cast<uint32>(DefinedAttribute::Color0) + j))));
						startLocation += sizeof(*mesh->mColors[0]);
					}

					vector<uint8> data = vector<uint8>(totalLengthPerElement * mesh->mNumVertices);
					for (uint32 j = 0, currentLocation = 0; j < mesh->mNumVertices; ++j)
					{
						if (mesh->HasPositions())
						{
							auto& vertex = mesh->mVertices[j];
							memcpy_s(&data[currentLocation], data.size() - currentLocation, &vertex, sizeof(vertex));
							currentLocation += sizeof(vertex);
						}
						if (mesh->HasNormals())
						{
							auto& normal = mesh->mNormals[j];
							memcpy_s(&data[currentLocation], data.size() - currentLocation, &normal, sizeof(normal));
							currentLocation += sizeof(normal);
						}
						for (uint32 k = 0; k < textureCoordinateCount; ++k)
						{
							auto& textureCoordinate = mesh->mTextureCoords[k][j];
							memcpy_s(&data[currentLocation], data.size() - currentLocation, &textureCoordinate, sizeof(textureCoordinate));
							currentLocation += sizeof(textureCoordinate);
						}
						for (uint32 k = 0; k < vertexColorCount; ++k)
						{
							auto& color = mesh->mColors[k][j];
							memcpy_s(&data[currentLocation], data.size() - currentLocation, &color, sizeof(color));
							currentLocation += sizeof(color);
						}
						assert(currentLocation <= data.size());
					}

					vector<uint16> indexData16;
					vector<uint32> indexData32;
					int32 indicesPerFace = mesh->mFaces[0].mNumIndices;
					bool useLargeIndexBuffer = false;
					if (mesh->mNumVertices > static_cast<uint32>(std::numeric_limits<uint16>::max()))
					{
						useLargeIndexBuffer = true;
					}
					if (!useLargeIndexBuffer)
					{
						indexData16.resize(mesh->mNumFaces * indicesPerFace);
						for (uint32 j = 0; j < mesh->mNumFaces; ++j)
						{
							aiFace& face = mesh->mFaces[j];
							//assert(face.mNumIndices == 3);
							for (int k = 0; k < indicesPerFace; ++k)
							{
								indexData16[j * indicesPerFace + k] = static_cast<uint16>(face.mIndices[k]);
							}
						}
					}
					else
					{
						indexData32.resize(mesh->mNumFaces * indicesPerFace);
						for (uint32 j = 0; j < mesh->mNumFaces; ++j)
						{
							aiFace& face = mesh->mFaces[j];
							assert(face.mNumIndices <= 3);
							for (int k = 0; k < indicesPerFace; ++k)
							{
								indexData32[j * indicesPerFace + k] = face.mIndices[k];
							}
						}
					}

					IndexBuffer::TopologicalType primitiveType;
					switch (mesh->mPrimitiveTypes)
					{
					case aiPrimitiveType::aiPrimitiveType_TRIANGLE:
						primitiveType = IndexBuffer::TopologicalType::Triangles;
						break;
					case aiPrimitiveType::aiPrimitiveType_LINE:
						primitiveType = IndexBuffer::TopologicalType::Lines;
						break;
					case aiPrimitiveType::aiPrimitiveType_POINT:
						primitiveType = IndexBuffer::TopologicalType::Points;
						break;
					default:
						assert(false);
						break;
					}

					if (!useLargeIndexBuffer)
					{
						result_->AddSubMeshData(ModelLoadingResultDetail::DataDetail::LayoutData(std::move(dataDescription), std::move(data), primitiveType, std::move(indexData16)));
					}
					else
					{
						result_->AddSubMeshData(ModelLoadingResultDetail::DataDetail::LayoutData(std::move(dataDescription), std::move(data), primitiveType, std::move(indexData32)));
					}

				}
			}


			void ProcessNode(aiNode const& node)
			{
				uint32* meshIndices = node.mMeshes;
				for (uint32 i = 0; i < node.mNumMeshes; ++i)
				{
					aiMesh* mesh = scene_.mMeshes[meshIndices[i]];
					result_->AddSubMesh(std::make_tuple(std::string(mesh->mName.C_Str()), mesh->mMaterialIndex, meshIndices[i]));
				}

				aiNode** children = node.mChildren;
				for (uint32 i = 0; i < node.mNumChildren; ++i)
				{
					ProcessNode(*children[i]);
				}
			}

		};

	}

	MeshLoader::MeshLoader()
	{
	}


	MeshLoader::~MeshLoader()
	{
	}

	MeshLoadingResultSP MeshLoader::LoadMesh(std::string const& fileName)
	{
		Assimp::Importer importer;

		aiScene const* scene = importer.ReadFile(fileName,
			//aiProcess_GenSmoothNormals |
			//aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			//aiProcess_SortByPType |
			0
			);

		if(!scene)
		{
			XREXContext::GetInstance().GetLogger().LogLine(importer.GetErrorString());
			return MakeSP<NullModelLoadingResult>();
		}
		// Everything will be cleaned up by the importer destructor
		return SceneProcessor(*scene, fileName).result_;
	}

}
