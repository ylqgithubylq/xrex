#pragma once

#include "Declare.hpp"

namespace XREX
{

	class XREX_API Timer
		: Noncopyable
	{
	public:
		Timer();
	
		~Timer();

		void Restart();
		/*
		 *	@return: time span since last call to Restart, or since creation.
		 */
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

}
