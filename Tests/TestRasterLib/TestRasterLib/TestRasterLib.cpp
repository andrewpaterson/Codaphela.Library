// TestRasterLib.cpp : Defines the entry point for the console application.
//

#include "BaseLib/LeakDetector.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Unknowns.h"
#include "TestLib/Assert.h"


void TestRasterMesh(void);
void TestRasterPosed(void);
void TestRasterCamera(void);
void TestRasteriser(void);
void TestRasterLights(void);
void TestRenderScene(void);
void TestRasterFrame(void);


int __cdecl main(void)
{
	int iResult;

	BreakOnAllocation(0);
	InitTotalStatistics();

	TypesInit();
	TypeConverterInit();
	MemoryInit();

	TestRasterMesh();
	TestRasterPosed();
	TestRasterCamera();
	TestRasteriser();
	TestRasterLights();
	TestRenderScene();
	TestRasterFrame();

	MemoryKill();
	TypeConverterKill();
	TypesKill();

	iResult = TestTotalStatistics();
	LeakDetection();
	return iResult;
}

