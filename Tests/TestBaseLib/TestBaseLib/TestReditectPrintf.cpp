#include "BaseLib/RedirectPrintf.h"
#include "TestLib/Assert.h"



CChars	gszTestPrintFunction;


void TestPrintFunction(const char* szString)
{
	gszTestPrintFunction.Append(szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRedirectPrintfLongParameter(void)
{
	char	szLong[2001];

	//Longer than eprintf's 1KB stack buffer so the formatted parameter is malloc'd.
	memset(szLong, 'x', 2000);
	szLong[2000] = 0;

	gszTestPrintFunction.Init();
	SetPrintFunction(TestPrintFunction);
	eprintf("%s", szLong);
	AssertString(szLong, gszTestPrintFunction.Text());
	gszTestPrintFunction.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRedirectPrintf(void)
{
	BeginTests();

	gszTestPrintFunction.Init();
	SetPrintFunction(TestPrintFunction);
	eprintf("%s %s\n", "Hello", "World");
	AssertString("Hello World\n", gszTestPrintFunction.Text());
	gszTestPrintFunction.Kill();

	TestRedirectPrintfLongParameter();

	TestStatistics();
}

