#include <string.h>
#include "BaseLib/Chars.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TextFile.h"
#include "CppParserLib/CPP65Builder.h"
#include "TestLib/Assert.h"
#include "TestCPP65Context.h"


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
// A function inlined from another unit is listed as the line that calls
// it: Main.cpp inlines Maths.cpp's Triple, and its code is under
// Main.cpp(27), the call.  CIRImports::Export copies functions as IR text
// without their source tokens, so imported code has none (a token of
// Maths.cpp would mean another line of Main.cpp).  Main.cpp is padded so
// such a token would be one of its declarations.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816ListingInlinedFromUnit(void)
{
	static const char*	aszUnits[] = { "Input/CPP65/ListingInlineProject/Maths.cpp", "Input/CPP65/ListingInlineProject/Main.cpp", NULL };
	CCPP65Builder		cBuilder;
	CFileUtil			cFileUtil;
	CTextFile			cFile;
	const char*			szListing;

	cFileUtil.RemoveDir("Output" _FS_ "CPP816InlineListing");
	cBuilder.Init();
	cBuilder.SetTarget(CPPT_W65C816);
	cBuilder.SetListingDirectory("Output" _FS_ "CPP816InlineListing");
	cBuilder.SetLineTable(true);
	TestCPP65AddIncludeDirectories(&cBuilder);
	TestCPP65AddUnits(&cBuilder, aszUnits, false);
	cBuilder.Compile();
	AssertString("", cBuilder.GetErrors());
	cBuilder.Kill();

	cFile.Init();
	AssertTrue(cFile.Read("Output" _FS_ "CPP816InlineListing" _FS_ "Main.lst"));
	szListing = cFile.Text() ? cFile.Text() : "";

	//Triple is inlined, its code listed under the call and none as Main.cpp's padding or Maths.cpp.
	AssertTrue(strstr(szListing, "jsl _Triple") == NULL);
	AssertTrue(strstr(szListing, "; Main.cpp(27): int r = Triple(gi, 5);") != NULL);
	AssertTrue(strstr(szListing, "Maths.cpp") == NULL);
	AssertTrue(strstr(szListing, "): int giPad") == NULL);
	cFile.Kill();
}


//The functions of TestCPP816Forwarders, natively for the expected result and (extern "C", for their labels) for the
//W65C816.  The workers do enough (TEST_CPP816_FORWARD_BULK, masked to come out the same in 16 and 32 bits) not to be
//inlined into the wrappers, as softfloat's aren't.
#define TEST_CPP816_FORWARD_STEP(u, v) u = ((u * 5u) ^ ((unsigned)(v) & 0xFFu)) & 0x3FFFu; if (u & 8u) { u = (u + 7u) & 0x3FFFu; } else { u = (u + 3u) & 0x3FFFu; } u = ((u << 2) | (u >> 11)) & 0x3FFFu;
#define TEST_CPP816_FORWARD_BULK(u, v) unsigned int u = 1u; TEST_CPP816_FORWARD_STEP(u, v) TEST_CPP816_FORWARD_STEP(u, v) TEST_CPP816_FORWARD_STEP(u, v) TEST_CPP816_FORWARD_STEP(u, v) \
	TEST_CPP816_FORWARD_STEP(u, v) TEST_CPP816_FORWARD_STEP(u, v) TEST_CPP816_FORWARD_STEP(u, v) TEST_CPP816_FORWARD_STEP(u, v) TEST_CPP816_FORWARD_STEP(u, v) TEST_CPP816_FORWARD_STEP(u, v) \
	TEST_CPP816_FORWARD_STEP(u, v) TEST_CPP816_FORWARD_STEP(u, v)
#define TEST_CPP816_FORWARD_FUNCTIONS \
	volatile int giFwd = 3; \
	unsigned long long FwdWork(unsigned long long a, unsigned long long b, int k) { TEST_CPP816_FORWARD_BULK(u, k) if (k <= 0) { return u; } return FwdWork(a, b, k - 1) * 3 + (a ^ (b >> k)) + u; } \
	unsigned long long FwdMul(unsigned long long a, unsigned long long b) { return FwdWork(a, b, 5); } \
	unsigned long long FwdBoth(unsigned long long a, unsigned long long b, bool s) { TEST_CPP816_FORWARD_BULK(u, b) if (b == 0) { return a + u; } return FwdBoth(s ? a - b : a + b, b >> 1, s) ^ u; } \
	unsigned long long FwdAdd(unsigned long long a, unsigned long long b) { return FwdBoth(a, b, false); } \
	unsigned long long FwdSub(unsigned long long a, unsigned long long b) { return FwdBoth(a, b, true); } \
	int FwdPair(int x, int y) { TEST_CPP816_FORWARD_BULK(u, x + y) if (x <= 0) { return (int)(u & 7u); } return (y ^ x) + FwdPair(x - 1, y) + (int)(u & 7u); } \
	int FwdSame(int x, int y) { return FwdPair(x, y); } \
	int FwdSwap(int x, int y) { return FwdPair(y, x); } \
	int FwdPlus(int x, int y) { return FwdPair(x + 1, y); } \
	int FwdFour(int p, int q, int r, int k) { TEST_CPP816_FORWARD_BULK(u, p + k) if (k <= 0) { return (int)(u & 7u); } return p * k + q - r + FwdFour(p, q, r, k - 1) + (int)(u & 7u); } \
	int FwdThree(int p, int q, int r) { return FwdFour(p, q, r, 9); } \
	unsigned long long (*volatile gpFwdMul)(unsigned long long, unsigned long long) = FwdMul; \
	unsigned long long (*volatile gpFwdAdd)(unsigned long long, unsigned long long) = FwdAdd; \
	unsigned long long (*volatile gpFwdSub)(unsigned long long, unsigned long long) = FwdSub; \
	int (*volatile gpFwdSame)(int, int) = FwdSame; \
	int (*volatile gpFwdThree)(int, int, int) = FwdThree; \
	unsigned long long FwdRun(void) \
	{ \
		unsigned long long w = 0; \
		for (int i = 0; i < 6; i++) \
		{ \
			unsigned long long a = giFwd * 1000003ULL + i; \
			unsigned long long b = giFwd + i * 7; \
			w = w * 7 + FwdMul(a, b) + gpFwdMul(b, a) + FwdAdd(a, b) + gpFwdAdd(a, i) + FwdSub(a, b) + gpFwdSub(b, i) + FwdWork(a, b, i) + FwdBoth(a, b, (i & 1) != 0); \
			w += FwdSame(i, giFwd) + gpFwdSame(giFwd, i) + FwdSwap(i, giFwd) + FwdPlus(i, giFwd) + FwdThree(i, giFwd, 2) + gpFwdThree(giFwd, i, 1) + FwdFour(i, 2, giFwd, i) + FwdPair(i, i); \
		} \
		return w; \
	}

#define TEST_CPP816_FORWARD_STRINGIZE2(...)	#__VA_ARGS__
#define TEST_CPP816_FORWARD_STRINGIZE(...)	TEST_CPP816_FORWARD_STRINGIZE2(__VA_ARGS__)

namespace TestCPP816ForwardNative
{
	TEST_CPP816_FORWARD_FUNCTIONS
}


//////////////////////////////////////////////////////////////////////////
//
// Whether szListing's szLabel (a function's register entry) is only loads
// of constants and a jml to a label starting szCallee.
//
//////////////////////////////////////////////////////////////////////////
static bool TestCPP816JumpsTo(const char* szListing, const char* szLabel, const char* szCallee)
{
	CChars		szFind;
	const char*	szAt;

	szFind.Init("\n");
	szFind.Append(szLabel);
	szFind.Append(":\n");
	szAt = strstr(szListing, szFind.Text());
	if (szAt)
	{
		szAt += szFind.Length();
		while ((strncmp(szAt, "\t\tlda ", 6) == 0) || (strncmp(szAt, "\t\tldx ", 6) == 0) || (strncmp(szAt, "\t\tldy ", 6) == 0))
		{
			szAt = strchr(szAt, '\n') + 1;
		}
		szFind.Kill();
		szFind.Init("\t\tjml ");
		szFind.Append(szCallee);
		if (strncmp(szAt, szFind.Text(), szFind.Length()) != 0)
		{
			szAt = NULL;
		}
	}
	szFind.Kill();
	return szAt != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A function that only passes its arguments on to another (a wrapper,
// CIR816Backend::Forwarder) jumps to it: those whose arguments are already
// where the callee wants them, with constants the callee takes in Y loaded
// (FwdMul's 5, FwdAdd's false, FwdSub's true), or none (FwdSame).  Not
// when the arguments swap (FwdSwap), change (FwdPlus) or a constant must
// be pushed (FwdThree's 9).  Each is called directly and through a pointer
// (its stub, then the jump).  The results are as natively.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816Forwarders(void)
{
	static const char*	aszNoUnits[] = { NULL };
	CChars				szSource;
	CChars				szListing;
	unsigned long long	ullExpected;

	ullExpected = TestCPP816ForwardNative::FwdRun();
	szSource.Init("extern \"C\"\n{\n");
	szSource.Append(TEST_CPP816_FORWARD_STRINGIZE(TEST_CPP816_FORWARD_FUNCTIONS));
	szSource.Append("\n}\nint main(void) { unsigned long long w = FwdRun(); return (int)((w ^ (w >> 16) ^ (w >> 32) ^ (w >> 48)) & 0x7FFF); }\n");

	szListing.Init();
	AssertInt((int)((ullExpected ^ (ullExpected >> 16) ^ (ullExpected >> 32) ^ (ullExpected >> 48)) & 0x7FFF), TestCPP816RunListed(szSource.Text(), aszNoUnits, &szListing, NULL, 0, true));
	AssertTrue(TestCPP816JumpsTo(szListing.Text(), "_FwdMul.ax", "_FwdWork"));
	AssertTrue(TestCPP816JumpsTo(szListing.Text(), "_FwdAdd.ax", "_FwdBoth"));
	AssertTrue(TestCPP816JumpsTo(szListing.Text(), "_FwdSub.ax", "_FwdBoth"));
	AssertTrue(TestCPP816JumpsTo(szListing.Text(), "_FwdSame.ax", "_FwdPair"));
	AssertFalse(TestCPP816JumpsTo(szListing.Text(), "_FwdSwap.ax", "_FwdPair"));
	AssertFalse(TestCPP816JumpsTo(szListing.Text(), "_FwdPlus.ax", "_FwdPair"));
	AssertFalse(TestCPP816JumpsTo(szListing.Text(), "_FwdThree.axy", "_FwdFour"));
	szListing.Kill();
	szSource.Kill();
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
	TestCPP816ListingInlinedFromUnit();
	TestCPP816Forwarders();

	TestStatistics();
}
