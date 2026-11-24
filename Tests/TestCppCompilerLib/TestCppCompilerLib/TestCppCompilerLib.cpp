#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "BaseLib/LeakDetector.h"
#include "TestLib/Assert.h"
#include "CppCompilerLib/CPP65Builder.h"
#include "TestCppCompilerLibParts.h"


void TestCppErrors(void);
void TestPreprocessor(void);
void TestProject(void);
void TestCppTokeniser(void);
void TestCppParser(void);
void TestCPPSyntaxBlock(void);
void TestCppHighlighter(void);
void TestCppIndex(void);
void TestIR(void);
void TestIRInterpreter(void);
void TestIRInterprocedural(void);
void TestIR65Backend(void);
void TestIR816Backend(void);
void TestCPP816(void);
void TestCPP816Assembly(void);
void TestCPP816Interprocedural(void);
void TestCPP65ListingIndex(void);
void TestCPP816Volatile(void);
void TestCPP65Debug(void);
void TestCPP65ForwardingReferences(void);
void TestCPPConformance(void);
void TestCPP65IR(void);
void TestCPP65Generator(void);
void TestCPP65Declarations(void);
void TestCPP65LazyHeaders(void);
void TestCPP65Spill(void);
void TestCPP65Jobs(void);
void TestCPP65HeaderCache(void);
void TestIRImportsTurns(void);
void TestDefineMap(void);
void TestCPPClassLayout(void);
void TestCPPContext(void);
void TestCPPOverloads(void);
void TestPreprocessorHeaderCache(void);


//A test file's entry point and its name, and whether it can be run in parts (TEST_PART).
struct STestCppCompilerLibFile
{
	void		(*fTest)(void);
	const char*	szName;
	bool		bParts;
};


//The files, in the order they run.
static const STestCppCompilerLibFile gasTestFiles[] =
{
	{ TestCppErrors, "TestCppErrors", false },
	{ TestPreprocessor, "TestPreprocessor", false },
	{ TestDefineMap, "TestDefineMap", false },
	{ TestCPPClassLayout, "TestCPPClassLayout", false },
	{ TestCPPContext, "TestCPPContext", false },
	{ TestCPPOverloads, "TestCPPOverloads", false },
	{ TestPreprocessorHeaderCache, "TestPreprocessorHeaderCache", false },
	{ TestProject, "TestProject", false },
	{ TestCppTokeniser, "TestCppTokeniser", false },
	{ TestCppParser, "TestCppParser", false },
	{ TestCPPSyntaxBlock, "TestCPPSyntaxBlock", false },
	{ TestCppHighlighter, "TestCppHighlighter", false },
	{ TestCppIndex, "TestCppIndex", false },
	{ TestCPPConformance, "TestCPPConformance", true },
	{ TestIR, "TestIR", false },
	{ TestIRInterpreter, "TestIRInterpreter", false },
	{ TestIRInterprocedural, "TestIRInterprocedural", false },
	{ TestIRImportsTurns, "TestIRImportsTurns", false },
	{ TestIR65Backend, "TestIR65Backend", false },
	{ TestIR816Backend, "TestIRW65C816Backend", true },
	{ TestCPP816Assembly, "TestCPPW65C816Assembly", false },
	{ TestCPP816Interprocedural, "TestCPPW65C816Interprocedural", false },
	{ TestCPP65ListingIndex, "TestCPP65ListingIndex", false },
	{ TestCPP816, "TestCPPW65C816", true },
	{ TestCPP816Volatile, "TestCPPW65C816Volatile", false },
	{ TestCPP65Debug, "TestCPP65Debug", false },
	{ TestCPP65ForwardingReferences, "TestCPP65ForwardingReferences", false },
	{ TestCPP65Generator, "TestCPP65Generator", true },
	{ TestCPP65IR, "TestCPP65IR", false },
	{ TestCPP65Declarations, "TestCPP65Declarations", false },
	{ TestCPP65LazyHeaders, "TestCPP65LazyHeaders", false },
	{ TestCPP65Spill, "TestCPP65Spill", false },
	{ TestCPP65Jobs, "TestCPP65Jobs", false },
	{ TestCPP65HeaderCache, "TestCPP65HeaderCache", false },
};
#define NUM_TEST_FILES	(sizeof(gasTestFiles) / sizeof(STestCppCompilerLibFile))


//The part being run (TestCppCompilerLibParts.h): -part p/n, or the calls -calls lists.
#define TEST_PART_MAX_CALLS		256
static size			guiTestPart = 0;
static size			guiTestParts = 1;
static bool			gbTestPartCalls = false;
static bool			gabTestPartCalls[TEST_PART_MAX_CALLS];
static size			guiTestPartRest = TEST_PART_MAX_CALLS;	//-rest: every call from it runs too.
static size			guiTestPartCall = 0;
static const char*	gszTestPartFile = "";
static clock_t		giTestPartStart = 0;
static lsize		giTestPartTests = 0;
static bool			gbTestPartTimes = false;


//////////////////////////////////////////////////////////////////////////
//
// A file that can be run in parts begins: its calls are counted from 0.
//
//////////////////////////////////////////////////////////////////////////
void TestPartBegin(const char* szFile)
{
	guiTestPartCall = 0;
	gszTestPartFile = szFile;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the file's next call is this part's (TEST_PART).
//
//////////////////////////////////////////////////////////////////////////
bool TestPartStarts(void)
{
	size	uiCall;
	bool	bRuns;

	uiCall = guiTestPartCall;
	guiTestPartCall++;
	if (gbTestPartCalls)
	{
		bRuns = (uiCall >= guiTestPartRest) || ((uiCall < TEST_PART_MAX_CALLS) && gabTestPartCalls[uiCall]);
	}
	else
	{
		bRuns = (uiCall % guiTestParts) == guiTestPart;
	}
	if (!bRuns && gbTestPartTimes)
	{
		printf("TESTSKIP %s %d\n", gszTestPartFile, (int)uiCall);
	}
	giTestPartStart = clock();
	giTestPartTests = TestsRun();
	return bRuns;
}


//////////////////////////////////////////////////////////////////////////
//
// A call TestPartStarts let run has: its time, with TEST_PART_TIMES.
//
//////////////////////////////////////////////////////////////////////////
void TestPartEnds(void)
{
	if (gbTestPartTimes)
	{
		printf("TESTPART %s %d %.3f %d\n", gszTestPartFile, (int)(guiTestPartCall - 1), (double)(clock() - giTestPartStart) / CLOCKS_PER_SEC, (int)(TestsRun() - giTestPartTests));
		fflush(stdout);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The process's part's name, for what it writes where the other parts write
// too: its log's name (TESTLIB_LOG) without Log.txt, or "" without one.
//
//////////////////////////////////////////////////////////////////////////
const char* TestPartLabel(void)
{
	static char	szLabel[128];
	const char*	szLog;
	size		uiLength;

	szLog = getenv("TESTLIB_LOG");
	if (!szLog)
	{
		return "";
	}
	uiLength = strlen(szLog);
	if ((uiLength >= 7) && (strcmp(szLog + uiLength - 7, "Log.txt") == 0))
	{
		uiLength -= 7;
	}
	if (uiLength >= sizeof(szLabel))
	{
		uiLength = sizeof(szLabel) - 1;
	}
	memcpy(szLabel, szLog, uiLength);
	szLabel[uiLength] = '\0';
	return szLabel;
}


//////////////////////////////////////////////////////////////////////////
//
// -calls' list (call numbers, comma separated) as the part's calls.  False if
// it isn't a list of numbers under TEST_PART_MAX_CALLS.
//
//////////////////////////////////////////////////////////////////////////
static bool TestPartCalls(const char* szCalls)
{
	const char*	sz;
	int			iCall;

	memset(gabTestPartCalls, 0, sizeof(gabTestPartCalls));
	gbTestPartCalls = true;
	for (sz = szCalls; *sz; sz++)
	{
		if ((*sz < '0') || (*sz > '9'))
		{
			return false;
		}
		iCall = atoi(sz);
		if (iCall >= TEST_PART_MAX_CALLS)
		{
			return false;
		}
		gabTestPartCalls[iCall] = true;
		while ((*sz >= '0') && (*sz <= '9'))
		{
			sz++;
		}
		if (*sz == '\0')
		{
			break;
		}
		if (*sz != ',')
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the file is one -only names (a comma separated list of names,
// NULL for every file).
//
//////////////////////////////////////////////////////////////////////////
static bool TestFileChosen(const char* szName, const char* szOnly)
{
	const char*	szAt;
	size		uiLength;

	if (!szOnly)
	{
		return true;
	}
	uiLength = strlen(szName);
	for (szAt = strstr(szOnly, szName); szAt; szAt = strstr(szAt + 1, szName))
	{
		if (((szAt == szOnly) || (szAt[-1] == ',')) && ((szAt[uiLength] == '\0') || (szAt[uiLength] == ',')))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Every test file, or with arguments: -list prints each file's name and 1 if
// it can be run in parts (else 0), and runs nothing; -only Name[,Name]
// runs those files; -part p/n runs part p (from 0) of n of each file that
// can be run in parts, and nothing of the others; -calls n,n,... runs those
// calls of them, and -rest n every call from n too (TestCppCompilerLibParts.h).  RunParallel.py runs the parts
// in processes of their own, TESTLIB_LOG naming each one's log.
//
//////////////////////////////////////////////////////////////////////////
int __cdecl main(int argc, char** argv)
{
	int			iTotalTestsFailed;
	const char*	szOnly;
	int			i;
	size		ui;

	szOnly = NULL;
	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-list") == 0)
		{
			for (ui = 0; ui < NUM_TEST_FILES; ui++)
			{
				printf("%s %d\n", gasTestFiles[ui].szName, gasTestFiles[ui].bParts ? 1 : 0);
			}
			return 0;
		}
		else if ((strcmp(argv[i], "-only") == 0) && (i + 1 < argc))
		{
			szOnly = argv[++i];
		}
		else if ((strcmp(argv[i], "-calls") == 0) && (i + 1 < argc))
		{
			if (!TestPartCalls(argv[++i]))
			{
				printf("-calls n,n,...: call numbers under %d\n", TEST_PART_MAX_CALLS);
				return 2;
			}
		}
		else if ((strcmp(argv[i], "-rest") == 0) && (i + 1 < argc))
		{
			guiTestPartRest = (size)atoi(argv[++i]);
		}
		else if ((strcmp(argv[i], "-part") == 0) && (i + 1 < argc))
		{
			i++;
			if ((sscanf(argv[i], "%zu/%zu", &guiTestPart, &guiTestParts) != 2) || (guiTestParts == 0) || (guiTestPart >= guiTestParts))
			{
				printf("-part p/n: p from 0 to n - 1\n");
				return 2;
			}
		}
		else
		{
			printf("TestCppCompilerLib [-list] [-only Name[,Name]] [-part p/n | -calls n,n,... [-rest n]]\n");
			return 2;
		}
	}

	gbTestPartTimes = getenv("TEST_PART_TIMES") != NULL;
	//Every compile with debug information also compiled without, the code compared (DebuggerFPlan.txt F1).
	CCPP65Builder::SetCheckDebugCode(getenv("TEST_CPP65_SAME_CODE") != NULL);
	BreakOnAllocation(0);
	InitTotalStatistics();
	NumberInit();

	if (getenv("TEST_CPP_CONFORMANCE_TRY"))
	{
		//One program, tried on each target (TestCPPConformanceTry), and nothing else.
		TestCPPConformance();
	}

	for (ui = 0; ui < NUM_TEST_FILES; ui++)
	{
		if (TestFileChosen(gasTestFiles[ui].szName, szOnly) && ((guiTestParts == 1 && !gbTestPartCalls) || gasTestFiles[ui].bParts))
		{
			gasTestFiles[ui].fTest();
		}
	}

	iTotalTestsFailed = TestTotalStatistics();

	LeakDetection();
	return iTotalTestsFailed;
}
