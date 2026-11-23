#include "BaseLib/Chars.h"
#include "BaseLib/FileUtil.h"
#include "CppCompilerLib/CPP65Builder.h"
#include "Assembler65Lib/Asm65StackWalker.h"
#include "Assembler65Lib/AsmW65C816Disassembler.h"
#include "Assembler65Lib/AsmW65C816FrameCheck.h"
#include "TestLib/Assert.h"
#include "TestCPP65Context.h"


CCPPCompileOptions*	TestCPP816Options(void);	//TestCPPW65C816.cpp


//How a program is built: for speed (Release, though without inlining so each function is a function of its own), for
//a debugger (Debug) or for one and not optimised (an Unoptimised project).
enum ETestCPP816Build
{
	TCB_Release,
	TCB_Debug,
	TCB_Unoptimised,
};


//A program built for the W65C816 with its debug info (CCPP65Builder::SetLineTable), and as a debugger runs it
//(TestCPP816DebugLoad): a step at a time, its calls checked (CAsmW65C816FrameCheck) and its stack walked.
struct STestCPP816Debug
{
	CCPP65Builder			cBuilder;
	CAsm65DebugRecords*		pcRecords;
	CAsm65Memory			cMemory;
	CFastW65C816			cFast;
	CAsmW65C816FrameCheck	cCheck;
	CAsm65StackWalker		cWalker;
	CArrayAsm65StackFrames	asFrames;
	bool					bLoaded;
};


//////////////////////////////////////////////////////////////////////////
//
// Builds szSource with its debug info, as the IDE does, eBuild's way, with
// Libc65's string routines (the optimiser makes loops calls of them).
//
//////////////////////////////////////////////////////////////////////////
static bool TestCPP816DebugBuild(STestCPP816Debug* psDebug, const char* szSource, ETestCPP816Build eBuild = TCB_Release)
{
	static const char*	aszUnits[] = { "Libc65/stringW65C816.asm", "Libc65/stringW65C816_16.asm", NULL };
	CChars				szProject;
	CFileUtil			cFileUtil;
	bool				bResult;

	psDebug->bLoaded = false;
	psDebug->cBuilder.Init();
	*psDebug->cBuilder.GetOptions() = *TestCPP816Options();
	psDebug->cBuilder.GetOptions()->mbInline = false;
	psDebug->cBuilder.GetOptions()->mbDebuggable = (eBuild != TCB_Release);
	psDebug->cBuilder.GetOptions()->mbOptimise = (eBuild != TCB_Unoptimised);
	psDebug->cBuilder.SetTarget(CPPT_W65C816);
	psDebug->cBuilder.SetLineTable(true);
	TestCPP65AddIncludeDirectories(&psDebug->cBuilder);
	AssertTrue(psDebug->cBuilder.AddStartUp());
	szProject.Init(".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "HCP65" _FS_ "Libraries" _FS_ "Libc65");
	cFileUtil.FullPath(&szProject);
	psDebug->cBuilder.AddMathTables(szProject.Text());
	szProject.Kill();
	AssertTrue(psDebug->cBuilder.AddSource("Test.cpp", szSource));
	TestCPP65AddUnits(&psDebug->cBuilder, aszUnits, false);
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
static void TestCPP816DebugLoad(STestCPP816Debug* psDebug)
{
	psDebug->cMemory.Init();
	psDebug->cMemory.Load(psDebug->cBuilder.GetOutput());
	psDebug->cMemory.InitFast65816(&psDebug->cFast);
	psDebug->cFast.Reset();
	psDebug->cCheck.Init(&psDebug->cMemory, &psDebug->cFast);
	psDebug->cWalker.Init(&psDebug->cMemory);
	psDebug->cWalker.AddRecords(psDebug->pcRecords);
	psDebug->asFrames.Init();
	psDebug->bLoaded = true;
}


//////////////////////////////////////////////////////////////////////////
//
// Runs the instruction at the program address, its calls followed.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPP816DebugStep(STestCPP816Debug* psDebug)
{
	uint8	uiOpcode;

	uiOpcode = psDebug->cCheck.BeforeStep();
	psDebug->cFast.Step();
	psDebug->cCheck.AfterStep(uiOpcode);
}


//////////////////////////////////////////////////////////////////////////
//
// Walks the stack from the W65C816's registers into asFrames.  Returns
// how many frames.
//
//////////////////////////////////////////////////////////////////////////
static size TestCPP816DebugWalk(STestCPP816Debug* psDebug)
{
	SAsm65StackRegisters	sRegisters;

	sRegisters.uiPC = psDebug->cFast.GetProgramAddress();
	sRegisters.uiS = psDebug->cFast.GetS();
	sRegisters.uiD = psDebug->cFast.GetD();
	sRegisters.uiC = psDebug->cFast.GetC();
	sRegisters.uiX = psDebug->cFast.GetX();
	sRegisters.uiY = psDebug->cFast.GetY();
	sRegisters.uiP = psDebug->cFast.GetP();
	return psDebug->cWalker.Walk(&sRegisters, &psDebug->asFrames, 64);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestCPP816DebugKill(STestCPP816Debug* psDebug)
{
	if (psDebug->bLoaded)
	{
		psDebug->asFrames.Kill();
		psDebug->cWalker.Kill();
		psDebug->cCheck.Kill();
		psDebug->cFast.Kill();
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
// The function named szName, NULL if there is none (which fails).
//
//////////////////////////////////////////////////////////////////////////
static SAsm65DebugFunction* TestCPP816DebugNamed(STestCPP816Debug* psDebug, const char* szName)
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
static void AssertCPP816DebugVariable(STestCPP816Debug* psDebug, SAsm65DebugFunction* psFunction, size uiIndex, const char* szName, bool bParameter, const char* szType, EAsm65DebugWhere eWhere, uint32 uiFirst, uint32 uiLast)
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
			//In the frame, above the scratch bytes and below the link.
			AssertTrue(psVariable->iOffset >= 23);
			AssertTrue(psVariable->iOffset <= psFunction->iLink);
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
void TestCPP816DebugVariables(void)
{
	STestCPP816Debug		sDebug;
	SAsm65DebugFunction*	psFunction;
	SAsm65DebugType*		psType;
	SAsm65DebugMember*		psMember;
	SAsm65DebugGlobal*		psGlobal;
	int64					iLabel;

	TestCPP816DebugBuild(&sDebug,
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
	psFunction = TestCPP816DebugNamed(&sDebug, "CShape::Area");
	AssertInt(2, psFunction ? psFunction->uiNumVariables : 0);
	AssertCPP816DebugVariable(&sDebug, psFunction, 0, "this", true, "CShape*", ADW_Frame, 8, 8);
	AssertCPP816DebugVariable(&sDebug, psFunction, 1, "iScale", true, "int", ADW_Frame, 8, 8);

	//An object passed by value is read through the address kept for it; a for's local is in scope in the for.
	psFunction = TestCPP816DebugNamed(&sDebug, "Total");
	AssertInt(4, psFunction ? psFunction->uiNumVariables : 0);
	AssertCPP816DebugVariable(&sDebug, psFunction, 0, "s", true, "SBig", ADW_Indirect, 12, 22);
	AssertCPP816DebugVariable(&sDebug, psFunction, 1, "iFrom", true, "int", ADW_Frame, 12, 22);
	AssertCPP816DebugVariable(&sDebug, psFunction, 2, "l", false, "long", ADW_Frame, 15, 22);
	AssertCPP816DebugVariable(&sDebug, psFunction, 3, "i", false, "int", ADW_Frame, 16, 19);
	AssertTrue(sDebug.pcRecords->GetVariable(psFunction, 2)->iOffset != sDebug.pcRecords->GetVariable(psFunction, 3)->iOffset);

	//The static local is Total's, the global is anyone's, each at its label.
	psGlobal = sDebug.pcRecords->FindGlobal("siCalls", psFunction);
	AssertTrue(psGlobal != NULL);
	AssertTrue(sDebug.pcRecords->FindGlobal("siCalls", NULL) == NULL);
	psGlobal = sDebug.pcRecords->FindGlobal("NS::giCount", NULL);
	AssertTrue(psGlobal != NULL);
	AssertTrue(sDebug.cBuilder.GetSymbol("_NS__giCount", &iLabel));
	AssertInt((uint32)iLabel, psGlobal ? psGlobal->sPlace.uiOffset : 0);

	psFunction = TestCPP816DebugNamed(&sDebug, "main");
	AssertCPP816DebugVariable(&sDebug, psFunction, 0, "cShape", false, "CShape", ADW_Frame, 25, 30);
	AssertCPP816DebugVariable(&sDebug, psFunction, 1, "sBig", false, "SBig", ADW_Frame, 26, 30);

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

	TestCPP816DebugKill(&sDebug);
}


//////////////////////////////////////////////////////////////////////////
//
// Walks the stack and appends to pszWrong where the walk isn't the calls
// the W65C816 is in (CAsmW65C816FrameCheck::WrongFrame).
//
//////////////////////////////////////////////////////////////////////////
static void TestCPP816DebugCheckWalk(STestCPP816Debug* psDebug, CChars* pszWrong)
{
	SAsm65StackFrame*	psFrame;
	SAsmW65C816Widths	sWidths;
	size				uiWrong;

	TestCPP816DebugWalk(psDebug);
	uiWrong = psDebug->cCheck.WrongFrame(&psDebug->asFrames);
	if (uiWrong != 0)
	{
		psFrame = psDebug->asFrames.Get(0);
		pszWrong->Append("$");
		pszWrong->AppendHexHiLo(&psFrame->uiPC, 3);
		pszWrong->Append(" in ");
		pszWrong->Append(psFrame->psFunction->szName);
		pszWrong->Append(" (");
		sWidths.bMemory8 = (psDebug->cFast.GetP() & 0x20) != 0;
		sWidths.bIndex8 = (psDebug->cFast.GetP() & 0x10) != 0;
		AsmW65C816Disassemble(psFrame->uiPC, psDebug->cMemory.Byte(psFrame->uiPC), 4, &sWidths, pszWrong);
		pszWrong->Append("): frame ");
		pszWrong->Append((int)uiWrong);
		pszWrong->Append(" of ");
		pszWrong->Append((int)psDebug->cCheck.NumCalls() + 1);
		pszWrong->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Runs the loaded program to its end, a step at a time, its stack walked
// at each step in a function the records describe (the first few walks
// that are wrong in pszWrong).  Returns main's int, -1000000 if it didn't
// stop.
//
//////////////////////////////////////////////////////////////////////////
static int32 TestCPP816DebugRunWalking(STestCPP816Debug* psDebug, CChars* pszWrong)
{
	size	uiSteps;
	size	uiWrong;
	size	uiLength;

	uiWrong = 0;
	for (uiSteps = 0; (uiSteps < 20000000) && !psDebug->cFast.IsStopped(); uiSteps++)
	{
		if ((uiWrong < 8) && psDebug->pcRecords->FindFunction(psDebug->cFast.GetProgramAddress()))
		{
			uiLength = pszWrong->Length();
			TestCPP816DebugCheckWalk(psDebug, pszWrong);
			uiWrong += (pszWrong->Length() > uiLength) ? 1 : 0;
		}
		TestCPP816DebugStep(psDebug);
	}
	AssertString("", psDebug->cCheck.GetWrites());
	AssertInt(0, (int)psDebug->cMemory.NumBadAccesses());
	AssertTrue(psDebug->cFast.IsStopped());
	return psDebug->cFast.IsStopped() ? (int16)psDebug->cFast.GetC() : -1000000;
}


//////////////////////////////////////////////////////////////////////////
//
// Runs the loaded program until it is about to run line uiLine of
// Test.cpp.  Returns whether it got there.
//
//////////////////////////////////////////////////////////////////////////
static bool TestCPP816DebugRunTo(STestCPP816Debug* psDebug, uint32 uiLine)
{
	CArrayUint32	auiAddresses;
	size			uiSteps;
	bool			bThere;

	auiAddresses.Init();
	AssertTrue(psDebug->cBuilder.GetDebugInfo()->GetLines()->FindLineAddresses("Test.cpp", uiLine, &auiAddresses) > 0);
	bThere = false;
	for (uiSteps = 0; (uiSteps < 20000000) && !psDebug->cFast.IsStopped() && !bThere; uiSteps++)
	{
		TestCPP816DebugStep(psDebug);
		bThere = auiAddresses.Find(psDebug->cFast.GetProgramAddress()) != ARRAY_ELEMENT_NOT_FOUND;
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
static const char* TestCPP816DebugFrameName(STestCPP816Debug* psDebug, size uiFrame)
{
	SAsm65StackFrame*	psFrame;

	if (uiFrame >= psDebug->asFrames.NumElements())
	{
		return "?";
	}
	psFrame = psDebug->asFrames.Get(uiFrame);
	return psFrame->psFunction ? psFrame->psFunction->szName.Text() : "";
}


//////////////////////////////////////////////////////////////////////////
//
// The int named szName in the uiFrame'th frame of the last walk:
// -1000000 if its function has none, or the debugger can't read it.
//
//////////////////////////////////////////////////////////////////////////
static int32 TestCPP816DebugLocal(STestCPP816Debug* psDebug, size uiFrame, const char* szName)
{
	SAsm65StackFrame*		psFrame;
	SAsm65DebugVariable*	psVariable;
	uint32					uiAddress;
	size					ui;

	psFrame = (uiFrame < psDebug->asFrames.NumElements()) ? psDebug->asFrames.Get(uiFrame) : NULL;
	for (ui = 0; psFrame && psFrame->psFunction && (ui < psFrame->psFunction->uiNumVariables); ui++)
	{
		psVariable = psFrame->pcRecords->GetVariable(psFrame->psFunction, ui);
		if (psVariable->szName.Equals(szName) && psDebug->cWalker.Locate(psFrame, psVariable, &uiAddress))
		{
			return (int16)psDebug->cMemory.GetWord(uiAddress);
		}
	}
	return -1000000;
}


//////////////////////////////////////////////////////////////////////////
//
// Stopped at a line in a recursion, the stack walks back through each
// call to main, each frame's locals and parameters read where the debug
// records say (a global too), built for a debugger optimised and not.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816DebugStack(void)
{
	static const ETestCPP816Build	aeBuilds[] = { TCB_Debug, TCB_Unoptimised };
	STestCPP816Debug				sDebug;
	SAsm65DebugGlobal*				psGlobal;
	size							uiBuild;
	int								i;

	for (uiBuild = 0; uiBuild < 2; uiBuild++)
	{
		TestCPP816DebugBuild(&sDebug,
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
			"}\n", aeBuilds[uiBuild]);					//17
		TestCPP816DebugLoad(&sDebug);

		//Factorial(1) to Factorial(5), main, and the start up, which has no records.
		AssertTrue(TestCPP816DebugRunTo(&sDebug, 8));
		AssertInt(7, (int)TestCPP816DebugWalk(&sDebug));
		for (i = 0; i < 5; i++)
		{
			AssertString("Factorial", TestCPP816DebugFrameName(&sDebug, i));
			AssertInt(i + 1, TestCPP816DebugLocal(&sDebug, i, "i"));
		}
		AssertString("main", TestCPP816DebugFrameName(&sDebug, 5));
		AssertInt(5, TestCPP816DebugLocal(&sDebug, 5, "iFrom"));
		AssertString("", TestCPP816DebugFrameName(&sDebug, 6));
		AssertInt(-1000000, TestCPP816DebugLocal(&sDebug, 5, "i"));

		psGlobal = sDebug.pcRecords->FindGlobal("giDepth", NULL);
		AssertTrue(psGlobal != NULL);
		AssertInt(5, psGlobal ? sDebug.cMemory.GetWord(psGlobal->sPlace.uiOffset) : 0);

		//On the way back out, each caller's local is what its callee returned.
		AssertTrue(TestCPP816DebugRunTo(&sDebug, 11));
		TestCPP816DebugWalk(&sDebug);
		AssertInt(2, TestCPP816DebugLocal(&sDebug, 0, "i"));
		AssertInt(1, TestCPP816DebugLocal(&sDebug, 0, "iBelow"));

		TestCPP816DebugKill(&sDebug);
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
// as built for speed (Release); and at every step in each, the stack
// walks back through the calls it is in.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816DebugModes(void)
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
		"	return 100;\n"
		"}\n";
	static const ETestCPP816Build	aeBuilds[] = { TCB_Release, TCB_Debug, TCB_Unoptimised };
	STestCPP816Debug				sDebug;
	CChars							szWrong;
	size							uiBuild;

	for (uiBuild = 0; uiBuild < 3; uiBuild++)
	{
		TestCPP816DebugBuild(&sDebug, szSource, aeBuilds[uiBuild]);
		TestCPP816DebugLoad(&sDebug);
		szWrong.Init();
		AssertInt(100, TestCPP816DebugRunWalking(&sDebug, &szWrong));
		AssertString("", szWrong.Text());
		szWrong.Kill();
		TestCPP816DebugKill(&sDebug);
	}
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
	TestCPP816DebugVariables();
	TestCPP816DebugStack();

	TestStatistics();
}
