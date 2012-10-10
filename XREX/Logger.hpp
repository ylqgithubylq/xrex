#pragma once

#include "Declare.hpp"

class Logger
	: Noncopyable
{

public:
	Logger();
	virtual ~Logger();

	void Log(std::string const& message);
	void Warn(std::string const& message);
	void Error(std::string const& message);

};

