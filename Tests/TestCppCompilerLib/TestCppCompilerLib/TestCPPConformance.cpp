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
};


#define CONFORMANCE_GROUP(NAME, NOT_DONE_65C64, NOT_DONE_W65C816) { #NAME, gasConformance##NAME, sizeof(gasConformance##NAME) / sizeof(SConformanceCase), { NOT_DONE_65C64, NOT_DONE_W65C816 } }


static const SConformanceGroup gasConformanceGroups[] =
{
	CONFORMANCE_GROUP(Works, "", ""),
	CONFORMANCE_GROUP(Stage1, "", ""),
	CONFORMANCE_GROUP(Stage2, "", ""),
	CONFORMANCE_GROUP(Stage3, "", ""),
	CONFORMANCE_GROUP(Stage4, "", ""),
	CONFORMANCE_GROUP(Stage5, "", ""),
	CONFORMANCE_GROUP(Stage6, "", ""),
	CONFORMANCE_GROUP(Stage7,
		"FunctionTryBlock(C) ",
		"FunctionTryBlock(C) "),
	CONFORMANCE_GROUP(Stage8, "", ""),
	CONFORMANCE_GROUP(Stage9, "", ""),
	CONFORMANCE_GROUP(Stage10, "", ""),
	CONFORMANCE_GROUP(Clause4,
		"ConvQualificationMemberDropRefused(A) ConvMemberPointer(C) ",
		"ConvQualificationMemberDropRefused(A) ConvMemberPointer(C) "),
	CONFORMANCE_GROUP(Clause5,
		"PrimThisType(C) PrimQualifiedNames(C) PrimMemberUnevaluated(C) LambdaClosureType(C) LambdaCaptureEqualThenCopyRefused(A) LambdaCaptureAndThenReferenceRefused(A) LambdaInUnevaluatedRefused(A) LambdaDefaultArgumentCaptureRefused(A) LambdaCaptureTypes(C) LambdaGenericForms(C) LambdaNotDefaultConstructibleRefused(A) LambdaNotAssignableRefused(A) CallIncompleteReturnInDecltype(W) FunctionalCastNarrowingRefused(A) FunctionalCastNarrowingCharRefused(A) FunctionalCastTwoWordTypeRefused(A) DynamicCastAccessAndAmbiguity(W) TypeidIncompleteRefused(A) StaticCastClasses(C) UnaryAddressOfRvalueRefused(A) UnaryAddressOfParenthesisedMemberRefused(A) NewArrays(C) NewNegativeBoundRefused(A) DeleteClassSized(W) DeleteFunctionPointerRefused(A) DeletePrivateDestructorRefused(A) NoexceptOperatorRules(C) CastNotationNoConversionRefused(A) MemberPointerOperators(W) MemberPointerConstWriteRefused(A) MemberPointerMutableConstRefused(A) MemberPointerConstObjectRefused(A) MemberPointerUnrelatedClassRefused(A) MemberPointerBaseObjectDerivedMemberRefused(A) MultiplicativeConstantDivideByZeroRefused(A) ConditionalTypes(W) ConditionalLvalues(C) ConditionalClassConversions(C) ConditionalMismatchRefused(A) ConditionalVoidAndValueRefused(A) AssignConstMemberClassRefused(A) ConstantConstVariables(C) ConstantRunTimeConstRefused(A) ConstantNonConstVariableRefused(A) ConstantConstDoubleRefused(A) ConstantVolatileRefused(A) ConstantConvertedNarrowingRefused(A) ConstantConvertedNegativeRefused(A) ConstantEnumeratorNarrowingRefused(A) ConstantCaseNarrowingRefused(A) ConstantClassConversion(C) ConstantUnevaluatedBranchTakenRefused(A) ConstantLambdaRefused(A) ConstantReinterpretCastRefused(A) ConstantModifiesGlobalRefused(A) ConstantAssignsGlobalRefused(A) ConstantNewRefused(A) ConstantNullDereferenceRefused(A) ConstantShiftTooFarRefused(A) ConstantNegativeShiftRefused(A) ConstantInactiveUnionMemberRefused(A) ConstantUnrelatedPointerCompareRefused(A) ConstantThisOutsideRefused(A) ConstantAddresses(C) ConstantAddressOfLocalRefused(A) ConstantReferenceToLocalRefused(A) ConstantObjects(C) ConstantEvaluationChangesLocals(C) ",
		"PrimThisType(C) PrimQualifiedNames(C) PrimMemberUnevaluated(C) LambdaClosureType(C) LambdaCaptureEqualThenCopyRefused(A) LambdaCaptureAndThenReferenceRefused(A) LambdaInUnevaluatedRefused(A) LambdaDefaultArgumentCaptureRefused(A) LambdaCaptureTypes(C) LambdaGenericForms(C) LambdaNotDefaultConstructibleRefused(A) LambdaNotAssignableRefused(A) CallIncompleteReturnInDecltype(W) FunctionalCastNarrowingRefused(A) FunctionalCastNarrowingCharRefused(A) FunctionalCastTwoWordTypeRefused(A) DynamicCastAccessAndAmbiguity(W) TypeidIncompleteRefused(A) StaticCastClasses(C) UnaryAddressOfRvalueRefused(A) UnaryAddressOfParenthesisedMemberRefused(A) NewArrays(C) NewNegativeBoundRefused(A) DeleteClassSized(W) DeleteFunctionPointerRefused(A) DeletePrivateDestructorRefused(A) NoexceptOperatorRules(C) CastNotationNoConversionRefused(A) MemberPointerOperators(W) MemberPointerConstWriteRefused(A) MemberPointerMutableConstRefused(A) MemberPointerConstObjectRefused(A) MemberPointerUnrelatedClassRefused(A) MemberPointerBaseObjectDerivedMemberRefused(A) MultiplicativeConstantDivideByZeroRefused(A) ConditionalTypes(W) ConditionalLvalues(C) ConditionalClassConversions(C) ConditionalMismatchRefused(A) ConditionalVoidAndValueRefused(A) AssignConstMemberClassRefused(A) ConstantConstVariables(C) ConstantRunTimeConstRefused(A) ConstantNonConstVariableRefused(A) ConstantConstDoubleRefused(A) ConstantVolatileRefused(A) ConstantConvertedNarrowingRefused(A) ConstantConvertedNegativeRefused(A) ConstantEnumeratorNarrowingRefused(A) ConstantCaseNarrowingRefused(A) ConstantClassConversion(C) ConstantUnevaluatedBranchTakenRefused(A) ConstantLambdaRefused(A) ConstantReinterpretCastRefused(A) ConstantModifiesGlobalRefused(A) ConstantAssignsGlobalRefused(A) ConstantNewRefused(A) ConstantNullDereferenceRefused(A) ConstantShiftTooFarRefused(A) ConstantNegativeShiftRefused(A) ConstantInactiveUnionMemberRefused(A) ConstantUnrelatedPointerCompareRefused(A) ConstantThisOutsideRefused(A) ConstantAddresses(C) ConstantAddressOfLocalRefused(A) ConstantReferenceToLocalRefused(A) ConstantObjects(C) ConstantEvaluationChangesLocals(C) "),
	CONFORMANCE_GROUP(Clause8,
		"DclAmbiguityFunctionOrObject(C) DclAmbiguityStatement(C) DclPointerToReferenceRefused(A) DclReferenceNoInitialiserRefused(A) DclReferenceToReferenceRefused(A) DclMemberPointers(W) DclMemberPointerToConstFunctionRefused(A) DclArrayOfVoidRefused(A) DclFunctionTypes(C) DclFunctionMemberTypes(C) DclFunctionReturnsArrayRefused(A) DclFunctionReturnsFunctionRefused(A) DclVoidParameterNamedRefused(A) DclFunctionRedeclaredDifferentReturnRefused(A) DclConstFreeFunctionRefused(A) DclDefaultArguments(C) DclDefaultArgumentGapRefused(A) DclDefaultArgumentTwiceRefused(A) DclDefaultArgumentLocalRefused(A) DclDefaultedAndDeleted(C) DclDeletedTemplateInstanceRefused(A) DclDeletedAfterDeclaredRefused(A) DclDefaultedNotSpecialRefused(A) DclDefaultedWithDefaultArgumentRefused(A) DclConstNeedsInitialiserRefused(A) DclConstClassNeedsConstructorRefused(A) DclCopyInitialisationExplicitRefused(A) DclCopyInitialisationTwoConversionsRefused(A) DclAggregates(C) DclAggregatePrivateMemberRefused(A) DclCharacterArrayTooLongRefused(A) DclListNarrowingInAggregateRefused(A) DclListExplicitCopyRefused(A) ",
		"DclAmbiguityFunctionOrObject(C) DclAmbiguityStatement(C) DclPointerToReferenceRefused(A) DclReferenceNoInitialiserRefused(A) DclReferenceToReferenceRefused(A) DclMemberPointers(W) DclMemberPointerToConstFunctionRefused(A) DclArrayOfVoidRefused(A) DclFunctionTypes(C) DclFunctionMemberTypes(C) DclFunctionReturnsArrayRefused(A) DclFunctionReturnsFunctionRefused(A) DclVoidParameterNamedRefused(A) DclFunctionRedeclaredDifferentReturnRefused(A) DclConstFreeFunctionRefused(A) DclDefaultArguments(C) DclDefaultArgumentGapRefused(A) DclDefaultArgumentTwiceRefused(A) DclDefaultArgumentLocalRefused(A) DclDefaultedAndDeleted(C) DclDeletedTemplateInstanceRefused(A) DclDeletedAfterDeclaredRefused(A) DclDefaultedNotSpecialRefused(A) DclDefaultedWithDefaultArgumentRefused(A) DclConstNeedsInitialiserRefused(A) DclConstClassNeedsConstructorRefused(A) DclCopyInitialisationExplicitRefused(A) DclCopyInitialisationTwoConversionsRefused(A) DclAggregates(C) DclAggregatePrivateMemberRefused(A) DclCharacterArrayTooLongRefused(A) DclListNarrowingInAggregateRefused(A) DclListExplicitCopyRefused(A) "),
};


#define NUM_CONFORMANCE_GROUPS	(sizeof(gasConformanceGroups) / sizeof(SConformanceGroup))


//////////////////////////////////////////////////////////////////////////
//
// Whether the program compiles for the target, alone (nothing is linked).
//
//////////////////////////////////////////////////////////////////////////
static bool ConformanceCompiles(const char* szSource, ECPPTarget eTarget, CChars* pszErrors)
{
	CCPP65Builder	cBuilder;
	CChars			szAssembly;
	bool			bCompiled;

	cBuilder.Init();
	szAssembly.Init();
	bCompiled = CCPP65Builder::Generate(&szAssembly, pszErrors, cBuilder.GetIncludeDirectories(), "Test.cpp", szSource, false, NULL, eTarget) && pszErrors->Empty();
	szAssembly.Kill();
	cBuilder.Kill();
	return bCompiled;
}


//////////////////////////////////////////////////////////////////////////
//
// Builds a case's program for a target: the start up code, the program,
// Libc65's string routines, and its soft float (softfloat.cpp, and the
// target's assembly of it) only for a program that calls it, as linking
// it into every case would double the time a case takes.
//
//////////////////////////////////////////////////////////////////////////
static bool ConformanceBuildWith(CCPP65Builder* pcBuilder, const char* szSource, ECPPTarget eTarget, bool bSoftFloat, CChars* pszErrors);


static bool ConformanceBuild(CCPP65Builder* pcBuilder, const char* szSource, ECPPTarget eTarget, CChars* pszErrors)
{
	CChars	szErrors;
	bool	bBuilt;

	szErrors.Init();
	bBuilt = ConformanceBuildWith(pcBuilder, szSource, eTarget, false, &szErrors);
	if (!bBuilt && strstr(szErrors.Text(), "_Soft"))
	{
		pcBuilder->Kill();
		pcBuilder->Init();
		szErrors.Clear();
		bBuilt = ConformanceBuildWith(pcBuilder, szSource, eTarget, true, &szErrors);
	}
	pszErrors->Append(&szErrors);
	szErrors.Kill();
	return bBuilt;
}


//////////////////////////////////////////////////////////////////////////
//
// ConformanceBuild's build, with soft float or without.
//
//////////////////////////////////////////////////////////////////////////
static bool ConformanceBuildWith(CCPP65Builder* pcBuilder, const char* szSource, ECPPTarget eTarget, bool bSoftFloat, CChars* pszErrors)
{
	static const char*	aszString65C64[] = { "Libc65/string.asm", NULL };
	static const char*	aszStringW65C816[] = { "Libc65/stringW65C816.asm", "Libc65/stringW65C816_16.asm", NULL };
	static const char*	aszSoftFloat[] = { "Libc65/softfloat.cpp", "Libc65/softfloat.asm", "Libc65/softfloatW65C816.asm", NULL };
	CFileUtil			cFileUtil;
	CChars				szLibc65;
	bool				bBuilt;

	pcBuilder->SetTarget(eTarget);
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
	bBuilt &= pcBuilder->Link();

	pszErrors->Append(pcBuilder->GetErrors());
	pcBuilder->GetMissingNames(pszErrors);
	return bBuilt && pszErrors->Empty();
}


//////////////////////////////////////////////////////////////////////////
//
// Runs a built program from reset.  False if it doesn't stop, with its
// stack where it started and nothing read or written where there is no
// memory; else *piResult is what main returned.
//
//////////////////////////////////////////////////////////////////////////
static bool ConformanceRun65C64(CCPP65Builder* pcBuilder, int32* piResult)
{
	STestCPP65Run	sRun;
	bool			bStopped;

	TestCPP65BeginBuilt(&sRun, pcBuilder);
	bStopped = (TestCPP65Run(&sRun, CONFORMANCE_STEPS) < CONFORMANCE_STEPS) && (TestCPP65MPU(&sRun)->GetS() == CPP65_STACK_TOP);
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
	bStopped = (cFast.Run(CONFORMANCE_STEPS) < CONFORMANCE_STEPS) && cFast.IsStopped() && (cMemory.NumBadAccesses() == 0) && ((uint32)cFast.GetS() == GetCPPTarget(CPPT_W65C816)->GetStackTop());
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
static const char* ConformanceCase(const SConformanceCase* psCase, ECPPTarget eTarget)
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
		if (ConformanceCompiles(psCase->szSource, eTarget, &szErrors))
		{
			szNotDone = "(A)";
		}
		szErrors.Clear();
	}
	else
	{
		cBuilder.Init();
		if (!ConformanceBuild(&cBuilder, psCase->szSource, eTarget, &szErrors))
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
		szCaseNotDone = ConformanceCase(&psGroup->pasCases[i], eTarget);
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
// directory for each unit's generated assembly.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPPConformanceTry(const char* szFileName)
{
	SConformanceCase	sCase;
	CChars				szSource;
	FILE*				pFile;
	int					iChar;

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
	ConformanceCase(&sCase, CPPT_65C64);
	ConformanceCase(&sCase, CPPT_W65C816);
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

