#pragma once

#include "Math.hpp"

namespace XREX
{

	class XREX_API Color
	{
	public:
		Color()
		{
		}
		Color(float r, float g, float b, float a)
			: value_(r, g, b, a)
		{
		}
		Color(Color const& rhs)
			: value_(rhs.value_)
		{
		}
		~Color()
		{
		}

		float R() const
		{
			return value_.X();
		}
		float G() const
		{
			return value_.Y();
		}
		float B() const
		{
			return value_.Z();
		}
		float A() const
		{
			return value_.W();
		}

		friend Color operator +(Color const& lhs, Color const& rhs)
		{
			Color temp;
			temp.value_ = lhs.value_ + rhs.value_;
			return temp;
		}

		friend Color operator -(Color const& lhs, Color const& rhs)
		{
			Color temp;
			temp.value_ = lhs.value_ - rhs.value_;
			return temp;
		}

		friend Color operator *(Color const& lhs, Color const& rhs)
		{
			Color temp;
			temp.value_ = lhs.value_ * rhs.value_;
			return temp;
		}

		friend Color operator *(Color const& lhs, float rhs)
		{
			Color temp;
			temp.value_ = lhs.value_ * rhs;
			return temp;
		}
		friend Color operator *(float const& lhs, Color const& rhs)
		{
			Color temp;
			temp.value_ = lhs * rhs.value_;
			return temp;
		}

		friend Color operator /(Color const& lhs, float const& rhs)
		{
			Color temp;
			temp.value_ = lhs.value_ / rhs;
			return temp;
		}

		Color const& operator +() const
		{
			return *this; 
		}
		Color operator -() const
		{
			Color temp;
			temp.value_ = -value_;
			return temp;
		}

		friend bool operator ==(Color const& lhs, Color const& rhs)
		{
			return lhs.value_ == rhs.value_;
		}

		friend bool operator !=(Color const& lhs, Color const& rhs)
		{
			return lhs.value_ != rhs.value_;
		}

		float const* GetArray() const
		{
			return value_.GetArray();
		}

	private:
		floatV4 value_;
	};

}
