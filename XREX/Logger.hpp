#pragma once

#include "Declare.hpp"

namespace XREX
{

	class XREX_API Logger
		: Noncopyable
	{

	public:
		Logger();
		virtual ~Logger();

		void Log(std::string const& message);
		void Warn(std::string const& message);
		void Error(std::string const& message);

	};

}
