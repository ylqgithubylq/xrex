#pragma once

#include "Base/BasicType.hpp"

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
		bool canCast = Detail::CastChecker<To, From>::Check(p);
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


	template<typename T>
	struct ExtractTemplateType
	{
		typedef T FullType;
		typedef void Type0;
		typedef void Type1;
		typedef void Type2;
	};
	template<template<typename> class X, typename T0>
	struct ExtractTemplateType<X<T0>>
	{
		typedef X<T0> FullType;
		typedef T0 Type0;
		typedef void Type1;
		typedef void Type2;
	};
	template<template<typename, typename> class X, typename T0, typename T1>
	struct ExtractTemplateType<X<T0, T1>>
	{
		typedef X<T0, T1> FullType;
		typedef T0 Type0;
		typedef T1 Type1;
		typedef void Type2;
	};
	template<template<typename, typename, typename> class X, typename T0, typename T1, typename T2>
	struct ExtractTemplateType<X<T0, T1, T2>>
	{
		typedef X<T0, T1, T2> FullType;
		typedef T0 Type0;
		typedef T1 Type1;
		typedef T2 Type2;
	};


}
