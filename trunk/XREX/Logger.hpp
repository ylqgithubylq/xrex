#pragma once

#include "Declare.hpp"

class Logger
	: Noncopyable
{
public:
	Logger& GetInstance();

public:
	Logger();
	virtual ~Logger();

	void Log();
	void Warn();
	void Error();

};

