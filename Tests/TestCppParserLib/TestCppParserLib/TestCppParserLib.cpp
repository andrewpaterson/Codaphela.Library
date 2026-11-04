#include "BaseLib/LeakDetector.h"
#include "TestLib/Assert.h"


void TestCppErrors(void);
void TestPreprocessor(void);
void TestProject(void);
void TestCppTokeniser(void);
void TestCppParser(void);
void TestCppHighlighter(void);
void TestCppIndex(void);
void TestIR(void);
void TestIR65Backend(void);
void TestIR816Backend(void);
void TestCPP816(void);
void TestCPP816Assembly(void);
void TestCPP816Interprocedural(void);
void TestCPP65ListingIndex(void);
void TestCPP816Volatile(void);
void TestCPP65IR(void);
void TestCPP65Generator(void);
void TestCPP65LazyHeaders(void);
void TestCPP65Spill(void);
void TestCPP65Jobs(void);
void TestCPP65HeaderCache(void);
void TestIRImportsTurns(void);
void TestDefineMap(void);
void TestPreprocessorHeaderCache(void);


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
	TestDefineMap();
	TestPreprocessorHeaderCache();
	TestProject();
	TestCppTokeniser();
	TestCppParser();
	TestCppHighlighter();
	TestCppIndex();
	TestIR();
	TestIRImportsTurns();
	TestIR65Backend();
	TestIR816Backend();
	TestCPP816Assembly();
	TestCPP816Interprocedural();
	TestCPP65ListingIndex();
	TestCPP816();
	TestCPP816Volatile();
	TestCPP65Generator();
	TestCPP65IR();
	TestCPP65LazyHeaders();
	TestCPP65Spill();
	TestCPP65Jobs();
	TestCPP65HeaderCache();

	iTotalTestsFailed = TestTotalStatistics();

	LeakDetection();
	return iTotalTestsFailed;
}

