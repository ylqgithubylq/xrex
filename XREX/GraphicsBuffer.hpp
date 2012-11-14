#pragma once

#include "Declare.hpp"

#include <string>
#include <vector>

namespace XREX
{

	class XREX_API GraphicsBuffer
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

		class XREX_API DataLayout
			: Noncopyable
		{
		public:
			struct XREX_API ElementLayout
			{
				uint32 start;
				uint32 strip;
				ElementType elementType;
				std::string channel;
				bool needNormalize;
				/*
				 *	@elementStrip: 0 indicates no strip between elements.
				 */
				ElementLayout(uint32 startLocation, uint32 elementStrip, ElementType type, std::string const& attributeChannel, bool normalize = false)
					: start(startLocation), strip(elementStrip), elementType(type), channel(attributeChannel), needNormalize(normalize)
				{
				}
				/*
				 *	@elementStrip: 0 indicates no strip between elements.
				 */
				ElementLayout(uint32 startLocation, uint32 elementStrip, ElementType type, std::string&& attributeChannel, bool normalize = false)
					: start(startLocation), strip(elementStrip), elementType(type), channel(std::move(attributeChannel)), needNormalize(normalize)
				{
				}
			};

		public:
			explicit DataLayout(uint32 elementCount)
				: elementCount_(elementCount)
			{
			}
			DataLayout(DataLayout&& rhs)
				: elementCount_(rhs.elementCount_), channelLayouts_(std::move(rhs.channelLayouts_))
			{
			}

			bool AddChannelLayout(ElementLayout&& elementLayout);
			ElementLayout const& GetChannelLayout(std::string const& channel) const;
			uint32 GetChannelLayoutCount() const
			{
				return channelLayouts_.size();
			}
			ElementLayout const& GetChannelLayoutAtIndex(uint32 index) const
			{
				return channelLayouts_[index];
			}
			uint32 GetElementCount() const
			{
				return elementCount_;
			}
		private:
			uint32 elementCount_;
			std::vector<ElementLayout> channelLayouts_;

		};

	public:
		/*
		 *	For single-channel buffer.
		 */
		template <typename T>
		GraphicsBuffer(BufferType type, Usage usage, std::vector<T> const& data, std::string const& channel = "", bool normalized = false)
			: type_(type), usage_(usage), layout_(data.size())
		{
			assert((type == BufferType::Index && channel == "") || (type != BufferType::Index && channel != "")); // index buffer must have channel == ""
			layout_.AddChannelLayout(DataLayout::ElementLayout(0, 0, TypeToElementType<T>::Type, channel));
			assert(sizeof(T) == GetElementSizeInByte(TypeToElementType<T>::Type));
			DoConsctruct(data.data(), data.size() * sizeof(T));
		}

		/*
		 *	For multi-channel buffer. a.k.a.: array of structures.
		 */
		template <typename T>
		GraphicsBuffer(BufferType type, Usage usage, std::vector<T> const& data, DataLayout&& layout)
			: type_(type), usage_(usage), layout_(std::move(layout))
		{
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
			return layout_.GetElementCount();
		}
		DataLayout const& GetDataLayout() const
		{
			return layout_;
		}

		void Bind();
		void BindToProgram(ProgramObjectSP const& program);
		void Unbind();

	private:
		void DoConsctruct(void const* data, uint32 dataSize);

	private:
		BufferType type_;
		Usage usage_;
		DataLayout layout_;
		uint32 glBindingTarget_;
		uint32 glBufferID_;
		std::vector<int32> lastAttributeLocations_; // used to store attribute binding location temporarily
	};

}
