/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdlib.h>
#include <string.h>
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "SourceFile.h"
#include "CTIdentifier.h"
#include "CTReservedWord.h"
#include "CTString.h"
#include "CppTokeniser.h"
#include "CPPSyntaxTree.h"
#include "CPP65IRFrontEnd.h"
#include "CPPConstants.h"
#include "CPPUnit.h"
#include "CPPSemanticPass.h"


//////////////////////////////////////////////////////////////////////////
//
// Small functions of other units the IR front end's inliner may use; this
// unit's are added to pcImports when it is finished (CCPP65IRFrontEnd::
// FinishUnit), in its turn uiTurn (CIRImports::BeginTurn).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::SetImports(CIRImports* pcImports, size uiTurn)
{
	mpcImports = pcImports;
	muiImportsTurn = uiTurn;
}


//////////////////////////////////////////////////////////////////////////
//
// After Generate: whether the IR front end missed a function another unit
// exported while it was inlining, before the unit's turn (CIRImports::
// Missed).  The unit must be generated again, with a new generator: it
// still has its turn, so then it finds everything it would have.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::MissedImports(void)
{
	return mbMissedImports;
}


//////////////////////////////////////////////////////////////////////////
//
// After GenerateBeforeTurn: the functions the IR front end looked for in
// the imports and didn't find, NULL if it has none.
//
//////////////////////////////////////////////////////////////////////////
CArrayChars* CCPPUnit::GetNotImported(void) const
{
	return mpcIRFrontEnd->GetNotImported();
}


//////////////////////////////////////////////////////////////////////////
//
// A profile of the program's functions: the W65C816 back end gives those it
// measured worth it page aligned frames (CIRW65C816Backend::AlignFrame).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::SetFrameProfile(CAsm65Profile* pcProfile)
{
	mpcFrameProfile = pcProfile;
}


//////////////////////////////////////////////////////////////////////////
//
// The code the IR front end writes gets a comment with the C++ source line
// before the code for it ("; File.cpp(12): text"), for a listing.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::SetListing(bool bListing)
{
	mbListing = bListing;
}


//////////////////////////////////////////////////////////////////////////
//
// Each class the unit lays out is written down as a line (AddLayout), for
// GetLayouts: what a target's layout rules made of the program's classes,
// to read or to compare with another build's.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::SetLayouts(bool bLayouts)
{
	mcClasses.SetLayouts(bLayouts);
}


//////////////////////////////////////////////////////////////////////////
//
// The arena the W65C816 back end's working arrays and strings are in while
// it writes a function (CCPP65IRFrontEnd::GenerateCode, CIRW65C816Backend::
// SetScratch).
//
//////////////////////////////////////////////////////////////////////////
CScratchPadAllocator* CCPPUnit::GetBackendScratch(void)
{
	return &mcBackendScratch;
}


//////////////////////////////////////////////////////////////////////////
//
// Where each function's code waits (in pieces) until the unit is assembled,
// NULL for the unit's own memory, and the unit's number in its keys
// (units sharing a store have different numbers).  Set it before
// generating; the unit's assembly is initialised with the same.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::SetSpillStore(CSpillStore* pcStore, uint32 uiUnit)
{
	mpcSpillStore = pcStore ? pcStore : &mcMemorySpillStore;
	muiSpillUnit = uiUnit;
}


//////////////////////////////////////////////////////////////////////////
//
// What compiling the unit is counted and timed in (the unit's own
// when pcStatistics is NULL).  The IR front end's functions are counted in
// it too.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::SetStatistics(CCPPCompileStatistics* pcStatistics)
{
	mpcStatistics = pcStatistics ? pcStatistics : &mcOwnStatistics;
	mpcIRFrontEnd->SetStatistics(mpcStatistics);
	mcDiagnostics.SetStatistics(mpcStatistics);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPCompileStatistics* CCPPUnit::GetStatistics(void) const
{
	return mpcStatistics;
}


//////////////////////////////////////////////////////////////////////////
//
// How the unit is compiled where there is a choice: a builder's options, or
// the defaults for NULL.  Before anything is generated.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::SetOptions(CCPPCompileOptions* pcOptions)
{
	mpcOptions = pcOptions ? pcOptions : GetDefaultCPPCompileOptions();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPCompileOptions* CCPPUnit::GetOptions(void) const
{
	return mpcOptions;
}


//////////////////////////////////////////////////////////////////////////
//
// The code the IR front end writes gets .line directives where its C++
// source line changes and a .fingerprint for each function (a hash of its
// IR and lliCompileTime), for the linker's line table: a profile by source
// line that knows which code it is for.  The code is the same as without.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::SetLineTable(bool bLineTable, int64 lliCompileTime)
{
	mbLineTable = bLineTable;
	mlliCompileTime = lliCompileTime;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the build's sources are C (CCPPCompileOptions::mbCConversions):
// what C allows and C++ doesn't, of the little that is told apart, is
// allowed in them.  A void* is a pointer to any object without a cast (C11
// 6.3.2.3 1), as malloc's result is used, and a number is an enum's value.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsCSource(void) const
{
	return mpcOptions->mbCConversions;
}


//////////////////////////////////////////////////////////////////////////
//
// Floating point in any code the unit writes (a float or double value,
// variable, literal or conversion) is an error at its statement.  Only
// what is written: a declaration that is never used (a header's float
// function) is not.  See CCPP65Builder::SetNoFloats.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::SetNoFloats(bool bNoFloats)
{
	mbNoFloats = bNoFloats;
}


//////////////////////////////////////////////////////////////////////////
//
// The unit's text was compiled before, as it is now, and nothing was wrong
// with it: a build's second compile of a unit (CCPP65Builder::
// CompileUnitAgain), whose warnings go nowhere.  What a compile does only
// to report is left out: the classes laid out where they are defined
// (CheckVirtSpecifiers) and the named instances (ReportNamedInstances).
// The code written is the same.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::SetCompiledBefore(bool bCompiledBefore)
{
	mbCompiledBefore = bCompiledBefore;
}


//////////////////////////////////////////////////////////////////////////
//
// The functions the unit calls that were neither its own nor in the
// imports when it was finished (a later unit may export them).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::GetMissingCallees(CArrayChars* paszNames)
{
	CArrayChars*	paszMissing;
	size			i;

	paszMissing = mpcIRFrontEnd->GetMissingCallees();
	for (i = 0; i < paszMissing->NumElements(); i++)
	{
		paszNames->AddIfUnique(paszMissing->GetText(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the labels of the functions the unit calls by name and tests for an
// exception (CCPP65IRFrontEnd::GetTestedCallees).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::GetTestedCallees(CArrayChars* paszNames)
{
	CArrayChars*	paszTested;
	size			i;

	paszTested = mpcIRFrontEnd->GetTestedCallees();
	for (i = 0; i < paszTested->NumElements(); i++)
	{
		paszNames->AddIfUnique(paszTested->GetText(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the lines of the classes the unit laid out (SetLayouts) to
// paszLines, which is sorted and has each line once.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::GetLayouts(CArrayChars* paszLines)
{
	mcClasses.GetLayouts(paszLines);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::Init(CCPPSyntaxTree* pcTree, ECPPTarget eTarget)
{
	mpcTree = pcTree;
	mpcTokeniser = pcTree->GetTokeniser();
	mpcTarget = GetCPPTarget(eTarget);
	mcTypes.Init(mpcTarget->GetIntBytes(), mpcTarget->GetPointerBytes());

	mszCode.Init();
	mszThunks.Init();
	maszRuntime.Init();
	mcBackendScratch.Init((size)CPP65_BACKEND_SCRATCH_CHUNK);
	mpcImports = NULL;
	muiImportsTurn = 0;
	mbMissedImports = false;
	mpcFrameProfile = NULL;
	mbListing = false;
	mbLineTable = false;
	mbNoFloats = false;
	mbCompiledBefore = false;
	mlliCompileTime = 0;
	mcMemorySpillStore.Init();
	mpcSpillStore = &mcMemorySpillStore;
	muiSpillUnit = 0;
	mauiCodePieces.Init();
	mpcSemanticPass = NULL;
	mpcObserver = &mcNoObserver;
	mbRemember = false;
	mcOwnStatistics.Init();
	mpcStatistics = &mcOwnStatistics;
	mpcOptions = GetDefaultCPPCompileOptions();

	//Every function body is written as IR (and generated by the target's IR back end).
	mpcIRFrontEnd = NewMalloc<CCPP65IRFrontEnd>();
	mpcIRFrontEnd->Init(this);
	mpcConstants = NewMalloc<CCPPConstants>();
	mpcConstants->Init(this);
	mcFolder.Init(this);
	mcTypeChecker.Init(this);
	mcDiagnostics.Init(mpcTokeniser, &mcContext, mpcStatistics);

	mapcGlobals.Init();
	mapcFunctions.Init();
	mcFunctionsOfNames.Init();
	mcContext.Init();
	mcData.Init();
	mcDebugWriter.Init(this);
	mcBinder.Init(this);
	mcDeclarators.Init(this);
	mcFunctionDeclarer.Init(this);
	mcGlobals.Init(this);
	mcClasses.Init(this);
	mcMangler.Init(mpcTokeniser, mcClasses.GetClassList(), &mcFunctionsOfNames);
	mapcTemplates.Init();
	mapcAllBindings.Init();
	mbValueArgumentRefused = false;
	mapcHiddenSymbols.Init();
	mcExplicitInstances.Init();
	mcEnums.Init(this);
	mcAttributes.Init(this);
	mpcInitializerList = NULL;
	mpcTypeInfo = NULL;
	mcClosures.Init(this);
	mcClassData.Init(this);
	mcStrings.Init(this);
	mcCheck.Init(this);
	mcWarnings.Init(this);
	mcBuiltins.Init(this);
	mcExceptions.Init(this);
	mcDesugarings.Init(this);
	mcInstantiator.Init(this);
	mapcVariableInstances.Init();
	mapcQueue.Init();
	muiQueueNext = 0;
	maszEmitted.Init();
	mbPureVirtual = false;
	mbBadCast = false;
	mbRecursiveInit = false;

	mapsFunctionScopes.Init();
	PushFunctionScope();
	miLabels = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::Kill(void)
{
	size					i;
	CCPPFunction*			pcFunction;
	CCPPTemplateArguments*	pcBindings;

	mcTypeChecker.Kill();
	SafeKill(mpcConstants);
	SafeKill(mpcIRFrontEnd);
	while (mapsFunctionScopes.NumElements() > 0)
	{
		PopFunctionScope();
	}
	mapsFunctionScopes.Kill();
	mcGlobals.Kill();
	mcBackendScratch.Kill();

	mcClasses.Kill();
	for (i = 0; i < mapcFunctions.NumElements(); i++)
	{
		pcFunction = mapcFunctions.GetPtr(i);
		pcFunction->Kill();
		SafeFree(pcFunction);
	}
	mcMangler.Kill();
	mcFunctionDeclarer.Kill();
	mcDeclarators.Kill();
	mcBinder.Kill();
	mcDebugWriter.Kill();
	mcData.Kill();
	mcContext.Kill();
	mcFunctionsOfNames.Kill();
	mapcFunctions.Kill();
	for (i = 0; i < mapcTemplates.NumElements(); i++)
	{
		mapcTemplates.GetPtr(i)->Kill();
		free(mapcTemplates.GetPtr(i));
	}
	mapcTemplates.Kill();
	for (i = 0; i < mapcAllBindings.NumElements(); i++)
	{
		pcBindings = mapcAllBindings.GetPtr(i);
		pcBindings->Kill();
		SafeFree(pcBindings);
	}
	mapcAllBindings.Kill();
	for (i = 0; i < mapcHiddenSymbols.NumElements(); i++)
	{
		free(mapcHiddenSymbols.GetPtr(i));
	}
	mapcHiddenSymbols.Kill();
	mcExplicitInstances.Kill();
	mcEnums.Kill();
	mcAttributes.Kill();
	mcClosures.Kill();
	mcDesugarings.Kill();
	mapcVariableInstances.Kill();
	mapcQueue.Kill();
	maszEmitted.Kill();
	FreeVariables(&mapcGlobals);
	mapcGlobals.Kill();

	mcDiagnostics.Kill();
	maszRuntime.Kill();
	mszThunks.Kill();
	mszCode.Kill();
	mauiCodePieces.Kill();
	mcMemorySpillStore.Kill();
	mcTypes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Appends the assembly for the translation unit to pszDest, to be assembled as an
// object (CAsm65C64Assembler::AssembleObject): the functions, the routines that make and
// destroy the globals (in the init and fini tables), the runtime routines they use,
// then the data.  Each function, global, static local, temporary and string literal
// is a section of its own so the linker can leave out the ones nothing uses.  Returns false if anything could not be generated; GetErrors
// says what.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::Generate(CChars* pszDest)
{
	CCPP65Assembly	cAssembly;
	bool			bResult;

	cAssembly.Init(mpcSpillStore, muiSpillUnit);
	bResult = Generate(&cAssembly);
	cAssembly.Text(pszDest);
	cAssembly.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// As Generate(CChars*), leaving each function's code in the spill store: pcDest
// (initialised with SetSpillStore's store and unit) has the rest, with markers where the
// functions' code goes, and is read a line at a time by the assembler.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::Generate(CCPP65Assembly* pcDest)
{
	GenerateBeforeTurn();
	return GenerateAfterTurn(pcDest);
}


//////////////////////////////////////////////////////////////////////////
//
// When the unit's declarations and functions are done the compiler is nowhere: everything entered was left,
// in order.  If not, the compiler has a mistake in it, and what it wrote can't be trusted.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::CheckContext(void)
{
	if (mcContext.IsBroken())
	{
		mcDiagnostics.ErrorAt(-1, "Internal error: the compiler's context was not left as it was entered");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The first part of Generate, which can be done before the unit's turn with
// the imports (SetImports): the functions are generated and inlined, with
// the imports exported so far.  GenerateAfterTurn does the rest, in the
// unit's turn.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::GenerateBeforeTurn(void)
{
	CCPPSyntaxTranslationUnit*	pcRoot;

	mpcStatistics->Begin(CPPCP_Declarations);
	pcRoot = mpcTree->GetRoot();
	if (pcRoot)
	{
		Declarations(&pcRoot->macpDeclarations);
	}

	//Functions generated when they are first called, which may call others, and the data of templates' instances:
	//class templates' static members and variable templates' instances.
	GenerateQueued();
	mcInstantiator.DefineInstanceData();
	while (mcGlobals.WriteExitRoutines() || mcExceptions.WriteThrownDestroyers() || mcGlobals.WriteDynamicCast())
	{
		GenerateQueued();
	}
	mcCheck.CheckUncalledFunctions();
	mcDiagnostics.ReportDeferred();
	if (!mbCompiledBefore)
	{
		mcCheck.ReportNamedInstances();
	}
	CheckContext();
	mpcStatistics->End(CPPCP_Declarations);

	//The IR front end inlines across the unit.
	mpcStatistics->Begin(CPPCP_Inline);
	mpcIRFrontEnd->InlineUnit();
	mpcStatistics->End(CPPCP_Inline);
	return mcDiagnostics.GetErrors()->NumErrors() == 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The rest of Generate, after GenerateBeforeTurn, in the unit's turn with
// the imports: the IR front end exports the unit's small functions and
// writes the functions it kept, then the rest of the unit is appended to
// pcDest.  False, with MissedImports, if the unit missed a function another
// unit exported since it inlined (it must be generated again).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::GenerateAfterTurn(CCPP65Assembly* pcDest)
{
	CChars			szStrings;
	CLargeChars*	pszText;
	bool			bFinished;

	//If the IR front end missed a function another unit exported, the unit is generated again (MissedImports).
	mpcStatistics->Begin(CPPCP_CodePasses);
	bFinished = mpcIRFrontEnd->FinishUnit();
	mpcStatistics->End(CPPCP_CodePasses);
	if (!bFinished)
	{
		mbMissedImports = true;
		return false;
	}

	AppendRuntime();

	szStrings.Init();
	mcData.AppendStrings(&szStrings);
	pszText = pcDest->GetText();
	pszText->Append(mszCode.Text(), mszCode.Length());
	pszText->Append(mszThunks.Text(), mszThunks.Length());
	pszText->Append(mcData.GetText()->Text(), mcData.GetText()->Length());
	pszText->Append(szStrings.Text(), szStrings.Length());
	szStrings.Kill();
	pcDest->SetPieces(&mauiCodePieces);
	return mcDiagnostics.GetErrors()->NumErrors() == 0;
}


//////////////////////////////////////////////////////////////////////////
//
// A whole program from one translation unit, as one object: the target's start up
// (CCPPTarget::AppendStartUp), then the translation unit.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::GenerateProgram(CChars* pszDest)
{
	mpcTarget->AppendStartUp(pszDest, mpcOptions->mbExceptions);
	return Generate(pszDest);
}


//////////////////////////////////////////////////////////////////////////
//
// True if the declarator declares a pointer that is itself __restrict
// (T* __restrict p): its outermost pointer is qualified.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsRestrictPointer(CCPPSyntaxDeclarator* pcDeclarator)
{
	CCPPSyntaxPointerOperator*	pcPointer;

	if (pcDeclarator->mpcNested || (pcDeclarator->macpSuffixes.NumElements() != 0) || (pcDeclarator->NumPointers() == 0))
	{
		return false;
	}
	pcPointer = pcDeclarator->GetPointer(pcDeclarator->NumPointers() - 1);
	return (pcPointer->meKind == CPPP_Pointer) && CPPHasKeyword(&pcPointer->macpQualifiers, CT___restrict, CT___restrict__);
}


//////////////////////////////////////////////////////////////////////////
//
// psType with a pointer operator's qualifiers (T* const p, T* volatile p):
// the pointer itself is const or volatile.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::QualifiedPointer(SCPP65Type* psType, CCPPSyntaxPointerOperator* pcPointer)
{
	return mcTypes.Qualified(psType, CPPHasKeyword(&pcPointer->macpQualifiers, CT_const, CT___const__), CPPHasKeyword(&pcPointer->macpQualifiers, CT_volatile, CT___volatile__));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPErrors* CCPPUnit::GetErrors(void)
{
	//Their tokens are C++ token indices.
	return mcDiagnostics.GetErrors();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPErrors* CCPPUnit::GetWarnings(void)
{
	return mcDiagnostics.GetWarnings();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCPPUnit::NumErrors(void) const
{
	return (int)mcDiagnostics.NumErrors();
}


//////////////////////////////////////////////////////////////////////////
//
// The labels of functions and global variables that were used but not defined in
// the translation unit.  They must come from somewhere else.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::GetExternals(CArrayChars* paszLabels)
{
	CCPPFunction*	pcFunction;
	CCPPVariable*	pcVariable;
	CChars			szLabel;
	size			i;

	for (i = 0; i < mapcGlobals.NumElements(); i++)
	{
		pcVariable = mapcGlobals.GetPtr(i);
		if (pcVariable->mbUsed && !pcVariable->mbDefined)
		{
			szLabel.Init();
			mcMangler.AppendGlobalLabel(&szLabel, pcVariable);
			paszLabels->AddIfUnique(szLabel);
			szLabel.Kill();
		}
	}

	for (i = 0; i < mapcFunctions.NumElements(); i++)
	{
		pcFunction = mapcFunctions.GetPtr(i);
		if (pcFunction->mbCalled && !pcFunction->mbGenerated)
		{
			szLabel.Init();
			mcMangler.AppendFunctionLabel(&szLabel, pcFunction);
			paszLabels->AddIfUnique(szLabel);
			szLabel.Kill();
		}
	}

	//The soft float runtime (Libc65/softfloat.cpp), not the routines AppendRuntime writes into the unit (R_DivideSigned
	//and R_DivideSigned64, the W65C816's R816_ ones).
	for (i = 0; i < maszRuntime.NumElements(); i++)
	{
		if ((strncmp(maszRuntime.GetText(i), "R_", 2) != 0) && (strncmp(maszRuntime.GetText(i), "R816_", 5) != 0))
		{
			paszLabels->AddIfUnique(maszRuntime.GetText(i));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Starts an instruction line.  The caller appends the rest and the new line.
//
//////////////////////////////////////////////////////////////////////////
CChars* CCPPUnit::Line(const char* szText)
{
	mszCode.Append("\t\t");
	return mszCode.Append(szText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::AppendName(CChars* psz, int64 lliName)
{
	char*	szName;
	int		iLength;

	szName = mpcTokeniser->GetIdentifier(lliName, &iLength);
	psz->Append('_');
	psz->Append(szName, iLength);
}


//////////////////////////////////////////////////////////////////////////
//
// The identifier a declarator declares, or -1 for an operator, a qualified name or
// no name.
//
//////////////////////////////////////////////////////////////////////////
int64 CCPPUnit::GetName(CCPPSyntaxDeclarator* pcDeclarator)
{
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;
	CCPPToken*					pcToken;

	pcName = pcDeclarator ? pcDeclarator->GetName() : NULL;
	if (!pcName || !pcName->IsSimple())
	{
		return -1;
	}
	pcComponent = pcName->GetLast();
	if (!pcComponent || (pcComponent->meKind != CPPNC_Identifier) || (pcComponent->miName < 0))
	{
		return -1;
	}
	pcToken = mpcTokeniser->Get(pcComponent->miName);
	if (!pcToken->IsIdentifier())
	{
		return -1;
	}
	return ((CCTIdentifier*)pcToken)->miIdentifier;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a declaration defines a member template of a class (not a class
// template) outside it: template<class T> int CValue::Size(T* p) { ... }.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsOutsideMemberTemplate(CCPPSyntax* pcDeclaration)
{
	CCPPSyntaxTemplateDeclaration*	pcTemplate;
	CCPPSyntaxName*					pcName;
	CCPPSyntaxNameComponent*		pcClass;

	if (pcDeclaration->GetKind() != CPPS_TemplateDeclaration)
	{
		return false;
	}
	pcTemplate = (CCPPSyntaxTemplateDeclaration*)pcDeclaration;
	if (pcTemplate->mbInstantiation || (pcTemplate->macpParameters.NumElements() == 0) || !pcTemplate->mpcDeclaration || (pcTemplate->mpcDeclaration->GetKind() != CPPS_FunctionDefinition))
	{
		return false;
	}
	pcName = ((CCPPSyntaxFunctionDefinition*)pcTemplate->mpcDeclaration)->mpcDeclarator->GetName();
	if (!pcName || !pcName->IsQualifiedByClass())
	{
		return false;
	}
	pcClass = pcName->GetQualifier();
	return pcClass->mpcSymbol && (pcClass->mpcSymbol->meKind == CPPSK_Class) && !mcInstantiator.GetComponentTemplate(pcClass);
}


//////////////////////////////////////////////////////////////////////////
//
// The declarations in order, but the member templates defined outside their
// class first (IsOutsideMemberTemplate): a function before one may call it,
// and its instance needs its definition.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::Declarations(CArraySyntax* papcDeclarations)
{
	size								i;
	CCPPSyntax*							pcDeclaration;
	CCPPSyntaxLinkageSpecification*		pcLinkage;
	bool								bResult;

	bResult = true;
	for (i = 0; i < papcDeclarations->NumElements(); i++)
	{
		pcDeclaration = papcDeclarations->GetPtr(i);
		if (IsOutsideMemberTemplate(pcDeclaration))
		{
			bResult &= mcInstantiator.TemplateDeclaration((CCPPSyntaxTemplateDeclaration*)pcDeclaration);
		}
	}
	for (i = 0; i < papcDeclarations->NumElements(); i++)
	{
		pcDeclaration = papcDeclarations->GetPtr(i);
		if (IsOutsideMemberTemplate(pcDeclaration))
		{
			continue;
		}
		switch (pcDeclaration->GetKind())
		{
		case CPPS_FunctionDefinition:
			bResult &= Function((CCPPSyntaxFunctionDefinition*)pcDeclaration, NULL);
			break;
		case CPPS_SimpleDeclaration:
			bResult &= mcGlobals.GlobalDeclaration((CCPPSyntaxSimpleDeclaration*)pcDeclaration);
			break;
		case CPPS_LinkageSpecification:
			//extern "C" int x; declares x, as if it were extern; extern "C" { int x; } defines it.  [dcl.link]
			pcLinkage = (CCPPSyntaxLinkageSpecification*)pcDeclaration;
			mcContext.EnterLinkage(IsCLinkage(pcLinkage), !pcLinkage->mbBraced);
			bResult &= Declarations(&pcLinkage->macpDeclarations);
			mcContext.LeaveLinkage();
			break;
		case CPPS_TemplateDeclaration:
			bResult &= mcInstantiator.TemplateDeclaration((CCPPSyntaxTemplateDeclaration*)pcDeclaration);
			break;
		case CPPS_Namespace:
			mcContext.EnterNamespace(((CCPPSyntaxNamespace*)pcDeclaration)->mpcScope);
			bResult &= Declarations(&((CCPPSyntaxNamespace*)pcDeclaration)->macpDeclarations);
			mcContext.LeaveNamespace();
			break;
		case CPPS_StaticAssert:
			bResult &= StaticAssert((CCPPSyntaxStaticAssert*)pcDeclaration);
			break;
		case CPPS_EmptyDeclaration:
		case CPPS_LazyDeclaration:		//A header declaration the unit never used, so never parsed.
			break;
		default:
			if (!IsScopeDeclaration(pcDeclaration))
			{
				bResult &= mcDiagnostics.Error(pcDeclaration, "This declaration is not supported by the compiler yet");
			}
			break;
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A declaration that only changes what names mean: the parser has dealt with
// it (using, using namespace, using X = T and namespace A = N, which is the
// only skipped declaration that isn't left out, as asm is).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsScopeDeclaration(CCPPSyntax* pcDeclaration)
{
	switch (pcDeclaration->GetKind())
	{
	case CPPS_AliasDeclaration:
	case CPPS_UsingDeclaration:
	case CPPS_UsingDirective:
		return true;
	case CPPS_SkippedDeclaration:
		return mpcTokeniser->Get(pcDeclaration->miFirstToken)->Is(CT_namespace);
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// extern "C" (anything but extern "C++").
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsCLinkage(CCPPSyntaxLinkageSpecification* pcLinkage)
{
	CChars		szLanguage;
	CCPPToken*	pcToken;
	bool		bC;

	szLanguage.Init();
	pcToken = mpcTokeniser->Get(pcLinkage->miLanguage);
	if (pcToken)
	{
		pcToken->Append(&szLanguage, mpcTokeniser);
	}
	bC = !szLanguage.Contains("C++");
	szLanguage.Kill();
	return bC;
}


//////////////////////////////////////////////////////////////////////////
//
// Enters a function, in whatever is being written: pcFunction is the
// context's function, with a scope of its own in which its parameters and
// locals are made (AddParameters, AllocateLocals) and its expressions typed
// or its body written.  LeaveFunction lets its locals go and what was being
// written is as it was, its function, its this and its locals.  Who enters
// a function leaves it.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::EnterFunction(CCPPFunction* pcFunction)
{
	PushFunctionScope();
	mcContext.EnterFunction(pcFunction);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::LeaveFunction(void)
{
	mcContext.LeaveFunction();
	PopFunctionScope();
}


//////////////////////////////////////////////////////////////////////////
//
// A new last scope, with no locals and no packs.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::PushFunctionScope(void)
{
	SCPPFunctionScope*	psScope;

	psScope = (SCPPFunctionScope*)malloc(sizeof(SCPPFunctionScope));
	psScope->apcLocals.Init();
	psScope->cFunctionPacks.Init();
	mapsFunctionScopes.Add(psScope);
}


//////////////////////////////////////////////////////////////////////////
//
// The last scope goes, and its locals with it.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::PopFunctionScope(void)
{
	SCPPFunctionScope*	psScope;

	psScope = mapsFunctionScopes.GetPtr(mapsFunctionScopes.NumElements() - 1);
	FreeVariables(&psScope->apcLocals);
	psScope->apcLocals.Kill();
	psScope->cFunctionPacks.Kill();
	free(psScope);
	mapsFunctionScopes.RemoveTail();
}


//////////////////////////////////////////////////////////////////////////
//
// A new function, a member of pcClass if there is one, and the last of the
// functions of its name (FirstFunctionOfName).
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPUnit::AddFunction(CCPPClass* pcClass, int64 lliName, ECPPFunctionKind eKind, SCPP65Type* psReturn)
{
	CCPPFunction*			pcFunction;
	SCPPFunctionsOfName*	psOfName;
	SCPPFunctionsOfName	sOfName;

	pcFunction = NewMalloc<CCPPFunction>();
	pcFunction->Init(pcClass, lliName, eKind, psReturn);
	mapcFunctions.Add(pcFunction);
	if (pcClass)
	{
		pcClass->mapcMembers.Add(pcFunction);
	}

	psOfName = mcFunctionsOfNames.Get(lliName);
	if (psOfName)
	{
		psOfName->pcLast->mpcNextOfName = pcFunction;
		psOfName->pcLast = pcFunction;
	}
	else
	{
		sOfName.pcFirst = pcFunction;
		sOfName.pcLast = pcFunction;
		mcFunctionsOfNames.Put(lliName, &sOfName);
	}
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// The first function added with the name, or NULL.  The others follow it (pcNextOfName) in the order they
// were added, as in mapcFunctions.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPUnit::FirstFunctionOfName(int64 lliName)
{
	SCPPFunctionsOfName*	psOfName;

	psOfName = mcFunctionsOfNames.Get(lliName);
	return psOfName ? psOfName->pcFirst : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Generates the functions waiting in the order they were queued, and those
// generating them queues after them.  True if there were any.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::GenerateQueued(void)
{
	bool	bGenerated;

	bGenerated = muiQueueNext < mapcQueue.NumElements();
	while (muiQueueNext < mapcQueue.NumElements())
	{
		muiQueueNext++;
		GenerateFunction(mapcQueue.GetPtr(muiQueueNext - 1));
	}
	mapcQueue.FakeSetUsedElements(0);
	muiQueueNext = 0;
	return bGenerated;
}


//////////////////////////////////////////////////////////////////////////
//
// A function is called: it is generated later if it is defined but not generated
// yet, and if another unit doesn't generate it (extern template,
// IsInstantiatedElsewhere): its label is then an external.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::RequireFunction(CCPPFunction* pcFunction)
{
	pcFunction->mbCalled = true;
	if (!pcFunction->mbGenerated && !pcFunction->mbQueued && (pcFunction->mpcDefinition || pcFunction->mbImplicit || pcFunction->mpcCovariantOf) && !mcInstantiator.IsInstantiatedElsewhere(pcFunction))
	{
		pcFunction->mbQueued = true;
		mapcQueue.Add(pcFunction);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Notes that a label is being written.  Returns false if it has been written
// already.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::Emit(CChars* pszLabel)
{
	if (maszEmitted.GetIndex(pszLabel) != ARRAY_ELEMENT_NOT_FOUND)
	{
		return false;
	}
	maszEmitted.Add(pszLabel);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPVariable* CCPPUnit::AddVariable(CArrayCPPVariables* papcVariables, CCPPSymbol* pcSymbol, SCPP65Type* psType, ECPPStorage eStorage, int iOffset)
{
	CCPPVariable*	pcVariable;

	pcVariable = NewMalloc<CCPPVariable>();
	pcVariable->Init(pcSymbol, psType, eStorage, iOffset, mcContext.IsCLinkage());
	papcVariables->Add(pcVariable);
	return pcVariable;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::FreeVariables(CArrayCPPVariables* papcVariables)
{
	size	i;

	for (i = 0; i < papcVariables->NumElements(); i++)
	{
		if (mpcConstants && (papcVariables->GetPtr(i)->meStorage == CPPVS_Local))
		{
			mpcConstants->ForgetLocal(papcVariables->GetPtr(i));
		}
		papcVariables->GetPtr(i)->Kill();
		free(papcVariables->GetPtr(i));
	}
	papcVariables->ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// The local, parameter or global a symbol names.  Locals are searched first.
//
//////////////////////////////////////////////////////////////////////////
CCPPVariable* CCPPUnit::GetVariable(CCPPSymbol* pcSymbol)
{
	size			i;
	CCPPVariable*	pcVariable;

	for (i = 0; i < GetLocals()->NumElements(); i++)
	{
		pcVariable = GetLocals()->GetPtr(i);
		if (pcVariable->mpcSymbol == pcSymbol)
		{
			return pcVariable;
		}
	}
	for (i = 0; i < mapcGlobals.NumElements(); i++)
	{
		pcVariable = mapcGlobals.GetPtr(i);
		if (pcVariable->mpcSymbol == pcSymbol)
		{
			return pcVariable;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of a parameter's variable in its function: the parameter's type
// in the function's type (ParameterType) with the const and volatile that
// don't count there (void f(const int i), void f(char* const p)).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::ParameterVariableType(SCPP65Type* psType, CCPPSyntaxParameter* pcParameter)
{
	CCPPSyntaxDeclarator*		pcDeclarator;
	CCPPSyntaxPointerOperator*	pcPointer;
	uint32						uiFlags;

	pcDeclarator = pcParameter->mpcDeclarator;
	if (pcDeclarator && ((pcDeclarator->macpSuffixes.NumElements() > 0) || pcDeclarator->mpcNested))
	{
		return psType;
	}
	if (pcDeclarator && (pcDeclarator->NumPointers() > 0))
	{
		pcPointer = pcDeclarator->GetPointer(pcDeclarator->NumPointers() - 1);
		return (pcPointer->meKind == CPPP_Pointer) ? QualifiedPointer(psType, pcPointer) : psType;
	}
	uiFlags = pcParameter->mpcSpecifiers ? pcParameter->mpcSpecifiers->muiFlags : 0;
	return mcTypes.Qualified(psType, (uiFlags & CPPDS_CONST) != 0, (uiFlags & CPPDS_VOLATILE) != 0);
}


//////////////////////////////////////////////////////////////////////////
//
// A function definition outside a class, a member of pcClass if it is not NULL (or
// of the class its qualified name names).  Inline and constexpr functions are
// generated when they are called; others now.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::Function(CCPPSyntaxFunctionDefinition* pcDefinition, CCPPClass* pcClass)
{
	CCPPSyntaxName*		pcName;
	CCPPFunction*		pcFunction;
	size				uiErrors;

	pcName = pcDefinition->mpcDeclarator->GetName();
	if (!pcClass && pcName && pcName->IsQualifiedByClass())
	{
		pcClass = GetNames()->GetComponentClass(pcName->GetQualifier(), NULL, pcDefinition);
		if (!pcClass)
		{
			return mcDiagnostics.Error(pcDefinition, "The function's class has not been defined");
		}
		if (!mcClasses.CompleteClass(pcClass, pcDefinition))
		{
			return false;
		}
	}

	if (pcDefinition->meBody == CPPFB_Skipped)
	{
		//A body skipped in a system header: only a declaration.
		mcDiagnostics.BeginQuiet();
		mcFunctionDeclarer.Declare(pcDefinition->mpcSpecifiers, pcDefinition->mpcDeclarator, pcClass, CPPDA_Declaration);
		mcDiagnostics.EndQuiet();
		return true;
	}
	pcFunction = mcFunctionDeclarer.Declare(pcDefinition->mpcSpecifiers, pcDefinition->mpcDeclarator, pcClass, CPPDA_Definition);
	if (!pcFunction)
	{
		return false;
	}
	if (pcFunction->mpcDefinition || pcFunction->mbGenerated)
	{
		return mcDiagnostics.Error(pcDefinition, "The function is defined twice");
	}
	if (!mcFunctionDeclarer.CheckDefaultedOrDeleted(pcFunction, pcDefinition))
	{
		return false;
	}
	pcFunction->SetDefinition(pcDefinition);
	if (!pcFunction->mpcDefinition && !pcFunction->mbImplicit)
	{
		//A body skipped in a system header.
		return true;
	}
	if (!mpcConstants->CheckDefinition(pcFunction))
	{
		return false;
	}
	pcFunction->mbInline = pcDefinition->mpcSpecifiers && (pcDefinition->mpcSpecifiers->muiFlags & (CPPDS_INLINE | CPPDS_CONSTEXPR));
	if (pcFunction->mbInline)
	{
		return true;
	}

	uiErrors = mcDiagnostics.GetErrors()->NumErrors();
	GenerateFunction(pcFunction);
	return mcDiagnostics.GetErrors()->NumErrors() == uiErrors;
}


//////////////////////////////////////////////////////////////////////////
//
// The IR front end writes pcFunction's body (CCPP65IRFrontEnd::Body), timed
// as the front end's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::WriteBody(CCPPFunction* pcFunction)
{
	bool	bWritten;

	mpcStatistics->Begin(CPPCP_FrontEnd);
	bWritten = mpcIRFrontEnd->Body(pcFunction);
	mpcStatistics->End(CPPCP_FrontEnd);
	return bWritten;
}


//////////////////////////////////////////////////////////////////////////
//
// Generates a function from its definition, or a constructor or destructor the
// generator writes, with the template arguments it was made with.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::GenerateFunction(CCPPFunction* pcFunction)
{
	CChars			szLabel;
	bool			bShared;

	if (pcFunction->mbGenerated)
	{
		return;
	}
	pcFunction->mbGenerated = true;
	pcFunction->mbQueued = false;
	if (mpcSemanticPass)
	{
		//Only checked.
		mcContext.EnterBindings(pcFunction->mpcBindings);
		mcCheck.CheckFunction(pcFunction);
		mcContext.LeaveBindings();
		return;
	}

	//Each function is a section of its own.  Inline, template and generated functions are written by every unit
	//that needs them, each in a COMDAT section the linker keeps one of.
	szLabel.Init();
	mcMangler.AppendFunctionLabel(&szLabel, pcFunction);
	bShared = pcFunction->IsShared();
	if (bShared && !Emit(&szLabel))
	{
		szLabel.Kill();
		return;
	}
	CCPPUnitData::AppendSection(&mszCode, "text", &szLabel, bShared ? ", comdat" : NULL);
	if (!pcFunction->IsUnitFunction())
	{
		mszCode.Append(mcFunctionDeclarer.IsWeak(pcFunction) ? "\t\t.weak " : "\t\t.global ")->Append(&szLabel)->AppendNewLine();
	}
	szLabel.Kill();
	mcContext.EnterBindings(pcFunction->mpcBindings);

	if (pcFunction->mpcCovariantOf)
	{
		//A covariant thunk (CCPPVtable::EntryFor).
		EnterFunction(pcFunction);
		if (!WriteBody(pcFunction))
		{
			szLabel.Init("A covariant result the IR front end can't write a thunk for is not supported on this target yet: ");
			mcMangler.AppendFunctionLabel(&szLabel, pcFunction);
			AppendUnsupported(&szLabel);
			mcDiagnostics.ErrorAt(pcFunction->mpcCovariantOf->mpcDeclarator ? pcFunction->mpcCovariantOf->mpcDeclarator->miFirstToken : -1, szLabel.Text());
			szLabel.Kill();
		}
		LeaveFunction();
	}
	else if (pcFunction->mbImplicit)
	{
		//An implicit default constructor, an inherited one or a destructor, or a copy defaulted after its class.
		EnterFunction(pcFunction);
		if (((pcFunction->mapsParameters.NumElements() != 0) && !pcFunction->mpcInherited && !pcFunction->IsCopy()) || !WriteBody(pcFunction))
		{
			szLabel.Init("An implicit copy constructor or assignment (or one the IR front end can't write) is not supported on this target yet: ");
			mcMangler.AppendFunctionLabel(&szLabel, pcFunction);
			if (pcFunction->mapsParameters.NumElements() == 0)
			{
				AppendUnsupported(&szLabel);
			}
			//Where its class is defined: an implicit function is written nowhere.
			mcDiagnostics.ErrorAt((pcFunction->mpcClass && pcFunction->mpcClass->mpcSpecifier) ? pcFunction->mpcClass->mpcSpecifier->miFirstToken : -1, szLabel.Text());
			szLabel.Kill();
		}
		LeaveFunction();
	}
	else
	{
		GenerateDefinition(pcFunction);
	}
	mcContext.LeaveBindings();
}


//////////////////////////////////////////////////////////////////////////
//
// The code for a function's definition: its parameters and locals get their places,
// a constructor makes its bases and fields first, a destructor sets its vtable
// pointer first (EndFunction destroys its fields and bases), then the body.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::GenerateDefinition(CCPPFunction* pcFunction)
{
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	size							uiErrors;
	CChars							szMessage;

	pcDefinition = pcFunction->mpcDefinition;
	if ((pcDefinition->macpMemberInitializers.NumElements() != 0) && (pcFunction->meKind != CPPFK_Constructor))
	{
		return mcDiagnostics.Error(pcDefinition, "Only constructors have member initialisers");
	}

	EnterFunction(pcFunction);
	AddParameters(pcFunction);
	if (!mcFunctionDeclarer.CheckAbstractSignature(pcFunction, pcDefinition->mpcDeclarator) || !mcFunctionDeclarer.CheckMemberInitialisers(pcFunction) || !AllocateLocals(pcDefinition->mpcBody) || !mcExceptions.CheckTryJumps(pcDefinition->mpcBody))
	{
		LeaveFunction();
		return false;
	}

	//The IR front end writes the whole function.  An error it reported (ReportTypeOf: C++ that is wrong) is the reason
	//when it can't; else it is C++ the front end doesn't do yet.
	uiErrors = mcDiagnostics.GetErrors()->NumErrors();
	if (WriteBody(pcFunction))
	{
		LeaveFunction();
		return true;
	}
	if (mcDiagnostics.GetErrors()->NumErrors() > uiErrors)
	{
		mpcIRFrontEnd->ClearUnsupported();
	}
	else if (mcDiagnostics.HasDeferred())
	{
		//A class the function uses was wrong (laid out quietly, so not reported yet): that is why.
		mpcIRFrontEnd->ClearUnsupported();
		mcDiagnostics.ReportDeferred();
	}
	else
	{
		szMessage.Init("This function uses C++ the ");
		szMessage.Append(mpcTarget->GetConfigurationName());
		szMessage.Append(" compiler doesn't support yet");
		AppendUnsupported(&szMessage);
		mcDiagnostics.Error(pcDefinition, szMessage.Text());
		szMessage.Kill();
	}
	LeaveFunction();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The parameters of the function BeginFunction started are its first
// locals.  Arguments take their own size on the stack, the first lowest,
// after the hidden result address of a function that returns an object.
// Object arguments are the function's to destroy.  A function parameter
// pack is a parameter for each of its types, each with its own symbol
// (PackElement).  They are named as pcDeclarator names them, or as the
// function's definition does.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::AddParameters(CCPPFunction* pcFunction, CCPPSyntaxDeclarator* pcDeclarator)
{
	CCPPSyntaxFunctionSuffix*		pcSuffix;
	CCPPSyntaxParameter*			pcParameter;
	SCPP65Type*						psType;
	CCPPVariable*					pcVariable;
	SCPPLambda*						psLambda;
	CCPPSymbol*						pcSymbol;
	size							i;
	size							j;
	size							k;
	size							uiPack;
	bool							bPack;

	pcSuffix = (pcDeclarator ? pcDeclarator : pcFunction->mpcDefinition->mpcDeclarator)->GetFunctionSuffix();
	j = 0;
	for (i = 0; i < pcSuffix->macpParameters.NumElements(); i++)
	{
		//A function parameter pack (Args... args, or a generic lambda's auto... args), which is the last, is as
		//many parameters as the function has more than the others declared.
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		bPack = pcSuffix->EndsWithPack() && (i + 1 == pcSuffix->macpParameters.NumElements());
		uiPack = (j < pcFunction->mapsParameters.NumElements()) ? 1 : 0;
		if (bPack)
		{
			uiPack = pcFunction->mapsParameters.NumElements() - j;
			if (pcParameter->mpcSymbol)
			{
				GetFunctionPacks()->Add(pcParameter->mpcSymbol);
			}
		}
		for (k = 0; k < uiPack; k++)
		{
			psType = pcFunction->mapsParameters.GetPtr(j);
			pcSymbol = (bPack && pcParameter->mpcSymbol) ? mcInstantiator.FunctionPackElement(pcFunction, pcParameter->mpcSymbol, k) : pcParameter->mpcSymbol;
			if (pcSymbol)
			{
				pcVariable = AddVariable(GetLocals(), pcSymbol, ParameterVariableType(psType, pcParameter), CPPVS_Parameter, (int)j);
				pcVariable->mbRestrict = !bPack && pcParameter->mpcDeclarator && IsRestrictPointer(pcParameter->mpcDeclarator);
				if (psType->IsClass())
				{
					//Laid out now, as the IR front end needs it.
					mcClasses.CompleteClass(psType->pcClass, pcParameter);
				}
			}
			j++;
		}
	}

	//A lambda's body expands the parameter packs of the functions the lambda is in too, into the elements it captured
	//(CapturePack).
	psLambda = mcClosures.BodyLambda(pcFunction);
	if (psLambda)
	{
		GetFunctionPacks()->AddAll(&psLambda->cFunctionPacks);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the compiler can do a value of psType: an error at pcNode (and
// false) for an integer wider than 64 bits.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::Supported(SCPP65Type* psType, CCPPSyntax* pcNode)
{
	if (psType->IsScalar() && (psType->iBytes > 8))
	{
		return mcDiagnostics.Error(pcNode, "Integers wider than 64 bits are not supported by the compiler");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Why the IR front end couldn't write a function (CCPP65IRFrontEnd::
// AppendUnsupported).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::AppendUnsupported(CChars* psz)
{
	mpcIRFrontEnd->AppendUnsupported(psz);
}


//////////////////////////////////////////////////////////////////////////
//
// Gives every local variable in the function its own place in the frame before the
// body is generated, so the frame size is known.  Places are not shared between
// blocks yet.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::AllocateLocals(CCPPSyntax* pcNode)
{
	CCPPSyntaxInitDeclarator*	pcInit;
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	SCPP65Type*					psType;
	size						i;
	bool						bResult;

	if (!pcNode)
	{
		return true;
	}
	if (pcNode->GetKind() == CPPS_LambdaExpression)
	{
		//Its body's locals are its closure's operator()'s.
		return true;
	}

	if (pcNode->GetKind() == CPPS_SimpleDeclaration)
	{
		pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcNode;
		if ((pcDeclaration->macpDeclarators.NumElements() == 0) && pcDeclaration->mpcSpecifiers && pcDeclaration->mpcSpecifiers->mpcType &&
			(pcDeclaration->mpcSpecifiers->mpcType->GetKind() == CPPS_ClassSpecifier) && !((CCPPSyntaxClassSpecifier*)pcDeclaration->mpcSpecifiers->mpcType)->mpcName)
		{
			return AnonymousLocal((CCPPSyntaxClassSpecifier*)pcDeclaration->mpcSpecifiers->mpcType);
		}
	}

	if ((pcNode->GetKind() == CPPS_InitDeclarator) && ((CCPPSyntaxInitDeclarator*)pcNode)->GetDeclaration())
	{
		pcInit = (CCPPSyntaxInitDeclarator*)pcNode;
		pcDeclaration = pcInit->GetDeclaration();
		if (pcInit->mpcSymbol && (pcInit->mpcSymbol->meKind == CPPSK_Variable))
		{
			if (pcDeclaration->mpcSpecifiers->muiFlags & CPPDS_EXTERN)
			{
				return mcGlobals.ExternLocal(pcInit, pcDeclaration->mpcSpecifiers);
			}
			if (pcDeclaration->mpcSpecifiers->muiFlags & (CPPDS_STATIC | CPPDS_THREAD_LOCAL))
			{
				//thread_local in a block is static too; there is one thread.  [dcl.stc] 3
				return mcGlobals.StaticLocal(pcInit, pcDeclaration->mpcSpecifiers);
			}
			psType = mcDeclarators.GetType(pcDeclaration->mpcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
			if (!psType)
			{
				return false;
			}
			if (psType->IsVoid())
			{
				//[basic.fundamental] 9
				return mcDiagnostics.Error(pcNode, "A variable can't be void");
			}
			if (!psType->IsScalar() && !psType->IsArray() && !psType->IsClass() && !psType->IsReference())
			{
				return mcDiagnostics.Error(pcNode, "This type of local variable is not supported by the compiler yet");
			}
			if (psType->ElementType()->IsClass() && !mcClasses.CompleteObjectClass(psType->ElementType()->pcClass, pcNode))
			{
				return false;
			}

			AddVariable(GetLocals(), pcInit->mpcSymbol, psType, CPPVS_Local, 0);
		}
	}

	if ((pcNode->GetKind() == CPPS_RangeForStatement) && !mcDesugarings.RangeForLocals((CCPPSyntaxRangeForStatement*)pcNode))
	{
		return false;
	}
	if ((pcNode->GetKind() == CPPS_CatchHandler) && !mcExceptions.HandlerLocal((CCPPSyntaxCatchHandler*)pcNode))
	{
		return false;
	}

	bResult = true;
	for (i = 0; i < pcNode->NumChildren(); i++)
	{
		bResult &= AllocateLocals(pcNode->GetChild(i));
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// An anonymous union (or struct) declared in a function: it gets a place in the frame and each of its members
// is a local variable at its offset in it.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::AnonymousLocal(CCPPSyntaxClassSpecifier* pcSpecifier)
{
	CCPPClass*	pcClass;
	SCPPField*	psField;
	CCPPVariable*	pcVariable;
	size			i;

	pcClass = mcClasses.GetUnnamedClass(pcSpecifier);
	if (!pcClass || !mcClasses.CompleteClass(pcClass, pcSpecifier))
	{
		return mcDiagnostics.Error(pcSpecifier, "This anonymous class is not supported by the compiler yet");
	}

	for (i = 0; i < pcClass->macFields.NumElements(); i++)
	{
		psField = pcClass->macFields.Get(i);
		if (!psField->pcSymbol)
		{
			return mcDiagnostics.Error(pcSpecifier, "Anonymous classes in anonymous local classes are not supported by the compiler yet");
		}
		if (psField->IsBitField())
		{
			return mcDiagnostics.Error(pcSpecifier, "Bit-fields in anonymous local classes are not supported by the compiler yet");
		}
		pcVariable = AddVariable(GetLocals(), psField->pcSymbol, psField->psType, CPPVS_Local, psField->iOffset);
		pcVariable->mpcAnonymous = pcClass;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A typedef declared again in its scope names the same type, and one with
// a class's or an enum's name in the class's scope names that class.
// [dcl.typedef] 3, 6
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckTypedefs(CCPPSyntaxSimpleDeclaration* pcDeclaration)
{
	CCPPSyntaxInitDeclarator*	pcInit;
	CCPPSymbol*					pcSymbol;
	CCPPSymbol*					pcOther;
	SCPP65Type*					psType;
	SCPP65Type*					psOther;
	size						i;
	bool						bResult;

	bResult = true;
	for (i = 0; i < pcDeclaration->macpDeclarators.NumElements(); i++)
	{
		pcInit = (CCPPSyntaxInitDeclarator*)pcDeclaration->macpDeclarators.GetPtr(i);
		pcSymbol = pcInit->mpcSymbol;
		if (!pcSymbol || (pcSymbol->meKind != CPPSK_Typedef) || !pcSymbol->mpcScope)
		{
			continue;
		}
		psType = NULL;
		for (pcOther = pcSymbol->mpcScope->FindHere(pcSymbol->mlliName); pcOther; pcOther = pcOther->mpcNext)
		{
			if ((pcOther == pcSymbol) || (pcOther->mpcScope != pcSymbol->mpcScope) || pcOther->mpcLazy || !pcOther->mpcDeclaration || (pcOther->mpcDeclaration->miFirstToken >= pcInit->miFirstToken))
			{
				continue;
			}
			if ((pcOther->meKind != CPPSK_Typedef) && (pcOther->meKind != CPPSK_Class) && (pcOther->meKind != CPPSK_Enum))
			{
				continue;
			}
			if (!psType)
			{
				mcDiagnostics.BeginQuiet();
				psType = mcDeclarators.GetType(pcDeclaration->mpcSpecifiers, pcInit->mpcDeclarator, pcInit, NULL);
				mcDiagnostics.EndQuiet();
				if (!psType)
				{
					break;
				}
			}
			if (pcOther->meKind == CPPSK_Typedef)
			{
				mcDiagnostics.BeginQuiet();
				psOther = mcDeclarators.GetSymbolType(pcOther, pcInit);
				mcDiagnostics.EndQuiet();
				if (psOther && !psOther->Same(psType))
				{
					bResult &= mcDiagnostics.Error(pcInit, "The typedef is declared again for another type");
				}
			}
			else if (!((psType->IsClass() && psType->pcClass->mpcSymbol == pcOther) || (psType->pcEnum == pcOther)))
			{
				bResult &= mcDiagnostics.Error(pcInit, "A typedef of a class's or an enum's name in its scope names that class or enum");
			}
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The static_asserts among a class's members, when the class is defined: it
// may never be laid out, and they are wrong all the same.  [dcl.dcl] 6
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::ClassStaticAsserts(CCPPSyntaxClassSpecifier* pcSpecifier)
{
	CCPPSyntax*	pcMember;
	size		i;
	bool		bResult;

	bResult = true;
	for (i = 0; i < pcSpecifier->macpMembers.NumElements(); i++)
	{
		pcMember = pcSpecifier->macpMembers.GetPtr(i);
		if (pcMember->GetKind() == CPPS_StaticAssert)
		{
			bResult &= StaticAssert((CCPPSyntaxStaticAssert*)pcMember);
		}
		else if ((pcMember->GetKind() == CPPS_SimpleDeclaration) && ((CCPPSyntaxSimpleDeclaration*)pcMember)->mpcSpecifiers)
		{
			bResult &= mcEnums.CheckEnumerators(((CCPPSyntaxSimpleDeclaration*)pcMember)->mpcSpecifiers->mpcType);
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A static_assert whose condition is a constant that is false is an error.  One the compiler cannot work out
// (sizeof a member function pointer ...) is ignored.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::StaticAssert(CCPPSyntaxStaticAssert* pcAssert)
{
	int64	lliValue;
	size	uiErrors;
	bool	bConstant;

	uiErrors = mcDiagnostics.GetErrors()->NumErrors();
	mpcConstants->ClearReason();
	mcDiagnostics.BeginQuiet();
	bConstant = mcFolder.Constant(pcAssert->mpcCondition, &lliValue);
	mcDiagnostics.EndQuiet();
	if (bConstant && (lliValue == 0))
	{
		return mcDiagnostics.Error(pcAssert, "The static_assert failed");
	}
	if (!bConstant && (mcDiagnostics.GetErrors()->NumErrors() == uiErrors))
	{
		mcDiagnostics.BeginQuiet();
		bConstant = mpcConstants->Value(pcAssert->mpcCondition, &mcTypes.msBool, &lliValue);
		mcDiagnostics.EndQuiet();
		if (bConstant && (lliValue == 0))
		{
			return mcDiagnostics.Error(pcAssert, "The static_assert failed");
		}
	}
	if (!bConstant && (mpcConstants->GetReason()[0] != '\0') && (strncmp(mpcConstants->GetReason(), "Its initialiser uses", 20) != 0))
	{
		//What can never be a constant (not what the compiler can't work out yet).  [dcl.dcl] 4
		return mcFolder.ConstantError(pcAssert->mpcCondition, "A static_assert's expression is a constant");
	}
	return mcDiagnostics.GetErrors()->NumErrors() == uiErrors;
}


//////////////////////////////////////////////////////////////////////////
//
// Runtime routines the generated code calls.  Only the ones used are added:
// R_Pure, which a pure virtual function's vtable entry is, R_BadCast, which a
// dynamic_cast to a reference that fails calls, R_RecursiveInit, which a
// static local reached again while it is being made calls, and the
// target's own (CCPPTarget::AppendRuntime).  They stop: there is nothing to
// throw.  And in a program built with exceptions, R_Exceptions.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::AppendRuntime(void)
{
	CChars	szLabel;

	if (mbPureVirtual && BeginRuntime("R_Pure"))
	{
		//A pure virtual function was called: std::terminate, in a program built with exceptions that has their
		//runtime; a stop without.
		if (mpcOptions->mbExceptions)
		{
			mpcTarget->AppendCallTerminate(&mszCode, "R_PureStop");
			mszCode.Append("R_PureStop:")->AppendNewLine();
		}
		Line("stp")->AppendNewLine();
		mszCode.AppendNewLine();
	}
	if (mbBadCast && BeginRuntime("R_BadCast"))
	{
		//A dynamic_cast to a reference failed (bad_cast, were there exceptions).
		Line("stp")->AppendNewLine();
		mszCode.AppendNewLine();
	}
	if (mpcOptions->mbExceptions)
	{
		//The program is built with exceptions: a byte that a start up the compiler doesn't write (HCP65's UserStart.asm)
		//finds through a weak reference, to know that main's carry is the exception flag.  One for the program (COMDAT),
		//kept though nothing refers to it but weakly.
		szLabel.Init(CPP_EXCEPTIONS_LABEL);
		if (Emit(&szLabel))
		{
			CCPPUnitData::AppendSection(mcData.GetText(), "rodata", &szLabel, ", comdat, keep");
			CCPPUnitData::AppendGlobal(mcData.GetText(), &szLabel);
			mcData.GetText()->Append(&szLabel)->Append(':')->AppendNewLine();
			mcData.GetText()->Append("\t\t.byte 1")->AppendNewLine();
		}
		szLabel.Kill();
	}
	if (mbRecursiveInit && BeginRuntime("R_RecursiveInit"))
	{
		//A static local was reached again while it was being made (undefined behaviour; g++ throws recursive_init_error).
		Line("stp")->AppendNewLine();
		mszCode.AppendNewLine();
	}
	mpcTarget->AppendRuntime(this);
}


//////////////////////////////////////////////////////////////////////////
//
// The runtime routines' labels the unit's code calls (CCPPRuntimeWriter).
//
//////////////////////////////////////////////////////////////////////////
CArrayChars* CCPPUnit::GetRuntimeCalls(void)
{
	return &maszRuntime;
}


//////////////////////////////////////////////////////////////////////////
//
// A runtime routine's COMDAT section and label (any unit may write it), unless
// the unit has written it already: then false.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::BeginRuntime(const char* szName)
{
	CChars	szLabel;

	szLabel.Init(szName);
	if (!Emit(&szLabel))
	{
		szLabel.Kill();
		return false;
	}
	szLabel.Kill();

	//Resident: it is called with A and B wider than a function's 32 bits, which the lazy loader can't keep.
	szLabel.Init(szName);
	CCPPUnitData::AppendSection(&mszCode, "text", &szLabel, ", comdat, resident");
	CCPPUnitData::AppendGlobal(&mszCode, &szLabel);
	szLabel.Kill();
	mszCode.Append(szName);
	mszCode.Append(':');
	mszCode.AppendNewLine();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Where a runtime routine's code goes (CCPPRuntimeWriter).
//
//////////////////////////////////////////////////////////////////////////
CChars* CCPPUnit::GetRuntimeCode(void)
{
	return &mszCode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::RuntimeError(const char* szMessage)
{
	mcDiagnostics.ErrorAt(-1, szMessage);
}


CCPP65Types* CCPPUnit::GetTypes(void) { return &mcTypes; }
CCPPContext* CCPPUnit::GetContext(void) { return &mcContext; }
CCPPDiagnostics* CCPPUnit::GetDiagnostics(void) { return &mcDiagnostics; }
CCPPBinder* CCPPUnit::GetBinder(void) { return &mcBinder; }
CCPPConstants* CCPPUnit::GetConstants(void) const { return mpcConstants; }
CCPPConstantFolder* CCPPUnit::GetFolder(void) { return &mcFolder; }
CCPPClosures* CCPPUnit::GetClosures(void) { return &mcClosures; }
CCPPClassData* CCPPUnit::GetClassData(void) { return &mcClassData; }
CCPPStrings* CCPPUnit::GetStrings(void) { return &mcStrings; }
CCPPCheck* CCPPUnit::GetCheck(void) { return &mcCheck; }
CCPPUnitObserver* CCPPUnit::GetObserver(void) { return mpcObserver; }
CCPPWarnings* CCPPUnit::GetWarner(void) { return &mcWarnings; }
CCPPBuiltins* CCPPUnit::GetBuiltins(void) { return &mcBuiltins; }
CCPPExceptions* CCPPUnit::GetExceptions(void) { return &mcExceptions; }
CCPPDesugarings* CCPPUnit::GetDesugarings(void) { return &mcDesugarings; }
CCPPInstantiator* CCPPUnit::GetInstantiator(void) { return &mcInstantiator; }
CCPPDeclarators* CCPPUnit::GetDeclarators(void) { return &mcDeclarators; }
CCPPFunctionDeclarer* CCPPUnit::GetFunctionDeclarer(void) { return &mcFunctionDeclarer; }
CCPPNames* CCPPUnit::GetNames(void) { return mcBinder.GetNames(); }
CCPPGlobals* CCPPUnit::GetGlobals(void) { return &mcGlobals; }
CCPPClasses* CCPPUnit::GetClasses(void) { return &mcClasses; }
CCPPEnums* CCPPUnit::GetEnums(void) { return &mcEnums; }
CCPPAttributes* CCPPUnit::GetAttributes(void) { return &mcAttributes; }
bool CCPPUnit::IsCompiledBefore(void) const { return mbCompiledBefore; }
CArrayCPPVariables* CCPPUnit::GetGlobalVariables(void) { return &mapcGlobals; }
CCPP65IRFrontEnd* CCPPUnit::GetIRFrontEnd(void) const { return mpcIRFrontEnd; }
CCPPMangler* CCPPUnit::GetMangler(void) { return &mcMangler; }
CCPPUnitData* CCPPUnit::GetData(void) { return &mcData; }
CCPP65DebugWriter* CCPPUnit::GetDebugWriter(void) { return &mcDebugWriter; }
CCPP65TypeChecker* CCPPUnit::GetTypeChecker(void) { return &mcTypeChecker; }
CChars* CCPPUnit::GetCode(void) { return &mszCode; }
CArrayCPPTemplates* CCPPUnit::GetTemplates(void) { return &mapcTemplates; }
CArrayCPPVariables* CCPPUnit::GetLocals(void) { return &mapsFunctionScopes.GetPtr(mapsFunctionScopes.NumElements() - 1)->apcLocals; }
CCPPFunctionPacks* CCPPUnit::GetFunctionPacks(void) { return &mapsFunctionScopes.GetPtr(mapsFunctionScopes.NumElements() - 1)->cFunctionPacks; }
CArrayLargeTemplate<uint16>* CCPPUnit::GetCodePieces(void) { return &mauiCodePieces; }
int* CCPPUnit::GetLabelCounter(void) { return &miLabels; }
CCPPTarget* CCPPUnit::GetTarget(void) const { return mpcTarget; }
CIRImports* CCPPUnit::GetImports(void) const { return mpcImports; }
CCPPTokeniser* CCPPUnit::GetTokeniser(void) const { return mpcTokeniser; }
CCPPSyntaxTree* CCPPUnit::GetTree(void) const { return mpcTree; }
CSpillStore* CCPPUnit::GetSpillStore(void) const { return mpcSpillStore; }
CCPPSemanticPass* CCPPUnit::GetSemanticPass(void) const { return mpcSemanticPass; }
CAsm65Profile* CCPPUnit::GetFrameProfile(void) const { return mpcFrameProfile; }
uint32 CCPPUnit::GetSpillUnit(void) const { return muiSpillUnit; }
size CCPPUnit::GetImportsTurn(void) const { return muiImportsTurn; }
bool CCPPUnit::HasLineTable(void) const { return mbLineTable; }
bool CCPPUnit::HasListing(void) const { return mbListing; }
bool CCPPUnit::HasNoFloats(void) const { return mbNoFloats; }
int64 CCPPUnit::GetCompileTime(void) const { return mlliCompileTime; }
