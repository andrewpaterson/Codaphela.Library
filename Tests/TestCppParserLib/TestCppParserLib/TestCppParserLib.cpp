#include "BaseLib/LeakDetector.h"
#include "TestLib/Assert.h"


void TestCppErrors(void);
void TestPreprocessor(void);
void TestProject(void);
void TestCppTokeniser(void);
void TestCppParser(void);
void TestCppHighlighter(void);
void TestIR(void);
void TestIR65Backend(void);
void TestIR816Backend(void);
void TestCPP816(void);
void TestCPP816Volatile(void);
void TestCPP65IR(void);
void TestCPP65Generator(void);
void TestCPP65LazyHeaders(void);
void TestCPP65Spill(void);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int __cdecl main(void)
{
	int	iTotalTestsFailed;

	BreakOnAllocation(0);
	InitTotalStatistics();

	TestCppErrors();
	TestPreprocessor();
	TestProject();
	TestCppTokeniser();
	TestCppParser();
	TestCppHighlighter();
	TestIR();
	TestIR65Backend();
	TestIR816Backend();
	TestCPP816();
	TestCPP816Volatile();
	TestCPP65Generator();
	TestCPP65IR();
	TestCPP65LazyHeaders();
	TestCPP65Spill();

	iTotalTestsFailed = TestTotalStatistics();

	LeakDetection();
	return iTotalTestsFailed;
}

