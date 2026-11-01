#include "BaseLib/LeakDetector.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/NumberControl.h"
#include "TestLib/Assert.h"
#include "../../TestBaseLib65/TestBaseLib65/TestProject65.h"


void TestStandardLib65Tests(void);


//////////////////////////////////////////////////////////////////////////
//
// StandardLib's own tests (TestStandardLib), compiled from their real source with CppParserLib's 65C64 generator
// and run on MicroprocessorLib's 65C64, or with "816" compiled for the W65C816 and run on its fast core.
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

	TestStandardLib65Tests();

	NumberKill();
	TypeConverterKill();
	TypesKill();

	iTotalTestsFailed = TestTotalStatistics();

	LeakDetection();
	return iTotalTestsFailed;
}

