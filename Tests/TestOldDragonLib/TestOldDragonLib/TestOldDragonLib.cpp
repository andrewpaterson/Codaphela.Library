// TestOldDragonLib.cpp : Defines the entry point for the console application.
//

#include "BaseLib/LeakDetector.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalMemory.h"
#include "TestLib/Assert.h"


void TestOldDragonReader(void);
void TestOldDragonConverter(void);


int __cdecl main(void)
{
	int iResult;

	BreakOnAllocation(0);
	InitTotalStatistics();

	TypesInit();
	TypeConverterInit();
	MemoryInit();

	TestOldDragonReader();
	TestOldDragonConverter();

	MemoryKill();
	TypeConverterKill();
	TypesKill();

	iResult = TestTotalStatistics();
	LeakDetection();
	return iResult;
}

