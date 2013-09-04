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

		Logger& LogLine(std::string const& message);
		Logger& LogLine(std::wstring const& message);

		Logger& BeginLine();

		Logger& Log(bool message);
		Logger& Log(int8 message);
		Logger& Log(uint8 message);
		Logger& Log(int16 message);
		Logger& Log(uint16 message);
		Logger& Log(int32 message);
		Logger& Log(uint32 message);
		Logger& Log(int64 message);
		Logger& Log(uint64 message);
		Logger& Log(float message);
		Logger& Log(double  message);
		Logger& Log(void* message);
		Logger& Log(void const* message);
		Logger& Log(char* message);
		Logger& Log(char const* message);
		Logger& Log(wchar_t* message);
		Logger& Log(wchar_t const* message);
		Logger& Log(std::string const& message);
		Logger& Log(std::wstring const& message);

		Logger& EndLine();

	private:
		struct Implementation;
		std::unique_ptr<Implementation> implementation_;
	};


}
