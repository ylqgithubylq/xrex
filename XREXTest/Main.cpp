#include <XREXAll.hpp>

//#include <CoreGL.hpp>

//#include <CoreGL.h>


#include "TestFile.hpp"
#include "TextureTest.h"
#include "GeneralTest.h"
#include "RenderToTextureTest.h"

using namespace std;


int main()
{

	switch (3)
	{
	case 1:
		{
			GeneralTest generaltest;
		}
		break;
	case 2:
		{
			TextureTest texturetest;
		}
		break;
	case 3:
		{
			RenderToTextureTest renderToTextureTest;
		}
	default:
		break;
	}


	
	//TestMath();
	//SQRTSpeedTest();
	//return 0;
	//TestFile t;
	//t.TestTransformation();

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
