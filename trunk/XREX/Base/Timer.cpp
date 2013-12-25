#include "XREX.hpp"


#include "Timer.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif
#include <windows.h>

#ifdef max // windows.h defined this...
#undef max

namespace XREX
{

	namespace
	{
		uint64 CPS()
		{
			static uint64 cps = 0;
			if (0 == cps)
			{
				LARGE_INTEGER frequency;
				QueryPerformanceFrequency(&frequency);
				cps = static_cast<uint64>(frequency.QuadPart);
			}
			return cps;
		}
	}


	Timer::Timer()
	{
		this->Restart();
	}
	Timer::~Timer()
	{
	}

	void Timer::Restart()
	{
		startTime_ = CurrentTime();
	}

	double Timer::Elapsed() const
	{
		return CurrentTime() - startTime_;
	}

	double Timer::MaxElapseTime() const
	{
		return static_cast<double>(std::numeric_limits<uint64>::max()) / CPS() - startTime_;
	}

	double Timer::MinElapseTimeSpan() const
	{
		return 1.0 / CPS();
	}

	double Timer::CurrentTime() const
	{
		LARGE_INTEGER count;
		QueryPerformanceCounter(&count);
		return static_cast<double>(count.QuadPart) / CPS();
	}

}

#endif