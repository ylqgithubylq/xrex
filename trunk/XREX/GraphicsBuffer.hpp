#pragma once

#include "Declare.hpp"

#include <string>
#include <vector>



class GraphicsBuffer
	: Noncopyable
{
public:
	enum class BufferType
	{
		Vertex,
		Index
	};

	enum class Usage
	{
		Static,
		Dynamic,
		Stream,

		UsageCount
	};

	class DataDescription
		: Noncopyable
	{
		// friend class GraphicsBuffer; TODO needed?
	public:
		struct ElementLayoutDescription
		{
			uint32 start;
			uint32 strip;
			ElementType elementType;
			std::string channel;
			bool needNormalize;
			/*
			 *	@elementStrip: 0 indicates no strip between elements.
			 */
			ElementLayoutDescription(uint32 startLocation, uint32 elementStrip, ElementType type, std::string const& attributeChannel, bool normalize = false)
				: start(startLocation), strip(elementStrip), elementType(type), channel(attributeChannel), needNormalize(normalize)
			{
			}
			/*
			 *	@elementStrip: 0 indicates no strip between elements.
			 */
			ElementLayoutDescription(uint32 startLocation, uint32 elementStrip, ElementType type, std::string&& attributeChannel, bool normalize = false)
				: start(startLocation), strip(elementStrip), elementType(type), channel(std::move(attributeChannel)), needNormalize(normalize)
			{
			}
		};

	public:
		explicit DataDescription(uint32 elementCount)
			: elementCount_(elementCount)
		{
		}
		DataDescription(DataDescription&& rhs)
			: elementCount_(rhs.elementCount_), channelLayouts_(std::move(rhs.channelLayouts_))
		{
		}

		bool AddChannelLayout(ElementLayoutDescription&& elementLayout);
		ElementLayoutDescription const& GetChannelLayout(std::string const& channel) const;
		uint32 GetChannelLayoutCount() const
		{
			return channelLayouts_.size();
		}
		ElementLayoutDescription const& GetChannelLayoutAtIndex(uint32 index) const
		{
			return channelLayouts_[index];
		}
		uint32 GetElementCount() const
		{
			return elementCount_;
		}
	private:
		uint32 elementCount_;
		std::vector<ElementLayoutDescription> channelLayouts_;

	};

public:
	/*
	 *	For single-channel buffer.
	 */
	template <typename T>
	GraphicsBuffer(BufferType type, Usage usage, std::vector<T> const& data, std::string const& channel = "", bool normalized = false)
		: type_(type), usage_(usage), description_(data.size())
	{
		assert((type == BufferType::Index && channel == "") || (type != BufferType::Index && channel != "")); // index buffer must have channel == ""
		description_.AddChannelLayout(DataDescription::ElementLayoutDescription(0, 0, TypeToElementType<T>::Type, channel));
		assert(sizeof(T) == GetElementSizeInByte(TypeToElementType<T>::Type));
		DoConsctruct(data.data(), data.size() * sizeof(T));
	}

	/*
	 *	For multi-channel buffer. a.k.a.: array of structures.
	 */
	template <typename T>
	GraphicsBuffer(BufferType type, Usage usage, std::vector<T> const& data, DataDescription&& description)
		: type_(type), usage_(usage), description_(std::move(description))
	{
		assert(sizeof(T) == GetElementSizeInByte(TypeToElementType<T>::Type));
		DoConsctruct(data.data(), data.size() * sizeof(T));
	}


	~GraphicsBuffer();

	BufferType GetType() const
	{
		return type_;
	}
	Usage GetUsage() const
	{
		return usage_;
	}
	uint32 GetElementCount() const
	{
		return description_.GetElementCount();
	}
	DataDescription const& GetDataDescription() const
	{
		return description_;
	}

	void Bind();
	void BindToProgram(ProgramObjectSP const& program);
	void Unbind();

private:
	void DoConsctruct(void const* data, uint32 dataSize);

private:
	BufferType type_;
	Usage usage_;
	DataDescription description_;
	uint32 glBindingTarget_;
	uint32 glBufferID_;
	std::vector<int32> lastAttributeLocations_; // used to store attribute binding location temporarily
};

