#include "BaseLib/LeakDetector.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/NumberControl.h"
#include "TestLib/Assert.h"
#include "TestProject65.h"


void TestBaseLib65Tests(void);
void TestFatW65C816Layout(void);


//////////////////////////////////////////////////////////////////////////
//
// BaseLib's own tests (TestBaseLib), compiled from their real source with CppCompilerLib's 65C64 generator and run on
// MicroprocessorLib's 65C64, or with "W65C816" ("816" for short) compiled for the W65C816 and run on its fast core.
//
//////////////////////////////////////////////////////////////////////////
int __cdecl main(int argc, char** argv)
{
	int	iTotalTestsFailed;

	if (!TestProject65ChooseTarget(argc, argv))
	{
		return 2;
	}
	BreakOnAllocation(0);
	InitTotalStatistics();

	TypesInit();
	TypeConverterInit();
	NumberInit();

	TestFatW65C816Layout();
	TestBaseLib65Tests();

	NumberKill();
	TypeConverterKill();
	TypesKill();

	iTotalTestsFailed = TestTotalStatistics();

	LeakDetection();
	return iTotalTestsFailed;
}

