#pragma once

#include "Base/BasicType.hpp"

#include "Base/Math.hpp"

#include <utility>
#include <memory>
#include <vector>

namespace XREX
{
	template <typename T, uint32 Dimension>
	struct Size
	{
		std::array<T, Dimension> data;
		explicit Size(std::array<T, Dimension> const& data)
			: data(data)
		{
		}
		explicit Size(T const& x)
		{
			static_assert(Dimension == 1, "");
			data[0] = x;
		}
		explicit Size(T const& x, T const& y)
		{
			static_assert(Dimension == 2, "");
			data[0] = x;
			data[1] = y;
		}
		explicit Size(T const& x, T const& y, T const& z)
		{
			static_assert(Dimension == 3, "");
			data[0] = x;
			data[1] = y;
			data[2] = z;
		}

		T const& X() const
		{
			static_assert(Dimension >= 1, "");
			return data[0];
		}
		T const& Y() const
		{
			static_assert(Dimension >= 2, "");
			return data[1];
		}
		T const& Z() const
		{
			static_assert(Dimension >= 3, "");

			return data[2];
		}

		T& operator [](uint32 index)
		{
			assert(index < Dimension);
			return data[index];
		}
		T const& operator [](uint32 index) const
		{
			assert(index < Dimension);
			return data[index];
		}
	};



	template <typename T>
	struct Rectangle
	{
		T x;
		T y;
		T width;
		T height;
		Rectangle(T const& x, T const& y, T const& width, T const& height)
			: x(x), y(y), width(width), height(height)
		{
		}
	};

	template <typename T>
	void SwapBackRemove(std::vector<T>& vector, typename std::vector<T>::iterator toBeRemove)
	{
		assert(toBeRemove != vector.end());
		auto last = --vector.end();
		*toBeRemove = *last;
		vector.pop_back();
	}

	class XREX_API Noncopyable
	{
	protected:
		Noncopyable() {}
		~Noncopyable() {}
	private:  // emphasize the following members are private
		Noncopyable(Noncopyable const&);
		Noncopyable& operator =(Noncopyable const&);
	};

	template <typename First, typename Second>
	struct STLPairHasher // std::pair do not have a hash specialization...
	{
		size_t operator ()(std::pair<First, Second> const& value)
		{
			return std::hash<First>()(value.first) * 31 + std::hash<Second>()(value.second);
		}
	};

	namespace Detail
	{
		template <typename T, typename F>
		struct CastChecker
		{
		};
		template <typename T, typename F>
		struct CastChecker<T*, F>
		{
			static bool Check(F p)
			{
				return dynamic_cast<T*>(p) == static_cast<T*>(p);
			}
		};
		template <typename T, typename F>
		struct CastChecker<T&, F>
		{
			static bool Check(F& p)
			{
				return dynamic_cast<T*>(&p) == static_cast<T*>(&p);
			}
		};
	}

	template <typename To, typename From>
	inline To CheckedCast(From& p)
	{
#ifdef XREX_DEBUG
		bool canCast = Detail::CastChecker<To, From>::Check(p); // this line cannot be used in a macro due to ,
		assert(canCast);
#endif // XREX_DEBUG
		return static_cast<To>(p);
	}


	template <typename To, typename From>
	inline std::shared_ptr<To> CheckedSPCast(std::shared_ptr<From> const& p)
	{
		assert(std::dynamic_pointer_cast<To>(p) == std::static_pointer_cast<To>(p));
		return std::static_pointer_cast<To>(p);
	}

	template <typename EnumType>
	inline std::underlying_type<EnumType> ValueFromEnum(EnumType enumInstance)
	{
		static_assert(std::is_enum<EnumType>::value, "parameter must be enum type.");
		return static_cast<std::underlying_type<EnumType>>(enumInstance);
	}
	template <typename EnumType, typename ValueType>
	inline EnumType EnumFromValue(ValueType value)
	{
		static_assert(std::is_enum<EnumType>::value, "EnumType must be enum type.");
		return static_cast<EnumType>(value);
	}

	template <typename T>
	inline std::unique_ptr<T> MakeUP()
	{
		return std::unique_ptr<T>(new T());
	}
	template <typename T, typename A0>
	inline std::unique_ptr<T> MakeUP(A0&& a0)
	{
		return std::unique_ptr<T>(new T(std::forward<A0>(a0)));
	}
	template <typename T, typename A0, typename A1>
	inline std::unique_ptr<T> MakeUP(A0&& a0, A1&& a1)
	{
		return std::unique_ptr<T>(new T(std::forward<A0>(a0), std::forward<A1>(a1)));
	}
	template <typename T, typename A0, typename A1, typename A2>
	inline std::unique_ptr<T> MakeUP(A0&& a0, A1&& a1, A2&& a2)
	{
		return std::unique_ptr<T>(new T(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2)));
	}
	template <typename T, typename A0, typename A1, typename A2, typename A3>
	inline std::unique_ptr<T> MakeUP(A0&& a0, A1&& a1, A2&& a2, A3&& a3)
	{
		return std::unique_ptr<T>(new T(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3)));
	}
	template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4>
	inline std::unique_ptr<T> MakeUP(A0&& a0, A1&& a1, A2&& a2, A3&& a3, A4&& a4)
	{
		return std::unique_ptr<T>(new T(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4)));
	}
	template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
	inline std::unique_ptr<T> MakeUP(A0&& a0, A1&& a1, A2&& a2, A3&& a3, A4&& a4, A5&& a5)
	{
		return std::unique_ptr<T>(new T(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5)));
	}

	template <typename T>
	inline std::shared_ptr<T> MakeSP()
	{
		return std::make_shared<T>();
	}
	template <typename T, typename A0>
	inline std::shared_ptr<T> MakeSP(A0&& a0)
	{
		return std::make_shared<T>(std::forward<A0>(a0));
	}
	template <typename T, typename A0, typename A1>
	inline std::shared_ptr<T> MakeSP(A0&& a0, A1&& a1)
	{
		return std::make_shared<T>(std::forward<A0>(a0), std::forward<A1>(a1));
	}
	template <typename T, typename A0, typename A1, typename A2>
	inline std::shared_ptr<T> MakeSP(A0&& a0, A1&& a1, A2&& a2)
	{
		return std::make_shared<T>(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2));
	}
	template <typename T, typename A0, typename A1, typename A2, typename A3>
	inline std::shared_ptr<T> MakeSP(A0&& a0, A1&& a1, A2&& a2, A3&& a3)
	{
		return std::make_shared<T>(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3));
	}
	template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4>
	inline std::shared_ptr<T> MakeSP(A0&& a0, A1&& a1, A2&& a2, A3&& a3, A4&& a4)
	{
		return std::make_shared<T>(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4));
	}
	template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
	inline std::shared_ptr<T> MakeSP(A0&& a0, A1&& a1, A2&& a2, A3&& a3, A4&& a4, A5&& a5)
	{ // std::make_shared only have 5 parameters
		return std::shared_ptr<T>(new T(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5)));
	}



	enum class ElementType
	{
		Void, // for special use

		Structure, // for special use

		Bool,
	
		Uint8,
		Uint16,
		Uint32,
		Int8,
		Int16,
		Int32,

		IntV2,
		IntV3,
		IntV4,

		UintV2,
		UintV3,
		UintV4,

		Float,
		FloatV2,
		FloatV3,
		FloatV4,
		FloatM44,

		Double,
		DoubleV2,
		DoubleV3,
		DoubleV4,
		DoubleM44,

		Sampler, // for TechniqueParameter use

		Sampler1D,
		IntSampler1D,
		UintSampler1D,
		Sampler2D,
		IntSampler2D,
		UintSampler2D,
		Sampler3D,
		IntSampler3D,
		UintSampler3D,
		SamplerCube,
		IntSamplerCube,
		UintSamplerCube,
		SamplerBuffer,
		IntSamplerBuffer,
		UintSamplerBuffer,

		Image, // for TechniqueParameter use

		Image1D,
		IntImage1D,
		UintImage1D,
		Image2D,
		IntImage2D,
		UintImage2D,
		Image3D,
		IntImage3D,
		UintImage3D,
		ImageCube,
		IntImageCube,
		UintImageCube,
		ImageBuffer,
		IntImageBuffer,
		UintImageBuffer,

		Buffer,
		ShaderResourceBuffer,

		AtomicUint32Counter,

		ParameterTypeCount,
	};

	XREX_API bool IsSamplerType(ElementType type);
	XREX_API bool IsImageType(ElementType type);
	XREX_API bool IsAtomicBufferType(ElementType type);

	/*
	 *	@return: how many primitive element in a type.
	 */
	XREX_API uint32 GetElementPrimitiveCount(ElementType type);

	XREX_API ElementType GetElementPrimitiveType(ElementType type);

	XREX_API uint32 GetElementSizeInBytes(ElementType type);

	template <typename T>
	struct TypeToElementType
	{
		static ElementType const Type = ElementType::ParameterTypeCount;
	};
	template <>
	struct TypeToElementType<bool>
	{
		static ElementType const Type = ElementType::Bool;
	};
	template <>
	struct TypeToElementType<uint8>
	{
		static ElementType const Type = ElementType::Uint8;
	};
	template <>
	struct TypeToElementType<uint16>
	{
		static ElementType const Type = ElementType::Uint16;
	};
	template <>
	struct TypeToElementType<uint32>
	{
		static ElementType const Type = ElementType::Uint32;
	};
	template <>
	struct TypeToElementType<int8>
	{
		static ElementType const Type = ElementType::Int8;
	};
	template <>
	struct TypeToElementType<int16>
	{
		static ElementType const Type = ElementType::Int16;
	};
	template <>
	struct TypeToElementType<int32>
	{
		static ElementType const Type = ElementType::Int32;
	};
	template <>
	struct TypeToElementType<float>
	{
		static ElementType const Type = ElementType::Float;
	};
	template <>
	struct TypeToElementType<floatV2>
	{
		static ElementType const Type = ElementType::FloatV2;
	};
	template <>
	struct TypeToElementType<floatV3>
	{
		static ElementType const Type = ElementType::FloatV3;
	};
	template <>
	struct TypeToElementType<floatV4>
	{
		static ElementType const Type = ElementType::FloatV4;
	};
	template <>
	struct TypeToElementType<floatM44>
	{
		static ElementType const Type = ElementType::FloatM44;
	};
	template <>
	struct TypeToElementType<double>
	{
		static ElementType const Type = ElementType::Double;
	};
	template <>
	struct TypeToElementType<doubleV2>
	{
		static ElementType const Type = ElementType::DoubleV2;
	};
	template <>
	struct TypeToElementType<doubleV3>
	{
		static ElementType const Type = ElementType::DoubleV3;
	};
	template <>
	struct TypeToElementType<doubleV4>
	{
		static ElementType const Type = ElementType::DoubleV4;
	};
	template <>
	struct TypeToElementType<doubleM44>
	{
		static ElementType const Type = ElementType::DoubleM44;
	};

	template <>
	struct TypeToElementType<TextureSP>
	{
		static ElementType const Type = ElementType::Sampler;
	};
	template <>
	struct TypeToElementType<TextureImageSP>
	{
		static ElementType const Type = ElementType::Image;
	};
	template <>
	struct TypeToElementType<ShaderResourceBufferSP>
	{
		static ElementType const Type = ElementType::ShaderResourceBuffer;
	};

	enum class TexelFormat
	{
		R8,
		RG8,
		RGB8,
		RGBA8,

		R8I,
		RG8I,
		RGB8I,
		RGBA8I,

		R8UI,
		RG8UI,
		RGB8UI,
		RGBA8UI,

		R16I,
		RG16I,
		RGB16I,
		RGBA16I,

		R16UI,
		RG16UI,
		RGB16UI,
		RGBA16UI,

		R16F,
		RG16F,
		RGB16F,
		RGBA16F,

		R32I,
		RG32I,
		RGB32I,
		RGBA32I,

		R32UI,
		RG32UI,
		RGB32UI,
		RGBA32UI,

		R32F,
		RG32F,
		RGB32F,
		RGBA32F,

		BGR8,
		BGRA8,
		BGR16F,
		BGRA16F,
		BGR32F,
		BGRA32F,

		Depth16,
		Depth24,
		Depth32,
		Depth32F,
		Depth24Stencil8,
		Stencil8,

		TexelFormatCount
	};

	XREX_API uint32 GetTexelSizeInBytes(TexelFormat format);

	XREX_API TexelFormat GetCorrespondingTexelFormat(ElementType type);

	enum class AccessType
	{
		ReadOnly,
		WriteOnly,
		ReadWrite,
	};
}
