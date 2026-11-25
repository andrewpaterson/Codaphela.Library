#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Chars.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/NumberControl.h"
#include "Assembler65Lib/Asm65Memory.h"
#include "CppCompilerLib/CPP65Builder.h"
#include "CppCompilerLib/CPPTarget.h"
#include "CppCompilerLib/CPPCompileOptions.h"
#include "CppCompilerLib/CPPPreprocessor.h"
#include "CppCompilerLib/CppTokeniser.h"
#include "CppCompilerLib/CppParser.h"
#include "CppCompilerLib/CPPSyntaxes.h"
#include "CppCompilerLib/CPPSemanticPass.h"
#include "TestLib/Assert.h"
#include "TestCPP65Context.h"


//A program that must be refused, where a case's value would be.
#define CONFORMANCE_REFUSED		(-1)
#define CONFORMANCE_TRIED		(-2)	//TEST_CPP_CONFORMANCE_TRY's program: what it does is written out.

//The most instructions a case runs for: they are small.
#define CONFORMANCE_STEPS		2000000


struct SConformanceCase
{
	const char*	szName;
	int32		iExpected;		//What main returns, or CONFORMANCE_REFUSED.
	const char*	szSource;
};


#include "TestCPPConformanceCases.h"
#include "TestCPPConformanceClauseCases.h"
#include "TestCppCompilerLibParts.h"
#include "TestCPPConformanceChecked.h"


//////////////////////////////////////////////////////////////////////////
//
// What the compiler can't do yet, for each stage and target: a case's name
// and (C) it didn't build, (W) it ran and returned the wrong value or
// didn't stop, (A) it was accepted and must be refused, or (U) it was
// refused, but only as C++ the compiler doesn't support yet: it doesn't know
// what is wrong with the program (ConformanceUnsupported).  Take a case off
// its list as the compiler does it (Conformance/lists.py prints the lists
// as they are now, and writes them here); the lists being empty is Part B
// done.
//
//////////////////////////////////////////////////////////////////////////
struct SConformanceGroup
{
	const char*				szName;
	const SConformanceCase*	pasCases;
	size					uiNumCases;
	const char*				aszNotDone[2];	//The 65C64's, then the W65C816's.
	bool					bExceptions;	//Its cases are compiled with exceptions (CCPPCompileOptions::mbExceptions).
};


#define CONFORMANCE_GROUP(NAME, NOT_DONE_65C64, NOT_DONE_W65C816) { #NAME, gasConformance##NAME, sizeof(gasConformance##NAME) / sizeof(SConformanceCase), { NOT_DONE_65C64, NOT_DONE_W65C816 }, false }
#define CONFORMANCE_EXCEPTIONS_GROUP(NAME, NOT_DONE_65C64, NOT_DONE_W65C816) { #NAME, gasConformance##NAME, sizeof(gasConformance##NAME) / sizeof(SConformanceCase), { NOT_DONE_65C64, NOT_DONE_W65C816 }, true }


static const SConformanceGroup gasConformanceGroups[] =
{
	CONFORMANCE_GROUP(Works, "", ""),
	CONFORMANCE_GROUP(Stage1, "", ""),
	CONFORMANCE_GROUP(Stage2, "", ""),
	CONFORMANCE_GROUP(Stage3, "", ""),
	CONFORMANCE_GROUP(Stage4, "", ""),
	CONFORMANCE_GROUP(Stage5, "", ""),
	CONFORMANCE_GROUP(Stage6, "", ""),
	CONFORMANCE_GROUP(Stage7, "", ""),
	CONFORMANCE_GROUP(Stage8, "", ""),
	CONFORMANCE_GROUP(Stage9, "", ""),
	CONFORMANCE_GROUP(Stage10, "", ""),
	CONFORMANCE_EXCEPTIONS_GROUP(Exceptions, "", ""),
	CONFORMANCE_GROUP(Clause2, "", ""),
	CONFORMANCE_GROUP(Clause3, "", ""),
	CONFORMANCE_GROUP(Clause4, "", ""),
	CONFORMANCE_GROUP(Clause5,
		"ConstantInactiveUnionMemberRefused(A) ",
		""),
	CONFORMANCE_GROUP(Clause6, "", ""),
	CONFORMANCE_GROUP(Clause7,
		"DclConstexprObjectDefaultConstructed(C) DclConstexprStaticMemberNeverInitialisedRefused(A) ",
		"DclConstexprObjectDefaultConstructed(C) DclConstexprStaticMemberNeverInitialisedRefused(A) "),
	CONFORMANCE_GROUP(Clause8, "", ""),
	CONFORMANCE_GROUP(Clause9, "", ""),
	CONFORMANCE_GROUP(Clause10, "", ""),
	CONFORMANCE_GROUP(Clause11, "", ""),
	CONFORMANCE_GROUP(Clause12, "", ""),
	CONFORMANCE_GROUP(Clause13, "", ""),
	CONFORMANCE_GROUP(Clause14, "", ""),
	CONFORMANCE_GROUP(Clause16, "", ""),
};


#define NUM_CONFORMANCE_GROUPS	(sizeof(gasConformanceGroups) / sizeof(SConformanceGroup))


//////////////////////////////////////////////////////////////////////////
//
// TEST_CPP_CONFORMANCE_DIAGNOSTICS in the environment names a file: what
// the compiler says of every case is written to it (its errors, then its
// warnings), for each target's build and for the semantic pass, in the
// order the cases are run.  Two compilers' files are compared to see that a
// change to how errors are made or kept changed nothing that is said
// (SemanticLayerPlan.txt).  Run the file alone, in one process.
//
//////////////////////////////////////////////////////////////////////////
static FILE*		gpConformanceDiagnostics = NULL;
static const char*	gszConformanceGroup = "";


static void ConformanceDiagnostics(const char* szCase, const char* szBy, const char* szErrors, const char* szWarnings)
{
	if (gpConformanceDiagnostics)
	{
		fprintf(gpConformanceDiagnostics, "== %s.%s %s\n%s", gszConformanceGroup, szCase, szBy, szErrors ? szErrors : "");
		if (szWarnings && (szWarnings[0] != '\0'))
		{
			fprintf(gpConformanceDiagnostics, "-- warnings\n%s", szWarnings);
		}
		fflush(gpConformanceDiagnostics);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the program compiles for the target, alone (nothing is linked).
//
//////////////////////////////////////////////////////////////////////////
static bool ConformanceCompiles(const char* szSource, ECPPTarget eTarget, bool bExceptions, CChars* pszErrors, CChars* pszWarnings)
{
	CCPP65Builder		cBuilder;
	CCPPCompileOptions	cOptions;
	CChars				szAssembly;
	bool				bCompiled;

	cBuilder.Init();
	TestCPP65AddIncludeDirectories(&cBuilder);
	cOptions.Init();
	cOptions.mbExceptions = bExceptions;
	szAssembly.Init();
	cBuilder.GetBuild()->SetTarget(eTarget);
	*cBuilder.GetBuild()->GetOptions() = cOptions;
	cBuilder.GetBuild()->SetWarnings(pszWarnings);
	bCompiled = cBuilder.GetBuild()->Generate("Test.cpp", szSource, false, &szAssembly, pszErrors) && pszErrors->Empty();
	szAssembly.Kill();
	cBuilder.Kill();
	return bCompiled;
}


//////////////////////////////////////////////////////////////////////////
//
// Builds a case's program for a target: the start up code, the program,
// Libc65's string routines, and its soft float (softfloat.cpp, and the
// target's assembly of it) and its operator new and delete (new.cpp, with
// malloc) only for a program that calls them, as linking them into every
// case would double the time a case takes.
//
//////////////////////////////////////////////////////////////////////////
static bool ConformanceBuildWith(CCPP65Builder* pcBuilder, const char* szSource, ECPPTarget eTarget, bool bExceptions, bool bSoftFloat, bool bLibrary, CChars* pszErrors);


static bool ConformanceBuild(CCPP65Builder* pcBuilder, const char* szSource, ECPPTarget eTarget, bool bExceptions, CChars* pszErrors)
{
	CChars	szErrors;
	bool	bBuilt;
	bool	bSoftFloat;
	bool	bLibrary;
	int		iTry;

	szErrors.Init();
	bSoftFloat = false;
	bLibrary = false;
	bBuilt = ConformanceBuildWith(pcBuilder, szSource, eTarget, bExceptions, bSoftFloat, bLibrary, &szErrors);
	for (iTry = 0; !bBuilt && (iTry < 2); iTry++)
	{
		//A missing label of soft float, or of the library's operator new and delete: built again with them.
		if (!bSoftFloat && strstr(szErrors.Text(), "_Soft"))
		{
			bSoftFloat = true;
		}
		else if (!bLibrary && !bExceptions && (strstr(szErrors.Text(), "_opNew") || strstr(szErrors.Text(), "_opDelete")))
		{
			bLibrary = true;
		}
		else
		{
			break;
		}
		pcBuilder->Kill();
		pcBuilder->Init();
		szErrors.Clear();
		bBuilt = ConformanceBuildWith(pcBuilder, szSource, eTarget, bExceptions, bSoftFloat, bLibrary, &szErrors);
	}
	pszErrors->Append(&szErrors);
	szErrors.Kill();
	return bBuilt;
}


//////////////////////////////////////////////////////////////////////////
//
// ConformanceBuild's build, with soft float or without, and Libc65's units
// that operator new and the exceptions' runtime stand on as a library
// (bLibrary, and always with exceptions) or without.
//
//////////////////////////////////////////////////////////////////////////
static bool ConformanceBuildWith(CCPP65Builder* pcBuilder, const char* szSource, ECPPTarget eTarget, bool bExceptions, bool bSoftFloat, bool bLibrary, CChars* pszErrors)
{
	static const char*	aszString65C64[] = { "Libc65/string.asm", NULL };
	static const char*	aszStringW65C816[] = { "Libc65/stringW65C816.asm", "Libc65/stringW65C816_16.asm", NULL };
	static const char*	aszSoftFloat[] = { "Libc65/softfloat.cpp", "Libc65/softfloat.asm", "Libc65/softfloatW65C816.asm", NULL };
	static const char*	aszExceptions[] = { "Libc65", "exception.cpp", "Libc65", "stdlib.cpp", "Libc65", "string.cpp", "Libc65", "ctype.cpp", "Libc65", "new.cpp", "BaseLib", "TrueMallocator.cpp", NULL };
	static const char*	aszExceptionsW65C816[] = { "BaseLib", "BankMallocator.cpp", "Libc65", "stdlibW65C816.asm", NULL };	//malloc's heap by banks.
	CFileUtil			cFileUtil;
	CChars				szLibc65;
	bool				bBuilt;
	size				i;

	pcBuilder->SetTarget(eTarget);
	pcBuilder->GetOptions()->mbExceptions = bExceptions;
	if (getenv("TEST_CPP_CONFORMANCE_LISTINGS"))
	{
		//Each unit's generated assembly, for looking at a case's code.
		pcBuilder->SetListingDirectory(getenv("TEST_CPP_CONFORMANCE_LISTINGS"));
	}
	TestCPP65AddIncludeDirectories(pcBuilder);
	bBuilt = pcBuilder->AddStartUp();
	if (eTarget == CPPT_W65C816)
	{
		TestCPP65LibrariesDirectory(&szLibc65, "Libc65");
		szLibc65.Append("Libc65");
		cFileUtil.FullPath(&szLibc65);
		pcBuilder->AddMathTables(szLibc65.Text());
		szLibc65.Kill();
	}
	bBuilt &= pcBuilder->AddSource("Test.cpp", szSource);
	TestCPP65AddUnits(pcBuilder, (eTarget == CPPT_W65C816) ? aszStringW65C816 : aszString65C64, eTarget != CPPT_W65C816);
	if (bSoftFloat)
	{
		TestCPP65AddUnits(pcBuilder, aszSoftFloat, eTarget != CPPT_W65C816);
	}
	if (bExceptions || bLibrary)
	{
		//The exceptions' runtime and what it stands on (malloc), as a library: a unit is linked only if the program
		//needs it, so a case that throws nothing is as it was.
		for (i = 0; aszExceptions[i]; i += 2)
		{
			TestCPP65LibrariesDirectory(&szLibc65, aszExceptions[i]);
			szLibc65.Append(aszExceptions[i]);
			cFileUtil.FullPath(&szLibc65);
			pcBuilder->AddUnit(aszExceptions[i], szLibc65.Text(), aszExceptions[i + 1], true);
			szLibc65.Kill();
		}
		for (i = 0; (eTarget == CPPT_W65C816) && aszExceptionsW65C816[i]; i += 2)
		{
			TestCPP65LibrariesDirectory(&szLibc65, aszExceptionsW65C816[i]);
			szLibc65.Append(aszExceptionsW65C816[i]);
			cFileUtil.FullPath(&szLibc65);
			pcBuilder->AddUnit(aszExceptionsW65C816[i], szLibc65.Text(), aszExceptionsW65C816[i + 1], true);
			szLibc65.Kill();
		}
	}
	bBuilt &= pcBuilder->Link();

	pszErrors->Append(pcBuilder->GetErrors());
	pcBuilder->GetMissingNames(pszErrors);
	return bBuilt && pszErrors->Empty();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a program that stopped at uiProgramCounter stopped in the start
// up's _Exit (its stack and its register entries, up to __ExitEnd): it
// ended itself, as a terminate handler does, with its status where main's
// result would be.
//
//////////////////////////////////////////////////////////////////////////
static bool ConformanceExited(CCPP65Builder* pcBuilder, uint32 uiProgramCounter)
{
	int64	lliStack;
	int64	lliRegister;
	int64	lliEnd;

	if (!pcBuilder->GetSymbol("__Exit", &lliStack) || !pcBuilder->GetSymbol("__Exit.a", &lliRegister) || !pcBuilder->GetSymbol("__ExitEnd", &lliEnd))
	{
		return false;
	}
	return ((int64)uiProgramCounter >= ((lliStack < lliRegister) ? lliStack : lliRegister)) && ((int64)uiProgramCounter <= lliEnd);
}


//////////////////////////////////////////////////////////////////////////
//
// Runs a built program from reset.  False if it doesn't stop, with its
// stack where it started (or in _Exit, wherever its stack is) and nothing
// read or written where there is no memory; else *piResult is what main
// returned, or what _Exit was given.
//
//////////////////////////////////////////////////////////////////////////
static bool ConformanceRun65C64(CCPP65Builder* pcBuilder, int32* piResult)
{
	STestCPP65Run	sRun;
	bool			bStopped;

	TestCPP65BeginBuilt(&sRun, pcBuilder);
	bStopped = (TestCPP65Run(&sRun, CONFORMANCE_STEPS) < CONFORMANCE_STEPS) && ((TestCPP65MPU(&sRun)->GetS() == CPP65_STACK_TOP) || (TestCPP65MPU(&sRun)->IsStopped() && ConformanceExited(pcBuilder, TestCPP65MPU(&sRun)->GetProgramCounter())));
	*piResult = (int32)TestCPP65MPU(&sRun)->GetA();
	TestCPP65End(&sRun);
	return bStopped;
}


static bool ConformanceRunW65C816(CCPP65Builder* pcBuilder, int32* piResult)
{
	CAsm65Memory	cMemory;
	CFastW65C816	cFast;
	bool			bStopped;

	cMemory.Init(ASM65_MEMORY_LOW_SIZE);
	cMemory.Load(pcBuilder->GetOutput());
	cMemory.InitFast65816(&cFast);
	cFast.Reset();
	bStopped = (cFast.Run(CONFORMANCE_STEPS) < CONFORMANCE_STEPS) && cFast.IsStopped() && (cMemory.NumBadAccesses() == 0);
	bStopped = bStopped && (((uint32)cFast.GetS() == GetCPPTarget(CPPT_W65C816)->GetStackTop()) || ConformanceExited(pcBuilder, ((uint32)cFast.GetProgramBank() << 16) | cFast.GetProgramCounter()));
	*piResult = (int16)cFast.GetC();
	cFast.Kill();
	cMemory.Kill();
	return bStopped;
}


//////////////////////////////////////////////////////////////////////////
//
// What the compiler does with a case on a target: NULL if it is right, else
// "(C)", "(W)" or "(A)" (SConformanceGroup).  TEST_CPP_CONFORMANCE_VERBOSE
// in the environment says why on stderr.
//
//////////////////////////////////////////////////////////////////////////
static bool ConformanceUnsupported(const char* szErrors);


static const char* ConformanceCase(const SConformanceCase* psCase, ECPPTarget eTarget, bool bExceptions)
{
	CCPP65Builder	cBuilder;
	CChars			szErrors;
	CChars			szWarnings;
	const char*		szNotDone;
	const char*		szTarget;
	int32			iResult;
	bool			bStopped;

	szErrors.Init();
	szNotDone = NULL;
	szTarget = (eTarget == CPPT_W65C816) ? "W65C816" : "65C64";
	if (psCase->iExpected == CONFORMANCE_REFUSED)
	{
		szWarnings.Init();
		if (ConformanceCompiles(psCase->szSource, eTarget, bExceptions, &szErrors, &szWarnings))
		{
			szNotDone = "(A)";
		}
		else if (ConformanceUnsupported(szErrors.Text()))
		{
			szNotDone = "(U)";
		}
		ConformanceDiagnostics(psCase->szName, szTarget, szErrors.Text(), szWarnings.Text());
		szWarnings.Kill();
	}
	else
	{
		cBuilder.Init();
		bStopped = ConformanceBuild(&cBuilder, psCase->szSource, eTarget, bExceptions, &szErrors);
		ConformanceDiagnostics(psCase->szName, szTarget, szErrors.Text(), cBuilder.GetWarnings());
		if (!bStopped)
		{
			szNotDone = "(C)";
		}
		else
		{
			bStopped = (eTarget == CPPT_W65C816) ? ConformanceRunW65C816(&cBuilder, &iResult) : ConformanceRun65C64(&cBuilder, &iResult);
			if (!bStopped || (iResult != psCase->iExpected))
			{
				szNotDone = "(W)";
				szErrors.Append(bStopped ? "returned " : "didn't stop, with ");
				szErrors.Append(iResult);
				szErrors.Append(" for ");
				szErrors.Append(psCase->iExpected);
			}
		}
		cBuilder.Kill();
	}

	if (szNotDone && (getenv("TEST_CPP_CONFORMANCE_VERBOSE") || (psCase->iExpected == CONFORMANCE_TRIED)))
	{
		fprintf(stderr, "%s %s%s: %s\n", szTarget, psCase->szName, szNotDone, szErrors.Text());
	}
	szErrors.Kill();
	return szNotDone;
}


//////////////////////////////////////////////////////////////////////////
//
// A group's cases on a target: those not done are the group's list.  With
// TEST_CPP_CONFORMANCE_LISTS in the environment the lists are written to
// stderr and not checked, to see where the compiler is.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPPConformanceGroup(const SConformanceGroup* psGroup, ECPPTarget eTarget)
{
	CChars			szNotDone;
	const char*		szCaseNotDone;
	size			i;
	size			uiNotDone;

	szNotDone.Init();
	uiNotDone = 0;
	for (i = 0; i < psGroup->uiNumCases; i++)
	{
		szCaseNotDone = ConformanceCase(&psGroup->pasCases[i], eTarget, psGroup->bExceptions);
		if (szCaseNotDone)
		{
			szNotDone.Append(psGroup->pasCases[i].szName);
			szNotDone.Append(szCaseNotDone);
			szNotDone.Append(' ');
			uiNotDone++;
		}
	}

	if (getenv("TEST_CPP_CONFORMANCE_LISTS"))
	{
		fprintf(stderr, "%s %s %d of %d: \"%s\"\n", psGroup->szName, (eTarget == CPPT_W65C816) ? "W65C816" : "65C64", (int)(psGroup->uiNumCases - uiNotDone), (int)psGroup->uiNumCases, szNotDone.Text());
	}
	else
	{
		AssertString(psGroup->aszNotDone[(eTarget == CPPT_W65C816) ? 1 : 0], szNotDone.Text());
	}
	szNotDone.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the first thing the compiler says of a program it refuses is that
// it doesn't support the program's C++ yet ("... by the compiler yet", "This
// function uses C++ the compiler doesn't support yet (file line n)" with no
// reason after it, or an internal error): the program is refused, but not
// for what is wrong with it, and would be as readily if it were right.
//
//////////////////////////////////////////////////////////////////////////
static bool ConformanceUnsupported(const char* szErrors)
{
	CChars		szFirst;
	const char*	szEnd;
	const char*	szSupport;
	const char*	szReason;
	bool		bUnsupported;

	szEnd = strchr(szErrors, '\n');
	szFirst.Init();
	szFirst.Append(szErrors, szEnd ? (size)(szEnd - szErrors) : strlen(szErrors));
	bUnsupported = (strstr(szFirst.Text(), "compiler yet") != NULL) || (strstr(szFirst.Text(), "Internal error") != NULL);
	szSupport = strstr(szFirst.Text(), "support yet");
	if (szSupport)
	{
		//Unsupported's exit says why when it knows: "... doesn't support yet (file line n): the reason".
		szReason = strstr(szSupport, "): ");
		bUnsupported = (szReason == NULL);
	}
	szFirst.Kill();
	return bUnsupported;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the semantic pass, which is the IDE's check of a file as it is
// written, finds nothing wrong with the program: parsed and checked as the
// compiler's target, nothing generated.
//
//////////////////////////////////////////////////////////////////////////
static bool ConformanceChecks(const char* szCase, const char* szSource, bool bExceptions, bool bPrint = false)
{
	CTranslationUnit	cFile;
	CCPPPreprocessor	cPreprocessor;
	CCPPTokeniser		cTokeniser;
	CCPPSyntaxTree		cTree;
	CCPPParser			cParser;
	CCPPSemanticPass	cPass;
	CCPPCompileOptions	cOptions;
	bool				bResult;

	cFile.Init("Test.cpp", NULL, false);
	cFile.SetContents((char*)szSource);
	cPreprocessor.Init(NULL);
	bResult = cPreprocessor.PreprocessTranslationUnit(&cFile);
	cPreprocessor.Kill();
	cTokeniser.Init();
	bResult &= cTokeniser.Tokenise(cFile.GetTokens());
	cTree.Init(&cTokeniser);
	cParser.Init(&cTokeniser, &cTree);
	cParser.Parse();
	bResult &= cParser.NumErrors() == 0;
	cParser.Kill();
	cPass.Init();
	cOptions.Init();
	cOptions.mbExceptions = bExceptions;
	cPass.SetOptions(&cOptions);
	bResult &= cPass.Check(&cTree, CPPT_W65C816);
	if (bPrint)
	{
		//TEST_CPP_CONFORMANCE_TRY: what the semantic pass (the IDE's checker) says of the program.
		fprintf(stderr, "Checked Try(%s): %s\n", bResult ? "-" : "E", (cPass.GetErrors()->NumErrors() > 0) ? cPass.GetErrors()->Get(0) : (bResult ? "no errors" : "the parser's or preprocessor's errors"));
	}
	ConformanceDiagnostics(szCase, "Checked", cPass.GetErrors()->Text(), NULL);
	cPass.Kill();
	cTree.Kill();
	cTokeniser.Kill();
	cFile.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// What the semantic pass makes of a group's cases, against what is known
// of it (TestCPPConformanceChecked.h): a case's name and (M) it missed what
// is wrong with a program that must be refused, so the IDE shows nothing
// where the build will fail, or (F) it found something wrong with a program
// that is right.  A case that includes a header is left out: the pass is
// given the case alone.  With TEST_CPP_CONFORMANCE_LISTS in the environment
// the list is written to stderr and not checked.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPPConformanceChecked(const SConformanceGroup* psGroup)
{
	const SConformanceCase*	psCase;
	CChars					szNotDone;
	const char*				szExpected;
	size					i;
	size					uiCases;
	size					uiNotDone;
	bool					bChecks;

	szNotDone.Init();
	uiCases = 0;
	uiNotDone = 0;
	for (i = 0; i < psGroup->uiNumCases; i++)
	{
		psCase = &psGroup->pasCases[i];
		if (strstr(psCase->szSource, "#include"))
		{
			continue;
		}
		uiCases++;
		bChecks = ConformanceChecks(psCase->szName, psCase->szSource, psGroup->bExceptions);
		if (bChecks != (psCase->iExpected != CONFORMANCE_REFUSED))
		{
			szNotDone.Append(psCase->szName);
			szNotDone.Append(bChecks ? "(M) " : "(F) ");
			uiNotDone++;
		}
	}

	szExpected = "";
	for (i = 0; i < sizeof(gasConformanceChecked) / sizeof(SConformanceChecked); i++)
	{
		if (strcmp(gasConformanceChecked[i].szGroup, psGroup->szName) == 0)
		{
			szExpected = gasConformanceChecked[i].szNotDone;
		}
	}
	if (getenv("TEST_CPP_CONFORMANCE_LISTS"))
	{
		fprintf(stderr, "%s Checked %d of %d: \"%s\"\n", psGroup->szName, (int)(uiCases - uiNotDone), (int)uiCases, szNotDone.Text());
	}
	else
	{
		AssertString(szExpected, szNotDone.Text());
	}
	szNotDone.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Writes the cases to the directory TEST_CPP_CONFORMANCE_EXPORT names, each
// as Group.Name.cpp, with Cases.txt listing them and their values: for
// another compiler to say what they should do (Conformance/hostcheck.py,
// with the Microsoft compiler).
//
//////////////////////////////////////////////////////////////////////////
static void TestCPPConformanceExport(const char* szDirectory)
{
	CFileUtil					cFileUtil;
	CTextFile					cFile;
	CChars						szName;
	CChars						szList;
	const SConformanceGroup*	psGroup;
	const SConformanceCase*		psCase;
	size						i;
	size						j;

	cFileUtil.MakeDir(szDirectory);
	szList.Init();
	for (i = 0; i < NUM_CONFORMANCE_GROUPS; i++)
	{
		psGroup = &gasConformanceGroups[i];
		for (j = 0; j < psGroup->uiNumCases; j++)
		{
			psCase = &psGroup->pasCases[j];
			szName.Init(psGroup->szName);
			szName.Append('.');
			szName.Append(psCase->szName);
			szList.Append(szName);
			szList.Append(' ');
			szList.Append(psCase->iExpected);
			szList.AppendNewLine();

			szName.Insert(0, FILE_SEPARATOR);
			szName.Insert(0, szDirectory);
			szName.Append(".cpp");
			cFile.Init(psCase->szSource);
			cFile.Write(szName.Text());
			cFile.Kill();
			szName.Kill();
		}
	}
	szName.Init(szDirectory);
	szName.Append(FILE_SEPARATOR);
	szName.Append("Cases.txt");
	cFile.Init(szList.Text());
	cFile.Write(szName.Text());
	cFile.Kill();
	szName.Kill();
	szList.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// TEST_CPP_CONFORMANCE_TRY in the environment names a source file: it is
// built and run on each target, what main returned or why it wasn't built
// is written to stderr, and nothing else is tested.  For trying a program
// out while the compiler is worked on; TEST_CPP_CONFORMANCE_LISTINGS names a
// directory for each unit's generated assembly, and
// TEST_CPP_CONFORMANCE_EXCEPTIONS compiles it with exceptions.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPPConformanceTry(const char* szFileName)
{
	SConformanceCase	sCase;
	CChars				szSource;
	FILE*				pFile;
	int					iChar;
	bool				bExceptions;

	szSource.Init();
	pFile = fopen(szFileName, "rb");
	for (iChar = pFile ? fgetc(pFile) : EOF; iChar != EOF; iChar = fgetc(pFile))
	{
		szSource.Append((char)iChar);
	}
	if (pFile)
	{
		fclose(pFile);
	}
	sCase.szName = "Try";
	sCase.iExpected = CONFORMANCE_TRIED;
	sCase.szSource = szSource.Text();
	bExceptions = getenv("TEST_CPP_CONFORMANCE_EXCEPTIONS") != NULL;
	ConformanceCase(&sCase, CPPT_65C64, bExceptions);
	ConformanceCase(&sCase, CPPT_W65C816, bExceptions);
	ConformanceChecks(sCase.szName, szSource.Text(), bExceptions, true);
	szSource.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// How much of C++14 the compiler does, measured: every case on both
// targets, against what is known not to be done, and the semantic pass's
// check of every case against what is known of it.
//
//////////////////////////////////////////////////////////////////////////
void TestCPPConformance(void)
{
	size	i;

	BeginTests();
	TEST_PART_BEGIN("TestCPPConformance");

	TypesInit();
	TypeConverterInit();

	if (getenv("TEST_CPP_CONFORMANCE_TRY"))
	{
		TestCPPConformanceTry(getenv("TEST_CPP_CONFORMANCE_TRY"));
		exit(0);
	}
	if (getenv("TEST_CPP_CONFORMANCE_EXPORT"))
	{
		TestCPPConformanceExport(getenv("TEST_CPP_CONFORMANCE_EXPORT"));
	}
	if (getenv("TEST_CPP_CONFORMANCE_DIAGNOSTICS"))
	{
		gpConformanceDiagnostics = fopen(getenv("TEST_CPP_CONFORMANCE_DIAGNOSTICS"), "wb");
	}
	for (i = 0; i < NUM_CONFORMANCE_GROUPS; i++)
	{
		gszConformanceGroup = gasConformanceGroups[i].szName;
		TEST_PART(TestCPPConformanceGroup(&gasConformanceGroups[i], CPPT_65C64));
		TEST_PART(TestCPPConformanceGroup(&gasConformanceGroups[i], CPPT_W65C816));
		TEST_PART(TestCPPConformanceChecked(&gasConformanceGroups[i]));
	}
	if (gpConformanceDiagnostics)
	{
		fclose(gpConformanceDiagnostics);
		gpConformanceDiagnostics = NULL;
	}

	TypeConverterKill();
	TypesKill();

	TestStatistics();
}

