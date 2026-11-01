#include "BaseLib/LogString.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLogToString(void)
{
	BeginTests();

	char	sz[] = "InThe";
	const char*	szResult;


	szResult = StringToString(&sz[0], &sz[1]);
	AssertString("In", szResult);

	TestStatistics();
}

