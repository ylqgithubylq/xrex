#pragma once

#include "Declare.hpp"

#include "Rendering/BufferView.hpp"
#include "Rendering/Texture.hpp"
#include "Rendering/TextureImage.hpp"

namespace XREX
{
	class XREX_API VariableInformation
	{
	public:
		VariableInformation()
			: elementType_(ElementType::Void), count_(0)
		{
		}
		VariableInformation(std::string const& name, ElementType type)
			: name_(name), elementType_(type), count_(0) // 0 is non-array
		{
		}
		VariableInformation(std::string const& name, ElementType type, uint32 count)
			: name_(name), elementType_(type), count_(count)
		{
		}
		std::string const& GetName() const
		{
			return name_;
		}
		ElementType GetElementType() const
		{
			return elementType_;
		}
		uint32 GetCount() const
		{
			return count_;
		}
	private:
		std::string name_;
		ElementType elementType_;
		uint32 count_;
	};

	class XREX_API AttributeInputInformation
	{
	public:
		AttributeInputInformation()
		{
		}
		AttributeInputInformation(VariableInformation const& information)
			: information_(information)
		{
		}

		std::string const& GetChannel() const
		{
			return information_.GetName();
		}
		ElementType GetElementType() const
		{
			return information_.GetElementType();
		}
		int32 GetElementCount() const
		{
			return information_.GetCount();
		}

	private:
		VariableInformation information_;
	};

	class XREX_API FragmentOutputInformation
	{
	public:
		FragmentOutputInformation()
		{
		}
		FragmentOutputInformation(VariableInformation const& information)
			: information_(information)
		{
		}

		std::string const& GetChannel() const
		{
			return information_.GetName();
		}
		ElementType GetElementType() const
		{
			return information_.GetElementType();
		}
		int32 GetElementCount() const
		{
			return information_.GetCount();
		}

	private:
		VariableInformation information_;
	};

	class XREX_API TextureInformation
	{
	public:
		TextureInformation()
			: textureType_(Texture::TextureType::TextureTypeCount), texelType_(ElementType::ElementTypeCount)
		{
		}
		TextureInformation(std::string const& channel, Texture::TextureType textureType, ElementType texelType, std::string const& samplerName)
			: channel_(channel), textureType_(textureType), texelType_(texelType), samplerName_(samplerName)
		{
		}

		std::string const& GetChannel() const
		{
			return channel_;
		}
		Texture::TextureType GetTextureType() const
		{
			return textureType_;
		}
		ElementType GetTexelType() const
		{
			return texelType_;
		}
		std::string const& GetSamplerName() const
		{
			return samplerName_;
		}
	private:
		std::string channel_;
		Texture::TextureType textureType_;
		ElementType texelType_;
		std::string samplerName_;
	};

	class XREX_API ImageInformation
	{
	public:
		ImageInformation()
			: imageType_(TextureImage::ImageType::ImageTypeCount), format_(TexelFormat::TexelFormatCount), accessType_(AccessType::ReadWrite)
		{
		}
		ImageInformation(std::string const& channel, TextureImage::ImageType imageType, TexelFormat format, AccessType accessType)
			: channel_(channel), imageType_(imageType), format_(format), accessType_(accessType)
		{
		}

		std::string const& GetChannel() const
		{
			return channel_;
		}
		TextureImage::ImageType GetImageType() const
		{
			return imageType_;
		}
		ElementType GetTexelType() const
		{
			return ElementType::ElementTypeCount; // TODO get from TexelFormat
		}
		TexelFormat GetTexelFormat() const
		{
			return format_;
		}
		AccessType GetAccessType() const
		{
			return accessType_;
		}
	private:
		std::string channel_;
		TextureImage::ImageType imageType_;
		TexelFormat format_;
		AccessType accessType_;
	};

	class XREX_API BufferInformation
	{
	public:
		BufferInformation()
			: type_(BufferView::BufferType::TypeCount)
		{
		}
		BufferInformation(std::string const& channel, BufferView::BufferType type, std::vector<VariableInformation const>&& bufferVariableInformations)
			: channel_(channel), type_(type), bufferVariableInformations_(std::move(bufferVariableInformations))
		{
		}

		std::string const& GetChannel() const
		{
			return channel_;
		}
		BufferView::BufferType GetBufferType() const
		{
			return type_;
		}

		std::vector<VariableInformation const> const& GetAllBufferVariableInformations() const
		{
			return bufferVariableInformations_;
		}

	private:
		std::string channel_;
		BufferView::BufferType type_;
		std::vector<VariableInformation const> bufferVariableInformations_;
	};


}