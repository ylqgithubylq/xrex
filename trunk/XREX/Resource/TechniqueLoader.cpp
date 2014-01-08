#include "XREX.hpp"

#include "TechniqueLoader.hpp"
#include "Rendering/RenderingTechnique.hpp"
#include "Rendering/TechniqueBuilder.hpp"
#include "Rendering/RenderingEngine.hpp"
#include "Rendering/SystemTechnique.hpp"
#include "Base/XREXContext.hpp"
#include "Resource/LocalResourceLoader.hpp"
#include "Base/Logger.hpp"

#include <rapidxml/rapidxml.hpp>

#include <sstream>
#include <filesystem>

#pragma warning(push)
#pragma warning(disable: 4573) // a bug, static member function used in lambda caused the compiler to think this pointer is needed.


namespace XREX
{
	namespace
	{
		class EnumReflectionException
			: public std::exception
		{
		public:
			EnumReflectionException(std::string const& type, std::string const& value)
				: message_("Type: " + type + " do not have a Enumeration: " + value)
			{
			}
			virtual char const* what() const override
			{
				return message_.c_str();
			}
		private:
			std::string message_;
		};

		bool BoolFromString(std::string const& name)
		{
			static std::string const Bool = "bool";
			static std::string const True = "true";
			static std::string const False = "false";
			if (name == True)
			{
				return true;
			}
			else if (name == False)
			{
				return false;
			}
			else
			{
				throw EnumReflectionException(Bool, name);
			}
		}

		ElementType ElementTypeFromString(std::string const& name)
		{
			static std::string const Type = "ElementType";
			static std::unordered_map<std::string, ElementType> const ElementTypes = []
			{
				std::remove_const<decltype(ElementTypes)>::type temp;
				temp["Bool"] = ElementType::Bool;
				temp["Uint8"] = ElementType::Uint8;
				temp["Uint16"] = ElementType::Uint16;
				temp["Uint32"] = ElementType::Uint32;
				temp["Int8"] = ElementType::Int8;
				temp["Int16"] = ElementType::Int16;
				temp["Int32"] = ElementType::Int32;
				temp["IntV2"] = ElementType::IntV2;
				temp["IntV3"] = ElementType::IntV3;
				temp["IntV4"] = ElementType::IntV4;
				temp["UintV2"] = ElementType::UintV2;
				temp["UintV3"] = ElementType::UintV3;
				temp["UintV4"] = ElementType::UintV4;
				temp["Float"] = ElementType::Float;
				temp["FloatV2"] = ElementType::FloatV2;
				temp["FloatV3"] = ElementType::FloatV3;
				temp["FloatV4"] = ElementType::FloatV4;
				temp["FloatM44"] = ElementType::FloatM44;
				temp["Double"] = ElementType::Double;
				temp["DoubleV2"] = ElementType::DoubleV2;
				temp["DoubleV3"] = ElementType::DoubleV3;
				temp["DoubleV4"] = ElementType::DoubleV4;
				temp["DoubleM44"] = ElementType::DoubleM44;
				return temp;
			} ();
			auto found = ElementTypes.find(name);
			if (found != ElementTypes.end())
			{
				return found->second;
			}
			else
			{
				throw EnumReflectionException(Type, name);
			}

		}

		TexelFormat TexelFormatFromString(std::string const& name)
		{
			static std::string const Type = "TexelFormat";
			static std::unordered_map<std::string, TexelFormat> const TexelFormats = []
			{
				std::remove_const<decltype(TexelFormats)>::type temp;
				temp["R8"] = TexelFormat::R8;
				temp["RG8"] = TexelFormat::RG8;
				temp["RGB8"] = TexelFormat::RGB8;
				temp["RGBA8"] = TexelFormat::RGBA8;
				temp["R8I"] = TexelFormat::R8I;
				temp["RG8I"] = TexelFormat::RG8I;
				temp["RGB8I"] = TexelFormat::RGB8I;
				temp["RGBA8I"] = TexelFormat::RGBA8I;
				temp["R8UI"] = TexelFormat::R8UI;
				temp["RG8UI"] = TexelFormat::RG8UI;
				temp["RGB8UI"] = TexelFormat::RGB8UI;
				temp["RGBA8UI"] = TexelFormat::RGBA8UI;
				temp["R16I"] = TexelFormat::R16I;
				temp["RG16I"] = TexelFormat::RG16I;
				temp["RGB16I"] = TexelFormat::RGB16I;
				temp["RGBA16I"] = TexelFormat::RGBA16I;
				temp["R16UI"] = TexelFormat::R16UI;
				temp["RG16UI"] = TexelFormat::RG16UI;
				temp["RGB16UI"] = TexelFormat::RGB16UI;
				temp["RGBA16UI"] = TexelFormat::RGBA16UI;
				temp["R16F"] = TexelFormat::R16F;
				temp["RG16F"] = TexelFormat::RG16F;
				temp["RGB16F"] = TexelFormat::RGB16F;
				temp["RGBA16F"] = TexelFormat::RGBA16F;
				temp["R32I"] = TexelFormat::R32I;
				temp["RG32I"] = TexelFormat::RG32I;
				temp["RGB32I"] = TexelFormat::RGB32I;
				temp["RGBA32I"] = TexelFormat::RGBA32I;
				temp["R32UI"] = TexelFormat::R32UI;
				temp["RG32UI"] = TexelFormat::RG32UI;
				temp["RGB32UI"] = TexelFormat::RGB32UI;
				temp["RGBA32UI"] = TexelFormat::RGBA32UI;
				temp["R32F"] = TexelFormat::R32F;
				temp["RG32F"] = TexelFormat::RG32F;
				temp["RGB32F"] = TexelFormat::RGB32F;
				temp["RGBA32F"] = TexelFormat::RGBA32F;
				temp["BGR8"] = TexelFormat::BGR8;
				temp["BGRA8"] = TexelFormat::BGRA8;
				temp["BGR16F"] = TexelFormat::BGR16F;
				temp["BGRA16F"] = TexelFormat::BGRA16F;
				temp["BGR32F"] = TexelFormat::BGR32F;
				temp["BGRA32F"] = TexelFormat::BGRA32F;
				temp["Depth16"] = TexelFormat::Depth16;
				temp["Depth24"] = TexelFormat::Depth24;
				temp["Depth32"] = TexelFormat::Depth32;
				temp["Depth32F"] = TexelFormat::Depth32F;
				temp["Depth24Stencil8"] = TexelFormat::Depth24Stencil8;
				temp["Stencil8"] = TexelFormat::Stencil8;
				return temp;
			} ();
			auto found = TexelFormats.find(name);
			if (found != TexelFormats.end())
			{
				return found->second;
			}
			else
			{
				throw EnumReflectionException(Type, name);
			}
		}
	}




	namespace
	{
		struct TechniqueAndModificationTime
		{
			std::string fullPath;
			std::weak_ptr<RenderingTechnique> technique;
			std::vector<std::pair<std::string, std::string>> macros;
			double modificationTime;
			TechniqueAndModificationTime(std::string fullPath, RenderingTechniqueSP const& technique, std::vector<std::pair<std::string, std::string>> macros, double modificationTime)
				: fullPath(std::move(fullPath)), technique(technique), macros(std::move(macros)), modificationTime(modificationTime)
			{
			}
		};

		struct FrameBufferAndModificationTime
		{
			std::string fullPath;
			std::weak_ptr<FrameBuffer> framebuffer;
			double modificationTime;
			FrameBufferAndModificationTime(std::string fullPath, FrameBufferSP const& framebuffer, double modificationTime)
				: fullPath(std::move(fullPath)), framebuffer(framebuffer), modificationTime(modificationTime)
			{
			}
		};

		struct TechniqueInformationAndModificationTime
		{
			TechniqueBuildingInformationSP information;
			double modificationTime;
			TechniqueInformationAndModificationTime(TechniqueBuildingInformationSP information, double modificationTime)
				: information(std::move(information)), modificationTime(modificationTime)
			{
			}
		};

		struct FrameBufferDescriptionAndModificationTime
		{
			FrameBufferLayoutDescriptionSP description;
			double modificationTime;
			FrameBufferDescriptionAndModificationTime(FrameBufferLayoutDescriptionSP description, double modificationTime)
				: description(std::move(description)), modificationTime(modificationTime)
			{
			}
		};

		// TODO
		/*
		 * Monitor files in the cache. When file changed, reload the file and update the cache.
		 * And traverse createdTechniques to find out if any technique need to recreate based on modificationTime.
		 */
		struct LoadCache
		{
			std::vector<std::string> fileToMonitor;
			std::unordered_map<std::string, TechniqueInformationAndModificationTime> loadedTechniqueInformations; // key is full file name
			std::unordered_map<std::string, FrameBufferDescriptionAndModificationTime> loadedFrameBufferDescriptions; // key is full file name
			std::vector<TechniqueAndModificationTime> createdTechniques;
			std::vector<FrameBufferAndModificationTime> createdFrameBuffers;

			RenderingTechniqueSP FindTechnique(std::string const& fullPath, std::vector<std::pair<std::string, std::string>> const& macros)
			{
				auto found = std::find_if(createdTechniques.begin(), createdTechniques.end(), [&fullPath, &macros] (TechniqueAndModificationTime const& techniqueCache)
				{
					if (techniqueCache.fullPath != fullPath)
					{
						return false;
					}
					if (techniqueCache.macros.size() != macros.size())
					{
						return false;
					}
					// check if all macros match, order independent
					for (auto& macro : techniqueCache.macros)
					{
						auto macroFound = std::find(macros.begin(), macros.end(), macro);
						if (macroFound == macros.end())
						{
							return false;
						}
					}
					return true;
				});

				if (found != createdTechniques.end())
				{
					if (found->technique.expired())
					{
						SwapBackRemove(createdTechniques, found);
						return nullptr;
					}
					else
					{
						return found->technique.lock();
					}
				}
				return nullptr;
			}

			FrameBufferSP FindFrameBuffer(std::string const& fullPath)
			{
				auto found = std::find_if(createdFrameBuffers.begin(), createdFrameBuffers.end(), [&fullPath] (FrameBufferAndModificationTime const& framebufferCache)
				{
					return framebufferCache.fullPath == fullPath;
				});
				if (found == createdFrameBuffers.end())
				{
					return nullptr;
				}
				return found->framebuffer.lock();
			}
		};

	}

	struct TechniqueLoader::CacheDetail
	{
		LoadCache cache;
	};

	namespace
	{
		struct TechniqueLoadingResultDetail
			: LoadingResult<RenderingTechnique>
		{

			struct DataDetail
				: private XREX::Noncopyable
			{
				LoadCache* cache;
				std::string fullPath;
				TechniqueBuildingInformationSP techniqueInformation;
				std::vector<std::pair<std::string, std::string>> macros;
				RenderingTechniqueSP loadedTechnique;
				double loadedTime;

				// for new loaded technique information to create technique use.
				DataDetail(LoadCache* cache, std::string fullPath, TechniqueBuildingInformationSP information, std::vector<std::pair<std::string, std::string>> macros, double loadedTime)
					: cache(cache), fullPath(std::move(fullPath)), techniqueInformation(std::move(information)), macros(std::move(macros)), loadedTime(loadedTime)
				{
				}

				// for cached technique use.
				DataDetail(RenderingTechniqueSP loadedTechnique)
					: cache(nullptr), loadedTechnique(std::move(loadedTechnique))
				{
				}

				RenderingTechniqueSP Get()
				{
					if (loadedTechnique == nullptr)
					{
						loadedTechnique = DoCreate();
					}
					return loadedTechnique;
				}

				RenderingTechniqueSP DoCreate()
				{
					TechniqueBuilder builder(techniqueInformation);
					for (auto& macro : macros)
					{
						builder.AddMacros(macro);
					}
					RenderingTechniqueSP technique = builder.GetRenderingTechnique();
					if (technique != nullptr)
					{
						cache->createdTechniques.push_back(TechniqueAndModificationTime(std::move(fullPath), technique, std::move(macros), loadedTime));
					}
					return technique;
				}

			};

			TechniqueLoadingResultDetail(LoadCache* cache, std::string fullPath, TechniqueBuildingInformationSP information, std::vector<std::pair<std::string, std::string>> macros, double loadedTime)
				: data_(MakeUP<DataDetail>(cache, std::move(fullPath), std::move(information), std::move(macros), loadedTime))
			{
			}

			TechniqueLoadingResultDetail(RenderingTechniqueSP loadedTechnique)
				: data_(MakeUP<DataDetail>(std::move(loadedTechnique)))
			{
			}

			virtual bool Succeeded() const override
			{
				return data_ != nullptr;
			}

			virtual RenderingTechniqueSP Create() override
			{
				return data_ == nullptr ? nullptr : data_->Get();
			}

			std::unique_ptr<DataDetail> data_;
		};

		struct FrameBufferLoadingResultDetail
			: LoadingResult<FrameBuffer>
		{

			struct DataDetail
				: private XREX::Noncopyable
			{
				LoadCache* cache;
				std::string fullPath;
				FrameBufferLayoutDescriptionSP framebufferDescription;
				FrameBufferSP loadedFrameBuffer;
				double loadedTime;

				// for new loaded technique information to create technique use.
				DataDetail(LoadCache* cache, std::string fullPath, FrameBufferLayoutDescriptionSP framebufferDescription, double loadedTime)
					: cache(cache), fullPath(std::move(fullPath)), framebufferDescription(std::move(framebufferDescription)), loadedTime(loadedTime)
				{
				}

				// for cached technique use.
				DataDetail(FrameBufferSP loadedFrameBuffer)
					: cache(nullptr), loadedFrameBuffer(std::move(loadedFrameBuffer))
				{
				}

				FrameBufferSP Get()
				{
					if (loadedFrameBuffer == nullptr)
					{
						loadedFrameBuffer = DoCreate();
					}
					return loadedFrameBuffer;
				}

				FrameBufferSP DoCreate()
				{
					FrameBufferBuilder builder(framebufferDescription);
					FrameBufferSP framebuffer = builder.GetFrameBuffer();
					if (framebuffer != nullptr)
					{
						cache->createdFrameBuffers.push_back(FrameBufferAndModificationTime(std::move(fullPath), framebuffer, loadedTime));
					}
					return framebuffer;
				}

			};

			FrameBufferLoadingResultDetail(LoadCache* cache, std::string fullPath, FrameBufferLayoutDescriptionSP framebufferDescription, double loadedTime)
				: data_(MakeUP<DataDetail>(cache, std::move(fullPath), std::move(framebufferDescription), loadedTime))
			{
			}

			FrameBufferLoadingResultDetail(FrameBufferSP loadedFramebuffer)
				: data_(MakeUP<DataDetail>(std::move(loadedFramebuffer)))
			{
			}

			virtual bool Succeeded() const override
			{
				return data_ != nullptr;
			}

			virtual FrameBufferSP Create() override
			{
				return data_ == nullptr ? nullptr : data_->Get();
			}

			std::unique_ptr<DataDetail> data_;
		};

		void LogUnknownElement(std::string const& elementName, rapidxml::xml_node<>* node)
		{
			XREXContext::GetInstance().GetLogger().BeginLine().Log("Unknown ").Log(elementName).Log(" element name: ").Log(node->name()).Log(", element ignored.").EndLine();
		}

		void LogUnknownAttribute(std::string const& elementName, rapidxml::xml_attribute<>* attribute)
		{
			XREXContext::GetInstance().GetLogger().BeginLine().Log("Unknown ").Log(elementName).Log(" attribute name: ").Log(attribute->name()).Log(", attribute ignored.").EndLine();
		}

		void LogUnknownAttributeValue(std::string const& elementName, rapidxml::xml_attribute<>* attribute)
		{
			XREXContext::GetInstance().GetLogger().BeginLine().Log("Unknown ").Log(elementName).Log(" attribute ").Log(attribute->name()).Log(" value: ").Log(attribute->value()).Log(", attribute ignored.").EndLine();
		}

		struct FrameBufferDescriptionGenerator
		{
			FrameBufferLayoutDescriptionSP description;
			double modificationTime;

			LoadCache& cache;

			std::string const& fullPath;
			std::string directoryPath;
			std::string fileName;

			FrameBufferDescriptionGenerator(LoadCache& cache, std::string const& fullPath)
				: cache(cache), fullPath(fullPath)
			{
				std::tr2::sys::path fullTechniquePath(fullPath);
				directoryPath = fullTechniquePath.parent_path().string() + "/";
				fileName = fullTechniquePath.filename();

				double currentFileModificationTime = 0; // TODO how to get modification time? how to hot reload?

				auto found = cache.loadedFrameBufferDescriptions.find(fullPath);
				if (found != cache.loadedFrameBufferDescriptions.end())
				{
					if (found->second.modificationTime == currentFileModificationTime)
					{
						// already loaded and not modified.
						description = found->second.description;
						modificationTime = found->second.modificationTime;
						return;
					}
					else
					{
						// TODO should be triggered by file modification, update framebuffer, internal replace
					}
				}

				std::shared_ptr<std::string> content = XREXContext::GetInstance().GetResourceLoader().LoadString(fullPath);
				if (content == nullptr)
				{
					XREXContext::GetInstance().GetLogger().BeginLine().Log("FrameBuffer file: ").Log(fullPath).Log(" not found.").EndLine();
					return;
				}
				modificationTime = 0; // TODO get file modification time

				rapidxml::xml_document<> document;
				document.parse<rapidxml::parse_default>(&(*content)[0]);
				rapidxml::xml_node<>* root = document.first_node();

				static std::string const FrameBufferElementName = "FrameBuffer";
				if (root->name() != FrameBufferElementName)
				{
					XREXContext::GetInstance().GetLogger().BeginLine().Log("FrameBuffer element name incorrect: ").Log(root->name()).Log(", should be: ").Log(FrameBufferElementName).Log(".").EndLine();
				}

				static std::string const FrameBufferString = "FrameBuffer";
				static std::string const Name = "Name";

				FrameBufferLayoutDescription::SizeMode sizeMode = FrameBufferLayoutDescription::SizeMode::Sceen;
				Size<uint32, 2> size = Size<uint32, 2>(0, 0);
				floatV2 scaling = floatV2(0, 0);
				static std::string const SizeModeString = "SizeMode";
				static std::string const ScalingString = "Scaling";
				static std::string const SizeString = "Size";

				static std::string const ScreenString = "Screen";
				static std::string const ProportionScreenString = "ProportionScreen";
				static std::string const FixedString = "Fixed";
				for (rapidxml::xml_attribute<>* attribute = root->first_attribute(); attribute != nullptr; attribute = attribute->next_attribute())
				{
					if (attribute->name() == SizeModeString)
					{

						if (attribute->value() == ScreenString)
						{
							sizeMode = FrameBufferLayoutDescription::SizeMode::Sceen;
						}
						else if (attribute->value() == ProportionScreenString)
						{
							sizeMode = FrameBufferLayoutDescription::SizeMode::ProportionSceen;
						}
						else if (attribute->value() == FixedString)
						{
							sizeMode = FrameBufferLayoutDescription::SizeMode::Fixed;
						}
					}
					else if (attribute->name() == ScalingString)
					{
						std::string noUse(attribute->value());
						std::stringstream ss(noUse);
						std::array<float, 2> value;
						ss >> value[0] >> noUse >> value[1];
						scaling = floatV2(value[0], value[1]);
					}
					else if (attribute->name() == SizeString)
					{
						std::string noUse(attribute->value());
						std::stringstream ss(noUse);
						std::array<uint32, 2> value;
						ss >> value[0] >> noUse >> value[1];
						size = Size<uint32, 2>(value[0], value[1]);
					}
					else
					{
						LogUnknownAttribute(FrameBufferString, attribute);
					}
				}

				description = MakeSP<FrameBufferLayoutDescription>(fullPath);

				if (sizeMode == FrameBufferLayoutDescription::SizeMode::ProportionSceen)
				{
					if (scaling == floatV2(0, 0))
					{
						XREXContext::GetInstance().GetLogger().BeginLine().Log("SizeMode: ").Log(ProportionScreenString).Log(" need attribute: ").Log(ScalingString).Log(", format: \"x, y\"").EndLine();
					}
					else
					{
						description->SetSizeMode(sizeMode);
						description->SetSizeScalingToScreen(scaling);
					}
				}
				else if (sizeMode == FrameBufferLayoutDescription::SizeMode::Fixed)
				{
					if (size.X() == 0 || size.Y() == 0)
					{
						XREXContext::GetInstance().GetLogger().BeginLine().Log("SizeMode: ").Log(FixedString).Log(" need attribute: ").Log(SizeString).Log(", format: \"x, y\"").EndLine();
					}
					else
					{
						description->SetSizeMode(sizeMode);
						description->SetSize(size);
					}
				}


				bool allChannelValid = true;
				std::unordered_set<std::string> nameDeclared;
				for (rapidxml::xml_node<>* subNode = root->first_node(); subNode != nullptr; subNode = subNode->next_sibling())
				{
					static std::string const ChannelString = "Channel";
					static std::string const DepthString = "Depth";
					static std::string const StencilString = "Stencil";
					static std::string const DepthStencilString = "DepthStencil";

					static std::string const Format = "Format";

					enum class FrameBufferChannelType
					{
						Channel,
						Depth,
						Stencil,
						DepthStencil,

						Error,
					};
					FrameBufferChannelType channelType;
					if (subNode->name() == ChannelString)
					{
						channelType = FrameBufferChannelType::Channel;
					}
					else if (subNode->name() == DepthString)
					{
						channelType = FrameBufferChannelType::Depth;
					}
					else if (subNode->name() == StencilString)
					{
						channelType = FrameBufferChannelType::Stencil;
					}
					else if (subNode->name() == DepthStencilString)
					{
						channelType = FrameBufferChannelType::DepthStencil;
					}
					else
					{
						channelType = FrameBufferChannelType::Error;
						LogUnknownElement(FrameBufferString, subNode);
						continue;
					}

					std::string channelName;
					TexelFormat format = TexelFormat::TexelFormatCount;

					bool attributeValid = true;
					for (rapidxml::xml_attribute<>* attribute = subNode->first_attribute(); attribute != nullptr; attribute = attribute->next_attribute())
					{
						if (attribute->name() == Name)
						{
							if (channelType == FrameBufferChannelType::Channel)
							{
								channelName = attribute->value();
								auto found = nameDeclared.find(channelName);
								if (found != nameDeclared.end())
								{
									XREXContext::GetInstance().GetLogger().BeginLine().Log("Channel with Name: ").Log(channelName).Log(" already defined in FrameBuffer, ignored.").EndLine();
									attributeValid = false;
								}
								nameDeclared.insert(channelName);
							}
							else
							{
								XREXContext::GetInstance().GetLogger().BeginLine().Log("Invalid attribute Name, only Channel element need Name attribute, element ignored.").EndLine();
								attributeValid = false;
							}
						}
						else if (attribute->name() == Format)
						{
							try
							{
								format = TexelFormatFromString(attribute->value());
							}
							catch (EnumReflectionException const&)
							{
								attributeValid = false;
								LogUnknownAttributeValue(ChannelString, attribute);
							}
						}
						else
						{
							LogUnknownAttribute(ChannelString, attribute);
						}
					}

					if (channelType == FrameBufferChannelType::Channel && channelName.empty())
					{
						XREXContext::GetInstance().GetLogger().LogLine("FrameBuffer Channel attribute Name missing.");
						attributeValid = false;
					}
					if (format == TexelFormat::TexelFormatCount)
					{
						XREXContext::GetInstance().GetLogger().LogLine("FrameBuffer element attribute Type missing.");
						attributeValid = false;
					}
					if (attributeValid)
					{
						if (channelType == FrameBufferChannelType::Channel)
						{
							description->AddChannel(FrameBufferLayoutDescription::ChannelDescription(channelName, format));
						}
						else if (channelType == FrameBufferChannelType::Depth)
						{
							description->SetDepth(format);
						}
						else if (channelType == FrameBufferChannelType::Stencil)
						{
							description->SetStencil(format);
						}
						else if (channelType == FrameBufferChannelType::DepthStencil)
						{
							description->SetDepthStencil(format);
						}
						else
						{
							// error channel type, ignore
						}
					}
					else
					{
						allChannelValid = false;
					}
				}

				cache.loadedFrameBufferDescriptions.insert(std::make_pair(fullPath, FrameBufferDescriptionAndModificationTime(description, modificationTime)));
			}

			FrameBufferLayoutDescriptionSP GetFrameBufferDescription()
			{
				return description;
			}
		};

		// It's so painful to do this without reflection...
		struct TechniqueInformationGenerator
		{
			TechniqueBuildingInformationSP information;
			double modificationTime;

			LoadCache& cache;

			std::string const& fullPath;
			std::string directoryPath;
			std::string fileName;

			TechniqueInformationGenerator(LoadCache& cache, std::string const& fullPath)
				: cache(cache), fullPath(fullPath)
			{
				std::tr2::sys::path fullTechniquePath(fullPath);
				directoryPath = fullTechniquePath.parent_path().string() + "/";
				fileName = fullTechniquePath.filename();

				double currentFileModificationTime = 0; // TODO how to get modification time? how to hot reload?

				auto found = cache.loadedTechniqueInformations.find(fullPath);
				if (found != cache.loadedTechniqueInformations.end())
				{
					if (found->second.modificationTime == currentFileModificationTime)
					{
						// already loaded and not modified.
						information = found->second.information;
						modificationTime = found->second.modificationTime;
						return;
					}
					else
					{
						// TODO should be triggered by file modification, update technique, internal replace
					}
				}


				std::shared_ptr<std::string> content = XREXContext::GetInstance().GetResourceLoader().LoadString(fullPath);
				if (content == nullptr)
				{
					XREXContext::GetInstance().GetLogger().BeginLine().Log("Technique file: ").Log(fullPath).Log(" not found.").EndLine();
					return;
				}
				modificationTime = 0; // TODO get file modification time

				static std::unordered_map<std::string, void (TechniqueInformationGenerator::*)(rapidxml::xml_node<>* node)> const TechniqueElementHandler = []
				{
					std::remove_const<decltype(TechniqueElementHandler)>::type temp;
					temp["Include"] = &TechniqueInformationGenerator::HandleInclude;
					temp["Sampler"] = &TechniqueInformationGenerator::HandleSampler;
					temp["Texture"] = &TechniqueInformationGenerator::HandleTexture;
					temp["Image"] = &TechniqueInformationGenerator::HandleImage;
					temp["UniformBuffer"] = &TechniqueInformationGenerator::HandleUniformBuffer;
					temp["AtomicCounterBuffer"] = &TechniqueInformationGenerator::HandleAtomicCounterBuffer;
					temp["ShaderStorageBuffer"] = &TechniqueInformationGenerator::HandleShaderStorageBuffer;
					temp["FrameBuffer"] = &TechniqueInformationGenerator::HandleFrameBuffer;
					temp["AttributeInput"] = &TechniqueInformationGenerator::HandleAttributeInput;
					temp["Code"] = &TechniqueInformationGenerator::HandleCode;
					temp["VertexShader"] = &TechniqueInformationGenerator::HandleVertexShader;
					temp["TessellationControlShader"] = &TechniqueInformationGenerator::HandleTessellationControlShader;
					temp["TessellationEvaluationShader"] = &TechniqueInformationGenerator::HandleTessellationEvaluationShader;
					temp["GeometryShader"] = &TechniqueInformationGenerator::HandleGeometryShader;
					temp["FragmentShader"] = &TechniqueInformationGenerator::HandleFragmentShader;
					temp["ComputeShader"] = &TechniqueInformationGenerator::HandleComputeShader;
					temp["RasterizerState"] = &TechniqueInformationGenerator::HandleRasterizerState;
					temp["DepthStencilState"] = &TechniqueInformationGenerator::HandleDepthStencilState;
					temp["BlendState"] = &TechniqueInformationGenerator::HandleBlendState;
					return temp;
				} ();

				rapidxml::xml_document<> document;
				document.parse<rapidxml::parse_default>(&(*content)[0]);
				rapidxml::xml_node<>* root = document.first_node();

				static std::string const TechniqueElementName = "Technique";
				if (root->name() != TechniqueElementName)
				{
					XREXContext::GetInstance().GetLogger().BeginLine().Log("Technique element name incorrect: ").Log(root->name()).Log(", should be: ").Log(TechniqueElementName).Log(".").EndLine();
				}

				// currently no valid attribute
				for (rapidxml::xml_attribute<>* attribute = root->first_attribute(); attribute != nullptr; attribute = attribute->next_attribute())
				{
					LogUnknownAttribute(TechniqueElementName, attribute);
				}

				information = MakeSP<TechniqueBuildingInformation>(fullPath);

				// root node sub nodes
				for (rapidxml::xml_node<>* node = root->first_node(); node != nullptr; node = node->next_sibling())
				{
					auto found = TechniqueElementHandler.find(node->name());
					if (found == TechniqueElementHandler.end())
					{
						LogUnknownElement(TechniqueElementName, node);
					}
					else
					{
						XREX_POINTER_CALL_MEMBER_FUNCTION(this, found->second)(node);
					}
				}

				cache.loadedTechniqueInformations.insert(std::make_pair(fullPath, TechniqueInformationAndModificationTime(information, modificationTime)));
			}

			void HandleInclude(rapidxml::xml_node<>* node)
			{
				TechniqueBuildingInformationSP includedInformation;

				int fileOrSystemCount = 0;
				for (rapidxml::xml_attribute<>* attribute = node->first_attribute(); attribute != nullptr; attribute = attribute->next_attribute())
				{
					static std::string const File = "File";
					static std::string const System = "System";
					if (attribute->name() == File)
					{
						if (fileOrSystemCount >= 1)
						{
							XREXContext::GetInstance().GetLogger().BeginLine().Log("Include attribute already have a include source.").EndLine();
							continue;
						}
						std::string includeTechniqueFullPath = directoryPath + attribute->value();
						auto found = cache.loadedTechniqueInformations.find(includeTechniqueFullPath);
						if (found == cache.loadedTechniqueInformations.end())
						{
							TechniqueInformationGenerator generator(cache, includeTechniqueFullPath);
							includedInformation = generator.GetTechniqueBuildingInformation();
						}
						else
						{
							includedInformation = found->second.information;
						}
						fileOrSystemCount += 1;
					}
					else if (attribute->name() == System)
					{
						if (fileOrSystemCount >= 1)
						{
							XREXContext::GetInstance().GetLogger().BeginLine().Log("Include attribute already have a include source.").EndLine();
							continue;
						}
						ISystemTechniqueFactory* factory = XREXContext::GetInstance().GetRenderingEngine().GetSystemTechniqueFactory(attribute->value());
						if (factory == nullptr)
						{
							XREXContext::GetInstance().GetLogger().BeginLine().Log("Include system technique: ").Log(attribute->value()).Log(", not found.").EndLine();
						}
						else
						{
							includedInformation = factory->GetTechniqueInformationToInclude();
						}
						fileOrSystemCount += 1;
					}
					else
					{
						LogUnknownAttribute("Include", attribute);
					}
				}
				for (rapidxml::xml_node<>* subNode = node->first_node(); subNode != nullptr; subNode = subNode->next_sibling())
				{
					XREXContext::GetInstance().GetLogger().BeginLine().Log("Include should not have sub element, but: ").Log(subNode->name()).Log(" exist.").EndLine();
				}

				if (fileOrSystemCount == 0)
				{
					XREXContext::GetInstance().GetLogger().BeginLine().Log("Include attribute do not specify which to include.").EndLine();
				}

				if (includedInformation != nullptr)
				{
					information->AddInclude(includedInformation);
				}
			}

			void HandleSampler(rapidxml::xml_node<>* node)
			{
				static std::string const SamplerString = "Sampler";
				static std::string const StateString = "State";
				std::string samplerName;
				for (rapidxml::xml_attribute<>* attribute = node->first_attribute(); attribute != nullptr; attribute = attribute->next_attribute())
				{
					static std::string const Name = "Name";
					if (attribute->name() == Name)
					{
						samplerName = attribute->value();
					}
					else
					{
						LogUnknownAttribute(SamplerString, attribute);
					}
				}

				struct SamplerStateSetter
				{
					SamplerState& state;
					explicit SamplerStateSetter(SamplerState& state)
						: state(state)
					{
					}
					void Set(rapidxml::xml_attribute<>* attribute)
					{
						static std::unordered_map<std::string, void (SamplerStateSetter::*)(rapidxml::xml_attribute<>* attribute)> const Setters = []
						{
							std::remove_const<decltype(Setters)>::type temp;
							temp["BorderColor"] = &SamplerStateSetter::HandleBorderColor;
							temp["AddressingModeS"] = &SamplerStateSetter::HandleAddressingModeS;
							temp["AddressingModeT"] = &SamplerStateSetter::HandleAddressingModeT;
							temp["AddressingModeR"] = &SamplerStateSetter::HandleAddressingModeR;
							temp["MinFilterMode"] = &SamplerStateSetter::HandleMinFilterMode;
							temp["MagFilterMode"] = &SamplerStateSetter::HandleMagFilterMode;
							temp["MaxAnisotropy"] = &SamplerStateSetter::HandleMaxAnisotropy;
							temp["MinLOD"] = &SamplerStateSetter::HandleMinLOD;
							temp["MaxLOD"] = &SamplerStateSetter::HandleMaxLOD;
							temp["MipmapLODBias"] = &SamplerStateSetter::HandleMipmapLODBias;
							temp["CompareEnable"] = &SamplerStateSetter::HandleCompareEnable;
							temp["CompareFunction"] = &SamplerStateSetter::HandleCompareFunction;
							return temp;
						} ();
						auto found = Setters.find(attribute->name());
						if (found == Setters.end())
						{
							LogUnknownAttribute(SamplerString, attribute);
						}
						else
						{
							XREX_POINTER_CALL_MEMBER_FUNCTION(this, found->second)(attribute);
						}
					}

					void HandleBorderColor(rapidxml::xml_attribute<>* attribute)
					{
						std::string noUse(attribute->value());
						std::stringstream ss(noUse);
						std::array<float, 4> value;
						value.assign(0);
						ss >> value[0] >> noUse >> value[1] >> noUse >> value[2] >> noUse >> value[3];
						// XREXContext::GetInstance().GetLogger().BeginLine().Log("Color format error: \'").Log(colorString).Log("\", should be \"x,x,x,x\".").EndLine();
						state.borderColor = Color(value[0], value[1], value[2], value[3]);
					}

					std::pair<SamplerState::TextureAddressingMode, bool> GetTextureAddressingMode(std::string const value)
					{
						static std::unordered_map<std::string, SamplerState::TextureAddressingMode> const Modes = []
						{
							std::remove_const<decltype(Modes)>::type temp;
							temp["Repeat"] = SamplerState::TextureAddressingMode::Repeat;
							temp["MirroredRepeat"] = SamplerState::TextureAddressingMode::MirroredRepeat;
							temp["ClampToEdge"] = SamplerState::TextureAddressingMode::ClampToEdge;
							temp["ClampToBorder"] = SamplerState::TextureAddressingMode::ClampToBorder;
							return temp;
						} ();
						auto found = Modes.find(value);
						if (found != Modes.end())
						{
							return std::make_pair(found->second, true);
						}
						return std::make_pair(SamplerState::TextureAddressingMode::Repeat, false);
					}

					void HandleAddressingModeS(rapidxml::xml_attribute<>* attribute)
					{
						std::pair<SamplerState::TextureAddressingMode, bool> mode = GetTextureAddressingMode(attribute->value());
						if (mode.second)
						{
							state.addressingModeS = mode.first;
						}
						else
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}
					void HandleAddressingModeT(rapidxml::xml_attribute<>* attribute)
					{
						std::pair<SamplerState::TextureAddressingMode, bool> mode = GetTextureAddressingMode(attribute->value());
						if (mode.second)
						{
							state.addressingModeT = mode.first;
						}
						else
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}
					void HandleAddressingModeR(rapidxml::xml_attribute<>* attribute)
					{
						std::pair<SamplerState::TextureAddressingMode, bool> mode = GetTextureAddressingMode(attribute->value());
						if (mode.second)
						{
							state.addressingModeR = mode.first;
						}
						else
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					std::pair<SamplerState::TextureFilterMode, bool> GetTextureFilterMode(std::string const value)
					{
						static std::unordered_map<std::string, SamplerState::TextureFilterMode> const Modes = []
						{
							std::remove_const<decltype(Modes)>::type temp;
							temp["Nearest"] = SamplerState::TextureFilterMode::Nearest;
							temp["Linear"] = SamplerState::TextureFilterMode::Linear;
							temp["NearestMipmapNearest"] = SamplerState::TextureFilterMode::NearestMipmapNearest;
							temp["LinearMipmapNearest"] = SamplerState::TextureFilterMode::LinearMipmapNearest;
							temp["NearestMipmapLinear"] = SamplerState::TextureFilterMode::NearestMipmapLinear;
							temp["LinearMipmapLinear"] = SamplerState::TextureFilterMode::LinearMipmapLinear;
							temp["Anisotropic"] = SamplerState::TextureFilterMode::Anisotropic;
							return temp;
						} ();
						auto found = Modes.find(value);
						if (found != Modes.end())
						{
							return std::make_pair(found->second, true);
						}
						return std::make_pair(SamplerState::TextureFilterMode::Nearest, false);
					}

					void HandleMinFilterMode(rapidxml::xml_attribute<>* attribute)
					{
						std::pair<SamplerState::TextureFilterMode, bool> mode = GetTextureFilterMode(attribute->value());
						if (mode.second)
						{
							state.minFilterMode = mode.first;
						}
						else
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}
					void HandleMagFilterMode(rapidxml::xml_attribute<>* attribute)
					{
						std::pair<SamplerState::TextureFilterMode, bool> mode = GetTextureFilterMode(attribute->value());
						if (mode.second)
						{
							state.magFilterMode = mode.first;
						}
						else
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}
					void HandleMaxAnisotropy(rapidxml::xml_attribute<>* attribute)
					{
						state.maxAnisotropy = static_cast<uint8>(std::atoi(attribute->value()));
					}
					void HandleMinLOD(rapidxml::xml_attribute<>* attribute)
					{
						state.minLOD = static_cast<float>(std::atof(attribute->value()));
					}
					void HandleMaxLOD(rapidxml::xml_attribute<>* attribute)
					{
						state.maxLOD = static_cast<float>(std::atof(attribute->value()));
					}
					void HandleMipmapLODBias(rapidxml::xml_attribute<>* attribute)
					{
						state.mipmapLODBias = static_cast<float>(std::atof(attribute->value()));
					}

					void HandleCompareEnable(rapidxml::xml_attribute<>* attribute)
					{
						try
						{
							state.compareEnable = BoolFromString(attribute->value());
						}
						catch (EnumReflectionException const&)
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					std::pair<SamplerState::CompareFunction, bool> GetCompareFunction(std::string const value)
					{
						static std::unordered_map<std::string, SamplerState::CompareFunction> const Functions = []
						{
							std::remove_const<decltype(Functions)>::type temp;
							temp["AlwaysFail"] = SamplerState::CompareFunction::AlwaysFail;
							temp["AlwaysPass"] = SamplerState::CompareFunction::AlwaysPass;
							temp["Equal"] = SamplerState::CompareFunction::Equal;
							temp["Greater"] = SamplerState::CompareFunction::Greater;
							temp["GreaterEqual"] = SamplerState::CompareFunction::GreaterEqual;
							temp["Less"] = SamplerState::CompareFunction::Less;
							temp["LessEqual"] = SamplerState::CompareFunction::LessEqual;
							temp["NotEqual"] = SamplerState::CompareFunction::NotEqual;
							return temp;
						} ();
						auto found = Functions.find(value);
						if (found != Functions.end())
						{
							return std::make_pair(found->second, true);
						}
						return std::make_pair(SamplerState::CompareFunction::AlwaysFail, false);
					}
					void HandleCompareFunction(rapidxml::xml_attribute<>* attribute)
					{
						std::pair<SamplerState::CompareFunction, bool> function = GetCompareFunction(attribute->value());
						if (function.second)
						{
							state.compareFunction = function.first;
						}
						else
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}
				};
				SamplerState state;
				SamplerStateSetter setter(state);

				for (rapidxml::xml_node<>* subNode = node->first_node(); subNode != nullptr; subNode = subNode->next_sibling())
				{
					if (subNode->name() == StateString)
					{
						for (rapidxml::xml_attribute<>* attribute = subNode->first_attribute(); attribute != nullptr; attribute = attribute->next_attribute())
						{
							setter.Set(attribute);
						}
					}
					else
					{
						LogUnknownElement(SamplerString, subNode);
					}
				}

				information->AddSamplerState(samplerName, state);
			}

			void HandleTexture(rapidxml::xml_node<>* node)
			{
				std::string channel;
				Texture::TextureType textureType = Texture::TextureType::TextureTypeCount;
				Texture::TexelType texelType = Texture::TexelType::TexelTypeCount;
				std::string samplerName;

				static std::string const TextureString = "Texture";

				static std::string const Name = "Name";
				static std::string const TextureType = "TextureType";
				static std::string const TexelType = "TexelType";
				static std::string const Sampler = "Sampler";

				for (rapidxml::xml_attribute<>* attribute = node->first_attribute(); attribute != nullptr; attribute = attribute->next_attribute())
				{
					if (attribute->name() == Name)
					{
						channel = attribute->value();
					}
					else if (attribute->name() == TextureType)
					{
						static std::unordered_map<std::string, Texture::TextureType> const TextureTypes = []
						{
							std::remove_const<decltype(TextureTypes)>::type temp;
							temp["Texture1D"] = Texture::TextureType::Texture1D;
							temp["Texture2D"] = Texture::TextureType::Texture2D;
							temp["Texture3D"] = Texture::TextureType::Texture3D;
							temp["TextureCube"] = Texture::TextureType::TextureCube;
							temp["TextureBuffer"] = Texture::TextureType::TextureBuffer;
							return temp;
						} ();
						auto found = TextureTypes.find(attribute->value());
						if (found != TextureTypes.end())
						{
							textureType = found->second;
						}
						else
						{
							LogUnknownAttributeValue(TextureString, attribute);
						}
					}
					else if (attribute->name() == TexelType)
					{
						static std::unordered_map<std::string, Texture::TexelType> const TexelTypes = []
						{
							std::remove_const<decltype(TexelTypes)>::type temp;
							temp["IntV1"] = Texture::TexelType::IntV1;
							temp["IntV2"] = Texture::TexelType::IntV2;
							temp["IntV3"] = Texture::TexelType::IntV3;
							temp["IntV4"] = Texture::TexelType::IntV4;
							temp["UintV1"] = Texture::TexelType::UintV1;
							temp["UintV2"] = Texture::TexelType::UintV2;
							temp["UintV3"] = Texture::TexelType::UintV3;
							temp["UintV4"] = Texture::TexelType::UintV4;
							temp["FloatV1"] = Texture::TexelType::FloatV1;
							temp["FloatV2"] = Texture::TexelType::FloatV2;
							temp["FloatV3"] = Texture::TexelType::FloatV3;
							temp["FloatV4"] = Texture::TexelType::FloatV4;
							temp["Shadow"] = Texture::TexelType::Shadow;
							return temp;
						} ();
						auto found = TexelTypes.find(attribute->value());
						if (found != TexelTypes.end())
						{
							texelType = found->second;
						}
						else
						{
							LogUnknownAttributeValue(TextureString, attribute);
						}
					}
					else if (attribute->name() == Sampler)
					{
						samplerName = attribute->value();
					}
					else
					{
						LogUnknownAttribute(TextureString, attribute);
					}
				}

				bool valid = true;
				if (channel.empty())
				{
					XREXContext::GetInstance().GetLogger().LogLine("Texture attribute Name missing.");
					valid = false;
				}
				if (textureType == Texture::TextureType::TextureTypeCount)
				{
					XREXContext::GetInstance().GetLogger().LogLine("Texture attribute TextureType missing.");
					valid = false;
				}
				if (texelType == Texture::TexelType::TexelTypeCount)
				{
					XREXContext::GetInstance().GetLogger().LogLine("Texture attribute TexelType missing.");
					valid = false;
				}
				if (samplerName.empty())
				{
					XREXContext::GetInstance().GetLogger().LogLine("Texture attribute Sampler missing.");
					valid = false;
				}
				if (valid)
				{
					information->AddTextureInformation(TextureInformation(channel, textureType, texelType, samplerName));
				}
			}

			void HandleImage(rapidxml::xml_node<>* node)
			{
				std::string channel;
				TextureImage::ImageType imageType = TextureImage::ImageType::ImageTypeCount;
				TexelFormat texelFormat = TexelFormat::TexelFormatCount;
				AccessType accessType = AccessType::ReadWrite;

				static std::string const ImageString = "Image";

				static std::string const Name = "Name";
				static std::string const ImageType = "ImageType";
				static std::string const TexelFormatString = "TexelFormat";
				static std::string const AccessTypeString = "AccessType";

				for (rapidxml::xml_attribute<>* attribute = node->first_attribute(); attribute != nullptr; attribute = attribute->next_attribute())
				{
					if (attribute->name() == Name)
					{
						channel = attribute->value();
					}
					else if (attribute->name() == ImageType)
					{
						static std::unordered_map<std::string, TextureImage::ImageType> const ImageTypes = []
						{
							std::remove_const<decltype(ImageTypes)>::type temp;
							temp["Image1D"] = TextureImage::ImageType::Image1D;
							temp["Image2D"] = TextureImage::ImageType::Image2D;
							temp["Image3D"] = TextureImage::ImageType::Image3D;
							temp["ImageCube"] = TextureImage::ImageType::ImageCube;
							temp["ImageBuffer"] = TextureImage::ImageType::ImageBuffer;
							return temp;
						} ();
						auto found = ImageTypes.find(attribute->value());
						if (found != ImageTypes.end())
						{
							imageType = found->second;
						}
						else
						{
							LogUnknownAttributeValue(ImageString, attribute);
						}
					}
					else if (attribute->name() == TexelFormatString)
					{
						try
						{
							texelFormat = TexelFormatFromString(attribute->value());
						}
						catch (EnumReflectionException const&)
						{
							LogUnknownAttributeValue(TexelFormatString, attribute);
						}
					}
					else if (attribute->name() == AccessTypeString)
					{
						static std::unordered_map<std::string, AccessType> const AccessTypes = []
						{
							std::remove_const<decltype(AccessTypes)>::type temp;
							temp["ReadOnly"] = AccessType::ReadOnly;
							temp["WriteOnly"] = AccessType::WriteOnly;
							temp["ReadWrite"] = AccessType::ReadWrite;
							return temp;
						} ();
						auto found = AccessTypes.find(attribute->value());
						if (found != AccessTypes.end())
						{
							accessType = found->second;
						}
						else
						{
							LogUnknownAttributeValue(AccessTypeString, attribute);
						}

					}
					else
					{
						LogUnknownAttribute(ImageString, attribute);
					}
				}

				bool valid = true;
				if (channel.empty())
				{
					XREXContext::GetInstance().GetLogger().LogLine("Image attribute Name missing.");
					valid = false;
				}
				if (imageType == TextureImage::ImageType::ImageTypeCount)
				{
					XREXContext::GetInstance().GetLogger().LogLine("Image attribute ImageType missing.");
					valid = false;
				}
				if (texelFormat == TexelFormat::TexelFormatCount)
				{
					XREXContext::GetInstance().GetLogger().LogLine("Image attribute TexelFormat missing.");
					valid = false;
				}
				if (valid)
				{
					information->AddImageInformation(ImageInformation(channel, imageType, texelFormat, accessType));
				}
			}

			void HandleShaderResourceBuffer(rapidxml::xml_node<>* node, BufferView::BufferType bufferType, std::string const& BufferTypeName, void (TechniqueBuildingInformation::*addBufferInformationFunction)(BufferInformation const& information))
			{
				static std::string const Name = "Name";
				static std::string const ShaderInstanceName = "ShaderInstanceName";

				std::string bufferName;
				std::string shaderInstanceName;

				for (rapidxml::xml_attribute<>* attribute = node->first_attribute(); attribute != nullptr; attribute = attribute->next_attribute())
				{
					if (attribute->name() == Name)
					{
						bufferName = attribute->value();
					}
					else if (attribute->name() == ShaderInstanceName)
					{
						shaderInstanceName = attribute->value();
					}
					else
					{
						LogUnknownAttribute(BufferTypeName, attribute);
					}
				}

				bool bufferValid = true;
				if (bufferName.empty())
				{
					XREXContext::GetInstance().GetLogger().BeginLine().Log(BufferTypeName).Log(" attribute Name missing.").EndLine();
					bufferValid = false;
				}

				std::vector<VariableInformation const> variableInformations;
				std::unordered_set<std::string> nameDeclared;

				for (rapidxml::xml_node<>* subNode = node->first_node(); subNode != nullptr; subNode = subNode->next_sibling())
				{
					static std::string const VariableString = "Variable";

					static std::string const Type = "Type";

					if (subNode->name() != VariableString)
					{
						LogUnknownElement(BufferTypeName, subNode);
						continue;
					}

					std::string variableName;
					ElementType type = ElementType::ElementTypeCount;

					bool attributeValid = true;
					for (rapidxml::xml_attribute<>* attribute = subNode->first_attribute(); attribute != nullptr; attribute = attribute->next_attribute())
					{
						if (attribute->name() == Name)
						{
							variableName = attribute->value();
							auto found = nameDeclared.find(variableName);
							if (found != nameDeclared.end())
							{
								XREXContext::GetInstance().GetLogger().BeginLine().Log("Variable with Name: ").Log(variableName).Log(" already defined in ").Log(BufferTypeName).Log(": ").Log(bufferName).Log(", ignored.").EndLine();
								attributeValid = false;
							}
							nameDeclared.insert(variableName);
						}
						else if (attribute->name() == Type)
						{
							try
							{
								type = ElementTypeFromString(attribute->value());
							}
							catch (EnumReflectionException const&)
							{
								attributeValid = false;
								LogUnknownAttributeValue(VariableString, attribute);
							}
						}
						else
						{
							LogUnknownAttribute(VariableString, attribute);
						}
					}

					if (variableName.empty())
					{
						XREXContext::GetInstance().GetLogger().BeginLine().Log(BufferTypeName).Log(" Variable attribute Name missing.").EndLine();
						attributeValid = false;
					}
					if (type == ElementType::ElementTypeCount)
					{
						XREXContext::GetInstance().GetLogger().BeginLine().Log(BufferTypeName).Log(" Variable attribute Type missing.").EndLine();
						attributeValid = false;
					}
					if (attributeValid)
					{
						variableInformations.push_back(VariableInformation(variableName, type));
					}
				}

				if (bufferValid)
				{
					XREX_OBJECT_CALL_MEMBER_FUNCTION(*information, addBufferInformationFunction)(BufferInformation(bufferName, shaderInstanceName, bufferType, std::move(variableInformations)));
				}
			}

			void HandleUniformBuffer(rapidxml::xml_node<>* node)
			{
				static std::string const BufferTypeString = "UniformBuffer";
				HandleShaderResourceBuffer(node, BufferView::BufferType::Uniform, BufferTypeString, &TechniqueBuildingInformation::AddUniformBufferInformation);
			}

			void HandleAtomicCounterBuffer(rapidxml::xml_node<>* node)
			{
				static std::string const BufferTypeString = "AtomicCounterBuffer";
				HandleShaderResourceBuffer(node, BufferView::BufferType::AtomicCounter, BufferTypeString, &TechniqueBuildingInformation::AddAtomicCounterBufferInformation);
			}

			void HandleShaderStorageBuffer(rapidxml::xml_node<>* node)
			{
				static std::string const BufferTypeString = "ShaderStorageBuffer";
				HandleShaderResourceBuffer(node, BufferView::BufferType::ShaderStorage, BufferTypeString, &TechniqueBuildingInformation::AddShaderStorageBufferInformation);
			}

			void HandleFrameBuffer(rapidxml::xml_node<>* node)
			{ // no need to check if it is already exist, because framebuffer must be defined in a technique file and technique loader will handle it.
				static std::string const FrameBufferString = "FrameBuffer";

				static std::string const XMLFile = "XMLFile";
				static std::string const System = "System";
				static std::string const Default = "Default";

				bool systemFrameBuffer = false;
				std::string fileName;
				std::string fullFileName;

				bool valid = true;

				for (rapidxml::xml_attribute<>* attribute = node->first_attribute(); attribute != nullptr; attribute = attribute->next_attribute())
				{
					if (attribute->name() == XMLFile)
					{
						fileName = attribute->value();
					}
					else if (attribute->name() == System)
					{
						std::string systemBufferName = attribute->value();
						systemFrameBuffer = true;
						if (systemBufferName != Default)
						{
							valid = false;
							XREXContext::GetInstance().GetLogger().BeginLine().Log("FrameBuffer attribute System value: ").Log(systemBufferName).Log(", but must be: ").Log(Default).Log(".");
						}
					}
					else
					{
						LogUnknownAttribute(FrameBufferString, attribute);
					}
				}

				if (!systemFrameBuffer)
				{
					if (fileName.empty())
					{
						XREXContext::GetInstance().GetLogger().LogLine("FrameBuffer attribute Name missing.");
						valid = false;
					}
					else
					{
						fullFileName = directoryPath + fileName;
					}
				}

				if (valid)
				{
					if (!systemFrameBuffer)
					{
						FrameBufferDescriptionGenerator generator(cache, fullFileName);
						information->SetFrameBufferDescription(generator.GetFrameBufferDescription());
					}
					else
					{
						information->SetFrameBufferDescription(XREXContext::GetInstance().GetRenderingEngine().GetDefaultFrameBuffer()->GetLayoutDescription());
					}
				}

			}

			void HandleAttributeInput(rapidxml::xml_node<>* node)
			{
				std::string channel;
				ElementType type;

				static std::string const AttributeInputString = "AttributeInput";

				static std::string const Name = "Name";
				static std::string const Type = "Type";

				for (rapidxml::xml_attribute<>* attribute = node->first_attribute(); attribute != nullptr; attribute = attribute->next_attribute())
				{
					if (attribute->name() == Name)
					{
						channel = attribute->value();
					}
					else if (attribute->name() == Type)
					{
						try
						{
							type = ElementTypeFromString(attribute->value());
						}
						catch (EnumReflectionException const&)
						{
							LogUnknownAttributeValue(AttributeInputString, attribute);
						}
					}
					else
					{
						LogUnknownAttribute(AttributeInputString, attribute);
					}
				}

				bool valid = true;
				if (channel.empty())
				{
					XREXContext::GetInstance().GetLogger().LogLine("AttributeInput attribute Name missing.");
					valid = false;
				}
				if (type == ElementType::ElementTypeCount)
				{
					XREXContext::GetInstance().GetLogger().LogLine("AttributeInput attribute Type missing.");
					valid = false;
				}
				if (valid)
				{
					information->AddAttributeInputInformation(AttributeInputInformation(channel, type));
				}
			}

			std::shared_ptr<std::string> GenerateCode(rapidxml::xml_node<>* node)
			{
				static std::string const CodeString = "Code";
				static std::string const File = "File";

				std::shared_ptr<std::string> code;
				bool fromFile = false;
				std::string fileName;
				std::string fullFileName;
				for (rapidxml::xml_attribute<>* attribute = node->first_attribute(); attribute != nullptr; attribute = attribute->next_attribute())
				{
					if (attribute->name() == File)
					{
						fromFile = true;
						fileName = attribute->value();
					}
					else
					{
						LogUnknownAttribute(CodeString, attribute);
					}
				}

				if (fromFile)
				{
					fullFileName = directoryPath + fileName;
					code = XREXContext::GetInstance().GetResourceLoader().LoadString(fullFileName);
					if (code == nullptr)
					{
						XREXContext::GetInstance().GetLogger().LogLine("Code element from file: \"").Log(fileName).Log("\", but file: \"").Log(fullFileName).Log("\" not found.");
					}
				}

				bool hasCode = fromFile;
				for (rapidxml::xml_node<>* subNode = node->first_node(); subNode != nullptr; subNode = subNode->next_sibling())
				{
					if (subNode->type() != rapidxml::node_type::node_cdata)
					{
						LogUnknownElement(CodeString, subNode);
					}
					else
					{
						if (!hasCode)
						{
							code = MakeSP<std::string>(subNode->value());
							hasCode = true;
						}
						else
						{
							XREXContext::GetInstance().GetLogger().LogLine("Code element can only have one CDATA node, more ignored.");
						}
					}
				}
				if (!hasCode)
				{
					code = MakeSP<std::string>(node->value());
				}
				return code;
			}

			void HandleCode(rapidxml::xml_node<>* node)
			{
				information->AddCommonCode(GenerateCode(node));
			}

			void HandleStageShader(rapidxml::xml_node<>* node, ShaderObject::ShaderType stage, std::string const& stageString)
			{
				for (rapidxml::xml_attribute<>* attribute = node->first_attribute(); attribute != nullptr; attribute = attribute->next_attribute())
				{
					LogUnknownAttribute(stageString, attribute);
				}
				for (rapidxml::xml_node<>* subNode = node->first_node(); subNode != nullptr; subNode = subNode->next_sibling())
				{
					static std::string const CodeString = "Code";
					if (subNode->name() == CodeString)
					{
						information->AddStageCode(stage, GenerateCode(subNode));
					}
					else
					{
						LogUnknownElement(CodeString, subNode);
					}
				}
			}

			void HandleVertexShader(rapidxml::xml_node<>* node)
			{
				static std::string const StageShaderString = "VertexShader";
				HandleStageShader(node, ShaderObject::ShaderType::VertexShader, StageShaderString);
			}

			void HandleTessellationControlShader(rapidxml::xml_node<>* node)
			{
				static std::string const StageShaderString = "TessellationControlShader";
				HandleStageShader(node, ShaderObject::ShaderType::TessellationControlShader, StageShaderString);
			}

			void HandleTessellationEvaluationShader(rapidxml::xml_node<>* node)
			{
				static std::string const StageShaderString = "TessellationEvaluationShader";
				HandleStageShader(node, ShaderObject::ShaderType::TessellationEvaluationShader, StageShaderString);
			}

			void HandleGeometryShader(rapidxml::xml_node<>* node)
			{
				static std::string const StageShaderString = "GeometryShader";
				HandleStageShader(node, ShaderObject::ShaderType::GeometryShader, StageShaderString);
			}

			void HandleFragmentShader(rapidxml::xml_node<>* node)
			{
				static std::string const StageShaderString = "FragmentShader";
				HandleStageShader(node, ShaderObject::ShaderType::FragmentShader, StageShaderString);
			}

			void HandleComputeShader(rapidxml::xml_node<>* node)
			{
				static std::string const StageShaderString = "ComputeShader";
				HandleStageShader(node, ShaderObject::ShaderType::ComputeShader, StageShaderString);
			}

			void HandleRasterizerState(rapidxml::xml_node<>* node)
			{
				static std::string const RasterizerString = "RasterizerState";
				static std::string const StateString = "State";
				for (rapidxml::xml_attribute<>* attribute = node->first_attribute(); attribute != nullptr; attribute = attribute->next_attribute())
				{
					LogUnknownAttribute(RasterizerString, attribute);
				}

				struct RasterizerStateSetter
				{
					RasterizerState& state;
					explicit RasterizerStateSetter(RasterizerState& state)
						: state(state)
					{
					}
					void Set(rapidxml::xml_attribute<>* attribute)
					{
						static std::unordered_map<std::string, void (RasterizerStateSetter::*)(rapidxml::xml_attribute<>* attribute)> const Setters = []
						{
							std::remove_const<decltype(Setters)>::type temp;
							temp["PolygonMode"] = &RasterizerStateSetter::HandlePolygonMode;
							temp["CullMode"] = &RasterizerStateSetter::HandleCullMode;
							temp["FrontFaceCCW"] = &RasterizerStateSetter::HandleFrontFaceCCW;
							temp["MultisampleEnable"] = &RasterizerStateSetter::HandleMultisampleEnable;
							return temp;
						} ();
						auto found = Setters.find(attribute->name());
						if (found == Setters.end())
						{
							LogUnknownAttribute(RasterizerString, attribute);
						}
						else
						{
							XREX_POINTER_CALL_MEMBER_FUNCTION(this, found->second)(attribute);
						}
					}

					std::pair<RasterizerState::PolygonMode, bool> GetPolygonMode(std::string const value)
					{
						static std::unordered_map<std::string, RasterizerState::PolygonMode> const PolygonModes = []
						{
							std::remove_const<decltype(PolygonModes)>::type temp;
							temp["Fill"] = RasterizerState::PolygonMode::Fill;
							temp["Line"] = RasterizerState::PolygonMode::Line;
							temp["Point"] = RasterizerState::PolygonMode::Point;
							return temp;
						} ();
						auto found = PolygonModes.find(value);
						if (found != PolygonModes.end())
						{
							return std::make_pair(found->second, true);
						}
						return std::make_pair(RasterizerState::PolygonMode::Fill, false);
					}

					void HandlePolygonMode(rapidxml::xml_attribute<>* attribute)
					{
						std::pair<RasterizerState::PolygonMode, bool> mode = GetPolygonMode(attribute->value());
						if (mode.second)
						{
							state.polygonMode = mode.first;
						}
						else
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					std::pair<RasterizerState::CullMode, bool> GetCullMode(std::string const value)
					{
						static std::unordered_map<std::string, RasterizerState::CullMode> const CullModes = []
						{
							std::remove_const<decltype(CullModes)>::type temp;
							temp["None"] = RasterizerState::CullMode::None;
							temp["Back"] = RasterizerState::CullMode::Back;
							temp["Front"] = RasterizerState::CullMode::Front;
							return temp;
						} ();
						auto found = CullModes.find(value);
						if (found != CullModes.end())
						{
							return std::make_pair(found->second, true);
						}
						return std::make_pair(RasterizerState::CullMode::None, false);
					}

					void HandleCullMode(rapidxml::xml_attribute<>* attribute)
					{
						std::pair<RasterizerState::CullMode, bool> mode = GetCullMode(attribute->value());
						if (mode.second)
						{
							state.cullMode = mode.first;
						}
						else
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					void HandleFrontFaceCCW(rapidxml::xml_attribute<>* attribute)
					{
						try
						{
							state.frontFaceCCW = BoolFromString(attribute->value());
						}
						catch (EnumReflectionException const&)
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					void HandleMultisampleEnable(rapidxml::xml_attribute<>* attribute)
					{
						try
						{
							state.multisampleEnable = BoolFromString(attribute->value());
						}
						catch (EnumReflectionException const&)
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}
				};
				RasterizerState state;
				RasterizerStateSetter setter(state);

				for (rapidxml::xml_node<>* subNode = node->first_node(); subNode != nullptr; subNode = subNode->next_sibling())
				{
					if (subNode->name() == StateString)
					{
						for (rapidxml::xml_attribute<>* attribute = subNode->first_attribute(); attribute != nullptr; attribute = attribute->next_attribute())
						{
							setter.Set(attribute);
						}
					}
					else
					{
						LogUnknownElement(RasterizerString, subNode);
					}
				}

				information->SetRasterizerState(state);
			}

			void HandleDepthStencilState(rapidxml::xml_node<>* node)
			{
				static std::string const DepthStencilString = "DepthStencilState";
				static std::string const StateString = "State";
				for (rapidxml::xml_attribute<>* attribute = node->first_attribute(); attribute != nullptr; attribute = attribute->next_attribute())
				{
					LogUnknownAttribute(DepthStencilString, attribute);
				}

				struct DepthStencilStateSetter
				{
					DepthStencilState& state;
					explicit DepthStencilStateSetter(DepthStencilState& state)
						: state(state)
					{
					}
					void Set(rapidxml::xml_attribute<>* attribute)
					{
						static std::unordered_map<std::string, void (DepthStencilStateSetter::*)(rapidxml::xml_attribute<>* attribute)> const Setters = []
						{
							std::remove_const<decltype(Setters)>::type temp;
							temp["DepthTestEnable"] = &DepthStencilStateSetter::HandleDepthTestEnable;
							temp["DepthWriteMask"] = &DepthStencilStateSetter::HandleDepthWriteMask;
							temp["DepthFunction"] = &DepthStencilStateSetter::HandleDepthFunction;
							temp["StencilTestEnable"] = &DepthStencilStateSetter::HandleStencilTestEnable;
							temp["FrontStencilFunction"] = &DepthStencilStateSetter::HandleFrontStencilFunction;
							temp["FrontStencilReadMask"] = &DepthStencilStateSetter::HandleFrontStencilReadMask;
							temp["FrontStencilWriteMask"] = &DepthStencilStateSetter::HandleFrontStencilWriteMask;
							temp["FrontStencilFail"] = &DepthStencilStateSetter::HandleFrontStencilFail;
							temp["FrontStencilDepthFail"] = &DepthStencilStateSetter::HandleFrontStencilDepthFail;
							temp["FrontStencilPass"] = &DepthStencilStateSetter::HandleFrontStencilPass;
							temp["BackStencilFunction"] = &DepthStencilStateSetter::HandleBackStencilFunction;
							temp["BackStencilReadMask"] = &DepthStencilStateSetter::HandleBackStencilReadMask;
							temp["BackStencilWriteMask"] = &DepthStencilStateSetter::HandleBackStencilWriteMask;
							temp["BackStencilFail"] = &DepthStencilStateSetter::HandleBackStencilFail;
							temp["BackStencilDepthFail"] = &DepthStencilStateSetter::HandleBackStencilDepthFail;
							temp["BackStencilPass"] = &DepthStencilStateSetter::HandleBackStencilPass;
							return temp;
						} ();
						auto found = Setters.find(attribute->name());
						if (found == Setters.end())
						{
							LogUnknownAttribute(DepthStencilString, attribute);
						}
						else
						{
							XREX_POINTER_CALL_MEMBER_FUNCTION(this, found->second)(attribute);
						}
					}

					void HandleDepthTestEnable(rapidxml::xml_attribute<>* attribute)
					{
						try
						{
							state.depthTestEnable = BoolFromString(attribute->value());
						}
						catch (EnumReflectionException const&)
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					void HandleDepthWriteMask(rapidxml::xml_attribute<>* attribute)
					{
						try
						{
							state.depthTestEnable = BoolFromString(attribute->value());
						}
						catch (EnumReflectionException const&)
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					std::pair<DepthStencilState::CompareFunction, bool> GetCompareFunction(std::string const value)
					{
						static std::unordered_map<std::string, DepthStencilState::CompareFunction> const Functions = []
						{
							std::remove_const<decltype(Functions)>::type temp;
							temp["AlwaysFail"] = DepthStencilState::CompareFunction::AlwaysFail;
							temp["AlwaysPass"] = DepthStencilState::CompareFunction::AlwaysPass;
							temp["Equal"] = DepthStencilState::CompareFunction::Equal;
							temp["Greater"] = DepthStencilState::CompareFunction::Greater;
							temp["GreaterEqual"] = DepthStencilState::CompareFunction::GreaterEqual;
							temp["Less"] = DepthStencilState::CompareFunction::Less;
							temp["LessEqual"] = DepthStencilState::CompareFunction::LessEqual;
							temp["NotEqual"] = DepthStencilState::CompareFunction::NotEqual;
							return temp;
						} ();
						auto found = Functions.find(value);
						if (found != Functions.end())
						{
							return std::make_pair(found->second, true);
						}
						return std::make_pair(DepthStencilState::CompareFunction::AlwaysFail, false);
					}

					void HandleDepthFunction(rapidxml::xml_attribute<>* attribute)
					{
						std::pair<DepthStencilState::CompareFunction, bool> function = GetCompareFunction(attribute->value());
						if (function.second)
						{
							state.depthFunction = function.first;
						}
						else
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					void HandleStencilTestEnable(rapidxml::xml_attribute<>* attribute)
					{
						try
						{
							state.stencilTestEnable = BoolFromString(attribute->value());
						}
						catch (EnumReflectionException const&)
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					std::pair<DepthStencilState::StencilOperation, bool> GetStencilOperation(std::string const value)
					{
						static std::unordered_map<std::string, DepthStencilState::StencilOperation> const Operations = []
						{
							std::remove_const<decltype(Operations)>::type temp;
							temp["Keep"] = DepthStencilState::StencilOperation::Keep;
							temp["Zero"] = DepthStencilState::StencilOperation::Zero;
							temp["Replace"] = DepthStencilState::StencilOperation::Replace;
							temp["Increase"] = DepthStencilState::StencilOperation::Increase;
							temp["Decrease"] = DepthStencilState::StencilOperation::Decrease;
							temp["Invert"] = DepthStencilState::StencilOperation::Invert;
							temp["IncreaseWarp"] = DepthStencilState::StencilOperation::IncreaseWarp;
							temp["DecreaseWarp"] = DepthStencilState::StencilOperation::DecreaseWarp;
							return temp;
						} ();
						auto found = Operations.find(value);
						if (found != Operations.end())
						{
							return std::make_pair(found->second, true);
						}
						return std::make_pair(DepthStencilState::StencilOperation::Keep, false);
					}

					void HandleFrontStencilFunction(rapidxml::xml_attribute<>* attribute)
					{
						std::pair<DepthStencilState::CompareFunction, bool> function = GetCompareFunction(attribute->value());
						if (function.second)
						{
							state.frontStencilFunction = function.first;
						}
						else
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					void HandleFrontStencilReadMask(rapidxml::xml_attribute<>* attribute)
					{
						state.frontStencilReadMask = static_cast<uint16>(std::atoi(attribute->value()));
					}

					void HandleFrontStencilWriteMask(rapidxml::xml_attribute<>* attribute)
					{
						state.frontStencilWriteMask = static_cast<uint16>(std::atoi(attribute->value()));
					}

					void HandleFrontStencilFail(rapidxml::xml_attribute<>* attribute)
					{
						std::pair<DepthStencilState::StencilOperation, bool> function = GetStencilOperation(attribute->value());
						if (function.second)
						{
							state.frontStencilFail = function.first;
						}
						else
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					void HandleFrontStencilDepthFail(rapidxml::xml_attribute<>* attribute)
					{
						std::pair<DepthStencilState::StencilOperation, bool> function = GetStencilOperation(attribute->value());
						if (function.second)
						{
							state.frontStencilDepthFail = function.first;
						}
						else
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					void HandleFrontStencilPass(rapidxml::xml_attribute<>* attribute)
					{
						std::pair<DepthStencilState::StencilOperation, bool> function = GetStencilOperation(attribute->value());
						if (function.second)
						{
							state.frontStencilPass = function.first;
						}
						else
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					void HandleBackStencilFunction(rapidxml::xml_attribute<>* attribute)
					{
						std::pair<DepthStencilState::CompareFunction, bool> function = GetCompareFunction(attribute->value());
						if (function.second)
						{
							state.backStencilFunction = function.first;
						}
						else
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					void HandleBackStencilReadMask(rapidxml::xml_attribute<>* attribute)
					{
						state.backStencilReadMask = static_cast<uint16>(std::atoi(attribute->value()));
					}

					void HandleBackStencilWriteMask(rapidxml::xml_attribute<>* attribute)
					{
						state.backStencilWriteMask = static_cast<uint16>(std::atoi(attribute->value()));
					}

					void HandleBackStencilFail(rapidxml::xml_attribute<>* attribute)
					{
						std::pair<DepthStencilState::StencilOperation, bool> function = GetStencilOperation(attribute->value());
						if (function.second)
						{
							state.backStencilFail = function.first;
						}
						else
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					void HandleBackStencilDepthFail(rapidxml::xml_attribute<>* attribute)
					{
						std::pair<DepthStencilState::StencilOperation, bool> function = GetStencilOperation(attribute->value());
						if (function.second)
						{
							state.backStencilDepthFail = function.first;
						}
						else
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					void HandleBackStencilPass(rapidxml::xml_attribute<>* attribute)
					{
						std::pair<DepthStencilState::StencilOperation, bool> function = GetStencilOperation(attribute->value());
						if (function.second)
						{
							state.backStencilPass = function.first;
						}
						else
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}
				};
				DepthStencilState state;
				DepthStencilStateSetter setter(state);

				for (rapidxml::xml_node<>* subNode = node->first_node(); subNode != nullptr; subNode = subNode->next_sibling())
				{
					static std::string const State = "State";
					if (subNode->name() == State)
					{
						for (rapidxml::xml_attribute<>* attribute = subNode->first_attribute(); attribute != nullptr; attribute = attribute->next_attribute())
						{
							setter.Set(attribute);
						}
					}
					else
					{
						LogUnknownElement(DepthStencilString, subNode);
					}
				}

				information->SetDepthStencilState(state);
			}

			void HandleBlendState(rapidxml::xml_node<>* node)
			{
				static std::string const BlendString = "BlendState";
				static std::string const StateString = "State";
				for (rapidxml::xml_attribute<>* attribute = node->first_attribute(); attribute != nullptr; attribute = attribute->next_attribute())
				{
					LogUnknownAttribute(BlendString, attribute);
				}

				struct BlendStateSetter
				{
					BlendState& state;
					explicit BlendStateSetter(BlendState& state)
						: state(state)
					{
					}
					void Set(rapidxml::xml_attribute<>* attribute)
					{
						static std::unordered_map<std::string, void (BlendStateSetter::*)(rapidxml::xml_attribute<>* attribute)> const Setters = []
						{
							std::remove_const<decltype(Setters)>::type temp;
							temp["AlphaToCoverageEnable"] = &BlendStateSetter::HandleAlphaToCoverageEnable;
							temp["BlendEnable"] = &BlendStateSetter::HandleBlendEnable;
							temp["BlendOperation"] = &BlendStateSetter::HandleBlendOperation;
							temp["SourceBlend"] = &BlendStateSetter::HandleSourceBlend;
							temp["DestinationBlend"] = &BlendStateSetter::HandleDestinationBlend;
							temp["BlendOperationAlpha"] = &BlendStateSetter::HandleBlendOperationAlpha;
							temp["SourceBlendAlpha"] = &BlendStateSetter::HandleSourceBlendAlpha;
							temp["DestinationBlendAlpha"] = &BlendStateSetter::HandleDestinationBlendAlpha;
							temp["RedMask"] = &BlendStateSetter::HandleRedMask;
							temp["GreenMask"] = &BlendStateSetter::HandleGreenMask;
							temp["BlueMask"] = &BlendStateSetter::HandleBlueMask;
							temp["AlphaMask"] = &BlendStateSetter::HandleAlphaMask;
							return temp;
						} ();
						auto found = Setters.find(attribute->name());
						if (found == Setters.end())
						{
							LogUnknownAttribute(BlendString, attribute);
						}
						else
						{
							XREX_POINTER_CALL_MEMBER_FUNCTION(this, found->second)(attribute);
						}
					}

					void HandleAlphaToCoverageEnable(rapidxml::xml_attribute<>* attribute)
					{
						try
						{
							state.alphaToCoverageEnable = BoolFromString(attribute->value());
						}
						catch (EnumReflectionException const&)
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					void HandleBlendEnable(rapidxml::xml_attribute<>* attribute)
					{
						try
						{
							state.blendEnable = BoolFromString(attribute->value());
						}
						catch (EnumReflectionException const&)
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					std::pair<BlendState::BlendOperation, bool> GetBlendOperation(std::string const value)
					{
						static std::unordered_map<std::string, BlendState::BlendOperation> const Functions = []
						{
							std::remove_const<decltype(Functions)>::type temp;
							temp["Add"] = BlendState::BlendOperation::Add;
							temp["Sub"] = BlendState::BlendOperation::Sub;
							temp["ReverseSub"] = BlendState::BlendOperation::ReverseSub;
							temp["Min"] = BlendState::BlendOperation::Min;
							temp["Max"] = BlendState::BlendOperation::Max;
							return temp;
						} ();
						auto found = Functions.find(value);
						if (found != Functions.end())
						{
							return std::make_pair(found->second, true);
						}
						return std::make_pair(BlendState::BlendOperation::Add, false);
					}

					std::pair<BlendState::AlphaBlendFactor, bool> GetAlphaBlendFactor(std::string const value)
					{
						static std::unordered_map<std::string, BlendState::AlphaBlendFactor> const Operations = []
						{
							std::remove_const<decltype(Operations)>::type temp;
							temp["Zero"] = BlendState::AlphaBlendFactor::Zero;
							temp["One"] = BlendState::AlphaBlendFactor::One;
							temp["SourceAlpha"] = BlendState::AlphaBlendFactor::SourceAlpha;
							temp["DestinationAlpha"] = BlendState::AlphaBlendFactor::DestinationAlpha;
							temp["OneMinusSourceAlpha"] = BlendState::AlphaBlendFactor::OneMinusSourceAlpha;
							temp["OneMinusDestinationAlpha"] = BlendState::AlphaBlendFactor::OneMinusDestinationAlpha;
							temp["SourceColor"] = BlendState::AlphaBlendFactor::SourceColor;
							temp["DestinationColor"] = BlendState::AlphaBlendFactor::DestinationColor;
							temp["OneMinusSourceColor"] = BlendState::AlphaBlendFactor::OneMinusSourceColor;
							temp["OneMinusDestinationColor"] = BlendState::AlphaBlendFactor::OneMinusDestinationColor;
							temp["SourceAlphaSaturate"] = BlendState::AlphaBlendFactor::SourceAlphaSaturate;
							return temp;
						} ();
						auto found = Operations.find(value);
						if (found != Operations.end())
						{
							return std::make_pair(found->second, true);
						}
						return std::make_pair(BlendState::AlphaBlendFactor::Zero, false);
					}

					void HandleBlendOperation(rapidxml::xml_attribute<>* attribute)
					{
						std::pair<BlendState::BlendOperation, bool> function = GetBlendOperation(attribute->value());
						if (function.second)
						{
							state.blendOperation = function.first;
						}
						else
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					void HandleSourceBlend(rapidxml::xml_attribute<>* attribute)
					{
						std::pair<BlendState::AlphaBlendFactor, bool> function = GetAlphaBlendFactor(attribute->value());
						if (function.second)
						{
							state.sourceBlend = function.first;
						}
						else
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					void HandleDestinationBlend(rapidxml::xml_attribute<>* attribute)
					{
						std::pair<BlendState::AlphaBlendFactor, bool> function = GetAlphaBlendFactor(attribute->value());
						if (function.second)
						{
							state.destinationBlend = function.first;
						}
						else
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					void HandleBlendOperationAlpha(rapidxml::xml_attribute<>* attribute)
					{
						std::pair<BlendState::BlendOperation, bool> function = GetBlendOperation(attribute->value());
						if (function.second)
						{
							state.blendOperationAlpha = function.first;
						}
						else
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					void HandleSourceBlendAlpha(rapidxml::xml_attribute<>* attribute)
					{
						std::pair<BlendState::AlphaBlendFactor, bool> function = GetAlphaBlendFactor(attribute->value());
						if (function.second)
						{
							state.sourceBlendAlpha = function.first;
						}
						else
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					void HandleDestinationBlendAlpha(rapidxml::xml_attribute<>* attribute)
					{
						std::pair<BlendState::AlphaBlendFactor, bool> function = GetAlphaBlendFactor(attribute->value());
						if (function.second)
						{
							state.destinationBlendAlpha = function.first;
						}
						else
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					void HandleRedMask(rapidxml::xml_attribute<>* attribute)
					{
						try
						{
							state.redMask = BoolFromString(attribute->value());
						}
						catch (EnumReflectionException const&)
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					void HandleGreenMask(rapidxml::xml_attribute<>* attribute)
					{
						try
						{
							state.greenMask = BoolFromString(attribute->value());
						}
						catch (EnumReflectionException const&)
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					void HandleBlueMask(rapidxml::xml_attribute<>* attribute)
					{
						try
						{
							state.blueMask = BoolFromString(attribute->value());
						}
						catch (EnumReflectionException const&)
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}

					void HandleAlphaMask(rapidxml::xml_attribute<>* attribute)
					{
						try
						{
							state.alphaMask = BoolFromString(attribute->value());
						}
						catch (EnumReflectionException const&)
						{
							LogUnknownAttributeValue(StateString, attribute);
						}
					}
				};
				BlendState state;
				BlendStateSetter setter(state);

				for (rapidxml::xml_node<>* subNode = node->first_node(); subNode != nullptr; subNode = subNode->next_sibling())
				{
					static std::string const State = "State";
					if (subNode->name() == State)
					{
						for (rapidxml::xml_attribute<>* attribute = subNode->first_attribute(); attribute != nullptr; attribute = attribute->next_attribute())
						{
							setter.Set(attribute);
						}
					}
					else
					{
						LogUnknownElement(BlendString, subNode);
					}
				}

				information->SetBlendState(state);
			}


			TechniqueBuildingInformationSP GetTechniqueBuildingInformation()
			{
				return information;
			}
		};

	}


	TechniqueLoader::TechniqueLoader()
		: cache_(MakeUP<CacheDetail>())
	{
	}

	TechniqueLoader::~TechniqueLoader()
	{
	}

	XREX::TechniqueLoadingResultSP TechniqueLoader::LoadTechnique(std::string const& fullPath, std::vector<std::pair<std::string, std::string>> macros)
	{
		RenderingTechniqueSP result = cache_->cache.FindTechnique(fullPath, macros); // NOTICE: not thread safe. when two thread try to load same technique simultaneously, result will be in an unknown state.
		if (result != nullptr) // do not reload technique informations if hot reload not active.
		{
			return MakeSP<TechniqueLoadingResultDetail>(result);
		}
		TechniqueInformationGenerator generator(cache_->cache, fullPath);
		return MakeSP<TechniqueLoadingResultDetail>(&cache_->cache, fullPath, generator.GetTechniqueBuildingInformation(), std::move(macros), generator.modificationTime);
	}

	XREX::FrameBufferLoadingResultSP TechniqueLoader::LoadFrameBuffer(std::string const& fullPath)
	{
		FrameBufferSP result = cache_->cache.FindFrameBuffer(fullPath); // NOTICE: not thread safe. when two thread try to load same technique simultaneously, result will be in an unknown state.
		if (result != nullptr) // do not reload technique informations if hot reload not active.
		{
			return MakeSP<FrameBufferLoadingResultDetail>(result);
		}
		FrameBufferDescriptionGenerator generator(cache_->cache, fullPath);
		return MakeSP<FrameBufferLoadingResultDetail>(&cache_->cache, fullPath, generator.GetFrameBufferDescription(), generator.modificationTime);
	}


}

#pragma warning(pop)
