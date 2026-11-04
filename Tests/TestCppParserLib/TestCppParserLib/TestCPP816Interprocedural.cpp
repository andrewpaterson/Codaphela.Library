#include <string.h>
#include "BaseLib/Chars.h"
#include "TestLib/Assert.h"


int32	TestCPP816Run(const char* szSource, bool bLockstep);
int32	TestCPP816RunListed(const char* szSource, const char** aszUnits, CChars* pszListing, uint32* puiResults, size uiResults, bool bReturns);
bool	TestCPP816Calls(const char* szListing, const char* szPrefix, const char* szSuffix);


//////////////////////////////////////////////////////////////////////////
//
// Mod/ref summaries (CCPP65IRFrontEnd::MarkNoWrites): what a function
// knows is in memory survives calls to the unit's functions that write
// none of it, and only those.  A global read, then written by a call, is
// read again; a function that reads but calls one that writes, writes;
// two that call each other and store only to their frames write nothing;
// a local array a callee fills is its caller's memory.  Each bit of the
// result is one that went wrong.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816NoWritesCalls(void)
{
	AssertInt(0, TestCPP816Run(
		"int giValue = 5;\n"
		"int gaiTable[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };\n"
		"int Peek(int i) { int a[4]; for (int j = 0; j < 4; j++) { a[j] = gaiTable[(i + j) & 7]; } return a[0] + a[3]; }\n"
		"void Poke(int v) { giValue = v; }\n"
		"int PeekThenPoke(int i) { int r = Peek(i); Poke(r); return r; }\n"
		"int Even(int n);\n"
		"int Odd(int n) { int t[2]; t[0] = n; t[1] = 0; return (t[0] == 0) ? 0 : Even(n - 1); }\n"
		"int Even(int n) { return (n == 0) ? 1 : Odd(n - 1); }\n"
		"void Fill(int* p, int n) { for (int i = 0; i < n; i++) { p[i] = i * 3; } }\n"
		"volatile int gi = 2;\n"
		"int main(void)\n"
		"{\n"
		"\tint w = 0;\n"
		"\tint a = giValue;\n"
		"\tint p = Peek(gi);\n"
		"\tint b = giValue;\n"
		"\tif ((a != 5) || (b != 5) || (p != 9)) { w |= 1; }\n"
		"\tPoke(11);\n"
		"\tif (giValue != 11) { w |= 2; }\n"
		"\tint c = giValue;\n"
		"\tint r = PeekThenPoke(gi + 1);\n"
		"\tif ((c != 11) || (r != 11) || (giValue != 11)) { w |= 4; }\n"
		"\tgiValue = 1;\n"
		"\tr = PeekThenPoke(0);\n"
		"\tif ((r != 5) || (giValue != 5)) { w |= 8; }\n"
		"\tint e = giValue;\n"
		"\tif ((Even(gi + 4) != 1) || (Odd(gi + 4) != 0) || (giValue != e)) { w |= 16; }\n"
		"\tint ai[6];\n"
		"\tai[2] = 100;\n"
		"\tint before = ai[2];\n"
		"\tFill(ai, 6);\n"
		"\tif ((before != 100) || (ai[2] != 6) || (ai[5] != 15)) { w |= 32; }\n"
		"\treturn w;\n"
		"}\n", false));
}


//////////////////////////////////////////////////////////////////////////
//
// Dead argument removal (CCPP65IRFrontEnd::RemoveDeadArguments): a static
// function's parameters it never reads go, and its calls' arguments with
// them, so its register suffix is that of the parameters left.  One in A,
// a long that would have been pushed, one between used ones and one of a
// recursive function (its own calls lose it too); a function whose
// address is taken, or that isn't static, keeps them all.  The results are
// those of the program as written.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816DeadArguments(void)
{
	static const char*	szSource =
		"volatile int gi = 3;\n"
		"static int Pick(int iUnused, int a, long lUnused, int b) { int s = 0; for (int i = 0; i < a; i++) { s += b * i; } return s; }\n"
		"static long Fact(int iUnused, int n) { return (n <= 1) ? 1 : n * Fact(0, n - 1); }\n"
		"static int Taken(int iUnused, int a) { int s = 0; for (int i = 0; i < a; i++) { s += i; } return s; }\n"
		"int Global(int iUnused, int a) { int s = 0; for (int i = 0; i < a; i++) { s += i * 2; } return s; }\n"
		"int (*volatile gpTaken)(int, int) = Taken;\n"
		"int main(void)\n"
		"{\n"
		"\tint w = 0;\n"
		"\tif (Pick(gi, gi + 2, 70000L * gi, 4) != 40) { w |= 1; }\n"
		"\tif (Pick(1, gi, 2L, gi) != 9) { w |= 2; }\n"
		"\tif (Fact(gi, gi + 3) != 720L) { w |= 4; }\n"
		"\tif ((gpTaken(1, gi) != 3) || (Taken(gi, 4) != 6)) { w |= 8; }\n"
		"\tif (Global(gi, gi) != 6) { w |= 16; }\n"
		"\treturn w;\n"
		"}\n";
	static const char*	aszNone[] = { NULL };
	CChars				szListing;

	AssertInt(0, TestCPP816RunListed(szSource, aszNone, &szListing, NULL, 0, true));

	//Each function's register entry: Pick(a, b) A and X (not A, X, two pushed words and Y), Fact(n) A, and Taken and
	//Global keep both of theirs (A and X).  (Pick and Global are inlined into main, so there are no calls to look at.)
	AssertTrue(strstr(szListing.Text(), "\n_Pick__Fssis.ax:") != NULL);
	AssertTrue(strstr(szListing.Text(), "\n_Fact__Fss.a:") != NULL);
	AssertTrue(TestCPP816Calls(szListing.Text(), "_Fact", ".a"));
	AssertTrue(strstr(szListing.Text(), "\n_Taken__Fss.ax:") != NULL);
	AssertTrue(strstr(szListing.Text(), "\n_Global__Fss.ax:") != NULL);
	szListing.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816Interprocedural(void)
{
	BeginTests();

	TestCPP816NoWritesCalls();
	TestCPP816DeadArguments();

	TestStatistics();
}
