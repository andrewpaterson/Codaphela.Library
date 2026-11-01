#include "BaseLib/LeakDetector.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/NumberControl.h"
#include "TestLib/Assert.h"
#include "TestProject65.h"


void TestBaseLib65Tests(void);
void TestFat816Layout(void);


//////////////////////////////////////////////////////////////////////////
//
// BaseLib's own tests (TestBaseLib), compiled from their real source with CppParserLib's 65C64 generator and
// run on MicroprocessorLib's 65C64, or with "816" compiled for the W65C816 and run on its fast core.
//
//////////////////////////////////////////////////////////////////////////
int __cdecl main(int argc, char** argv)
{
	int	iTotalTestsFailed;

	TestProject65ChooseTarget(argc, argv);
	BreakOnAllocation(0);
	InitTotalStatistics();

	TypesInit();
	TypeConverterInit();
	NumberInit();

	TestFat816Layout();
	TestBaseLib65Tests();

	NumberKill();
	TypeConverterKill();
	TypesKill();

	iTotalTestsFailed = TestTotalStatistics();

	LeakDetection();
	return iTotalTestsFailed;
}

