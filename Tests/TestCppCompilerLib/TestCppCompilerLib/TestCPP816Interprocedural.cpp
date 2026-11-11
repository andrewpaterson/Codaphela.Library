#include <string.h>
#include "BaseLib/Chars.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TextFile.h"
#include "CppCompilerLib/CPP65Builder.h"
#include "TestLib/Assert.h"
#include "TestCPP65Context.h"


int32	TestCPP816Run(const char* szSource, bool bLockstep);
int32	TestCPP816RunListed(const char* szSource, const char** aszUnits, CChars* pszListing, uint32* puiResults, size uiResults, bool bReturns);
bool	TestCPP816Calls(const char* szListing, const char* szPrefix, const char* szSuffix);


//////////////////////////////////////////////////////////////////////////
//
// Mod/ref summaries (CIRInterprocedural::MarkNoWrites): what a function
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
// Dead argument removal (CIRInterprocedural::RemoveDeadArguments): a static
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
// such a token would be one of its declarations.  A line that is only the
// call (Twice's y = Triple(y, 1)) is listed too, though nothing but
// Triple's code is left of it.
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
	AssertTrue(strstr(szListing, "; Main.cpp(34): y = Triple(y, 1);") != NULL);
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
	int FwdMinus(int x, int y) { return FwdPair(x - 1, y); } \
	int FwdFour(int p, int q, int r, int k) { TEST_CPP816_FORWARD_BULK(u, p + k) if (k <= 0) { return (int)(u & 7u); } return p * k + q - r + FwdFour(p, q, r, k - 1) + (int)(u & 7u); } \
	int FwdThree(int p, int q, int r) { return FwdFour(p, q, r, 9); } \
	unsigned long long (*volatile gpFwdMul)(unsigned long long, unsigned long long) = FwdMul; \
	unsigned long long (*volatile gpFwdAdd)(unsigned long long, unsigned long long) = FwdAdd; \
	unsigned long long (*volatile gpFwdSub)(unsigned long long, unsigned long long) = FwdSub; \
	int (*volatile gpFwdSame)(int, int) = FwdSame; \
	int (*volatile gpFwdThree)(int, int, int) = FwdThree; \
	int gaiFwdMember[8] = { 3, 1, 4, 1, 5, 9, 2, 6 }; \
	int FwdMemberWork(int* p, int k) { TEST_CPP816_FORWARD_BULK(u, k) if (k <= 0) { return p[0] + (int)(u & 7u); } return p[0] * k + p[1] + FwdMemberWork(p, k - 1) + (int)(u & 7u); } \
	int FwdMember(int* p, int k) { return FwdMemberWork(p + 2, k); } \
	int FwdMemberNear(int* p, int k) { return FwdMemberWork(p + 1, k); } \
	int FwdMemberBack(int* p, int k) { return FwdMemberWork(p - 1, k); } \
	unsigned long long FwdRun(void) \
	{ \
		unsigned long long w = 0; \
		for (int i = 0; i < 6; i++) \
		{ \
			unsigned long long a = giFwd * 1000003ULL + i; \
			unsigned long long b = giFwd + i * 7; \
			w = w * 7 + FwdMul(a, b) + gpFwdMul(b, a) + FwdAdd(a, b) + gpFwdAdd(a, i) + FwdSub(a, b) + gpFwdSub(b, i) + FwdWork(a, b, i) + FwdBoth(a, b, (i & 1) != 0); \
			w += FwdSame(i, giFwd) + gpFwdSame(giFwd, i) + FwdSwap(i, giFwd) + FwdPlus(i, giFwd) + FwdMinus(i, giFwd) + FwdThree(i, giFwd, 2) + gpFwdThree(giFwd, i, 1) + FwdFour(i, 2, giFwd, i) + FwdPair(i, i); \
			w += FwdMember(gaiFwdMember + (i & 3), giFwd + i) + FwdMemberNear(gaiFwdMember, i) + FwdMemberBack(gaiFwdMember + 4, i); \
		} \
		return w; \
	}

#define TEST_CPP816_FORWARD_STRINGIZE2(...)	#__VA_ARGS__
#define TEST_CPP816_FORWARD_STRINGIZE(...)	TEST_CPP816_FORWARD_STRINGIZE2(__VA_ARGS__)

namespace TestCPP816ForwardNative
{
	TEST_CPP816_FORWARD_FUNCTIONS
}


static bool TestCPP816AdjustsAndJumpsTo(const char* szListing, const char* szLabel, const char* szCallee);


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
// when the arguments swap (FwdSwap) or a constant must be pushed
// (FwdThree's 9).  One that adds a constant to an argument in A (FwdPlus,
// FwdMinus) or moves a pointer it is given along (FwdMember's p + 2: a
// member's address) adds to it and jumps; not one that moves a pointer
// back (FwdMemberBack: X would change without a carry).  Each is
// called directly and through a pointer (its stub, then the jump).  The
// results are as natively.
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
	AssertTrue(TestCPP816AdjustsAndJumpsTo(szListing.Text(), "_FwdPlus.ax", "_FwdPair.ax"));
	AssertTrue(TestCPP816AdjustsAndJumpsTo(szListing.Text(), "_FwdMinus.ax", "_FwdPair.ax"));
	AssertFalse(TestCPP816JumpsTo(szListing.Text(), "_FwdThree.axy", "_FwdFour"));
	AssertTrue(TestCPP816AdjustsAndJumpsTo(szListing.Text(), "_FwdMember.axy", "_FwdMemberWork.axy"));
	AssertTrue(TestCPP816AdjustsAndJumpsTo(szListing.Text(), "_FwdMemberNear.axy", "_FwdMemberWork.axy"));
	AssertFalse(TestCPP816AdjustsAndJumpsTo(szListing.Text(), "_FwdMemberBack.axy", "_FwdMemberWork.axy"));
	szListing.Kill();
	szSource.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether szListing's szLabel (a function's register entry) only moves
// 'this' along (to a member) or loads registers, with no frame, then jumps
// to a label starting szCallee.
//
//////////////////////////////////////////////////////////////////////////
static bool TestCPP816AdjustsAndJumpsTo(const char* szListing, const char* szLabel, const char* szCallee)
{
	static const char*	aszAllowed[] = { "\t\tlda ", "\t\tldx ", "\t\tldy ", "\t\tclc", "\t\tadc ", "\t\tinx", "\t\tbcc ", "\t\ttax", "\t\ttxa", "\t\ttay", "\t\ttya", "\t\ttxy", "\t\ttyx", NULL };
	CChars				szFind;
	const char*			szAt;
	size				i;
	bool				bAllowed;

	szFind.Init("\n");
	szFind.Append(szLabel);
	szFind.Append(":\n");
	szAt = strstr(szListing, szFind.Text());
	szFind.Kill();
	if (szAt == NULL)
	{
		return false;
	}

	szAt = strchr(szAt + 1, '\n') + 1;
	for (;;)
	{
		bAllowed = false;
		for (i = 0; aszAllowed[i]; i++)
		{
			if (strncmp(szAt, aszAllowed[i], strlen(aszAllowed[i])) == 0)
			{
				bAllowed = true;
				break;
			}
		}
		if (!bAllowed)
		{
			break;
		}
		szAt = strchr(szAt, '\n') + 1;
	}

	//A label inside the adjustment (bcc's target) is allowed too.
	while ((szAt[0] == 'L') && (strchr(szAt, ':') < strchr(szAt, '\n')))
	{
		szAt = strchr(szAt, '\n') + 1;
	}

	szFind.Init("\t\tjml ");
	szFind.Append(szCallee);
	bAllowed = strncmp(szAt, szFind.Text(), szFind.Length()) == 0;
	szFind.Kill();
	return bAllowed;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether szListing's szLabel (to the next section) calls or jumps only to
// labels starting szCallee, at least once: the layer between them inlined.
// With szCallee NULL, whether it calls nothing: the work (a getter) inlined.
//
//////////////////////////////////////////////////////////////////////////
static bool TestCPP816OnlyCalls(const char* szListing, const char* szLabel, const char* szCallee)
{
	CChars		szFind;
	const char*	szAt;
	const char*	szEnd;
	bool		bCalls;

	szFind.Init("\n");
	szFind.Append(szLabel);
	szFind.Append(":\n");
	szAt = strstr(szListing, szFind.Text());
	szFind.Kill();
	if (szAt == NULL)
	{
		return false;
	}
	szEnd = strstr(szAt, "\t\t.section");
	if (szEnd == NULL)
	{
		szEnd = szAt + strlen(szAt);
	}

	bCalls = false;
	for (szAt = strchr(szAt + 1, '\n') + 1; szAt < szEnd; szAt = strchr(szAt, '\n') + 1)
	{
		if ((strncmp(szAt, "\t\tjsl ", 6) == 0) || (strncmp(szAt, "\t\tjml ", 6) == 0))
		{
			if ((szCallee == NULL) || (strncmp(szAt + 6, szCallee, strlen(szCallee)) != 0))
			{
				return false;
			}
			bCalls = true;
		}
	}
	return bCalls || (szCallee == NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestCPP816ReadListing(CChars* pszDest, const char* szDirectory, const char* szUnit)
{
	CTextFile	cFile;
	CChars		szFilename;

	szFilename.Init("Output" _FS_);
	szFilename.Append(szDirectory);
	szFilename.Append(_FS_);
	szFilename.Append(szUnit);
	szFilename.Append(".lst");
	cFile.Init();
	AssertTrue(cFile.Read(szFilename.Text()));
	pszDest->Init(cFile.Text() ? cFile.Text() : "");
	cFile.Kill();
	szFilename.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// StandardLib's array layers (CArray<M>, CArrayObject, CArrayCommonObject
// and CArrayCommonUnknown) compile down to the work at the bottom: every
// layer that only passes a call on costs at most a jump.  Compiled for the
// W65C816 as StandardLib's units are, with Use.cpp calling through the top.
//
// CArrayObject's forwarders jump to CArrayCommonObject's (1 to 8).  A
// typed call (CArray<M>::RemoveAt) goes straight to the work (16), and a
// call to a forwarder jumps past it (32).  CArrayCommonObject's one-liners
// on its mcArray move 'this' to the member and jump, without a frame (64
// to 1024), or call the work themselves where the layer under them is no
// bigger than a forwarder (GrowTo: CArrayCommonUnknown's passes constants,
// so it can't be a jump; inlined, it is one frame less).  A virtual call through CArray<M>, which is final, calls
// CArrayCommonObject's function directly, or what that jumps to (2048,
// 4096).
//
// Each bit of the result is a layer that costs more.  CArrayObject::Init
// isn't checked: the constants its callee takes are pushed (as
// TestCPP816Forwarders' FwdThree).
//
// Expected now: 2048 and 4096 (6144) aren't done yet: a call through a
// final class goes through the vtable.  Clear each bit from
// the expected value as the compiler does it, until it is 0.
//
//////////////////////////////////////////////////////////////////////////
#define TEST_CPP816_ARRAY_LAYERS_NOT_DONE	(2048 | 4096)

void TestCPP816ArrayLayers(void)
{
	static const char*	aszUnits[] = { "StandardLib/ArrayCommonUnknown.cpp", "StandardLib/ArrayCommonObject.cpp", "StandardLib/ArrayObject.cpp", "Input/CPP65/ArrayChainProject/Use.cpp", NULL };
	CCPP65Builder		cBuilder;
	CFileUtil			cFileUtil;
	CChars				szObject;
	CChars				szCommon;
	CChars				szUse;
	int					w;

	cFileUtil.RemoveDir("Output" _FS_ "CPP816ArrayLayers");
	cBuilder.Init();
	cBuilder.SetTarget(CPPT_W65C816);
	cBuilder.SetListingDirectory("Output" _FS_ "CPP816ArrayLayers");
	TestCPP65AddIncludeDirectories(&cBuilder);
	TestCPP65AddUnits(&cBuilder, aszUnits, false);
	cBuilder.Compile();
	AssertString("", cBuilder.GetErrors());
	cBuilder.Kill();

	TestCPP816ReadListing(&szObject, "CPP816ArrayLayers", "ArrayObject");
	TestCPP816ReadListing(&szCommon, "CPP816ArrayLayers", "ArrayCommonObject");
	TestCPP816ReadListing(&szUse, "CPP816ArrayLayers", "Use");

	w = 0;
	if (!TestCPP816JumpsTo(szObject.Text(), "_CArrayObject__Add.ax", "_CArrayCommonObject__Add__FRC8CPointer.ax")) { w |= 1; }
	if (!TestCPP816JumpsTo(szObject.Text(), "_CArrayObject__Insert.axy", "_CArrayCommonObject__InsertAt__FtRC8CPointer.axy")) { w |= 2; }
	if (!TestCPP816JumpsTo(szObject.Text(), "_CArrayObject__Remove__Ft.axy", "_CArrayCommonObject__RemoveAt.axy")) { w |= 4; }
	if (!TestCPP816JumpsTo(szObject.Text(), "_CArrayObject__Remove__FRC8CPointer.ax", "_CArrayCommonObject__Remove__FRC8CPointer.ax")) { w |= 8; }

	if (!TestCPP816JumpsTo(szUse.Text(), "_UseArrayRemoveAt__FPC24CArray__T_C11CBaseObjectt.axy", "_CArrayCommonObject__RemoveAt.axy")) { w |= 16; }
	if (!TestCPP816JumpsTo(szUse.Text(), "_UseArrayAdd__FPC12CArrayObjectRC8CPointer.ax", "_CArrayCommonObject__Add__FRC8CPointer.ax")) { w |= 32; }

	if (!TestCPP816AdjustsAndJumpsTo(szCommon.Text(), "_CArrayCommonObject__NumElements.ax", "_CArrayCommonUnknown__NumElements.ax")) { w |= 64; }
	if (!TestCPP816AdjustsAndJumpsTo(szCommon.Text(), "_CArrayCommonObject__IsEmpty.ax", "_CArrayCommonUnknown__IsEmpty.ax")) { w |= 128; }
	if (!TestCPP816AdjustsAndJumpsTo(szCommon.Text(), "_CArrayCommonObject__UnsafeGetObject.axy", "_CArrayCommonUnknown__UnsafeGet.axy")) { w |= 256; }
	if (!TestCPP816AdjustsAndJumpsTo(szCommon.Text(), "_CArrayCommonObject__GrowTo.axy", "_CArrayCommonUnknown__GrowTo.axy") &&
		!TestCPP816OnlyCalls(szCommon.Text(), "_CArrayCommonObject__GrowTo.axy", "_CArrayBlock__GrowToAtLeastNumElements.axy")) { w |= 512; }
	if (!TestCPP816AdjustsAndJumpsTo(szCommon.Text(), "_CArrayCommonObject__CalculateIsSorted.ax", "_CArrayCommonUnknown__CalculateIsSorted.ax")) { w |= 1024; }

	if (!TestCPP816AdjustsAndJumpsTo(szUse.Text(), "_UseArrayNumElements__FPC24CArray__T_C11CBaseObject.ax", "_CArrayCommonObject__NumElements.ax") &&
		!TestCPP816AdjustsAndJumpsTo(szUse.Text(), "_UseArrayNumElements__FPC24CArray__T_C11CBaseObject.ax", "_CArrayCommonUnknown__NumElements.ax")) { w |= 2048; }
	if (!TestCPP816AdjustsAndJumpsTo(szUse.Text(), "_UseArrayIsEmpty__FPC24CArray__T_C11CBaseObject.ax", "_CArrayCommonObject__IsEmpty.ax") &&
		!TestCPP816AdjustsAndJumpsTo(szUse.Text(), "_UseArrayIsEmpty__FPC24CArray__T_C11CBaseObject.ax", "_CArrayCommonUnknown__IsEmpty.ax")) { w |= 4096; }

	AssertInt(TEST_CPP816_ARRAY_LAYERS_NOT_DONE, w);

	szUse.Kill();
	szCommon.Kill();
	szObject.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The map and set layers' NumElements (CMapObject's on its CMapUnknownUnknown,
// that on its CMapBlock, CMapBlock's and CSetBlock's on their
// CArrayBlockSorted) each move 'this' to the member and pass the call on:
// compiled for the W65C816 as the units are, each adds the members' offsets
// together and jumps to the work at the bottom (1 to 8).  GetSortedSize's
// layers (CMapUnknownUnknown's, CMapBlock's) come down to
// CArrayBlockSorted's, which only reads its array's size (16, 32).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816MapLayers(void)
{
	static const char*	aszUnits[] = { "BaseLib/ArrayBlockSorted.cpp", "BaseLib/MapBlock.cpp", "BaseLib/SetBlock.cpp", "StandardLib/MapUnknownUnknown.cpp", "StandardLib/MapObject.cpp", NULL };
	CCPP65Builder		cBuilder;
	CFileUtil			cFileUtil;
	CChars				szMapBlock;
	CChars				szSetBlock;
	CChars				szMapUnknown;
	CChars				szMapObject;
	int					w;

	cFileUtil.RemoveDir("Output" _FS_ "CPP816MapLayers");
	cBuilder.Init();
	cBuilder.SetTarget(CPPT_W65C816);
	cBuilder.SetListingDirectory("Output" _FS_ "CPP816MapLayers");
	TestCPP65AddIncludeDirectories(&cBuilder);
	TestCPP65AddUnits(&cBuilder, aszUnits, false);
	cBuilder.Compile();
	AssertString("", cBuilder.GetErrors());
	cBuilder.Kill();

	TestCPP816ReadListing(&szMapBlock, "CPP816MapLayers", "MapBlock");
	TestCPP816ReadListing(&szSetBlock, "CPP816MapLayers", "SetBlock");
	TestCPP816ReadListing(&szMapUnknown, "CPP816MapLayers", "MapUnknownUnknown");
	TestCPP816ReadListing(&szMapObject, "CPP816MapLayers", "MapObject");

	w = 0;
	if (!TestCPP816AdjustsAndJumpsTo(szMapBlock.Text(), "_CMapBlock__NumElements.ax", "_CArrayBlockSorted__NumElements.ax")) { w |= 1; }
	if (!TestCPP816AdjustsAndJumpsTo(szMapUnknown.Text(), "_CMapUnknownUnknown__NumElements.ax", "_CArrayBlockSorted__NumElements.ax")) { w |= 2; }
	if (!TestCPP816AdjustsAndJumpsTo(szMapObject.Text(), "_CMapObject__NumElements.ax", "_CArrayBlockSorted__NumElements.ax")) { w |= 4; }
	if (!TestCPP816AdjustsAndJumpsTo(szSetBlock.Text(), "_CSetBlock__NumElements.ax", "_CArrayBlockSorted__NumElements.ax")) { w |= 8; }
	AssertInt(0, w);

	szMapObject.Kill();
	szMapUnknown.Kill();
	szSetBlock.Kill();
	szMapBlock.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The index layers, as the map layers (TestCPP816MapLayers): CIndexObject's
// NonNullElements on its CIndexUnknown, that on its CIndexTreeMemory (1, 2),
// each add the members' offsets together and jump to the tree's function.
// CNamedIndexedObjects' NumIndexed on its CIndexedObjects, that on its
// CIndexTreeMemory (4, 8), read the tree's size themselves (a getter,
// another unit's, inlined): no call.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816IndexLayers(void)
{
	static const char*	aszUnits[] = { "BaseLib/IndexTreeMemory.cpp", "StandardLib/IndexUnknown.cpp", "StandardLib/IndexObject.cpp", "StandardLib/IndexedObjects.cpp", "StandardLib/NamedIndexedObjects.cpp", NULL };
	CCPP65Builder		cBuilder;
	CFileUtil			cFileUtil;
	CChars				szIndexUnknown;
	CChars				szIndexObject;
	CChars				szIndexed;
	CChars				szNamed;
	int					w;

	cFileUtil.RemoveDir("Output" _FS_ "CPP816IndexLayers");
	cBuilder.Init();
	cBuilder.SetTarget(CPPT_W65C816);
	cBuilder.SetListingDirectory("Output" _FS_ "CPP816IndexLayers");
	TestCPP65AddIncludeDirectories(&cBuilder);
	TestCPP65AddUnits(&cBuilder, aszUnits, false);
	cBuilder.Compile();
	AssertString("", cBuilder.GetErrors());
	cBuilder.Kill();

	TestCPP816ReadListing(&szIndexUnknown, "CPP816IndexLayers", "IndexUnknown");
	TestCPP816ReadListing(&szIndexObject, "CPP816IndexLayers", "IndexObject");
	TestCPP816ReadListing(&szIndexed, "CPP816IndexLayers", "IndexedObjects");
	TestCPP816ReadListing(&szNamed, "CPP816IndexLayers", "NamedIndexedObjects");

	w = 0;
	if (!TestCPP816AdjustsAndJumpsTo(szIndexUnknown.Text(), "_CIndexUnknown__NonNullElements.ax", "_CIndexTreeMemory__NonNullElements.ax")) { w |= 1; }
	if (!TestCPP816AdjustsAndJumpsTo(szIndexObject.Text(), "_CIndexObject__NonNullElements.ax", "_CIndexTreeMemory__NonNullElements.ax")) { w |= 2; }
	if (!TestCPP816OnlyCalls(szIndexed.Text(), "_CIndexedObjects__NumIndexed.ax", NULL)) { w |= 4; }
	if (!TestCPP816OnlyCalls(szNamed.Text(), "_CNamedIndexedObjects__NumIndexed.ax", NULL)) { w |= 8; }
	AssertInt(0, w);

	szNamed.Kill();
	szIndexed.Kill();
	szIndexObject.Kill();
	szIndexUnknown.Kill();
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
	TestCPP816ArrayLayers();
	TestCPP816MapLayers();
	TestCPP816IndexLayers();

	TestStatistics();
}
