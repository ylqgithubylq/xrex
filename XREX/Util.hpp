#pragma once

#include "BasicType.hpp"

#include "Math.hpp"

#include <utility>
#include <memory>
#include <vector>

namespace XREX
{

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

	template <typename To, typename From>
	inline To CheckedCast(From p)
	{
		static_assert(std::is_pointer<To>::value, "type To must be a pointer.");
		static_assert(std::is_pointer<From>::value, "type From must be a pointer.");
		assert(dynamic_cast<To>(p) == static_cast<To>(p));
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



	enum class ElementType
	{
		Void, // for special use

		Bool,
	
		Uint8,
		Uint16,
		Uint32,
		Int8,
		Int16,
		Int32,

		Float,
		FloatV2,
		FloatV3,
		FloatV4,
		FloatM44,

		Sampler1D,
		Sampler2D,
		Sampler3D,
		SamplerCube,

		ParameterTypeCount
	};

	/*
	 *	@return: how many primitive element in a type.
	 */
	XREX_API uint32 GetElementPrimitiveCount(ElementType type);

	XREX_API ElementType GetElementPrimitiveType(ElementType type);

	XREX_API uint32 GetElementSizeInByte(ElementType type);

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
	// TODO samplers

}
