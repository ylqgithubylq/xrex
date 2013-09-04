#include "XREX.hpp"

#include "Logger.hpp"

#include <iostream>

namespace XREX
{
	struct Logger::Implementation
	{
		Implementation()
			: wo(std::wcout), o(std::cout)
		{
		}
		std::wostream& wo;
		std::ostream& o;
	};

	Logger::Logger()
		: implementation_(MakeUP<Implementation>())
	{
	}


	Logger::~Logger()
	{
		implementation_->wo.flush();
	}

	Logger& Logger::LogLine(std::string const& message)
	{
		BeginLine();
		implementation_->o << message;
		implementation_->o.flush();
		EndLine();
		return *this;
	}
	Logger& Logger::LogLine(std::wstring const& message)
	{
		BeginLine();
		implementation_->wo << message;
		EndLine();
		return *this;
	}

	Logger& Logger::BeginLine()
	{
		// TODO insert current time
		return *this;
	}

	Logger& Logger::EndLine()
	{
		implementation_->wo << std::endl;
		return *this;
	}

	Logger& Logger::Log(bool message)
	{
		implementation_->wo << message;
		return *this;
	}
	Logger& Logger::Log(int8 message)
	{
		implementation_->wo << message;
		return *this;
	}
	Logger& Logger::Log(uint8 message)
	{
		implementation_->wo << message;
		return *this;
	}
	Logger& Logger::Log(int16 message)
	{
		implementation_->wo << message;
		return *this;
	}
	Logger& Logger::Log(uint16 message)
	{
		implementation_->wo << message;
		return *this;
	}
	Logger& Logger::Log(int32 message)
	{
		implementation_->wo << message;
		return *this;
	}
	Logger& Logger::Log(uint32 message)
	{
		implementation_->wo << message;
		return *this;
	}
	Logger& Logger::Log(int64 message)
	{
		implementation_->wo << message;
		return *this;
	}
	Logger& Logger::Log(uint64 message)
	{
		implementation_->wo << message;
		return *this;
	}
	Logger& Logger::Log(float message)
	{
		implementation_->wo << message;
		return *this;
	}
	Logger& Logger::Log(double message)
	{
		implementation_->wo << message;
		return *this;
	}
	Logger& Logger::Log(void* message)
	{
		implementation_->wo << message;
		return *this;
	}
	Logger& Logger::Log(void const* message)
	{
		implementation_->wo << message;
		return *this;
	}
	Logger& Logger::Log(char* message)
	{
		implementation_->o << message;
		implementation_->o.flush();
		return *this;
	}
	Logger& Logger::Log(char const* message)
	{
		implementation_->o << message;
		return *this;
	}
	Logger& Logger::Log(wchar_t* message)
	{
		implementation_->wo << message;
		implementation_->o.flush();
		return *this;
	}
	Logger& Logger::Log(wchar_t const* message)
	{
		implementation_->wo << message;
		return *this;
	}
	Logger& Logger::Log(std::string const& message)
	{
		implementation_->o << message;
		implementation_->o.flush();
		return *this;
	}
	Logger& Logger::Log(std::wstring const& message)
	{
		implementation_->wo << message;
		return *this;
	}
}
