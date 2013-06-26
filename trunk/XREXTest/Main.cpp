#include <XREXAll.hpp>

//#include <CoreGL.hpp>

//#include <CoreGL.h>



#include "TestFile.hpp"
#include "TextureTest.h"
#include "VoxelTest.h"
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
			VoxelTest voxeltest;
		}
		break;
	case 3:
		{
			GeneralTest generaltest;
		}
		break;

	default:
		break;
	}
	return 0;
	//TestMath();
	//SQRTSpeedTest();
	//return 0;
	//TestFile t;
	//t.TestTransformation();

// 	struct LeakTest
// 	{
// 		shared_ptr<LeakTest> p;
// 	};
// 	shared_ptr<LeakTest> ltp0 = MakeSP<LeakTest>();
// 	shared_ptr<LeakTest> ltp1 = MakeSP<LeakTest>();
// 	ltp0->p = ltp1;
// 	ltp1->p = ltp0;
	return 0;
}

//memory leak check
// 
// #define _CRTDBG_MAP_ALLOC
// #include <crtdbg.h>
// 
// struct DML
// {
// 	~DML()
// 	{
// 		if (_CrtDumpMemoryLeaks())
// 		{
// 			cout << "memory leaks." << endl;
// 		}
// 		else
// 		{
// 			cout << "no memeory leaks." << endl;
// 		}
// 		cin.get();
// 	}
// } _dml;