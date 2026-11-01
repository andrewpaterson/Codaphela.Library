// TestSupportLib.cpp : Defines the entry point for the console application.
//

#include "BaseLib/LeakDetector.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Unknowns.h"
#include "TestLib/Assert.h"


void TestGerberParser(void);
void TestGerberInterpreter(void);
void TestExcellon(void);
void TestGerberWriter(void);
void TestGerberVector(void);
void TestGerberRaster(void);
void TestGerberFileSet(void);
void TestGerberPanel(void);
void TestGerberPanelMerge(void);


int __cdecl main(void)
{
	int iResult;

	BreakOnAllocation(0);
	InitTotalStatistics();


	TypesInit();
	TypeConverterInit();
	MemoryInit();

	TestGerberParser();
	TestGerberInterpreter();
	TestExcellon();
	TestGerberWriter();
	TestGerberVector();
	TestGerberRaster();
	TestGerberFileSet();
	TestGerberPanel();
	TestGerberPanelMerge();

	MemoryKill();
	TypeConverterKill();
	TypesKill();


	iResult = TestTotalStatistics();
	LeakDetection();
	return iResult;
}

