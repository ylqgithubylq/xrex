#include "XREX.hpp"
#include "MeshLoader.hpp"

#include "DefinedShaderName.hpp"
#include "Mesh.hpp"
#include "RenderingLayout.hpp"
#include "RenderingEffect.hpp"
#include "GraphicsBuffer.hpp"
#include "XREXContext.hpp"
#include "RenderingFactory.hpp"
#include "ResourceManager.hpp"
#include "Material.hpp"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <vector>
#include <sstream>
#include <array>

#include <filesystem>

using std::vector;

namespace XREX
{
	namespace
	{
		struct SceneProcessor
		{
			MeshSP* outMesh_;
			aiScene const& scene_;
			vector<RenderingLayoutSP> layouts_;
			vector<MaterialSP> materials_;
			vector<TextureSP> textures_;
			std::string directoryPath_;

			SceneProcessor(aiScene const& theScene, std::string const& filePath, MeshSP* theOutMesh)
				: scene_(theScene), outMesh_(theOutMesh)
			{
				std::tr2::sys::path scenePath(filePath);
				directoryPath_ = scenePath.parent_path().string() + "/";
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
				materials_.resize(scene_.mNumMaterials);
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
					materials_[i] = material;
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
						//std::make_pair(aiTextureType::aiTextureType_AMBIENT, GetUniformString(DefinedUniform::DiffuseMap)),
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
					aiTextureOp textureOp;
					aiTextureMapMode textureMapMode;

					for (auto& textureType : TextureTypes)
					{
						textureCount = loaderMaterial->GetTextureCount(textureType.first);
						assert(textureCount <= 1); // larger are not support
						for (uint32 j = 0; j < textureCount; ++j)
						{
							if (AI_SUCCESS == loaderMaterial->GetTexture(textureType.first, j, &path, &textureMapping, &uvIndex, &blend, &textureOp, &textureMapMode))
							{
								TextureSP texture = XREXContext::GetInstance().GetResourceManager().GetTexture2D(path.C_Str());
								if (!texture)
								{
									texture = XREXContext::GetInstance().GetResourceManager().GetTexture2D(directoryPath_ + path.C_Str());
								}
							
								assert(textureType.second != ""); // TODO log failure rather than assert
								material->SetParameter(textureType.second, texture);
								// TODO generate sampler
							}
						}
					}
				}
			}

			void ProcessMesh()
			{
				layouts_.resize(scene_.mNumMeshes);
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
					uint32 textureCoordinateCount = 0;
					uint32 vertexColorCount = 0;
					bool makeSureNoGap = true;
					if (mesh->HasPositions())
					{
						totalLengthPerElement += sizeof(*mesh->mVertices);
					}
					if (mesh->HasNormals())
					{
						totalLengthPerElement += sizeof(*mesh->mNormals);
					}
					for (uint32 j = 0; j < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++j)
					{
						if (mesh->HasTextureCoords(j))
						{
							totalLengthPerElement += sizeof(*mesh->mTextureCoords[0]);
							++textureCoordinateCount;
							assert(makeSureNoGap);
						}
						else
						{
							makeSureNoGap = false;
						}
					}
					makeSureNoGap = true;
					for (uint32 j = 0; j < AI_MAX_NUMBER_OF_COLOR_SETS; ++j)
					{
						if (mesh->HasVertexColors(j))
						{
							totalLengthPerElement += sizeof(*mesh->mColors[0]);
							++vertexColorCount;
						}
						else
						{
							makeSureNoGap = false;
						}
					}

					int startLocation = 0;
					GraphicsBuffer::DataLayout dataDescription = GraphicsBuffer::DataLayout(mesh->mNumVertices);
					if (mesh->HasPositions())
					{
						dataDescription.AddChannelLayout(GraphicsBuffer::DataLayout::ElementLayout(startLocation, totalLengthPerElement, ElementType::FloatV3, GetAttributeString(DefinedAttribute::Position)));
						startLocation += sizeof(*mesh->mVertices);
					}
					if (mesh->HasNormals())
					{
						dataDescription.AddChannelLayout(GraphicsBuffer::DataLayout::ElementLayout(startLocation, totalLengthPerElement, ElementType::FloatV3, GetAttributeString(DefinedAttribute::Normal)));
						startLocation += sizeof(*mesh->mNormals);
					}
					for (uint32 j = 0; j < textureCoordinateCount; ++j)
					{
						dataDescription.AddChannelLayout(GraphicsBuffer::DataLayout::ElementLayout(startLocation, totalLengthPerElement, ElementType::FloatV3,
							GetAttributeString(static_cast<DefinedAttribute>(static_cast<uint32>(DefinedAttribute::TextureCoordinate0) + j))));
						startLocation += sizeof(*mesh->mTextureCoords[0]);
					}
					for (uint32 j = 0; j < vertexColorCount; ++j)
					{
						dataDescription.AddChannelLayout(GraphicsBuffer::DataLayout::ElementLayout(startLocation, totalLengthPerElement, ElementType::FloatV4,
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
							assert(face.mNumIndices == 3);
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
							assert(face.mNumIndices == 3);
							for (int k = 0; k < indicesPerFace; ++k)
							{
								indexData32[j * indicesPerFace + k] = face.mIndices[k];
							}
						}
					}


					GraphicsBufferSP vertices = XREXContext::GetInstance().GetRenderingFactory().CreateGraphicsVertexBuffer(GraphicsBuffer::Usage::Static, data, std::move(dataDescription));
					GraphicsBufferSP indices = !useLargeIndexBuffer
						? XREXContext::GetInstance().GetRenderingFactory().CreateGraphicsIndexBuffer(GraphicsBuffer::Usage::Static, indexData16)
						: XREXContext::GetInstance().GetRenderingFactory().CreateGraphicsIndexBuffer(GraphicsBuffer::Usage::Static, indexData32);
					RenderingLayout::DrawingMode mode;
					switch (mesh->mPrimitiveTypes)
					{
					case aiPrimitiveType::aiPrimitiveType_TRIANGLE:
						mode = RenderingLayout::DrawingMode::Triangles;
						break;
					case aiPrimitiveType::aiPrimitiveType_LINE:
						mode = RenderingLayout::DrawingMode::Lines;
						break;
					case aiPrimitiveType::aiPrimitiveType_POINT:
						mode = RenderingLayout::DrawingMode::Points;
						break;
					default:
						assert(false);
						break;
					}
					vector<GraphicsBufferSP> vertexBuffers(1);
					vertexBuffers[0] = vertices;
					RenderingLayoutSP layout = XREXContext::GetInstance().GetRenderingFactory().CreateRenderingLayout(vertexBuffers, indices, mode);
					layouts_[i] = layout;
				}
			}


			void ProcessNode(aiNode const& node)
			{
				uint32* meshIndices = node.mMeshes;
				for (uint32 i = 0; i < node.mNumMeshes; ++i)
				{
					aiMesh* mesh = scene_.mMeshes[meshIndices[i]];
					(*outMesh_)->CreateSubMesh(mesh->mName.C_Str(), materials_[mesh->mMaterialIndex], layouts_[meshIndices[i]], nullptr);
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

	MeshSP MeshLoader::LoadMesh(std::string const& fileName)
	{
		Assimp::Importer importer;

		aiScene const* scene = importer.ReadFile(fileName, 
			// aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType
			);

		if(!scene)
		{
			// TODO log the error?
			// cout << importer.GetErrorString() << endl;
			return nullptr;
		}
		MeshSP mesh = MakeSP<Mesh>(fileName);
		SceneProcessor(*scene, fileName, &mesh);

		// Everything will be cleaned up by the importer destructor
		return mesh;
	}

}
