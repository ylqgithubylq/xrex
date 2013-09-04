#pragma once

#include "Base/Math.hpp"

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
		Color(Color const& right)
			: value_(right.value_)
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

		friend Color operator +(Color const& left, Color const& right)
		{
			Color temp;
			temp.value_ = left.value_ + right.value_;
			return temp;
		}

		friend Color operator -(Color const& left, Color const& right)
		{
			Color temp;
			temp.value_ = left.value_ - right.value_;
			return temp;
		}

		friend Color operator *(Color const& left, Color const& right)
		{
			Color temp;
			temp.value_ = left.value_ * right.value_;
			return temp;
		}

		friend Color operator *(Color const& left, float right)
		{
			Color temp;
			temp.value_ = left.value_ * right;
			return temp;
		}
		friend Color operator *(float const& left, Color const& right)
		{
			Color temp;
			temp.value_ = left * right.value_;
			return temp;
		}

		friend Color operator /(Color const& left, float const& right)
		{
			Color temp;
			temp.value_ = left.value_ / right;
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

		friend bool operator ==(Color const& left, Color const& right)
		{
			return left.value_ == right.value_;
		}

		friend bool operator !=(Color const& left, Color const& right)
		{
			return left.value_ != right.value_;
		}

		float const* GetArray() const
		{
			return value_.GetArray();
		}

	private:
		floatV4 value_;
	};

}
