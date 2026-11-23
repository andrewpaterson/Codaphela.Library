#include "BaseLib/Chars.h"
#include "BaseLib/FileUtil.h"
#include "CppCompilerLib/CPP65Builder.h"
#include "TestLib/Assert.h"
#include "TestCPP65Context.h"


int32				TestCPP816Run(const char* szSource, bool bLockstep);	//TestCPPW65C816.cpp
CCPPCompileOptions*	TestCPP816Options(void);


//A program built for the W65C816 with its debug info (CCPP65Builder::SetLineTable), and how it was compiled.
struct STestCPP816Debug
{
	CCPP65Builder		cBuilder;
	CAsm65DebugRecords*	pcRecords;
};


//////////////////////////////////////////////////////////////////////////
//
// Builds szSource with its debug info, as the IDE does, without inlining
// (so each function is a function of its own).
//
//////////////////////////////////////////////////////////////////////////
static bool TestCPP816DebugBuild(STestCPP816Debug* psDebug, const char* szSource)
{
	CChars		szProject;
	CFileUtil	cFileUtil;
	bool		bResult;

	psDebug->cBuilder.Init();
	psDebug->cBuilder.GetOptions()->mbInline = false;
	psDebug->cBuilder.SetTarget(CPPT_W65C816);
	psDebug->cBuilder.SetLineTable(true);
	TestCPP65AddIncludeDirectories(&psDebug->cBuilder);
	AssertTrue(psDebug->cBuilder.AddStartUp());
	szProject.Init(".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "HCP65" _FS_ "Libraries" _FS_ "Libc65");
	cFileUtil.FullPath(&szProject);
	psDebug->cBuilder.AddMathTables(szProject.Text());
	szProject.Kill();
	AssertTrue(psDebug->cBuilder.AddSource("Test.cpp", szSource));
	bResult = psDebug->cBuilder.Link();
	AssertTrue(bResult);
	AssertString("", psDebug->cBuilder.GetErrors());
	psDebug->pcRecords = psDebug->cBuilder.GetDebugInfo()->GetRecords();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestCPP816DebugKill(STestCPP816Debug* psDebug)
{
	psDebug->cBuilder.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The function record at szLabel's address, NULL if there is none (which
// fails): it starts there, and its body is in it.
//
//////////////////////////////////////////////////////////////////////////
static SAsm65DebugFunction* TestCPP816DebugFunction(STestCPP816Debug* psDebug, const char* szLabel)
{
	int64					iAddress;
	SAsm65DebugFunction*	psFunction;

	iAddress = -1;
	AssertTrue(psDebug->cBuilder.GetSymbol(szLabel, &iAddress));
	psFunction = (iAddress >= 0) ? psDebug->pcRecords->FindFunction((uint32)iAddress) : NULL;
	AssertTrue(psFunction != NULL);
	if (psFunction)
	{
		AssertInt((uint32)iAddress, psFunction->sStart.uiOffset);
		AssertTrue(psFunction->uiBody >= psFunction->sStart.uiOffset);
		AssertTrue(psFunction->uiBody < psFunction->uiEnd);
	}
	return psFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// The function at szLabel is named szName and its caller is found as
// eLink says (an offset past the link's own bytes, when it has one).
//
//////////////////////////////////////////////////////////////////////////
static void AssertCPP816DebugFrame(STestCPP816Debug* psDebug, const char* szLabel, const char* szName, EAsm65DebugLink eLink)
{
	static const char*		aszLinks[] = { "none", "frame", "saved" };
	SAsm65DebugFunction*	psFunction;
	CChars					szExpected;
	CChars					szActual;

	psFunction = TestCPP816DebugFunction(psDebug, szLabel);
	if (psFunction)
	{
		szExpected.Init(szName)->Append(' ')->Append(aszLinks[eLink]);
		szActual.Init(psFunction->szName)->Append(' ')->Append((psFunction->uiLink <= ADL_SavedS) ? aszLinks[psFunction->uiLink] : "?");
		AssertString(szExpected.Text(), szActual.Text());
		AssertTrue((eLink == ADL_None) ? (psFunction->iLink == 0) : (psFunction->iLink > 0));
		szActual.Kill();
		szExpected.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function is named szName.
//
//////////////////////////////////////////////////////////////////////////
static bool TestCPP816DebugFind(STestCPP816Debug* psDebug, const char* szName)
{
	size	ui;

	for (ui = 0; ui < psDebug->pcRecords->NumFunctions(); ui++)
	{
		if (psDebug->pcRecords->GetFunction(ui)->szName.Equals(szName))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Each function the compiler writes is described by its name as C++
// writes it, where its prologue ends and how its caller is found: a frame
// of its own (normal, with register words, or with D at S), an aligned
// frame whose S is kept, or no frame.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816DebugFrames(void)
{
	STestCPP816Debug		sDebug;

	TestCPP816DebugBuild(&sDebug,
		"namespace NS\n"
		"{\n"
		"	int Twice(int i) { return i + i; }\n"
		"}\n"
		"class CShape\n"
		"{\n"
		"public:\n"
		"	int	miSides;\n"
		"	CShape() { miSides = 3; }\n"
		"	~CShape() { miSides = 0; }\n"
		"	virtual int Area(void) { return miSides; }\n"
		"	int operator+(int i) { return miSides + i; }\n"
		"};\n"
		"int Sum(int* ai, int iCount)\n"
		"{\n"
		"	int i;\n"
		"	int iTotal = 0;\n"
		"	for (i = 0; i < iCount; i++)\n"
		"	{\n"
		"		iTotal += ai[i];\n"
		"	}\n"
		"	return iTotal;\n"
		"}\n"
		"int Factorial(int i)\n"
		"{\n"
		"	return (i <= 1) ? 1 : i * Factorial(i - 1);\n"
		"}\n"
		"int Leaf(void) { return 7; }\n"
		"int main(void)\n"
		"{\n"
		"	int ai[4] = { 1, 2, 3, 4 };\n"
		"	CShape cShape;\n"
		"	return Sum(ai, 4) + Factorial(5) + cShape.Area() + (cShape + 1) + NS::Twice(2) + Leaf();\n"
		"}\n");

	//A loop over the direct page: an aligned frame, S kept in it.  Recursive, so never aligned: a frame of its own,
	//its register word in it.  Nothing in the frame: none at all.
	AssertCPP816DebugFrame(&sDebug, "_Sum__FPii", "Sum", ADL_SavedS);
	AssertCPP816DebugFrame(&sDebug, "_Factorial__Fi", "Factorial", ADL_Frame);
	AssertCPP816DebugFrame(&sDebug, "_Leaf", "Leaf", ADL_None);
	AssertCPP816DebugFrame(&sDebug, "_NS__Twice__Fi", "NS::Twice", ADL_Frame);
	AssertCPP816DebugFrame(&sDebug, "_CShape__Area", "CShape::Area", ADL_Frame);
	AssertCPP816DebugFrame(&sDebug, "_main", "main", ADL_Frame);

	//Constructors, destructors and operators by their names.
	AssertTrue(TestCPP816DebugFind(&sDebug, "CShape::CShape"));
	AssertTrue(TestCPP816DebugFind(&sDebug, "CShape::~CShape"));
	AssertTrue(TestCPP816DebugFind(&sDebug, "CShape::Area"));
	AssertTrue(TestCPP816DebugFind(&sDebug, "CShape::operator +"));

	TestCPP816DebugKill(&sDebug);
}


//////////////////////////////////////////////////////////////////////////
//
// Programs whose variables are what a debugger shows (parameters by
// value, changed and not; objects by value, by address and through this;
// an anonymous union; locals stored and never read; recursion and loops)
// give the same results built for a debugger, optimised (Debug) and not,
// as built for speed (Release).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816DebugModes(void)
{
	static const char*	szSource =
		"struct SPoint { int x; int y; };\n"
		"struct SBig { long al[8]; int i; };\n"
		"class CCounter\n"
		"{\n"
		"public:\n"
		"	int	miCount;\n"
		"	CCounter() { miCount = 0; }\n"
		"	int Add(int i) { int iOld = miCount; miCount += i; return iOld; }\n"
		"};\n"
		"int Distance(SPoint s, int iScale)\n"
		"{\n"
		"	iScale = iScale * 2;\n"
		"	return (s.x + s.y) * iScale;\n"
		"}\n"
		"long Total(SBig s)\n"
		"{\n"
		"	long l = 0;\n"
		"	int i;\n"
		"	for (i = 0; i < 8; i++) { l += s.al[i]; }\n"
		"	return l + s.i;\n"
		"}\n"
		"int Fibonacci(int i) { return (i < 2) ? i : Fibonacci(i - 1) + Fibonacci(i - 2); }\n"
		"int Unread(int i)\n"
		"{\n"
		"	int iSquare = i * i;\n"
		"	int iCube = iSquare * i;\n"
		"	return i + 1;\n"
		"}\n"
		"int Union(int i)\n"
		"{\n"
		"	union { int iWord; char ac[2]; };\n"
		"	iWord = i;\n"
		"	return ac[0] + ac[1];\n"
		"}\n"
		"int main(void)\n"
		"{\n"
		"	SPoint s;\n"
		"	SBig sBig;\n"
		"	CCounter cCounter;\n"
		"	int i;\n"
		"	s.x = 2; s.y = 3;\n"
		"	for (i = 0; i < 8; i++) { sBig.al[i] = 1000L * i; }\n"
		"	sBig.i = 5;\n"
		"	cCounter.Add(4);\n"
		"	cCounter.Add(6);\n"
		"	if (Distance(s, 3) != 30) return 1;\n"
		"	if (Total(sBig) != 28005L) return 2;\n"
		"	if (Fibonacci(10) != 55) return 3;\n"
		"	if (Unread(7) != 8) return 4;\n"
		"	if (Union(0x0102) != 3) return 5;\n"
		"	if (cCounter.miCount != 10) return 6;\n"
		"	return 100;\n"
		"}\n";
	CCPPCompileOptions	cWas;

	cWas = *TestCPP816Options();
	AssertInt(100, TestCPP816Run(szSource, false));
	TestCPP816Options()->mbDebuggable = true;
	AssertInt(100, TestCPP816Run(szSource, false));
	TestCPP816Options()->mbOptimise = false;
	AssertInt(100, TestCPP816Run(szSource, false));
	*TestCPP816Options() = cWas;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816Debug(void)
{
	BeginTests();

	TestCPP816DebugFrames();
	TestCPP816DebugModes();

	TestStatistics();
}
