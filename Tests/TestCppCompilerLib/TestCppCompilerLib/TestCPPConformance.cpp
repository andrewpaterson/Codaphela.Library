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


//////////////////////////////////////////////////////////////////////////
//
// What the compiler can't do yet, for each stage and target: a case's name
// and (C) it didn't build, (W) it ran and returned the wrong value or
// didn't stop, or (A) it was accepted and must be refused.  Take a case off
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
	CONFORMANCE_GROUP(Clause3, "", ""),
	CONFORMANCE_GROUP(Clause4, "", ""),
	CONFORMANCE_GROUP(Clause5,
		"ConstantInactiveUnionMemberRefused(A) ",
		""),
	CONFORMANCE_GROUP(Clause6,
		"StmtLabelAtBlockEndRefused(A) StmtConditionContextual(C) StmtWhileConditionRemade(C) StmtSwitchDuplicateCaseRefused(A) StmtSwitchDuplicateConvertedRefused(A) StmtSwitchClassCondition(C) StmtSwitchJumpPastInitRefused(A) StmtForConditionObject(C) StmtReturnExplicitRefused(A) StmtReturnValueInVoidRefused(A) StmtRangeForTemporary(C) ",
		"StmtLabelAtBlockEndRefused(A) StmtConditionContextual(C) StmtWhileConditionRemade(C) StmtSwitchDuplicateCaseRefused(A) StmtSwitchDuplicateConvertedRefused(A) StmtSwitchClassCondition(C) StmtSwitchJumpPastInitRefused(A) StmtForConditionObject(C) StmtReturnExplicitRefused(A) StmtReturnValueInVoidRefused(A) StmtRangeForTemporary(C) "),
	CONFORMANCE_GROUP(Clause7,
		"DclStaticAssertNotConstantRefused(A) DclTwoStorageClassesRefused(A) DclStaticMemberDefinitionRefused(A) DclMutableConstRefused(A) DclMutableStaticRefused(A) DclVirtualOutsideClassRefused(A) DclExplicitOutsideClassRefused(A) DclExplicitNotConstructorRefused(A) DclTypedefRedeclaredOtherRefused(A) DclTypedefNamesClassOtherRefused(A) DclFriendOutsideClassRefused(A) DclConstexprImplies(W) DclConstexprChangeRefused(A) DclConstexprVirtualRefused(A) DclUnsignedDoubleRefused(A) DclLongLongLongRefused(A) DclShortCharRefused(A) DclElaboratedUnionAsStructRefused(A) DclEnumOpaqueNoBaseRefused(A) DclEnumOtherBaseRefused(A) DclEnumValueNotFitRefused(A) DclNamespaceMemberUndeclaredRefused(A) DclUsingDeclarationNotBaseRefused(A) DclAsmDeclaration(C) DclLinkageCOverloadRefused(A) ",
		"DclStaticAssertNotConstantRefused(A) DclTwoStorageClassesRefused(A) DclStaticMemberDefinitionRefused(A) DclMutableConstRefused(A) DclMutableStaticRefused(A) DclVirtualOutsideClassRefused(A) DclExplicitOutsideClassRefused(A) DclExplicitNotConstructorRefused(A) DclTypedefRedeclaredOtherRefused(A) DclTypedefNamesClassOtherRefused(A) DclFriendOutsideClassRefused(A) DclConstexprImplies(W) DclConstexprChangeRefused(A) DclConstexprVirtualRefused(A) DclUnsignedDoubleRefused(A) DclLongLongLongRefused(A) DclShortCharRefused(A) DclElaboratedUnionAsStructRefused(A) DclEnumOpaqueNoBaseRefused(A) DclEnumOtherBaseRefused(A) DclEnumValueNotFitRefused(A) DclNamespaceMemberUndeclaredRefused(A) DclUsingDeclarationNotBaseRefused(A) DclAsmDeclaration(C) DclLinkageCOverloadRefused(A) "),
	CONFORMANCE_GROUP(Clause8, "", ""),
	CONFORMANCE_GROUP(Clause9,
		"ClassMemberOfOwnTypeRefused(A) ClassMemberIncompleteRefused(A) ClassMemberDeclaredTwiceRefused(A) ClassMemberFunctionDeclaredTwiceRefused(A) ClassMemberNamedAsClassRefused(A) ClassMemberFunctionUndeclaredRefused(A) ClassConstFunctionChangesMemberRefused(A) ClassRefQualifierMixedRefused(A) ClassStaticFunctionConstRefused(A) ClassStaticInClassInitialiserRefused(A) ClassLocalStaticMemberRefused(A) ClassUnionClassMember(W) ClassUnionClassMemberDeletedRefused(A) ClassUnionBaseRefused(A) ClassUnionAsBaseRefused(A) ClassUnionVirtualRefused(A) ClassUnionReferenceMemberRefused(A) ClassAnonymousUnion(C) ClassAnonymousUnionNotStaticRefused(A) ClassAnonymousUnionFunctionRefused(A) ClassBitFieldBoolAndEnum(W) ClassBitFieldStaticRefused(A) ClassBitFieldNegativeRefused(A) ClassBitFieldZeroNamedRefused(A) ClassNestedClass(C) ClassNestedNoOuterThisRefused(A) ClassLocalMemberTemplateRefused(A) ",
		"ClassMemberOfOwnTypeRefused(A) ClassMemberIncompleteRefused(A) ClassMemberDeclaredTwiceRefused(A) ClassMemberFunctionDeclaredTwiceRefused(A) ClassMemberNamedAsClassRefused(A) ClassMemberFunctionUndeclaredRefused(A) ClassConstFunctionChangesMemberRefused(A) ClassRefQualifierMixedRefused(A) ClassStaticFunctionConstRefused(A) ClassStaticInClassInitialiserRefused(A) ClassLocalStaticMemberRefused(A) ClassUnionClassMember(W) ClassUnionClassMemberDeletedRefused(A) ClassUnionBaseRefused(A) ClassUnionAsBaseRefused(A) ClassUnionVirtualRefused(A) ClassUnionReferenceMemberRefused(A) ClassAnonymousUnion(C) ClassAnonymousUnionNotStaticRefused(A) ClassAnonymousUnionFunctionRefused(A) ClassBitFieldBoolAndEnum(W) ClassBitFieldStaticRefused(A) ClassBitFieldNegativeRefused(A) ClassBitFieldZeroNamedRefused(A) ClassNestedClass(C) ClassNestedNoOuterThisRefused(A) ClassLocalMemberTemplateRefused(A) "),
	CONFORMANCE_GROUP(Clause10,
		"DerivedDirectBaseTwiceRefused(A) DerivedFinalOverriderAmbiguousRefused(A) DerivedCovariantAdjusted(W) DerivedReturnNotCovariantRefused(A) DerivedCovariantUnrelatedRefused(A) DerivedAbstractParameterRefused(A) DerivedAbstractResultRefused(A) ",
		"DerivedDirectBaseTwiceRefused(A) DerivedFinalOverriderAmbiguousRefused(A) DerivedCovariantAdjusted(W) DerivedReturnNotCovariantRefused(A) DerivedCovariantUnrelatedRefused(A) DerivedAbstractParameterRefused(A) DerivedAbstractResultRefused(A) "),
	CONFORMANCE_GROUP(Clause11,
		"AccessPrivateStaticDefinition(C) AccessInjectedNameThroughPrivateBaseRefused(A) AccessGlobalNameOfPrivateBase(C) AccessFriendNotTransitiveRefused(A) AccessFriendNotInheritedRefused(A) AccessProtectedThroughBaseRefused(A) AccessProtectedThroughSiblingRefused(A) AccessProtectedMemberPointer(C) AccessProtectedMemberPointerBaseRefused(A) AccessEnclosingToNestedPrivateRefused(A) AccessUsingDeclarationOfPrivateRefused(A) AccessPrivateDestructorRefused(A) ",
		"AccessPrivateStaticDefinition(C) AccessInjectedNameThroughPrivateBaseRefused(A) AccessGlobalNameOfPrivateBase(C) AccessFriendNotTransitiveRefused(A) AccessFriendNotInheritedRefused(A) AccessProtectedThroughBaseRefused(A) AccessProtectedThroughSiblingRefused(A) AccessProtectedMemberPointer(C) AccessProtectedMemberPointerBaseRefused(A) AccessEnclosingToNestedPrivateRefused(A) AccessUsingDeclarationOfPrivateRefused(A) AccessPrivateDestructorRefused(A) "),
	CONFORMANCE_GROUP(Clause14, "", ""),
};


#define NUM_CONFORMANCE_GROUPS	(sizeof(gasConformanceGroups) / sizeof(SConformanceGroup))


//////////////////////////////////////////////////////////////////////////
//
// Whether the program compiles for the target, alone (nothing is linked).
//
//////////////////////////////////////////////////////////////////////////
static bool ConformanceCompiles(const char* szSource, ECPPTarget eTarget, bool bExceptions, CChars* pszErrors)
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
	bCompiled = CCPP65Builder::Generate(&szAssembly, pszErrors, cBuilder.GetIncludeDirectories(), "Test.cpp", szSource, false, NULL, eTarget, NULL, NULL, false, false, false, true, &cOptions) && pszErrors->Empty();
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
static const char* ConformanceCase(const SConformanceCase* psCase, ECPPTarget eTarget, bool bExceptions)
{
	CCPP65Builder	cBuilder;
	CChars			szErrors;
	const char*		szNotDone;
	int32			iResult;
	bool			bStopped;

	szErrors.Init();
	szNotDone = NULL;
	if (psCase->iExpected == CONFORMANCE_REFUSED)
	{
		if (ConformanceCompiles(psCase->szSource, eTarget, bExceptions, &szErrors))
		{
			szNotDone = "(A)";
		}
		szErrors.Clear();
	}
	else
	{
		cBuilder.Init();
		if (!ConformanceBuild(&cBuilder, psCase->szSource, eTarget, bExceptions, &szErrors))
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
		fprintf(stderr, "%s %s%s: %s\n", (eTarget == CPPT_W65C816) ? "W65C816" : "65C64", psCase->szName, szNotDone, szErrors.Text());
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
	szSource.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// How much of C++14 the compiler does, measured: every case on both
// targets, against what is known not to be done.
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
	for (i = 0; i < NUM_CONFORMANCE_GROUPS; i++)
	{
		TEST_PART(TestCPPConformanceGroup(&gasConformanceGroups[i], CPPT_65C64));
		TEST_PART(TestCPPConformanceGroup(&gasConformanceGroups[i], CPPT_W65C816));
	}

	TypeConverterKill();
	TypesKill();

	TestStatistics();
}

