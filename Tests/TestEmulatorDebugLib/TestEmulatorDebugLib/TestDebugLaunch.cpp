#include "EmulatorDebugLib/DebugLaunch.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// The value after an argument; none if it isn't there, an empty one if it
// is last; argv[0] (the program) is never an argument.
//
//////////////////////////////////////////////////////////////////////////
void TestDebugLaunchValue(void)
{
	char*	aszLine[] = { (char*)DEBUG_ARGUMENT_CARD, (char*)DEBUG_ARGUMENT_WAIT, (char*)DEBUG_ARGUMENT_CARD, (char*)"D:\\Cards\\Boot.img", (char*)DEBUG_ARGUMENT_SHARED_MEMORY };

	AssertString("D:\\Cards\\Boot.img", DebugLaunchValue(4, aszLine, DEBUG_ARGUMENT_CARD));
	AssertString("", DebugLaunchValue(5, aszLine, DEBUG_ARGUMENT_SHARED_MEMORY));
	AssertTrue(DebugLaunchValue(5, aszLine, DEBUG_ARGUMENT_NETWORK) == NULL);
	AssertTrue(DebugLaunchValue(1, aszLine, DEBUG_ARGUMENT_CARD) == NULL);
	AssertTrue(DebugLaunchValue(0, aszLine, DEBUG_ARGUMENT_CARD) == NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDebugLaunch(void)
{
	BeginTests();

	TestDebugLaunchValue();

	TestStatistics();
}

