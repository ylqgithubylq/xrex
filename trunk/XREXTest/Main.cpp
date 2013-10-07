#include <XREXAll.hpp>

//#include <CoreGL.hpp>

//#include <CoreGL.h>


#include "TestFile.hpp"
#include "TextureTest.h"
#include "GeneralTest.h"




int main()
{

	switch (2)
	{
	case 1:
		{
			TextureTest texturetest;
		}
		break;
	case 2:
		{
			GeneralTest generaltest;
		}
		break;

	default:
		break;
	}


	
	//TestMath();
	//SQRTSpeedTest();
	//return 0;
	//TestFile t;
	//t.TestTransformation();
	{ // intentionally memory leak
		struct LeakTest
		{
			shared_ptr<LeakTest> p;
		};
		shared_ptr<LeakTest> ltp0 = MakeSP<LeakTest>();
		shared_ptr<LeakTest> ltp1 = MakeSP<LeakTest>();
		ltp0->p = ltp1;
		ltp1->p = ltp0;
	}




	return 0;
}

#define MEMORY_LEAK_CHECK
#ifdef MEMORY_LEAK_CHECK
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <iostream>

struct DML
{
	DML()
	{
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	}
	~DML()
	{
		if (_CrtDumpMemoryLeaks())
		{
			std::cout << "memory leaks." << std::endl;
		}
		else
		{
			std::cout << "no memory leaks." << std::endl;
		}
		//std::cin.get();
	}
} _dml;

#endif // MEMORY_LEAK_CHECK
