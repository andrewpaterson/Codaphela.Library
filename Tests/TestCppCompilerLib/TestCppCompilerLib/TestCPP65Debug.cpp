#include "BaseLib/Chars.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/MapStringInt.h"
#include "CppCompilerLib/CPP65Builder.h"
#include "CppCompilerLib/CPP65DebugWatch.h"
#include "Assembler65Lib/AsmW65C816StackWalker.h"
#include "Assembler65Lib/AsmW65C816FrameCheck.h"
#include "Assembler65Lib/Asm65C64FrameCheck.h"
#include "TestLib/Assert.h"
#include "TestCPP65Context.h"


CCPPCompileOptions*	TestCPP816Options(void);	//TestCPPW65C816.cpp


//How a program is built: for speed (Release, though without inlining so each function is a function of its own, or
//with it as the IDE builds Release), for a debugger (Debug) or for one and not optimised (an Unoptimised project).
enum ETestCPP65Build
{
	TCB_Release,
	TCB_ReleaseInlined,
	TCB_Debug,
	TCB_Unoptimised,
};


//A processor and a way to build for a debugger.
struct STestCPP65DebugBuild
{
	ECPPTarget			eTarget;
	ETestCPP65Build	eBuild;
};


//The builds the debugger's windows are tested on: each processor's Debug and Unoptimised.
static const STestCPP65DebugBuild	gasTestCPP65DebugBuilds[] = { { CPPT_W65C816, TCB_Debug }, { CPPT_W65C816, TCB_Unoptimised }, { CPPT_65C64, TCB_Debug }, { CPPT_65C64, TCB_Unoptimised } };
#define TEST_CPP65_DEBUG_BUILDS	(sizeof(gasTestCPP65DebugBuilds) / sizeof(gasTestCPP65DebugBuilds[0]))


//A program built for the W65C816 or the 65C64 with its debug info (CCPP65Builder::SetLineTable), and as a debugger runs
//it (TestCPP65DebugLoad): a step at a time, its calls checked (CAsm65FrameCheck, the processor's) and its stack walked
//(the records' processor's walker).
struct STestCPP65Debug
{
	CCPP65Builder			cBuilder;
	CAsm65DebugRecords*		pcRecords;
	CAsm65Memory			cMemory;
	CFastW65C816			cFastW65C816;
	CFast65C64				cFast65C64;
	CAsmW65C816FrameCheck	cCheckW65C816;
	CAsm65C64FrameCheck		cCheck65C64;
	CAsm65FrameCheck*		pcCheck;
	CAsm65StackWalker*		pcWalker;
	CArrayAsm65StackFrames	asFrames;
	CCPP65DebugValues		cValues;
	CCPP65DebugWatch		cWatch;
	bool					bLoaded;
};


//////////////////////////////////////////////////////////////////////////
//
// Builds szSource with its debug info, as the IDE does, eBuild's way, for
// eTarget (the W65C816 or the 65C64), with Libc65's string routines (the
// optimiser makes loops calls of them) and soft float.
//
//////////////////////////////////////////////////////////////////////////
static bool TestCPP65DebugBuild(STestCPP65Debug* psDebug, const char* szSource, ETestCPP65Build eBuild = TCB_Release, ECPPTarget eTarget = CPPT_W65C816)
{
	static const char*	aszW65C816Units[] = { "Libc65/stringW65C816.asm", "Libc65/stringW65C816_16.asm", "Libc65/softfloat.cpp", "Libc65/softfloatW65C816.asm", NULL };
	static const char*	asz65C64Units[] = { "Libc65/string.asm", "Libc65/softfloat.cpp", "Libc65/softfloat.asm", NULL };
	CChars				szProject;
	CFileUtil			cFileUtil;
	bool				bResult;

	psDebug->bLoaded = false;
	psDebug->cBuilder.Init();
	*psDebug->cBuilder.GetOptions() = (eTarget == CPPT_W65C816) ? *TestCPP816Options() : *TestCPP65Options();
	psDebug->cBuilder.GetOptions()->mbInline = (eBuild == TCB_ReleaseInlined);
	psDebug->cBuilder.GetOptions()->mbDebuggable = (eBuild != TCB_Release) && (eBuild != TCB_ReleaseInlined);
	psDebug->cBuilder.GetOptions()->mbOptimise = (eBuild != TCB_Unoptimised);
	psDebug->cBuilder.SetTarget(eTarget);
	psDebug->cBuilder.SetLineTable(true);
	TestCPP65AddIncludeDirectories(&psDebug->cBuilder);
	AssertTrue(psDebug->cBuilder.AddStartUp());
	if (eTarget == CPPT_W65C816)
	{
		szProject.Init(".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "HCP65" _FS_ "Libraries" _FS_ "Libc65");
		cFileUtil.FullPath(&szProject);
		psDebug->cBuilder.AddMathTables(szProject.Text());
		szProject.Kill();
	}
	AssertTrue(psDebug->cBuilder.AddSource("Test.cpp", szSource));
	TestCPP65AddUnits(&psDebug->cBuilder, (eTarget == CPPT_W65C816) ? aszW65C816Units : asz65C64Units, false);
	if (eTarget == CPPT_65C64)
	{
		//The 65C64's assembly units (TestCPP65AddUnits adds a W65C816's with its C++).
		TestCPP65AddUnits(&psDebug->cBuilder, asz65C64Units, true);
	}
	bResult = psDebug->cBuilder.Link();
	AssertTrue(bResult);
	AssertString("", psDebug->cBuilder.GetErrors());
	psDebug->pcRecords = psDebug->cBuilder.GetDebugInfo()->GetRecords();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Loads the built program into memory and resets the W65C816, ready to
// step, its stack walked with the program's records.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPP65DebugLoad(STestCPP65Debug* psDebug)
{
	psDebug->cMemory.Init();
	if (psDebug->pcRecords->GetProcessor() == ADP_65C64)
	{
		psDebug->cMemory.LoadProgram(psDebug->cBuilder.GetOutput(), CPP65_PROGRAM_ADDRESS);
		psDebug->cMemory.InitFast65C64(&psDebug->cFast65C64);
		psDebug->cFast65C64.Reset();
		psDebug->cCheck65C64.Init(&psDebug->cMemory, &psDebug->cFast65C64);
		psDebug->pcCheck = &psDebug->cCheck65C64;
	}
	else
	{
		psDebug->cMemory.Load(psDebug->cBuilder.GetOutput());
		psDebug->cMemory.InitFast65816(&psDebug->cFastW65C816);
		psDebug->cFastW65C816.Reset();
		psDebug->cCheckW65C816.Init(&psDebug->cMemory, &psDebug->cFastW65C816);
		psDebug->pcCheck = &psDebug->cCheckW65C816;
	}
	psDebug->pcWalker = Asm65NewStackWalker(psDebug->pcRecords->GetProcessor(), &psDebug->cMemory);
	psDebug->pcWalker->AddRecords(psDebug->pcRecords, ASS_Kernel);
	psDebug->asFrames.Init();
	psDebug->cValues.Init(&psDebug->cMemory);
	psDebug->cWatch.Init(&psDebug->cValues, psDebug->pcWalker);
	psDebug->bLoaded = true;
}


//////////////////////////////////////////////////////////////////////////
//
// Runs the instruction at the program address, its calls followed.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPP65DebugStep(STestCPP65Debug* psDebug)
{
	psDebug->pcCheck->Run(1);
}


//////////////////////////////////////////////////////////////////////////
//
// The program address of the instruction the processor runs next.
//
//////////////////////////////////////////////////////////////////////////
static uint32 TestCPP65DebugPC(STestCPP65Debug* psDebug)
{
	if (psDebug->pcCheck == &psDebug->cCheck65C64)
	{
		return psDebug->cFast65C64.GetProgramCounter();
	}
	return psDebug->cFastW65C816.GetProgramAddress();
}


//////////////////////////////////////////////////////////////////////////
//
// What a test expects on the W65C816 (szW65C816) or the 65C64 (sz65C64),
// for the processor the program was built for.
//
//////////////////////////////////////////////////////////////////////////
static const char* TestCPP65DebugFor(STestCPP65Debug* psDebug, const char* szW65C816, const char* sz65C64)
{
	return (psDebug->pcRecords->GetProcessor() == ADP_65C64) ? sz65C64 : szW65C816;
}


//////////////////////////////////////////////////////////////////////////
//
// The processor's registers as they are.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPP65DebugRegisters(STestCPP65Debug* psDebug, SAsm65StackRegisters* psRegisters)
{
	if (psDebug->pcCheck == &psDebug->cCheck65C64)
	{
		psRegisters->Init(&psDebug->cFast65C64, ASS_Kernel);
	}
	else
	{
		psRegisters->Init(&psDebug->cFastW65C816, ASS_Kernel);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Walks the stack from the W65C816's registers into asFrames.  Returns
// how many frames.
//
//////////////////////////////////////////////////////////////////////////
static size TestCPP65DebugWalk(STestCPP65Debug* psDebug)
{
	SAsm65StackRegisters	sRegisters;

	TestCPP65DebugRegisters(psDebug, &sRegisters);
	return psDebug->pcWalker->Walk(&sRegisters, &psDebug->asFrames, 64);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestCPP65DebugKill(STestCPP65Debug* psDebug)
{
	if (psDebug->bLoaded)
	{
		psDebug->cWatch.Kill();
		psDebug->cValues.Kill();
		psDebug->asFrames.Kill();
		SafeKill(psDebug->pcWalker);
		psDebug->pcCheck->Kill();
		if (psDebug->pcCheck == &psDebug->cCheck65C64)
		{
			psDebug->cFast65C64.Kill();
		}
		else
		{
			psDebug->cFastW65C816.Kill();
		}
		psDebug->cMemory.Kill();
	}
	psDebug->cBuilder.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The function record at szLabel's address, NULL if there is none (which
// fails): it starts there, and its body is in it.
//
//////////////////////////////////////////////////////////////////////////
static SAsm65DebugFunction* TestCPP65DebugFunction(STestCPP65Debug* psDebug, const char* szLabel)
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
// The function at szLabel is named szName and its frame address is found
// as eFrame says: from D (D plus what the prologue made, past phd's and
// jsl's 5 bytes, or the S kept there plus those 5), the caller's D saved
// below jsl's push; or by following its code.  Its variables are from D.
//
//////////////////////////////////////////////////////////////////////////
static void AssertCPP65DebugFrame(STestCPP65Debug* psDebug, const char* szLabel, const char* szName, EAsm65DebugFrame eFrame)
{
	static const char*		aszFrames[] = { "follow", "register", "memory" };
	SAsm65DebugFunction*	psFunction;
	CChars					szExpected;
	CChars					szActual;

	psFunction = TestCPP65DebugFunction(psDebug, szLabel);
	if (psFunction)
	{
		szExpected.Init(szName)->Append(' ')->Append(aszFrames[eFrame]);
		szActual.Init(psFunction->szName)->Append(' ')->Append((psFunction->uiFrame <= ADF_Memory) ? aszFrames[psFunction->uiFrame] : "?");
		AssertString(szExpected.Text(), szActual.Text());
		AssertInt(AW65DR_D, psFunction->uiBase);
		if (eFrame == ADF_FollowS)
		{
			AssertInt(0, psFunction->uiNumSaved);
		}
		else
		{
			AssertInt(AW65DR_D, psFunction->uiFrameRegister);
			AssertTrue(psFunction->iFrameOffset > ((eFrame == ADF_Register) ? 5 : 0));
			AssertInt((eFrame == ADF_Memory) ? 5 : 0, psFunction->iFrameAdjust);
			AssertInt(1, psFunction->uiNumSaved);
			AssertInt(AW65DR_D, psDebug->pcRecords->GetSaved(psFunction, 0)->uiWhat);
			AssertInt(-4, psDebug->pcRecords->GetSaved(psFunction, 0)->iOffset);
		}
		szActual.Kill();
		szExpected.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function is named szName.
//
//////////////////////////////////////////////////////////////////////////
static bool TestCPP65DebugFind(STestCPP65Debug* psDebug, const char* szName)
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
void TestCPP65DebugFrames(void)
{
	STestCPP65Debug		sDebug;

	TestCPP65DebugBuild(&sDebug,
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
	AssertCPP65DebugFrame(&sDebug, "_Sum__FPii", "Sum", ADF_Memory);
	AssertCPP65DebugFrame(&sDebug, "_Factorial__Fi", "Factorial", ADF_Register);
	AssertCPP65DebugFrame(&sDebug, "_Leaf", "Leaf", ADF_FollowS);
	AssertCPP65DebugFrame(&sDebug, "_NS__Twice__Fi", "NS::Twice", ADF_Register);
	AssertCPP65DebugFrame(&sDebug, "_CShape__Area", "CShape::Area", ADF_Register);
	AssertCPP65DebugFrame(&sDebug, "_main", "main", ADF_Register);

	//Constructors, destructors and operators by their names.
	AssertTrue(TestCPP65DebugFind(&sDebug, "CShape::CShape"));
	AssertTrue(TestCPP65DebugFind(&sDebug, "CShape::~CShape"));
	AssertTrue(TestCPP65DebugFind(&sDebug, "CShape::Area"));
	AssertTrue(TestCPP65DebugFind(&sDebug, "CShape::operator +"));

	TestCPP65DebugKill(&sDebug);
}


//////////////////////////////////////////////////////////////////////////
//
// The function named szName, NULL if there is none (which fails).
//
//////////////////////////////////////////////////////////////////////////
static SAsm65DebugFunction* TestCPP65DebugNamed(STestCPP65Debug* psDebug, const char* szName)
{
	size	ui;

	for (ui = 0; ui < psDebug->pcRecords->NumFunctions(); ui++)
	{
		if (psDebug->pcRecords->GetFunction(ui)->szName.Equals(szName))
		{
			return psDebug->pcRecords->GetFunction(ui);
		}
	}
	AssertString(szName, "");
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The function's uiIndex'th variable is szName, a parameter or not, of
// the type named szType, where eWhere says and in scope from line iFirst
// to iLast.
//
//////////////////////////////////////////////////////////////////////////
static void AssertCPP65DebugVariable(STestCPP65Debug* psDebug, SAsm65DebugFunction* psFunction, size uiIndex, const char* szName, bool bParameter, const char* szType, EAsm65DebugWhere eWhere, uint32 uiFirst, uint32 uiLast)
{
	SAsm65DebugVariable*	psVariable;
	SAsm65DebugType*		psType;

	psVariable = psFunction ? psDebug->pcRecords->GetVariable(psFunction, uiIndex) : NULL;
	AssertTrue(psVariable != NULL);
	if (psVariable)
	{
		AssertString(szName, psVariable->szName.Text());
		AssertBool(bParameter, psVariable->bParameter);
		psType = psDebug->pcRecords->GetType(psVariable->uiType);
		AssertString(szType, psType ? psType->szName.Text() : "");
		AssertInt(eWhere, psVariable->uiWhere);
		AssertInt(uiFirst, psVariable->uiFirstLine);
		AssertInt(uiLast, psVariable->uiLastLine);
		if (eWhere != ADW_None)
		{
			//In the frame, above the scratch bytes and below the caller's D (phd's and jsl's 5 bytes below the frame address).
			AssertTrue(psVariable->iOffset >= 23);
			AssertTrue(psVariable->iOffset <= psFunction->iFrameOffset - 5);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A Debug build describes each function's parameters (this first) and
// locals, each in its own place in the frame and in scope where C++ has
// it; an object passed by value, through the address kept for it; static
// locals and globals at their labels; and each type they have once, a
// class with its bases, vtable pointer and fields (bit fields too), an
// enum with its enumerators.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65DebugVariables(void)
{
	STestCPP65Debug		sDebug;
	SAsm65DebugFunction*	psFunction;
	SAsm65DebugType*		psType;
	SAsm65DebugMember*		psMember;
	SAsm65DebugGlobal*		psGlobal;
	int64					iLabel;

	TestCPP65DebugBuild(&sDebug,
		"enum EColour { EC_Red = 1, EC_Blue = 4 };\n"			//1
		"struct SBase { int miBase; };\n"							//2
		"class CShape : public SBase\n"							//3
		"{\n"														//4
		"public:\n"												//5
		"	unsigned	muiFlag : 3;\n"							//6
		"	EColour		meColour;\n"							//7
		"	virtual int Area(int iScale) { return miBase * iScale; }\n"	//8
		"};\n"														//9
		"struct SBig { long al[8]; };\n"							//10
		"namespace NS { int giCount = 3; }\n"						//11
		"long Total(SBig s, int iFrom)\n"							//12
		"{\n"														//13
		"	static int	siCalls = 0;\n"							//14
		"	long		l = 0;\n"								//15
		"	for (int i = iFrom; i < 8; i++)\n"					//16
		"	{\n"													//17
		"		l += s.al[i];\n"									//18
		"	}\n"													//19
		"	siCalls++;\n"											//20
		"	return l;\n"											//21
		"}\n"														//22
		"int main(void)\n"											//23
		"{\n"														//24
		"	CShape	cShape;\n"										//25
		"	SBig	sBig;\n"										//26
		"	cShape.miBase = NS::giCount;\n"						//27
		"	cShape.meColour = EC_Blue;\n"							//28
		"	return cShape.Area(2) + (int)Total(sBig, 0);\n"		//29
		"}\n", TCB_Debug);										//30

	//this first, then the parameters, in the function's lines.
	psFunction = TestCPP65DebugNamed(&sDebug, "CShape::Area");
	AssertInt(2, psFunction ? psFunction->uiNumVariables : 0);
	AssertCPP65DebugVariable(&sDebug, psFunction, 0, "this", true, "CShape*", ADW_Frame, 8, 8);
	AssertCPP65DebugVariable(&sDebug, psFunction, 1, "iScale", true, "int", ADW_Frame, 8, 8);

	//An object passed by value is read through the address kept for it; a for's local is in scope in the for.
	psFunction = TestCPP65DebugNamed(&sDebug, "Total");
	AssertInt(4, psFunction ? psFunction->uiNumVariables : 0);
	AssertCPP65DebugVariable(&sDebug, psFunction, 0, "s", true, "SBig", ADW_Indirect, 12, 22);
	AssertCPP65DebugVariable(&sDebug, psFunction, 1, "iFrom", true, "int", ADW_Frame, 12, 22);
	AssertCPP65DebugVariable(&sDebug, psFunction, 2, "l", false, "long", ADW_Frame, 15, 22);
	AssertCPP65DebugVariable(&sDebug, psFunction, 3, "i", false, "int", ADW_Frame, 16, 19);
	AssertTrue(sDebug.pcRecords->GetVariable(psFunction, 2)->iOffset != sDebug.pcRecords->GetVariable(psFunction, 3)->iOffset);

	//The static local is Total's, the global is anyone's, each at its label.
	psGlobal = sDebug.pcRecords->FindGlobal("siCalls", psFunction);
	AssertTrue(psGlobal != NULL);
	AssertTrue(sDebug.pcRecords->FindGlobal("siCalls", NULL) == NULL);
	psGlobal = sDebug.pcRecords->FindGlobal("NS::giCount", NULL);
	AssertTrue(psGlobal != NULL);
	AssertTrue(sDebug.cBuilder.GetSymbol("_NS__giCount", &iLabel));
	AssertInt((uint32)iLabel, psGlobal ? psGlobal->sPlace.uiOffset : 0);

	psFunction = TestCPP65DebugNamed(&sDebug, "main");
	AssertCPP65DebugVariable(&sDebug, psFunction, 0, "cShape", false, "CShape", ADW_Frame, 25, 30);
	AssertCPP65DebugVariable(&sDebug, psFunction, 1, "sBig", false, "SBig", ADW_Frame, 26, 30);

	//CShape: its base, its vtable pointer, a bit field and an enum field.
	psType = sDebug.pcRecords->GetType(sDebug.pcRecords->GetVariable(psFunction, 0)->uiType);
	AssertInt(ADT_Class, psType->uiKind);
	AssertInt(4, psType->uiNumMembers);
	psMember = sDebug.pcRecords->GetMember(psType, 0);
	AssertInt(ADM_Base, psMember->uiKind);
	AssertString("SBase", psMember->szName.Text());
	AssertInt(ADM_VTable, sDebug.pcRecords->GetMember(psType, 1)->uiKind);
	psMember = sDebug.pcRecords->GetMember(psType, 2);
	AssertString("muiFlag", psMember->szName.Text());
	AssertInt(3, psMember->uiBits);
	psMember = sDebug.pcRecords->GetMember(psType, 3);
	AssertString("meColour", psMember->szName.Text());
	psType = sDebug.pcRecords->GetType(psMember->uiType);
	AssertInt(ADT_Enum, psType->uiKind);
	AssertInt(2, psType->uiNumMembers);
	AssertString("int", sDebug.pcRecords->GetType(psType->uiTarget)->szName.Text());

	TestCPP65DebugKill(&sDebug);
}


//////////////////////////////////////////////////////////////////////////
//
// Runs the loaded program to its end, a step at a time, its stack walked
// at each step in a function the records describe (the first few walks
// that are wrong in pszWrong).  Returns main's int, -1000000 if it didn't
// stop.
//
//////////////////////////////////////////////////////////////////////////
static int32 TestCPP65DebugRunWalking(STestCPP65Debug* psDebug, CChars* pszWrong)
{
	psDebug->pcCheck->RunWalking(20000000, psDebug->pcWalker, pszWrong);
	AssertInt(0, (int)psDebug->cMemory.NumBadAccesses());
	AssertTrue(psDebug->pcCheck->IsStopped());
	if (!psDebug->pcCheck->IsStopped())
	{
		return -1000000;
	}
	if (psDebug->pcCheck == &psDebug->cCheck65C64)
	{
		return (int32)psDebug->cFast65C64.GetA();
	}
	AssertString("", psDebug->cCheckW65C816.GetWrites());
	return (int16)psDebug->cFastW65C816.GetC();
}


//////////////////////////////////////////////////////////////////////////
//
// Runs the loaded program until it is about to run line uiLine of
// Test.cpp.  Returns whether it got there.
//
//////////////////////////////////////////////////////////////////////////
static bool TestCPP65DebugRunTo(STestCPP65Debug* psDebug, uint32 uiLine)
{
	CArrayUint32	auiAddresses;
	size			uiSteps;
	bool			bThere;

	auiAddresses.Init();
	AssertTrue(psDebug->cBuilder.GetDebugInfo()->GetLines()->FindLineAddresses("Test.cpp", uiLine, &auiAddresses) > 0);
	bThere = false;
	for (uiSteps = 0; (uiSteps < 20000000) && !psDebug->pcCheck->IsStopped() && !bThere; uiSteps++)
	{
		TestCPP65DebugStep(psDebug);
		bThere = auiAddresses.Find(TestCPP65DebugPC(psDebug)) != ARRAY_ELEMENT_NOT_FOUND;
	}
	auiAddresses.Kill();
	AssertTrue(bThere);
	return bThere;
}


//////////////////////////////////////////////////////////////////////////
//
// The name of the uiFrame'th frame's function in the last walk: "" for
// code the records don't describe, "?" past the walk's end.
//
//////////////////////////////////////////////////////////////////////////
static const char* TestCPP65DebugFrameName(STestCPP65Debug* psDebug, size uiFrame)
{
	SAsm65StackFrame*	psFrame;

	if (uiFrame >= psDebug->asFrames.NumElements())
	{
		return "?";
	}
	psFrame = psDebug->asFrames.Get(uiFrame);
	return psFrame->psFunction ? psFrame->GetName() : "";
}


//////////////////////////////////////////////////////////////////////////
//
// The variable named szName of the uiFrame'th frame of the last walk, as
// a debugger shows it (CCPP65DebugValues).  False if its function has
// none, or it isn't kept anywhere.
//
//////////////////////////////////////////////////////////////////////////
static bool TestCPP65DebugValue(STestCPP65Debug* psDebug, size uiFrame, const char* szName, SCPP65DebugValue* psValue)
{
	SAsm65StackFrame*		psFrame;
	SAsm65DebugVariable*	psVariable;
	size					ui;

	psFrame = (uiFrame < psDebug->asFrames.NumElements()) ? psDebug->asFrames.Get(uiFrame) : NULL;
	for (ui = 0; psFrame && psFrame->psFunction && (ui < psFrame->psFunction->uiNumVariables); ui++)
	{
		psVariable = psFrame->pcRecords->GetVariable(psFrame->psFunction, ui);
		if (psVariable->szName.Equals(szName))
		{
			return psDebug->cValues.Variable(psDebug->pcWalker, psFrame, psVariable, psValue);
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The int named szName in the uiFrame'th frame of the last walk:
// -1000000 if its function has none, or the debugger can't read it.
//
//////////////////////////////////////////////////////////////////////////
static int32 TestCPP65DebugLocal(STestCPP65Debug* psDebug, size uiFrame, const char* szName)
{
	SCPP65DebugValue	sValue;
	uint64				uiValue;

	if (!TestCPP65DebugValue(psDebug, uiFrame, szName, &sValue) || !psDebug->cValues.Read(&sValue, &uiValue))
	{
		return -1000000;
	}
	return (int32)uiValue;
}


//////////////////////////////////////////////////////////////////////////
//
// A value's Value and Type columns, " = " between them.
//
//////////////////////////////////////////////////////////////////////////
static void AssertCPP65DebugShown(STestCPP65Debug* psDebug, const char* szExpected, SCPP65DebugValue* psValue)
{
	CChars	sz;

	sz.Init();
	AssertTrue(psDebug->cValues.Format(psValue, &sz));
	sz.Append(" = ");
	AssertTrue(psDebug->cValues.AppendType(psValue, &sz));
	AssertString(szExpected, sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The variable named szName of the uiFrame'th frame shown (Value = Type).
//
//////////////////////////////////////////////////////////////////////////
static void AssertCPP65DebugVariableShown(STestCPP65Debug* psDebug, size uiFrame, const char* szName, const char* szExpected)
{
	SCPP65DebugValue	sValue;

	AssertTrue(TestCPP65DebugValue(psDebug, uiFrame, szName, &sValue));
	AssertCPP65DebugShown(psDebug, szExpected, &sValue);
}


//////////////////////////////////////////////////////////////////////////
//
// The watch szExpression in the uiFrame'th frame of the last walk, shown
// (Value = Type), or the error it gives.
//
//////////////////////////////////////////////////////////////////////////
static void AssertCPP65DebugWatch(STestCPP65Debug* psDebug, size uiFrame, const char* szExpression, const char* szExpected, SCPP65DebugValue* psValue = NULL)
{
	SCPP65DebugValue	sValue;

	psValue = psValue ? psValue : &sValue;
	if (!psDebug->cWatch.Evaluate(psDebug->asFrames.Get(uiFrame), 0, szExpression, psValue))
	{
		AssertString(szExpected, psDebug->cWatch.GetError());
		return;
	}
	AssertCPP65DebugShown(psDebug, szExpected, psValue);
}


//////////////////////////////////////////////////////////////////////////
//
// psValue's uiIndex'th child, its name szName, shown (Value = Type).
//
//////////////////////////////////////////////////////////////////////////
static void AssertCPP65DebugChild(STestCPP65Debug* psDebug, SCPP65DebugValue* psValue, size uiIndex, const char* szName, const char* szExpected, SCPP65DebugValue* psChild)
{
	CChars	sz;

	sz.Init();
	AssertTrue(psDebug->cValues.GetChild(psValue, uiIndex, &sz, psChild));
	AssertString(szName, sz.Text());
	sz.Kill();
	AssertCPP65DebugShown(psDebug, szExpected, psChild);
}


//////////////////////////////////////////////////////////////////////////
//
// Types szText into psValue's Value (C2) and writes what that makes to
// memory, or to the register it is in (the 65C64's X, Y, V, W and B, the
// W65C816's X): false if it can't be written so.
//
//////////////////////////////////////////////////////////////////////////
static bool TestCPP65DebugWrite(STestCPP65Debug* psDebug, SCPP65DebugValue* psValue, const char* szText)
{
	static const E65Register	aeRegisters[A65DR_Count] = { R65_S, R65_V, R65_W, R65_A, R65_B, R65_X, R65_Y, R65_Zero };
	SCPP65DebugWrite			sWrite;
	uint64						uiValue;
	size						ui;

	if (!psDebug->cValues.Edit(psValue, szText, &sWrite))
	{
		return false;
	}
	if (sWrite.uiRegister != CPP65_DEBUG_NO_REGISTER)
	{
		memcpy(&uiValue, sWrite.auiBytes, sizeof(uiValue));
		if (psDebug->pcCheck == &psDebug->cCheck65C64)
		{
			if ((sWrite.uiRegister < A65DR_V) || (sWrite.uiRegister > A65DR_Y) || (sWrite.uiRegister == A65DR_A))
			{
				return false;
			}
			psDebug->cFast65C64.TestSetRegister(aeRegisters[sWrite.uiRegister], uiValue);
			return true;
		}
		if (sWrite.uiRegister != AW65DR_X)
		{
			return false;
		}
		psDebug->cFastW65C816.TestSetX((uint16)uiValue);
		return true;
	}
	for (ui = 0; ui < sWrite.uiSize; ui++)
	{
		psDebug->cMemory.SetByte(sWrite.uiAddress + (uint32)ui, sWrite.auiBytes[ui]);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// TestCPP65DebugWrite to the variable named szName in the uiFrame'th
// frame.
//
//////////////////////////////////////////////////////////////////////////
static bool TestCPP65DebugEdit(STestCPP65Debug* psDebug, size uiFrame, const char* szName, const char* szText)
{
	SCPP65DebugValue	sValue;

	return TestCPP65DebugValue(psDebug, uiFrame, szName, &sValue) && TestCPP65DebugWrite(psDebug, &sValue, szText);
}


//////////////////////////////////////////////////////////////////////////
//
// Stopped at a line in a recursion, the stack walks back through each
// call to main, each frame's locals and parameters read where the debug
// records say (a global too), built for a debugger optimised and not, on
// both processors.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65DebugStack(void)
{
	STestCPP65Debug				sDebug;
	SAsm65DebugGlobal*				psGlobal;
	size							uiBuild;
	int								i;

	for (uiBuild = 0; uiBuild < TEST_CPP65_DEBUG_BUILDS; uiBuild++)
	{
		TestCPP65DebugBuild(&sDebug,
			"int giDepth = 0;\n"						//1
			"int Factorial(int i)\n"					//2
			"{\n"										//3
			"	int iBelow;\n"							//4
			"	giDepth++;\n"							//5
			"	if (i <= 1)\n"							//6
			"	{\n"									//7
			"		return 1;\n"						//8
			"	}\n"									//9
			"	iBelow = Factorial(i - 1);\n"			//10
			"	return i * iBelow;\n"					//11
			"}\n"										//12
			"int main(void)\n"							//13
			"{\n"										//14
			"	int iFrom = 5;\n"						//15
			"	return Factorial(iFrom);\n"			//16
			"}\n", gasTestCPP65DebugBuilds[uiBuild].eBuild, gasTestCPP65DebugBuilds[uiBuild].eTarget);	//17
		TestCPP65DebugLoad(&sDebug);

		//Factorial(1) to Factorial(5), main, and the start up, which has no records.
		AssertTrue(TestCPP65DebugRunTo(&sDebug, 8));
		AssertInt(7, (int)TestCPP65DebugWalk(&sDebug));
		for (i = 0; i < 5; i++)
		{
			AssertString("Factorial", TestCPP65DebugFrameName(&sDebug, i));
			AssertInt(i + 1, TestCPP65DebugLocal(&sDebug, i, "i"));
		}
		AssertString("main", TestCPP65DebugFrameName(&sDebug, 5));
		AssertInt(5, TestCPP65DebugLocal(&sDebug, 5, "iFrom"));
		AssertString("", TestCPP65DebugFrameName(&sDebug, 6));
		AssertInt(-1000000, TestCPP65DebugLocal(&sDebug, 5, "i"));

		AssertCPP65DebugWatch(&sDebug, 0, "giDepth * 10 + i", "51 = int");
		AssertCPP65DebugWatch(&sDebug, 5, "iFrom", "5 = int");
		AssertCPP65DebugWatch(&sDebug, 5, "i", "i is undefined");

		psGlobal = sDebug.pcRecords->FindGlobal("giDepth", NULL);
		AssertTrue(psGlobal != NULL);
		AssertInt(5, psGlobal ? sDebug.cMemory.GetWord(psGlobal->sPlace.uiOffset) : 0);

		//On the way back out, each caller's local is what its callee returned.
		AssertTrue(TestCPP65DebugRunTo(&sDebug, 11));
		TestCPP65DebugWalk(&sDebug);
		AssertInt(2, TestCPP65DebugLocal(&sDebug, 0, "i"));
		AssertInt(1, TestCPP65DebugLocal(&sDebug, 0, "iBelow"));

		TestCPP65DebugKill(&sDebug);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Programs whose variables are what a debugger shows (parameters by
// value, changed and not; objects by value, by address and through this;
// an anonymous union; locals stored and never read; a variadic function's,
// whose arguments va_start finds from them; a pointer returned from a
// frame past the direct page; recursion and loops)
// give the same results built for a debugger, optimised (Debug) and not,
// as built for speed (Release, with inlining and without), on both
// processors; and at every step in
// each, the stack walks back through the calls it is in.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65DebugModes(void)
{
	static const char*	szSource =
		"#include <stdarg.h>\n"
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
		"int SumAll(int iCount, ...)\n"
		"{\n"
		"	va_list	a;\n"
		"	int		i;\n"
		"	int		iTotal = 0;\n"
		"	va_start(a, iCount);\n"
		"	for (i = 0; i < iCount; i++) { iTotal += va_arg(a, int); }\n"
		"	va_end(a);\n"
		"	return iTotal;\n"
		"}\n"
		"float Round(float fInput, long iExponent)\n"
		"{\n"
		"	float	fValue;\n"
		"	long	iAdjusted;\n"
		"	iAdjusted = (iExponent - 1 + 0x7f) << 23;\n"
		"	fValue = *((float*)((void*)&iAdjusted));\n"
		"	if (((fValue < 0.0f) && (fInput > 0.0f)) || ((fValue > 0.0f) && (fInput < 0.0f))) { fInput -= fValue; }\n"
		"	else { fInput += fValue; }\n"
		"	return fInput;\n"
		"}\n"
		"char* Big(char* szFrom)\n"
		"{\n"
		"	char	ac[400];\n"
		"	char*	pc;\n"
		"	ac[0] = szFrom[0];\n"
		"	pc = (ac[0] == 'x') ? szFrom + 1 : szFrom;\n"
		"	return pc;\n"
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
		"	if (SumAll(3, 4, 5, 6) != 15) return 7;\n"
		"	if (*Big((char*)\"xyz\") != 'y') return 8;\n"
		"	if ((int)Round(2.0f, 2) != 4) return 9;\n"
		"	return 100;\n"
		"}\n";
	static const ETestCPP65Build	aeBuilds[] = { TCB_Release, TCB_ReleaseInlined, TCB_Debug, TCB_Unoptimised };
	static const ECPPTarget			aeTargets[] = { CPPT_W65C816, CPPT_65C64 };
	STestCPP65Debug				sDebug;
	CChars							szWrong;
	size							uiBuild;
	size							uiTarget;

	for (uiTarget = 0; uiTarget < 2; uiTarget++)
	{
		for (uiBuild = 0; uiBuild < 4; uiBuild++)
		{
			TestCPP65DebugBuild(&sDebug, szSource, aeBuilds[uiBuild], aeTargets[uiTarget]);
			TestCPP65DebugLoad(&sDebug);
			szWrong.Init();
			AssertInt(100, TestCPP65DebugRunWalking(&sDebug, &szWrong));
			AssertString("", szWrong.Text());
			szWrong.Kill();
			TestCPP65DebugKill(&sDebug);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Stopped in a call, its parameters and its caller's locals shown as
// Visual Studio's Locals window shows them (C1): integers, characters, a
// string, enums (one of a flags enum's or'ed), floats, a bool, a pointer
// to an object of a derived class (its class from its vtable), an array,
// a base's members, bit fields.  Edited (C2), the program goes on with the
// new values; what doesn't fit isn't written.  Built for a debugger,
// optimised and not, on both processors.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65DebugValues(void)
{
	STestCPP65Debug				sDebug;
	SCPP65DebugValue				sValue;
	SCPP65DebugValue				sChild;
	SCPP65DebugValue				sGrandchild;
	CChars							szWrong;
	CChars							sz;
	size							uiBuild;

	for (uiBuild = 0; uiBuild < TEST_CPP65_DEBUG_BUILDS; uiBuild++)
	{
		TestCPP65DebugBuild(&sDebug,
			"enum EColour { EC_Red, EC_Green = 5, EC_Blue };\n"										//1
			"enum EFlags { EF_A = 1, EF_B = 2, EF_C = 4 };\n"											//2
			"struct SBits { unsigned int uiLow : 3; int iHigh : 5; };\n"								//3
			"class CShape\n"																			//4
			"{\n"																						//5
			"public:\n"																				//6
			"	int	miSides;\n"																		//7
			"	virtual int Area(void) { return 0; }\n"												//8
			"};\n"																						//9
			"class CSquare : public CShape\n"															//10
			"{\n"																						//11
			"public:\n"																				//12
			"	int	miSide;\n"																			//13
			"	int Area(void) override { return miSide * miSide; }\n"									//14
			"};\n"																						//15
			"int Use(CShape* pcShape, const char* szName, int* piCounts, EColour eColour, EFlags eFlags, float f, double d, bool b, char c, SBits* psBits)\n"	//16
			"{\n"																						//17
			"	return pcShape->Area() + szName[0] + piCounts[1] + eColour + eFlags + (int)f + (int)d + b + c + psBits->iHigh;\n"	//18
			"}\n"																						//19
			"int main(void)\n"																			//20
			"{\n"																						//21
			"	CSquare	cSquare;\n"																	//22
			"	int		aiCounts[3] = { 7, 8, 9 };\n"													//23
			"	SBits	sBits;\n"																		//24
			"	cSquare.miSides = 4;\n"																//25
			"	cSquare.miSide = 3;\n"																	//26
			"	sBits.uiLow = 5;\n"																	//27
			"	sBits.iHigh = -3;\n"																	//28
			"	return Use(&cSquare, \"Ada\\n\", aiCounts, EC_Green, (EFlags)(EF_A | EF_C), 1.5f, 2.25, true, 'c', &sBits);\n"	//29
			"}\n", gasTestCPP65DebugBuilds[uiBuild].eBuild, gasTestCPP65DebugBuilds[uiBuild].eTarget);	//30
		TestCPP65DebugLoad(&sDebug);
		AssertTrue(TestCPP65DebugRunTo(&sDebug, 18));
		TestCPP65DebugWalk(&sDebug);
		AssertString("Use", TestCPP65DebugFrameName(&sDebug, 0));

		AssertCPP65DebugVariableShown(&sDebug, 0, "eColour", "EC_Green = EColour");
		AssertCPP65DebugVariableShown(&sDebug, 0, "eFlags", "EF_A | EF_C = EFlags");
		AssertCPP65DebugVariableShown(&sDebug, 0, "f", "1.5 = float");
		AssertCPP65DebugVariableShown(&sDebug, 0, "d", "2.25 = double");
		AssertCPP65DebugVariableShown(&sDebug, 0, "b", "true = bool");
		AssertCPP65DebugVariableShown(&sDebug, 0, "c", "99 'c' = char");

		//A string; what an int* points at; a bit field each way.
		sz.Init();
		AssertTrue(TestCPP65DebugValue(&sDebug, 0, "szName", &sValue));
		AssertTrue(sDebug.cValues.Format(&sValue, &sz));
		AssertTrue(sz.EndsWith(" \"Ada\\n\""));
		AssertTrue(sz.StartsWith("0x"));
		sz.Kill();
		AssertTrue(TestCPP65DebugValue(&sDebug, 0, "piCounts", &sValue));
		AssertInt(1, (int)sDebug.cValues.NumChildren(&sValue));
		AssertCPP65DebugChild(&sDebug, &sValue, 0, "*", "7 = int", &sChild);
		AssertTrue(TestCPP65DebugValue(&sDebug, 0, "psBits", &sValue));
		AssertInt(2, (int)sDebug.cValues.NumChildren(&sValue));
		AssertCPP65DebugChild(&sDebug, &sValue, 0, "uiLow", "5 = unsigned int", &sChild);
		AssertCPP65DebugChild(&sDebug, &sValue, 1, "iHigh", "-3 = int", &sChild);

		//A CShape* to a CSquare: its members are CShape's.
		sz.Init();
		AssertTrue(TestCPP65DebugValue(&sDebug, 0, "pcShape", &sValue));
		AssertTrue(sDebug.cValues.AppendType(&sValue, &sz));
		AssertString("CShape* {CSquare}", sz.Text());
		sz.Kill();
		AssertInt(1, (int)sDebug.cValues.NumChildren(&sValue));
		AssertCPP65DebugChild(&sDebug, &sValue, 0, "miSides", "4 = int", &sChild);

		//main's: the CSquare, its base and its own field; the array.
		AssertString("main", TestCPP65DebugFrameName(&sDebug, 1));
		AssertCPP65DebugVariableShown(&sDebug, 1, "cSquare", "{...} = CSquare");
		AssertTrue(TestCPP65DebugValue(&sDebug, 1, "cSquare", &sValue));
		AssertInt(2, (int)sDebug.cValues.NumChildren(&sValue));
		AssertCPP65DebugChild(&sDebug, &sValue, 0, "CShape", "{...} = CShape {CSquare}", &sChild);
		AssertCPP65DebugChild(&sDebug, &sChild, 0, "miSides", "4 = int", &sGrandchild);
		AssertCPP65DebugChild(&sDebug, &sValue, 1, "miSide", "3 = int", &sChild);
		AssertCPP65DebugVariableShown(&sDebug, 1, "aiCounts", "{...} = int[3]");
		AssertTrue(TestCPP65DebugValue(&sDebug, 1, "aiCounts", &sValue));
		AssertInt(3, (int)sDebug.cValues.NumChildren(&sValue));
		AssertCPP65DebugChild(&sDebug, &sValue, 2, "[2]", "9 = int", &sChild);

		//Hexadecimal Display.
		sDebug.cValues.SetHexadecimal(true);
		AssertCPP65DebugVariableShown(&sDebug, 0, "c", "0x63 'c' = char");
		AssertCPP65DebugChild(&sDebug, &sValue, 2, "[2]", TestCPP65DebugFor(&sDebug, "0x0009 = int", "0x00000009 = int"), &sChild);
		sDebug.cValues.SetHexadecimal(false);

		//Watch expressions (C3), with format suffixes.
		AssertCPP65DebugWatch(&sDebug, 0, "piCounts[2]", "9 = int");
		AssertCPP65DebugWatch(&sDebug, 0, "*piCounts + 1", "8 = int");
		AssertCPP65DebugWatch(&sDebug, 0, "pcShape->miSides", "4 = int");
		AssertCPP65DebugWatch(&sDebug, 0, "((CSquare*)pcShape)->miSide * 2", "6 = int");
		AssertCPP65DebugWatch(&sDebug, 0, "eColour == EC_Green && !(eFlags & EF_B)", "true = bool");
		AssertCPP65DebugWatch(&sDebug, 0, "eFlags & EF_C", "4 = int");
		AssertCPP65DebugWatch(&sDebug, 0, "f * 2", "3 = double");
		AssertCPP65DebugWatch(&sDebug, 0, "1.5f + 1", "2.5 = double");
		AssertCPP65DebugWatch(&sDebug, 0, "-c", "-99 = int");
		AssertCPP65DebugWatch(&sDebug, 0, "(char)65", "65 'A' = char");
		AssertCPP65DebugWatch(&sDebug, 0, "sizeof(int) + sizeof(long)", TestCPP65DebugFor(&sDebug, "6 = unsigned int", "8 = unsigned int"));
		AssertCPP65DebugWatch(&sDebug, 0, "psBits->iHigh + psBits->uiLow", "2 = int");
		AssertCPP65DebugWatch(&sDebug, 0, "szName,s", "\"Ada\\n\" = const char*");
		AssertCPP65DebugWatch(&sDebug, 0, "c,x", "0x63 'c' = char");
		AssertCPP65DebugWatch(&sDebug, 0, "piCounts,3", "{...} = int[3]", &sValue);
		AssertCPP65DebugChild(&sDebug, &sValue, 1, "[1]", "8 = int", &sChild);
		AssertCPP65DebugWatch(&sDebug, 0, "piCounts,3,x", "Unexpected ','");
		AssertCPP65DebugWatch(&sDebug, 1, "aiCounts[1] + aiCounts[2]", "17 = int");
		AssertCPP65DebugWatch(&sDebug, 1, "*(&aiCounts[0] + 2)", "9 = int");
		AssertCPP65DebugWatch(&sDebug, 1, "&aiCounts[2] - aiCounts", TestCPP65DebugFor(&sDebug, "2 = long", "2 = int"));
		AssertCPP65DebugWatch(&sDebug, 1, "sBits.iHigh", "-3 = int");
		AssertCPP65DebugWatch(&sDebug, 1, "cSquare.miSides < cSquare.miSide", "false = bool");
		AssertCPP65DebugWatch(&sDebug, 0, "nope", "nope is undefined");
		AssertCPP65DebugWatch(&sDebug, 0, "pcShape->Area()", "Functions aren't called");
		AssertCPP65DebugWatch(&sDebug, 0, "c = 1", "Assignments aren't evaluated");
		AssertCPP65DebugWatch(&sDebug, 0, "1 / (c - c)", "Divided by zero");
		AssertCPP65DebugWatch(&sDebug, 0, "pcShape->nope", "No member named nope");
		AssertCPP65DebugWatch(&sDebug, 0, "(1 + 2", "Expected ) 'end of expression'");

		//Edited: 9 + 'A' + 20 + EC_Blue + 5 + 2 + 2 + false + 'd' - 1, where it was 9 + 'A' + 8 + EC_Green + 5 + 1 + 2 + true + 'c' - 3.
		AssertFalse(TestCPP65DebugEdit(&sDebug, 0, "eColour", "EC_Purple"));
		AssertFalse(TestCPP65DebugEdit(&sDebug, 0, "c", "300"));
		AssertFalse(TestCPP65DebugEdit(&sDebug, 0, "b", "maybe"));
		AssertFalse(TestCPP65DebugEdit(&sDebug, 1, "aiCounts", "1"));
		AssertTrue(TestCPP65DebugEdit(&sDebug, 0, "eColour", "EC_Blue"));
		AssertTrue(TestCPP65DebugEdit(&sDebug, 0, "f", "2.5"));
		AssertTrue(TestCPP65DebugEdit(&sDebug, 0, "b", "false"));
		AssertTrue(TestCPP65DebugEdit(&sDebug, 0, "c", "'d'"));
		AssertTrue(TestCPP65DebugValue(&sDebug, 1, "aiCounts", &sValue));
		AssertCPP65DebugChild(&sDebug, &sValue, 1, "[1]", "8 = int", &sChild);
		AssertTrue(TestCPP65DebugWrite(&sDebug, &sChild, "20"));
		AssertTrue(TestCPP65DebugValue(&sDebug, 0, "psBits", &sValue));
		AssertCPP65DebugChild(&sDebug, &sValue, 1, "iHigh", "-3 = int", &sChild);
		AssertFalse(TestCPP65DebugWrite(&sDebug, &sChild, "16"));
		AssertTrue(TestCPP65DebugWrite(&sDebug, &sChild, "-1"));
		AssertCPP65DebugChild(&sDebug, &sValue, 0, "uiLow", "5 = unsigned int", &sChild);
		AssertCPP65DebugVariableShown(&sDebug, 0, "eColour", "EC_Blue = EColour");
		AssertCPP65DebugVariableShown(&sDebug, 0, "f", "2.5 = float");
		AssertCPP65DebugVariableShown(&sDebug, 0, "b", "false = bool");
		AssertCPP65DebugVariableShown(&sDebug, 0, "c", "100 'd' = char");
		AssertCPP65DebugWatch(&sDebug, 0, "psBits->iHigh", "-1 = int");
		AssertCPP65DebugWatch(&sDebug, 0, "psBits->uiLow", "5 = unsigned int");
		AssertCPP65DebugWatch(&sDebug, 0, "piCounts[1]", "20 = int");
		szWrong.Init();
		AssertInt(9 + 'A' + 20 + 6 + 5 + 2 + 2 + 0 + 'd' - 1, TestCPP65DebugRunWalking(&sDebug, &szWrong));
		AssertString("", szWrong.Text());
		szWrong.Kill();
		TestCPP65DebugKill(&sDebug);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The uiFrame'th frame of the last walk as the Call Stack shows it: its
// function and parameters.
//
//////////////////////////////////////////////////////////////////////////
static void AssertCPP65DebugCall(STestCPP65Debug* psDebug, size uiFrame, const char* szExpected)
{
	CChars	sz;

	sz.Init();
	psDebug->cValues.AppendCall(psDebug->pcWalker, psDebug->asFrames.Get(uiFrame), &sz);
	AssertString(szExpected, sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The uiFrame'th frame of the last walk's line (CAsm65StackFrame::
// GetLineAddress): 0 if the line table has none.  A row with an inlined
// call inside it is at that call's line.
//
//////////////////////////////////////////////////////////////////////////
static uint32 TestCPP65DebugFrameLine(STestCPP65Debug* psDebug, size uiFrame)
{
	SAsm65LineEntry*	psLine;

	if (psDebug->asFrames.Get(uiFrame)->GetCalled())
	{
		return psDebug->asFrames.Get(uiFrame)->GetCalled()->uiLine;
	}
	psLine = psDebug->cBuilder.GetDebugInfo()->GetLines()->FindLine(psDebug->asFrames.Get(uiFrame)->GetLineAddress());
	return psLine ? psLine->uiLine : 0;
}


//////////////////////////////////////////////////////////////////////////
//
// A Release program (Debugger65C64Plan.txt A3) whose main keeps an array,
// an object and an int whose address is taken in memory, and an int the
// optimiser folds away.
//
//////////////////////////////////////////////////////////////////////////
static const char* gszTestCPP65DebugRelease =
	"struct SPoint { int x; int y; };\n"									//1
	"int Sum(int* pi, int iCount)\n"										//2
	"{\n"																	//3
	"	int i;\n"															//4
	"	int iTotal = 0;\n"													//5
	"	for (i = 0; i < iCount; i++) { iTotal += pi[i]; }\n"				//6
	"	return iTotal;\n"													//7
	"}\n"																	//8
	"int Area(SPoint* ps) { return ps->x * ps->y; }\n"						//9
	"void Bump(int* pi) { *pi = *pi + 1; }\n"								//10
	"int Taken(int iValue, SPoint sAt)\n"									//11
	"{\n"																	//12
	"	Bump(&iValue);\n"													//13
	"	return iValue * sAt.y + sAt.x;\n"									//14
	"}\n"																	//15
	"int main(void)\n"														//16
	"{\n"																	//17
	"	int		aiCounts[3] = { 7, 8, 9 };\n"								//18
	"	SPoint	s;\n"														//19
	"	int		iTaken = 4;\n"												//20
	"	int		iFolded = 3;\n"												//21
	"	s.x = 2;\n"															//22
	"	s.y = 5;\n"															//23
	"	Bump(&iTaken);\n"													//24
	"	return Sum(aiCounts, 3) + Area(&s) + iTaken + iFolded + Taken(7, s);\n"	//25
	"}\n";																	//26																	//21


//////////////////////////////////////////////////////////////////////////
//
// Where the variable szName of the function named szFunction is kept
// (EAsm65DebugWhere), -1 if it isn't described.  A register function's
// variables are its body's (an entry of its own after its stub).
//
//////////////////////////////////////////////////////////////////////////
static int TestCPP65DebugWhere(STestCPP65Debug* psDebug, const char* szFunction, const char* szName)
{
	SAsm65DebugFunction*	psFunction;
	SAsm65DebugVariable*	psVariable;
	size					uiFunction;
	size					ui;

	for (uiFunction = 0; uiFunction < psDebug->pcRecords->NumFunctions(); uiFunction++)
	{
		psFunction = psDebug->pcRecords->GetFunction(uiFunction);
		for (ui = 0; psFunction->szName.Equals(szFunction) && (ui < psFunction->uiNumVariables); ui++)
		{
			psVariable = psDebug->pcRecords->GetVariable(psFunction, ui);
			if (psVariable->szName.Equals(szName))
			{
				return psVariable->uiWhere;
			}
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// A Release build describes the variables whose slot is theirs alone for
// the whole function, on both processors: what stays in memory (an array,
// an object, an int whose address is taken) is shown right in a caller's
// frame; a variable promoted to values is where its values are while they
// are kept (ranges, DebuggerFPlan.txt F1), and one folded to a constant is
// that value (DebuggerFixesPlan.txt G4).  So are parameters where they were passed: one whose
// address is taken, an object its caller pushed; one the optimiser makes
// a value is where its value is (Area's ps).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65DebugRelease(void)
{
	static const ECPPTarget	aeTargets[] = { CPPT_W65C816, CPPT_65C64 };
	STestCPP65Debug		sDebug;
	SCPP65DebugValue		sValue;
	SCPP65DebugValue		sChild;
	CChars					szWrong;
	size					uiTarget;

	for (uiTarget = 0; uiTarget < 2; uiTarget++)
	{
		TestCPP65DebugBuild(&sDebug, gszTestCPP65DebugRelease, TCB_Release, aeTargets[uiTarget]);
		AssertInt(ADW_Frame, TestCPP65DebugWhere(&sDebug, "main", "aiCounts"));
		AssertInt(ADW_Frame, TestCPP65DebugWhere(&sDebug, "main", "s"));
		AssertInt(ADW_Frame, TestCPP65DebugWhere(&sDebug, "main", "iTaken"));
		AssertInt(ADW_Ranges, TestCPP65DebugWhere(&sDebug, "main", "iFolded"));
		AssertInt(ADW_Ranges, TestCPP65DebugWhere(&sDebug, "Sum", "iTotal"));
		AssertInt(ADW_Ranges, TestCPP65DebugWhere(&sDebug, "Sum", "iCount"));
		AssertInt(ADW_Ranges, TestCPP65DebugWhere(&sDebug, "Area", "ps"));
		AssertInt(ADW_Frame, TestCPP65DebugWhere(&sDebug, "Taken", "iValue"));
		AssertInt(ADW_Frame, TestCPP65DebugWhere(&sDebug, "Taken", "sAt"));
		TestCPP65DebugLoad(&sDebug);

		//Stopped in Area, main's: the array, the object, the int Bump added one to.
		AssertTrue(TestCPP65DebugRunTo(&sDebug, 9));
		TestCPP65DebugWalk(&sDebug);
		AssertString("Area", TestCPP65DebugFrameName(&sDebug, 0));
		AssertString("main", TestCPP65DebugFrameName(&sDebug, 1));
		AssertTrue(TestCPP65DebugValue(&sDebug, 0, "ps", &sValue));
		AssertCPP65DebugChild(&sDebug, &sValue, 0, "x", "2 = int", &sChild);
		AssertCPP65DebugChild(&sDebug, &sValue, 1, "y", "5 = int", &sChild);
		AssertTrue(TestCPP65DebugValue(&sDebug, 1, "aiCounts", &sValue));
		AssertInt(3, (int)sDebug.cValues.NumChildren(&sValue));
		AssertCPP65DebugChild(&sDebug, &sValue, 0, "[0]", "7 = int", &sChild);
		AssertCPP65DebugChild(&sDebug, &sValue, 2, "[2]", "9 = int", &sChild);
		AssertTrue(TestCPP65DebugValue(&sDebug, 1, "s", &sValue));
		AssertCPP65DebugChild(&sDebug, &sValue, 0, "x", "2 = int", &sChild);
		AssertCPP65DebugChild(&sDebug, &sValue, 1, "y", "5 = int", &sChild);
		AssertInt(5, TestCPP65DebugLocal(&sDebug, 1, "iTaken"));
		AssertCPP65DebugWatch(&sDebug, 1, "aiCounts[1] + iTaken", "13 = int");
		AssertCPP65DebugWatch(&sDebug, 1, "iFolded", "3 = int");

		//Stopped in Taken after Bump: the parameter whose address went to Bump, and the object its caller pushed.
		AssertTrue(TestCPP65DebugRunTo(&sDebug, 14));
		TestCPP65DebugWalk(&sDebug);
		AssertString("Taken", TestCPP65DebugFrameName(&sDebug, 0));
		AssertInt(8, TestCPP65DebugLocal(&sDebug, 0, "iValue"));
		AssertTrue(TestCPP65DebugValue(&sDebug, 0, "sAt", &sValue));
		AssertCPP65DebugChild(&sDebug, &sValue, 0, "x", "2 = int", &sChild);
		AssertCPP65DebugChild(&sDebug, &sValue, 1, "y", "5 = int", &sChild);
		AssertCPP65DebugWatch(&sDebug, 0, "iValue * sAt.y", "40 = int");
		AssertCPP65DebugCall(&sDebug, 0, "Taken(int iValue = 8, SPoint sAt = {...})");

		szWrong.Init();
		AssertInt(24 + 10 + 5 + 3 + 8 * 5 + 2, TestCPP65DebugRunWalking(&sDebug, &szWrong));
		AssertString("", szWrong.Text());
		szWrong.Kill();
		TestCPP65DebugKill(&sDebug);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Built for speed with inlining, as the IDE's Release is, on both
// processors: a function written again once calls are inlined into it is
// described as it is then (its variables where its slots went last), so
// main's array is still in its frame, and what became a value once Bump
// was inlined (the int whose address went to it) is described by its
// values (ranges).  A function called with a constant is copied for each
// (CIRInterprocedural, as Libc65's FloorDivide is): a copy has its
// variables too, described only once it has code of its own (one inlined
// leaves none to describe them after).  The stack walks at every step.
// A call inlined is a row of its own in a walk with inlines.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65DebugReleaseInlined(void)
{
	static const ECPPTarget	aeTargets[] = { CPPT_W65C816, CPPT_65C64 };
	STestCPP65Debug			sDebug;
	SAsm65StackRegisters	sRegisters;
	CChars					szWrong;
	size					uiTarget;

	for (uiTarget = 0; uiTarget < 2; uiTarget++)
	{
		TestCPP65DebugBuild(&sDebug, gszTestCPP65DebugRelease, TCB_ReleaseInlined, aeTargets[uiTarget]);
		AssertInt(ADW_Frame, TestCPP65DebugWhere(&sDebug, "main", "aiCounts"));
		AssertInt(ADW_Ranges, TestCPP65DebugWhere(&sDebug, "main", "iTaken"));
		AssertInt(ADW_Ranges, TestCPP65DebugWhere(&sDebug, "main", "iFolded"));
		TestCPP65DebugLoad(&sDebug);

		szWrong.Init();
		AssertInt(24 + 10 + 5 + 3 + 8 * 5 + 2, TestCPP65DebugRunWalking(&sDebug, &szWrong));
		AssertString("", szWrong.Text());
		szWrong.Kill();
		TestCPP65DebugKill(&sDebug);
	}

	//Pick, inlined into main's loop (DebuggerFPlan.txt F2): a row of its own at its line, its parameter and its static
	//its own; main's row below it at the call, its variables its own.
	for (uiTarget = 0; uiTarget < 2; uiTarget++)
	{
		TestCPP65DebugBuild(&sDebug,
			"int giValues[4] = { 3, 1, 4, 1 };\n"
			"int Pick(int iIndex)\n"
			"{\n"
			"	static int	siCalls = 0;\n"
			"	siCalls++;\n"
			"	return giValues[iIndex & 3] * 2;\n"
			"}\n"
			"int main(void)\n"
			"{\n"
			"	int	iSum = 0;\n"
			"	for (int i = 0; i < 4; i++) { iSum += Pick(i); }\n"
			"	return iSum;\n"
			"}\n", TCB_ReleaseInlined, aeTargets[uiTarget]);
		TestCPP65DebugLoad(&sDebug);
		AssertTrue(TestCPP65DebugRunTo(&sDebug, 6));
		TestCPP65DebugRunTo(&sDebug, 6);
		TestCPP65DebugRegisters(&sDebug, &sRegisters);
		sDebug.pcWalker->Walk(&sRegisters, &sDebug.asFrames, 64, true);
		AssertString("Pick", TestCPP65DebugFrameName(&sDebug, 0));
		AssertTrue(sDebug.asFrames.Get(0)->IsInline());
		AssertInt(6, TestCPP65DebugFrameLine(&sDebug, 0));
		AssertString("main", TestCPP65DebugFrameName(&sDebug, 1));
		AssertFalse(sDebug.asFrames.Get(1)->IsInline());
		AssertInt(11, TestCPP65DebugFrameLine(&sDebug, 1));
		AssertInt(1, TestCPP65DebugLocal(&sDebug, 0, "iIndex"));
		AssertTrue(sDebug.pcRecords->FindVariable(sDebug.asFrames.Get(0)->psFunction, "iSum", 6, sDebug.asFrames.Get(0)->iInline) == NULL);
		AssertTrue(sDebug.pcRecords->FindVariable(sDebug.asFrames.Get(1)->psFunction, "iSum", 11, sDebug.asFrames.Get(1)->iInline) != NULL);

		//Pick's static, of its row (DebuggerFixesPlan.txt G6): not main's.
		AssertCPP65DebugWatch(&sDebug, 0, "siCalls", "2 = int");
		AssertCPP65DebugWatch(&sDebug, 1, "siCalls", "siCalls is undefined");
		szWrong.Init();
		AssertInt(18, TestCPP65DebugRunWalking(&sDebug, &szWrong));
		AssertString("", szWrong.Text());
		szWrong.Kill();
		TestCPP65DebugKill(&sDebug);
	}

	for (uiTarget = 0; uiTarget < 2; uiTarget++)
	{
		TestCPP65DebugBuild(&sDebug,
			"int giValue = -7;\n"
			"static int FloorDivide(int i, int iBy)\n"
			"{\n"
			"	int	aiParts[2];\n"
			"	aiParts[0] = i / iBy;\n"
			"	aiParts[1] = i % iBy;\n"
			"	if ((aiParts[1] != 0) && ((i < 0) != (iBy < 0))) { aiParts[0]--; }\n"
			"	return aiParts[giValue & 1];\n"
			"}\n"
			"int main(void)\n"
			"{\n"
			"	int	iTotal;\n"
			"	iTotal = FloorDivide(giValue, 4) + FloorDivide(giValue, 100) * 10 + FloorDivide(giValue * 100, 400) * 100;\n"
			"	return iTotal;\n"
			"}\n", TCB_ReleaseInlined, aeTargets[uiTarget]);
		TestCPP65DebugLoad(&sDebug);
		szWrong.Init();
		AssertInt(-3 + -7 * 10 + -300 * 100, TestCPP65DebugRunWalking(&sDebug, &szWrong));
		AssertString("", szWrong.Text());
		szWrong.Kill();
		TestCPP65DebugKill(&sDebug);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The uiFrame'th frame's variables as Locals lists them, their names
// with a space after each.
//
//////////////////////////////////////////////////////////////////////////
static void AssertCPP65DebugLocals(STestCPP65Debug* psDebug, size uiFrame, const char* szExpected)
{
	SAsm65StackFrame*	psFrame;
	CArrayUint32		auiVisible;
	CChars				sz;
	size				ui;

	psFrame = psDebug->asFrames.Get(uiFrame);
	auiVisible.Init();
	sz.Init();
	psFrame->pcRecords->FindVisible(psFrame->psFunction, TestCPP65DebugFrameLine(psDebug, uiFrame), &auiVisible);
	for (ui = 0; ui < auiVisible.NumElements(); ui++)
	{
		sz.Append(psFrame->pcRecords->GetVariable(psFrame->psFunction, auiVisible.GetValue(ui))->szName)->Append(' ');
	}
	AssertString(szExpected, sz.Text());
	sz.Kill();
	auiVisible.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// What Autos shows for the uiFrame'th frame, from szSource's lines at and
// before the frame's: the expressions with a space after each.
//
//////////////////////////////////////////////////////////////////////////
static void AssertCPP65DebugAutos(STestCPP65Debug* psDebug, size uiFrame, const char* szSource, const char* szExpected)
{
	CArrayChars	aszLines;
	CArrayChars	aszAutos;
	CChars		szLines;
	CChars		sz;
	uint32		uiLine;
	size		ui;

	uiLine = TestCPP65DebugFrameLine(psDebug, uiFrame);
	aszLines.Init();
	aszLines.Split(szSource, '\n');
	szLines.Init();
	szLines.Append(aszLines.GetText(uiLine - 2))->AppendNewLine()->Append(aszLines.GetText(uiLine - 1));
	aszAutos.Init();
	psDebug->cWatch.Autos(psDebug->asFrames.Get(uiFrame), uiLine, szLines.Text(), &aszAutos);
	sz.Init();
	for (ui = 0; ui < aszAutos.NumElements(); ui++)
	{
		sz.Append(aszAutos.GetText(ui))->Append(' ');
	}
	AssertString(szExpected, sz.Text());
	sz.Kill();
	aszAutos.Kill();
	szLines.Kill();
	aszLines.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// What the debugger's windows show (DebuggerPlan.txt E): each frame's
// Call Stack text, the variables Locals lists at its line (an inner i
// hiding main's), what Autos shows from the lines, and what a function
// returned just after it returned.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65DebugWindows(void)
{
	static const char*		szSource =
		"struct SPoint { int x; int y; };\n"						//1
		"long Scale(SPoint* psPoint, int iBy)\n"					//2
		"{\n"														//3
		"	long lArea;\n"											//4
		"	lArea = (long)psPoint->x * psPoint->y;\n"				//5
		"	lArea = lArea * iBy;  // psPoint->x isn't used\n"		//6
		"	return lArea;\n"										//7
		"}\n"														//8
		"int main(void)\n"											//9
		"{\n"														//10
		"	SPoint	sPoint;\n"										//11
		"	long	lResult;\n"										//12
		"	int		i = 1;\n"										//13
		"	sPoint.x = 300; sPoint.y = 400;\n"						//14
		"	{\n"													//15
		"		int i = 2;\n"										//16
		"		lResult = Scale(&sPoint, i) /* \"i\" */;\n"			//17
		"	}\n"													//18
		"	return (int)(lResult / 100) + i;\n"					//19
		"}\n";														//20
	STestCPP65Debug				sDebug;
	SAsm65StackFrame				sCaller;
	SAsm65StackRegisters			sRegisters;
	SCPP65DebugValue				sValue;
	CChars							sz;
	size							uiBuild;

	for (uiBuild = 0; uiBuild < TEST_CPP65_DEBUG_BUILDS; uiBuild++)
	{
		TestCPP65DebugBuild(&sDebug, szSource, gasTestCPP65DebugBuilds[uiBuild].eBuild, gasTestCPP65DebugBuilds[uiBuild].eTarget);
		TestCPP65DebugLoad(&sDebug);
		AssertTrue(TestCPP65DebugRunTo(&sDebug, 5));
		TestCPP65DebugWalk(&sDebug);
		AssertString("Scale", TestCPP65DebugFrameName(&sDebug, 0));
		AssertString("main", TestCPP65DebugFrameName(&sDebug, 1));
		AssertInt(5, TestCPP65DebugFrameLine(&sDebug, 0));
		AssertInt(17, TestCPP65DebugFrameLine(&sDebug, 1));

		//The Call Stack's rows.
		sz.Init();
		AssertTrue(sDebug.cValues.AppendCall(sDebug.pcWalker, sDebug.asFrames.Get(0), &sz));
		AssertTrue(sz.StartsWith("Scale(SPoint* psPoint = 0x"));
		AssertTrue(sz.EndsWith(", int iBy = 2)"));
		sz.Clear();
		AssertTrue(sDebug.cValues.AppendCall(sDebug.pcWalker, sDebug.asFrames.Get(1), &sz));
		AssertString("main()", sz.Text());
		sz.Kill();

		//Locals: main's outer i is hidden at its call.
		AssertCPP65DebugLocals(&sDebug, 0, "psPoint iBy lArea ");
		AssertCPP65DebugLocals(&sDebug, 1, "sPoint lResult i ");
		AssertCPP65DebugWatch(&sDebug, 1, "i", "2 = int");

		//Autos: the names on the line and the one before, not calls, types, comments or strings.
		AssertCPP65DebugAutos(&sDebug, 0, szSource, "lArea psPoint->x psPoint->y ");
		AssertCPP65DebugAutos(&sDebug, 1, szSource, "i lResult sPoint ");
		AssertTrue(TestCPP65DebugRunTo(&sDebug, 7));
		TestCPP65DebugWalk(&sDebug);
		AssertCPP65DebugAutos(&sDebug, 0, szSource, "lArea iBy ");

		//Stepped out: what Scale returned (the W65C816's A and X, the 65C64's A).
		sCaller = *sDebug.asFrames.Get(1);
		while (!sDebug.pcCheck->IsStopped() && (TestCPP65DebugPC(&sDebug) != sCaller.uiPC))
		{
			TestCPP65DebugStep(&sDebug);
		}
		TestCPP65DebugRegisters(&sDebug, &sRegisters);
		AssertTrue(sDebug.cValues.Result(sDebug.pcRecords, sDebug.asFrames.Get(0)->psFunction, &sRegisters, &sValue));
		AssertCPP65DebugShown(&sDebug, "240000 = long", &sValue);
		TestCPP65DebugKill(&sDebug);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The uiNth (from 0) function named szName is described as szExpected
// says: how its frame address is found ("follow", "V + 12" or
// "[D + 6] + 5"), what its variables are from (", base V") and each thing
// it saved (", saved V -7", ", saved [128] -11"), in its processor's
// registers' names.
//
//////////////////////////////////////////////////////////////////////////
static void AssertCPP65DebugDescribed(STestCPP65Debug* psDebug, const char* szName, size uiNth, const char* szExpected)
{
	CAsm65DebugRecords*		pcRecords;
	SAsm65DebugFunction*	psFunction;
	SAsm65DebugSaved*		psSaved;
	EAsm65DebugProcessor	eProcessor;
	CChars					sz;
	size					ui;

	pcRecords = psDebug->pcRecords;
	eProcessor = pcRecords->GetProcessor();
	psFunction = NULL;
	for (ui = 0; ui < pcRecords->NumFunctions(); ui++)
	{
		if (pcRecords->GetFunction(ui)->szName.Equals(szName) && (uiNth-- == 0))
		{
			psFunction = pcRecords->GetFunction(ui);
			break;
		}
	}
	AssertTrue(psFunction != NULL);
	if (!psFunction)
	{
		return;
	}
	sz.Init();
	switch (psFunction->uiFrame)
	{
	case ADF_Register:
		sz.Append(Asm65DebugRegisterName(eProcessor, psFunction->uiFrameRegister))->Append(" + ")->Append(psFunction->iFrameOffset);
		break;
	case ADF_Memory:
		sz.Append('[')->Append(Asm65DebugRegisterName(eProcessor, psFunction->uiFrameRegister))->Append(" + ")->Append(psFunction->iFrameOffset)->Append("] + ")->Append(psFunction->iFrameAdjust);
		break;
	default:
		sz.Append("follow");
		break;
	}
	if (psFunction->uiBase != ASM65_DEBUG_FRAME)
	{
		sz.Append(", base ")->Append(Asm65DebugRegisterName(eProcessor, psFunction->uiBase));
	}
	for (ui = 0; ui < psFunction->uiNumSaved; ui++)
	{
		psSaved = pcRecords->GetSaved(psFunction, ui);
		sz.Append(", saved ");
		if (psSaved->bMemory)
		{
			sz.Append('[')->Append((int32)psSaved->uiWhat)->Append(']');
		}
		else
		{
			sz.Append(Asm65DebugRegisterName(eProcessor, psSaved->uiWhat));
		}
		sz.Append(' ')->Append(psSaved->iOffset);
	}
	AssertString(szExpected, sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The 65C64's functions are described by the frame model too: a frame of
// its own from V (the caller's V saved below the jsr's 4 byte return, its
// variables from V); with V a home the caller's V saved and the rest found
// by following its code; with no frame, following its code; a register
// function's body an entry of its own after its stub.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65C64DebugFrames(void)
{
	static const char*	szSource =
		"int Twice(int i) { return i + i; }\n"
		"int One(int a)\n"
		"{\n"
		"	int x = Twice(a);\n"
		"	return x + Twice(x);\n"
		"}\n"
		"int main(void)\n"
		"{\n"
		"	int ai[4] = { 1, 2, 3, 4 };\n"
		"	return One(ai[2]);\n"
		"}\n";
	STestCPP65Debug	sDebug;

	//Debug: every function with variables has a frame, its variables in it.
	TestCPP65DebugBuild(&sDebug, szSource, TCB_Debug, CPPT_65C64);
	AssertInt(ADP_65C64, sDebug.pcRecords->GetProcessor());
	AssertCPP65DebugDescribed(&sDebug, "Twice", 0, "follow");
	AssertCPP65DebugDescribed(&sDebug, "Twice", 1, "V + 12, base V, saved V -7");
	AssertInt(1, (int)TestCPP65DebugFunction(&sDebug, "_main")->uiNumVariables);
	AssertCPP65DebugDescribed(&sDebug, "main", 0, "V + 24, base V, saved V -7");
	TestCPP65DebugKill(&sDebug);

	//Release: Twice has no frame; One keeps x in V across its calls; main's array is split into values.
	TestCPP65DebugBuild(&sDebug, szSource, TCB_Release, CPPT_65C64);
	AssertCPP65DebugDescribed(&sDebug, "Twice", 1, "follow");
	AssertCPP65DebugDescribed(&sDebug, "One", 1, "follow, saved V -7");
	AssertInt(ADW_None, TestCPP65DebugWhere(&sDebug, "main", "ai"));
	TestCPP65DebugKill(&sDebug);

}


//////////////////////////////////////////////////////////////////////////
//
// Debug value markers (DebuggerFPlan.txt F1) change no code: Release
// programs with variables promoted out of their slots (loops, reassigned
// locals, parameters, a variable only a phi holds) compiled with a line
// table on both processors, each also compiled without markers and the
// two compared (CCPPBuild::SetCheckDebugCode).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65DebugMarkersChangeNoCode(void)
{
	static const char*	aszSources[] = {
		"int main() { int s = 0; for (int i = 1; i <= 100; i++) { s += i; } return s; }",
		"int Pick(int a, int b) { int c = a; if (b > a) { c = b; } c = c * 2; return c; }\nint main() { return Pick(3, 4) + Pick(9, 2); }",
		"int Steps(int n) { int k = 0; while (n != 1) { n = (n & 1) ? 3 * n + 1 : n / 2; k++; } return k; }\nint main() { return Steps(27); }",
		gszTestCPP65DebugRelease,
		NULL };
	static ECPPTarget	aeTargets[] = { CPPT_65C64, CPPT_W65C816 };
	CCPP65Builder		cBuilder;
	CChars				szAssembly;
	CChars				szErrors;
	size				ui;
	size				uj;

	CCPPBuild::SetCheckDebugCode(true);
	for (uj = 0; uj < sizeof(aeTargets) / sizeof(ECPPTarget); uj++)
	{
		for (ui = 0; aszSources[ui]; ui++)
		{
			cBuilder.Init();
			TestCPP65AddIncludeDirectories(&cBuilder);
			szAssembly.Init();
			szErrors.Init();
			cBuilder.GetBuild()->SetTarget(aeTargets[uj]);
			cBuilder.GetBuild()->SetLineTable(true);
			AssertTrue(cBuilder.GetBuild()->Generate("Test.cpp", aszSources[ui], true, &szAssembly, &szErrors));
			AssertString("", szErrors.Text());
			szErrors.Kill();
			szAssembly.Kill();
			cBuilder.Kill();
		}
	}
	CCPPBuild::SetCheckDebugCode(getenv("TEST_CPP65_SAME_CODE") != NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// Runs the loaded program to its end a step at a time, every frame's
// integer variables (an inlined call's row its callee's) read at each
// step where the debugger can read them:
// with bRecord all of them, each value put in pcSeen as "Function.name=
// value"; else only those whose place moves (ADW_Ranges), each value
// expected in pcSeen (the first few that aren't in pszWrong).  Returns how
// many were read.
//
//////////////////////////////////////////////////////////////////////////
static size TestCPP65DebugRunReading(STestCPP65Debug* psDebug, CMapStringInt* pcSeen, bool bRecord, CChars* pszWrong)
{
	SAsm65StackRegisters	sRegisters;
	SAsm65StackFrame*		psFrame;
	SAsm65DebugVariable*	psVariable;
	SAsm65DebugType*		psType;
	SCPP65DebugValue		sValue;
	CChars					szKey;
	uint64					uiValue;
	size					uiSteps;
	size					uiFrame;
	size					ui;
	size					uiRead;
	size					uiWrong;

	uiRead = 0;
	uiWrong = 0;
	szKey.Init();
	for (uiSteps = 0; (uiSteps < 2000000) && !psDebug->pcCheck->IsStopped(); uiSteps++)
	{
		TestCPP65DebugRegisters(psDebug, &sRegisters);
		psDebug->pcWalker->Walk(&sRegisters, &psDebug->asFrames, 64, true);
		for (uiFrame = 0; uiFrame < psDebug->asFrames.NumElements(); uiFrame++)
		{
			psFrame = psDebug->asFrames.Get(uiFrame);
			for (ui = 0; psFrame->psFunction && (ui < psFrame->psFunction->uiNumVariables); ui++)
			{
				psVariable = psFrame->pcRecords->GetVariable(psFrame->psFunction, ui);
				psType = psFrame->pcRecords->GetType(psVariable->uiType);
				if ((psVariable->iInline != psFrame->iInline) || (!bRecord && (psVariable->uiWhere != ADW_Ranges)) || !psType || (psType->uiKind != ADT_Integer) ||
					!psDebug->cValues.Variable(psDebug->pcWalker, psFrame, psVariable, &sValue) || !psDebug->cValues.Read(&sValue, &uiValue))
				{
					continue;
				}
				szKey.Set(psFrame->GetName());
				szKey.Append('.')->Append(psVariable->szName)->Append('=')->Append((int64)uiValue);
				uiRead++;
				if (bRecord)
				{
					pcSeen->Put(szKey.Text(), 1);
				}
				else if ((pcSeen->Get(szKey.Text(), 0) == 0) && (uiWrong++ < 5))
				{
					pszWrong->Append(szKey)->Append(" at ")->Append((int64)TestCPP65DebugPC(psDebug))->AppendNewLine();
				}
			}
		}
		TestCPP65DebugStep(psDebug);
	}
	szKey.Kill();
	AssertTrue(psDebug->pcCheck->IsStopped());
	return uiRead;
}


//A program whose variables take many values, in registers and the frame, across calls and round loops.
static const char* gszTestCPP65DebugEverywhere =
	"int Mix(int a, int b)\n"
	"{\n"
	"	int c = a * 3 + b;\n"
	"	int d = c - a;\n"
	"	if (d > 100) { d = d - 100; }\n"
	"	return c * 7 + d;\n"
	"}\n"
	"int Count(int n)\n"
	"{\n"
	"	int iSum = 0;\n"
	"	int iOdd = 0;\n"
	"	for (int i = 1; i <= n; i++)\n"
	"	{\n"
	"		iSum += i * i;\n"
	"		if (i & 1) { iOdd += Mix(i, iSum); }\n"
	"	}\n"
	"	return iSum + iOdd;\n"
	"}\n"
	"int main(void) { return Count(9) & 0x3FFF; }\n";


//////////////////////////////////////////////////////////////////////////
//
// Optimised code's variables are right wherever they are shown
// (DebuggerFPlan.txt F1-7, F2): on both processors each program runs built
// Debug, every value each variable takes recorded, then built Release
// (with inlining too: an inlined callee's variables are its row's), where
// at every step every variable with a place is read and must hold one of
// the values it held in the Debug run (a home another value took
// over, or a range that starts before its value is there, would show
// one it never had).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65DebugValuesEverywhere(void)
{
	static const ECPPTarget	aeTargets[] = { CPPT_W65C816, CPPT_65C64 };
	static const char*		aszSources[] = { gszTestCPP65DebugEverywhere, gszTestCPP65DebugRelease, NULL };
	static const ETestCPP65Build	aeBuilds[] = { TCB_Release, TCB_ReleaseInlined };
	STestCPP65Debug			sDebug;
	size					uiBuild;
	CMapStringInt			cSeen;
	CChars					szWrong;
	size					uiTarget;
	size					uiSource;
	size					uiRead;

	for (uiTarget = 0; uiTarget < 2; uiTarget++)
	{
		for (uiSource = 0; aszSources[uiSource]; uiSource++)
		{
			cSeen.Init();
			TestCPP65DebugBuild(&sDebug, aszSources[uiSource], TCB_Debug, aeTargets[uiTarget]);
			TestCPP65DebugLoad(&sDebug);
			TestCPP65DebugRunReading(&sDebug, &cSeen, true, NULL);
			TestCPP65DebugKill(&sDebug);

			for (uiBuild = 0; uiBuild < 2; uiBuild++)
			{
				TestCPP65DebugBuild(&sDebug, aszSources[uiSource], aeBuilds[uiBuild], aeTargets[uiTarget]);
				TestCPP65DebugLoad(&sDebug);
				szWrong.Init();
				uiRead = TestCPP65DebugRunReading(&sDebug, &cSeen, false, &szWrong);
				AssertString("", szWrong.Text());
				AssertTrue(uiRead > 20);
				szWrong.Kill();
				TestCPP65DebugKill(&sDebug);
			}
			cSeen.Kill();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A Release variable in a register (DebuggerFPlan.txt F1-6), on both
// processors: Autos lists it, editing it writes the register (the
// program's result shows it), and it has no address.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65DebugRegisterVariables(void)
{
	static const ECPPTarget	aeTargets[] = { CPPT_W65C816, CPPT_65C64 };
	STestCPP65Debug			sDebug;
	SCPP65DebugValue		sValue;
	CArrayChars				aszAutos;
	CChars					szWrong;
	size					uiTarget;

	for (uiTarget = 0; uiTarget < 2; uiTarget++)
	{
		TestCPP65DebugBuild(&sDebug,
			"int Total(int n)\n"
			"{\n"
			"	int iTotal = 0;\n"
			"	for (int i = 1; i <= n; i++)\n"
			"	{\n"
			"		iTotal += i;\n"
			"	}\n"
			"	return iTotal;\n"
			"}\n"
			"int main(void) { return Total(10); }\n", TCB_Release, aeTargets[uiTarget]);
		TestCPP65DebugLoad(&sDebug);

		//Round the loop twice: iTotal is 1 + 2 as the third time starts.
		AssertTrue(TestCPP65DebugRunTo(&sDebug, 6));
		TestCPP65DebugStep(&sDebug);
		AssertTrue(TestCPP65DebugRunTo(&sDebug, 6));
		TestCPP65DebugStep(&sDebug);
		AssertTrue(TestCPP65DebugRunTo(&sDebug, 6));
		TestCPP65DebugWalk(&sDebug);
		AssertInt(3, TestCPP65DebugLocal(&sDebug, 0, "iTotal"));
		AssertTrue(TestCPP65DebugValue(&sDebug, 0, "iTotal", &sValue));
		if (aeTargets[uiTarget] == CPPT_65C64)
		{
			AssertTrue(sValue.uiRegister != CPP65_DEBUG_NO_REGISTER);
			AssertCPP65DebugWatch(&sDebug, 0, "&iTotal", "A variable in a register has no address");
		}
		aszAutos.Init();
		sDebug.cWatch.Autos(sDebug.asFrames.Get(0), 6, "		iTotal += i;", &aszAutos);
		AssertTrue(aszAutos.Contains("iTotal"));
		AssertTrue(aszAutos.Contains("i"));
		aszAutos.Kill();

		AssertTrue(TestCPP65DebugEdit(&sDebug, 0, "iTotal", "1003"));
		TestCPP65DebugWalk(&sDebug);
		AssertInt(1003, TestCPP65DebugLocal(&sDebug, 0, "iTotal"));
		szWrong.Init();
		AssertInt(1055, TestCPP65DebugRunWalking(&sDebug, &szWrong));
		szWrong.Kill();
		TestCPP65DebugKill(&sDebug);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// this in a Release member function (DebuggerGapsPlan.txt A), on both
// processors: only a value there, it is where its value is
// (DebuggerFPlan.txt F1), and its members are read through it.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65DebugReleaseThis(void)
{
	static const ECPPTarget	aeTargets[] = { CPPT_W65C816, CPPT_65C64 };
	STestCPP65Debug			sDebug;
	CChars					szWrong;
	size					uiTarget;

	for (uiTarget = 0; uiTarget < 2; uiTarget++)
	{
		TestCPP65DebugBuild(&sDebug,
			"class CShape\n"
			"{\n"
			"public:\n"
			"	int	miSides;\n"
			"	int	miSize;\n"
			"	int	Perimeter(void);\n"
			"};\n"
			"int CShape::Perimeter(void)\n"
			"{\n"
			"	return miSides * miSize;\n"
			"}\n"
			"int main(void)\n"
			"{\n"
			"	CShape	cShape;\n"
			"	cShape.miSides = 3;\n"
			"	cShape.miSize = 7;\n"
			"	return cShape.Perimeter();\n"
			"}\n", TCB_Release, aeTargets[uiTarget]);
		AssertInt(ADW_Ranges, TestCPP65DebugWhere(&sDebug, "CShape::Perimeter", "this"));
		TestCPP65DebugLoad(&sDebug);
		AssertTrue(TestCPP65DebugRunTo(&sDebug, 10));
		TestCPP65DebugWalk(&sDebug);
		AssertString("CShape::Perimeter", TestCPP65DebugFrameName(&sDebug, 0));
		AssertCPP65DebugWatch(&sDebug, 0, "this->miSides", "3 = int");
		AssertCPP65DebugWatch(&sDebug, 0, "miSize", "7 = int");
		szWrong.Init();
		AssertInt(21, TestCPP65DebugRunWalking(&sDebug, &szWrong));
		AssertString("", szWrong.Text());
		szWrong.Kill();
		TestCPP65DebugKill(&sDebug);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A 65C64 Release function with no frame of its own (DebuggerFixesPlan.txt
// G1): its variables are from its frame address, which the walk finds, so
// its parameters past the register arguments are read where its caller
// pushed them.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65DebugFramelessParameters(void)
{
	STestCPP65Debug			sDebug;
	SAsm65DebugFunction*	psFunction;
	CChars					szWrong;

	TestCPP65DebugBuild(&sDebug,
		"int giValues[5] = { 1, 2, 3, 4, 5 };\n"
		"int Five(int a, int b, int c, int d, int e)\n"
		"{\n"
		"	return a + b * 2 + c * 3 + d * 4 + e * 5;\n"
		"}\n"
		"int main(void)\n"
		"{\n"
		"	return Five(giValues[0], giValues[1], giValues[2], giValues[3], giValues[4]);\n"
		"}\n", TCB_Release, CPPT_65C64);
	psFunction = TestCPP65DebugNamed(&sDebug, "Five");
	AssertTrue(psFunction != NULL);
	AssertInt(ASM65_DEBUG_FRAME, psFunction->uiBase);
	AssertInt(ADW_Ranges, TestCPP65DebugWhere(&sDebug, "Five", "e"));
	TestCPP65DebugLoad(&sDebug);
	AssertTrue(TestCPP65DebugRunTo(&sDebug, 4));
	TestCPP65DebugWalk(&sDebug);
	AssertString("Five", TestCPP65DebugFrameName(&sDebug, 0));
	AssertTrue(sDebug.asFrames.Get(0)->bFrame);
	AssertInt(4, TestCPP65DebugLocal(&sDebug, 0, "d"));
	AssertInt(5, TestCPP65DebugLocal(&sDebug, 0, "e"));
	szWrong.Init();
	AssertInt(1 + 4 + 9 + 16 + 25, TestCPP65DebugRunWalking(&sDebug, &szWrong));
	AssertString("", szWrong.Text());
	szWrong.Kill();
	TestCPP65DebugKill(&sDebug);
}


//////////////////////////////////////////////////////////////////////////
//
// A Debug function's local read at each instruction from its last line to
// its return (DebuggerFixesPlan.txt G3), on both processors: its epilogue
// gives the caller's D or V back before it returns, and the local is still
// read from its own frame (its base from its frame address).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65DebugEpilogueVariables(void)
{
	static const ECPPTarget	aeTargets[] = { CPPT_W65C816, CPPT_65C64 };
	STestCPP65Debug			sDebug;
	CChars					szWrong;
	size					uiTarget;
	size					uiSteps;

	for (uiTarget = 0; uiTarget < 2; uiTarget++)
	{
		TestCPP65DebugBuild(&sDebug,
			"int Twice(int i)\n"
			"{\n"
			"	int	iDouble = i * 2;\n"
			"	return iDouble + 1;\n"
			"}\n"
			"int main(void)\n"
			"{\n"
			"	int	iMain = 99;\n"
			"	return Twice(20) + iMain;\n"
			"}\n", TCB_Debug, aeTargets[uiTarget]);
		TestCPP65DebugLoad(&sDebug);
		AssertTrue(TestCPP65DebugRunTo(&sDebug, 4));
		for (uiSteps = 0; uiSteps < 100; uiSteps++)
		{
			TestCPP65DebugWalk(&sDebug);
			if (StringCompare(TestCPP65DebugFrameName(&sDebug, 0), "Twice") != 0)
			{
				break;
			}
			AssertInt(40, TestCPP65DebugLocal(&sDebug, 0, "iDouble"));
			AssertInt(99, TestCPP65DebugLocal(&sDebug, 1, "iMain"));
			TestCPP65DebugStep(&sDebug);
		}
		AssertTrue(uiSteps > 3);
		AssertString("main", TestCPP65DebugFrameName(&sDebug, 0));
		szWrong.Init();
		AssertInt(41 + 99, TestCPP65DebugRunWalking(&sDebug, &szWrong));
		AssertString("", szWrong.Text());
		szWrong.Kill();
		TestCPP65DebugKill(&sDebug);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65Debug(void)
{
	BeginTests();

	TestCPP65DebugFrames();
	TestCPP65DebugModes();
	TestCPP65DebugVariables();
	TestCPP65DebugStack();
	TestCPP65DebugValues();
	TestCPP65DebugWindows();
	TestCPP65DebugRelease();
	TestCPP65DebugReleaseInlined();
	TestCPP65C64DebugFrames();
	TestCPP65DebugMarkersChangeNoCode();
	TestCPP65DebugValuesEverywhere();
	TestCPP65DebugRegisterVariables();
	TestCPP65DebugReleaseThis();
	TestCPP65DebugFramelessParameters();
	TestCPP65DebugEpilogueVariables();

	TestStatistics();
}
