#pragma once

#include "Declare.hpp"

class Timer
	: Noncopyable
{
public:
	Timer();
	
	~Timer();

	void Restart();

	double Elapsed() const;
	
	/*
	 *	Maximum elapsed time can be supported.
	 */
	double MaxElapseTime() const;
	
	double MinElapseTimeSpan() const;

	double CurrentTime() const;

private:
	double startTime_;
};
