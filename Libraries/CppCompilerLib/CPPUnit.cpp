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
// NULL for the generator's own memory, and the unit's number in its keys
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
// What compiling the unit is counted and timed in (the generator's own
// when pcStatistics is NULL).  The IR front end's functions are counted in
// it too.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::SetStatistics(CCPPCompileStatistics* pcStatistics)
{
	mpcStatistics = pcStatistics ? pcStatistics : &mcOwnStatistics;
	mpcIRFrontEnd->SetStatistics(mpcStatistics);
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
	mbRemember = false;
	mcOwnStatistics.Init();
	mpcStatistics = &mcOwnStatistics;
	mpcOptions = GetDefaultCPPCompileOptions();

	//Every function body is written as IR (and generated by the target's IR back end).
	mpcIRFrontEnd = NewMalloc<CCPP65IRFrontEnd>();
	mpcIRFrontEnd->Init(this);
	mpcConstants = NewMalloc<CCPPConstants>();
	mpcConstants->Init(this);
	mcTypeChecker.Init(this);
	mcErrors.Init();
	masClassErrors.Init();
	mcWarnings.Init();

	mapcGlobals.Init();
	mapcFunctions.Init();
	mcFunctionsOfNames.Init(true);
	mcContext.Init();
	mcData.Init();
	mcBinder.Init(this);
	mcDeclarators.Init(this);
	mcMangler.Init(mpcTokeniser, &mapcClasses, &mcFunctionsOfNames);
	mapcClasses.Init();
	mapcTemplates.Init();
	mapcAllBindings.Init();
	mapapsPacks.Init();
	mapalliPacks.Init();
	mcFunctionPacks.Init();
	mapcObjectConstants.Init();
	masBracedLists.Init();
	mbValueArgumentRefused = false;
	masPackElements.Init();
	masExpandedArguments.Init();
	mapcPackSymbols.Init();
	mapcHiddenSymbols.Init();
	mcRangeFors.Init(true);
	mcLiteralCalls.Init(true);
	mcDecltypeMembers.Init(true);
	mcImplicitAssignments.Init(true);
	mcExplicitInstances.Init(true);
	mpcInitializerList = NULL;
	mpcTypeInfo = NULL;
	mapsLambdas.Init();
	mapcMemberSpecialisations.Init();
	mapcMemberTemplateSpecialisations.Init();
	mapcMemberClassTemplates.Init();
	mapcMemberVariableTemplates.Init();
	mapcStaticMemberTemplates.Init();
	mpcDefining = NULL;
	mapcVariableInstances.Init();
	miUnevaluated = 0;
	miSaysDeprecated = -1;
	mpcImplicitZero = NULL;
	mcImplicitValues.Init(true);
	mapcQueue.Init();
	muiQueueNext = 0;
	mapcFailed.Init();
	mcTypeOfs.Init(true);
	maszEmitted.Init();
	miWarningChecks = 0;
	mapcConstantTypes.Init();
	mbPureVirtual = false;
	mbBadCast = false;
	mbRecursiveInit = false;
	mbExitLists = false;
	mbExitListsWritten = false;
	mbDynamicCast = false;
	mbDynamicCastWritten = false;
	maszExitFinalisers.Init();
	mapsExitFinalisers.Init();
	maszThrownDestroyers.Init();
	mapsThrownDestroyers.Init();
	maiExitShared.Init();

	mapcLocals.Init();
	mapcStaticLocals.Init();
	miStatics = 0;
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
	CCPPClass*				pcClass;
	CCPPTemplateArguments*	pcBindings;
	CArrayCPP65Types*		papsPack;
	CArrayInt64*			palliPack;
	CArraySyntax*			papcExpanded;

	mcTypeChecker.Kill();
	SafeKill(mpcConstants);
	SafeKill(mpcIRFrontEnd);
	FreeVariables(&mapcLocals);
	mapcLocals.Kill();
	FreeVariables(&mapcStaticLocals);
	mapcStaticLocals.Kill();
	mcBackendScratch.Kill();

	for (i = 0; i < mapcClasses.NumElements(); i++)
	{
		pcClass = mapcClasses.GetPtr(i);
		pcClass->Kill();
		SafeFree(pcClass);
	}
	mapcClasses.Kill();
	for (i = 0; i < mapcFunctions.NumElements(); i++)
	{
		pcFunction = mapcFunctions.GetPtr(i);
		pcFunction->Kill();
		SafeFree(pcFunction);
	}
	mcMangler.Kill();
	mcDeclarators.Kill();
	mcBinder.Kill();
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
	for (i = 0; i < mapapsPacks.NumElements(); i++)
	{
		papsPack = mapapsPacks.GetPtr(i);
		SafeKill(papsPack);
	}
	mapapsPacks.Kill();
	for (i = 0; i < mapalliPacks.NumElements(); i++)
	{
		palliPack = mapalliPacks.GetPtr(i);
		SafeKill(palliPack);
	}
	mapalliPacks.Kill();
	mcFunctionPacks.Kill();
	mapcObjectConstants.Kill();
	masBracedLists.Kill();
	masPackElements.Kill();
	for (i = 0; i < masExpandedArguments.NumElements(); i++)
	{
		papcExpanded = masExpandedArguments.Get(i)->papcExpanded;
		if (papcExpanded)
		{
			papcExpanded->Kill();
			free(papcExpanded);
		}
	}
	masExpandedArguments.Kill();
	for (i = 0; i < mapcPackSymbols.NumElements(); i++)
	{
		free(mapcPackSymbols.GetPtr(i));
	}
	mapcPackSymbols.Kill();
	for (i = 0; i < mapcHiddenSymbols.NumElements(); i++)
	{
		free(mapcHiddenSymbols.GetPtr(i));
	}
	mapcHiddenSymbols.Kill();
	mcRangeFors.Kill();
	mcLiteralCalls.Kill();
	mcDecltypeMembers.Kill();
	mcImplicitAssignments.Kill();
	mcExplicitInstances.Kill();
	FreeLambdas();
	mapcMemberSpecialisations.Kill();
	mapcMemberTemplateSpecialisations.Kill();
	mapcStaticMemberTemplates.Kill();
	mapcMemberVariableTemplates.Kill();
	mapcMemberClassTemplates.Kill();
	mapcVariableInstances.Kill();
	mcImplicitValues.Kill();
	mapcConstantTypes.Kill();
	mapcQueue.Kill();
	mapcFailed.Kill();
	mcTypeOfs.Kill();
	maszEmitted.Kill();
	FreeVariables(&mapcGlobals);
	mapcGlobals.Kill();

	mcWarnings.Kill();
	masClassErrors.Kill();
	mcErrors.Kill();
	maiExitShared.Kill();
	mapsThrownDestroyers.Kill();
	maszThrownDestroyers.Kill();
	mapsExitFinalisers.Kill();
	maszExitFinalisers.Kill();
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
		ErrorAt(-1, "Internal error: the compiler's context was not left as it was entered");
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
	DefineInstanceData();
	while (WriteExitRoutines() || WriteThrownDestroyers() || WriteDynamicCast())
	{
		GenerateQueued();
	}
	CheckUncalledFunctions();
	ReportClassErrors();
	if (!mbCompiledBefore)
	{
		ReportNamedInstances();
	}
	CheckContext();
	mpcStatistics->End(CPPCP_Declarations);

	//The IR front end inlines across the unit.
	mpcStatistics->Begin(CPPCP_Inline);
	mpcIRFrontEnd->InlineUnit();
	mpcStatistics->End(CPPCP_Inline);
	return mcErrors.NumErrors() == 0;
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
	return mcErrors.NumErrors() == 0;
}


//////////////////////////////////////////////////////////////////////////
//
// A global reference (pszLabel's) that pcInitialiser binds to a temporary
// (const long& r = 7, BindsToTemporary): the temporary is a global of its
// own, pszBound (pszLabel and __Bound), made from the initialiser and
// destroyed as a global of its type is.  Its data goes before the
// reference's, which is its address, or its base's *piOffset into it.
// pszBound is left empty for any other initialiser.  False (and *pbResult cleared) after an error.
// [class.temporary] 6
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::BoundTemporary(SCPP65Type* psReference, CCPPSyntax* pcInitialiser, CChars* pszLabel, bool bGlobal, const char* szOptions, CChars* pszBound, int* piOffset, bool* pbResult)
{
	SCPP65Type*	psType;
	CCPPSyntax*	pcValue;
	bool		bClass;

	pcValue = pcInitialiser;
	if (pcValue && (pcValue->GetKind() == CPPS_ParenthesisedInitializer) && (ListElements(pcValue)->NumElements() == 1))
	{
		pcValue = ListElements(pcValue)->GetPtr(0);
	}
	if (!pcValue || (pcValue->GetKind() == CPPS_InitializerList))
	{
		return true;
	}
	psType = psReference->psBase->Unqualified();
	if (!BindsToTemporary(psReference, pcValue))
	{
		//const S& r = make() and const S& r = D(): the object made for the reference, which may be of a class derived from S.
		psType = TypeOf(pcValue);
		if (!psReference->psBase->IsClass() || !psType || !psType->IsClass() || IsObjectLvalue(pcValue))
		{
			return true;
		}
	}

	bClass = psType->IsClass();
	if (bClass && !CompleteObjectClass(psType->pcClass, pcInitialiser))
	{
		*pbResult = false;
		return false;
	}
	*piOffset = 0;
	if (bClass && (psType->pcClass != psReference->psBase->pcClass))
	{
		CompleteOffset(psType->pcClass, psReference->psBase->pcClass, piOffset);
	}
	pszBound->Append(pszLabel);
	pszBound->Append("__Bound");
	mcData.BeginData(pszBound, DataAlignment(psType, bClass), bGlobal, szOptions, false);
	mcData.AppendZeros(psType->iBytes);
	*pbResult &= GlobalStartUpRoutines(psType, pcInitialiser, pszBound, true, bClass && (psType->pcClass->mpcDestructor != NULL), false);
	mcData.NextStartUp();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A global's R_Initialise and R_Finalise are written by the IR front end (CCPP65IRFrontEnd::GlobalRoutine).  bConstruct when it needs constructing, bDestroy when it needs
// destroying.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::GlobalStartUpRoutines(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CChars* pszData, bool bConstruct, bool bDestroy, bool bThread)
{
	CChars	szLabel;
	CChars	szFinaliser;
	CChars	szNode;
	bool	bResult;
	bool	bListed;
	int		i;

	//One made when the program starts is put on an exit list once it has been (bThread: a thread_local's), so objects
	//are destroyed in the reverse of the order they were made, with the atexit functions between them; one made in the
	//data, before any of those, is in the fini table, run after the lists.  [basic.start.term] 1, 3
	bListed = bConstruct && bDestroy;
	szFinaliser.Init("R_Finalise");
	szFinaliser.Append(mcData.GetStartUp());
	szNode.Init(pszData);
	szNode.Append("__ExitNode");
	bResult = true;
	for (i = 0; i < 2; i++)
	{
		if (!((i == 0) ? bConstruct : bDestroy))
		{
			continue;
		}
		szLabel.Init((i == 0) ? "R_Initialise" : "R_Finalise");
		szLabel.Append(mcData.GetStartUp());
		CCPPUnitData::AppendSection(&mszCode, "text", &szLabel, NULL);
		if (mpcIRFrontEnd->GlobalRoutine(szLabel.Text(), pszData->Text(), psType, pcInitialiser, i == 1, ((i == 0) && bListed) ? szNode.Text() : NULL, szFinaliser.Text(), bThread))
		{
			if ((i == 0) || !bListed)
			{
				mcData.AppendStartUpEntry((i == 0) ? "init" : "fini", &szLabel, pszData, mpcOptions->mbKeepUnusedObjects);
			}
		}
		else
		{
			bResult = ErrorAt(-1, "This global's construction or destruction uses C++ the IR front end doesn't support yet");
		}
		szLabel.Kill();
	}
	if (bListed)
	{
		mcData.AppendExitNode(&szNode, pszData, 2 * mcTypes.Pointer(&mcTypes.msVoid)->iBytes);
	}
	szNode.Kill();
	szFinaliser.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// True if every unit that needs the function writes it, each in a COMDAT
// section the linker keeps one of (inline, a template's instance, or
// implicit), so what is in it is one thing in every unit: its lambdas'
// closures and its static locals.  [basic.def.odr] 6, [dcl.fct.spec] 4
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsSharedFunction(CCPPFunction* pcFunction)
{
	return pcFunction && !pcFunction->IsUnitFunction() && (pcFunction->mbInline || pcFunction->mbImplicit || pcFunction->mpcTemplate);
}


//////////////////////////////////////////////////////////////////////////
//
// What throw's operand makes can be thrown: not an object of an incomplete
// or abstract class, nor a pointer to an incomplete class.  [except.throw] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckThrown(CCPPSyntaxThrowExpression* pcThrow)
{
	SCPP65Type*	psType;

	psType = TypeOf(pcThrow->mpcOperand);
	if (!psType)
	{
		return false;
	}
	psType = psType->WithoutReference()->Unqualified();
	if (psType->IsVoid())
	{
		return Error(pcThrow, "A throw's operand can't be void");
	}
	if (psType->IsClass())
	{
		if (!CompleteClass(psType->pcClass, pcThrow))
		{
			return false;
		}
		if (psType->pcClass->IsAbstract())
		{
			return Error(pcThrow, "An object of an abstract class can't be thrown");
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The innermost try block or handler pcNode is in, looking no further out
// than pcStop: the block's or the handler's node, or NULL for none.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPUnit::TryRegionOf(CCPPSyntax* pcNode, CCPPSyntax* pcStop)
{
	CCPPSyntax*	pcParent;

	for (pcParent = pcNode->mpcParent; pcParent && (pcNode != pcStop); pcNode = pcParent, pcParent = pcParent->mpcParent)
	{
		if (pcParent->GetKind() == CPPS_CatchHandler)
		{
			return pcParent;
		}
		if ((pcParent->GetKind() == CPPS_TryStatement) && (((CCPPSyntaxTryStatement*)pcParent)->mpcBlock == pcNode))
		{
			return pcNode;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The body's labels and gotos, for CheckTryJumps; and each case label is
// checked here: it can't be in a try block or a handler that is inside its
// switch.  A lambda's body is its own function's.  [except] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CollectTryJumps(CCPPSyntax* pcNode, CCPPSyntax* pcBody, CArraySyntax* papcLabels, CArraySyntax* papcGotos)
{
	CCPPSyntax*	pcSwitch;
	size		i;
	bool		bResult;

	if (!pcNode || (pcNode->GetKind() == CPPS_LambdaExpression))
	{
		return true;
	}
	bResult = true;
	if (pcNode->GetKind() == CPPS_LabelStatement)
	{
		papcLabels->Add(pcNode);
	}
	else if (pcNode->GetKind() == CPPS_GotoStatement)
	{
		papcGotos->Add(pcNode);
	}
	else if (pcNode->GetKind() == CPPS_CaseStatement)
	{
		for (pcSwitch = pcNode->mpcParent; pcSwitch && (pcSwitch->GetKind() != CPPS_SwitchStatement) && (pcSwitch != pcBody); pcSwitch = pcSwitch->mpcParent)
		{
		}
		if (pcSwitch && (pcSwitch->GetKind() == CPPS_SwitchStatement) && TryRegionOf(pcNode, pcSwitch))
		{
			bResult = Error(pcNode, "A switch can't jump into a try block or a handler");
		}
	}
	for (i = 0; i < pcNode->NumChildren(); i++)
	{
		bResult &= CollectTryJumps(pcNode->GetChild(i), pcBody, papcLabels, papcGotos);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// No goto or switch of the function's body jumps into a try block or a
// handler: a goto whose label is in one must be in it too.  [except] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckTryJumps(CCPPSyntax* pcBody)
{
	CArraySyntax	apcLabels;
	CArraySyntax	apcGotos;
	CCPPSyntax*		pcGoto;
	CCPPSyntax*		pcLabel;
	CCPPSyntax*		pcRegion;
	CCPPSyntax*		pcParent;
	CCPPToken*		pcGotoName;
	CCPPToken*		pcLabelName;
	size			i;
	size			j;
	bool			bResult;

	if (!mpcOptions->mbExceptions || !pcBody)
	{
		return true;
	}
	apcLabels.Init();
	apcGotos.Init();
	bResult = CollectTryJumps(pcBody, pcBody, &apcLabels, &apcGotos);
	for (i = 0; i < apcGotos.NumElements(); i++)
	{
		pcGoto = apcGotos.GetPtr(i);
		pcGotoName = mpcTokeniser->Get(((CCPPSyntaxGotoStatement*)pcGoto)->miLabel);
		for (j = 0; j < apcLabels.NumElements(); j++)
		{
			pcLabel = apcLabels.GetPtr(j);
			pcLabelName = mpcTokeniser->Get(((CCPPSyntaxLabelStatement*)pcLabel)->miName);
			if (!pcGotoName || !pcLabelName || !pcGotoName->IsIdentifier() || !pcLabelName->IsIdentifier() ||
				(((CCTIdentifier*)pcGotoName)->miIdentifier != ((CCTIdentifier*)pcLabelName)->miIdentifier))
			{
				continue;
			}
			pcRegion = TryRegionOf(pcLabel, pcBody);
			for (pcParent = pcGoto; pcRegion && pcParent && (pcParent != pcRegion); pcParent = pcParent->mpcParent)
			{
			}
			if (pcRegion && !pcParent)
			{
				bResult = Error(pcGoto, "A goto can't jump into a try block or a handler");
			}
		}
	}
	apcGotos.Kill();
	apcLabels.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The type a handler is written with (catch (const E& e): const E&); NULL,
// with the error said, if it can't be worked out.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::HandlerType(CCPPSyntaxCatchHandler* pcHandler)
{
	if (!pcHandler->mpcParameter)
	{
		return NULL;
	}
	return mcDeclarators.GetType(pcHandler->mpcParameter->mpcSpecifiers, pcHandler->mpcParameter->mpcDeclarator, pcHandler->mpcParameter, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// A handler's parameter is a local variable of the function the handler is
// in.  Its type can't be an rvalue reference, or an incomplete or abstract
// class (a pointer or reference to an incomplete one neither, but void's).
// [except.handle] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::HandlerLocal(CCPPSyntaxCatchHandler* pcHandler)
{
	CCPPSyntaxTryStatement*	pcTry;
	SCPP65Type*				psType;
	SCPP65Type*				psObject;

	if (!pcHandler->mpcParameter)
	{
		//catch (...) is its try's last handler.  [except.handle] 5
		pcTry = (pcHandler->mpcParent && (pcHandler->mpcParent->GetKind() == CPPS_TryStatement)) ? (CCPPSyntaxTryStatement*)pcHandler->mpcParent : NULL;
		if (pcTry && (pcTry->macpHandlers.NumElements() > 0) && (pcTry->macpHandlers.GetPtr(pcTry->macpHandlers.NumElements() - 1) != pcHandler))
		{
			return Error(pcHandler, "catch (...) must be the last handler of its try");
		}
		return true;
	}
	psType = HandlerType(pcHandler);
	if (!psType)
	{
		return false;
	}
	if (psType->IsReference() && psType->bRvalue)
	{
		return Error(pcHandler->mpcParameter, "A handler's parameter can't be an rvalue reference");
	}
	psObject = psType->WithoutReference()->Unqualified();
	if (psObject->IsArray() || psObject->IsFunction())
	{
		return Error(pcHandler->mpcParameter, "A handler of an array or a function is not supported by the compiler yet");
	}
	if (psObject->IsClass() && !CompleteObjectClass(psObject->pcClass, pcHandler->mpcParameter))
	{
		return false;
	}
	if (pcHandler->mpcParameter->mpcSymbol)
	{
		AddVariable(&mapcLocals, pcHandler->mpcParameter->mpcSymbol, psType, CPPVS_Local, 0);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The label of the function that destroys a thrown object of psType (a
// class with a destructor), for the runtime to call when the exception's
// last handler is left; the function is written with the unit's others
// (WriteThrownDestroyers), in a COMDAT section, once.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::ThrownDestroyer(SCPP65Type* psType, CChars* pszLabel)
{
	CChars	szLabel;
	size	i;

	szLabel.Init("R_DestroyThrown");
	mcMangler.AppendType(&szLabel, psType);
	pszLabel->Append(&szLabel);
	for (i = 0; i < maszThrownDestroyers.NumElements(); i++)
	{
		if (szLabel.Equals(maszThrownDestroyers.GetText(i)))
		{
			szLabel.Kill();
			return;
		}
	}
	if (Emit(&szLabel))
	{
		maszThrownDestroyers.Add(szLabel.Text());
		mapsThrownDestroyers.Add(psType);
	}
	szLabel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Once the unit's functions are written: the functions that destroy the
// thrown objects of the classes they throw (ThrownDestroyer).  True if it
// wrote any (which may queue destructors to generate).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::WriteThrownDestroyers(void)
{
	CChars	szLabel;
	size	i;
	bool	bWritten;

	bWritten = false;
	for (i = 0; i < maszThrownDestroyers.NumElements(); i++)
	{
		szLabel.Init(maszThrownDestroyers.GetText(i));
		CCPPUnitData::AppendSection(&mszCode, "text", &szLabel, ", comdat");
		CCPPUnitData::AppendGlobal(&mszCode, &szLabel);
		if (!mpcIRFrontEnd->ThrownDestroyerRoutine(szLabel.Text(), mapsThrownDestroyers.GetPtr(i)))
		{
			ErrorAt(-1, "This thrown object's destruction uses C++ the IR front end doesn't support yet");
		}
		szLabel.Kill();
		bWritten = true;
	}
	maszThrownDestroyers.ReInit();
	mapsThrownDestroyers.ReInit();
	return bWritten;
}


//////////////////////////////////////////////////////////////////////////
//
// Once the unit's functions are written: the R_Finalise of each static
// local destroyed at exit that they reach (StaticLocal), and the exit
// lists' runner if anything is pushed on them (R_RunExitLists, a COMDAT
// section any unit may write).  True if it wrote anything (which may
// queue destructors to generate).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::WriteExitRoutines(void)
{
	CChars	szFinaliser;
	CChars	szLabel;
	size	i;
	bool	bWritten;

	bWritten = false;
	for (i = 0; i < maszExitFinalisers.NumElements(); i++)
	{
		szFinaliser.Init("R_Finalise");
		szFinaliser.Append(maszExitFinalisers.GetText(i));
		CCPPUnitData::AppendSection(&mszCode, "text", &szFinaliser, maiExitShared.GetValue(i) ? ", comdat" : NULL);
		if (maiExitShared.GetValue(i))
		{
			CCPPUnitData::AppendGlobal(&mszCode, &szFinaliser);
		}
		if (!mpcIRFrontEnd->GlobalRoutine(szFinaliser.Text(), maszExitFinalisers.GetText(i), mapsExitFinalisers.GetPtr(i), NULL, true, NULL, NULL, false, maiExitShared.GetValue(i) == 0))
		{
			ErrorAt(-1, "This static local's destruction uses C++ the IR front end doesn't support yet");
		}
		szFinaliser.Kill();
		bWritten = true;
	}
	maszExitFinalisers.ReInit();
	mapsExitFinalisers.ReInit();
	maiExitShared.ReInit();

	if (mbExitLists && !mbExitListsWritten)
	{
		mbExitListsWritten = true;
		szLabel.Init("R_RunExitLists");
		if (Emit(&szLabel))
		{
			CCPPUnitData::AppendSection(&mszCode, "text", &szLabel, ", comdat");
			CCPPUnitData::AppendGlobal(&mszCode, &szLabel);
			mpcIRFrontEnd->ExitListsRoutine(szLabel.Text());
			bWritten = true;
		}
		szLabel.Kill();
	}
	return bWritten;
}


//////////////////////////////////////////////////////////////////////////
//
// R_DynamicCast, the routine the unit's dynamic_casts call (UseDynamicCast),
// in a section the linker keeps one of, once.  True if it was written now.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::WriteDynamicCast(void)
{
	CChars	szLabel;
	bool	bWritten;

	if (!mbDynamicCast || mbDynamicCastWritten)
	{
		return false;
	}
	mbDynamicCastWritten = true;
	bWritten = false;
	szLabel.Init(CPP65_DYNAMIC_CAST);
	if (Emit(&szLabel))
	{
		CCPPUnitData::AppendSection(&mszCode, "text", &szLabel, ", comdat");
		CCPPUnitData::AppendGlobal(&mszCode, &szLabel);
		mpcIRFrontEnd->DynamicCastRoutine(szLabel.Text());
		bWritten = true;
	}
	szLabel.Kill();
	return bWritten;
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
// A global or static local with no initialiser starts as zeros: on a
// target whose start up clears bss (CCPPTarget::HasZeroedBss) it goes
// there, and takes no space in the program's image.  An object is cleared
// there too, before its constructor runs from the init table.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsBss(CCPPSyntax* pcInitialiser, SCPP65Type* psType)
{
	return mpcTarget->HasZeroedBss() && !pcInitialiser && !psType->IsReference() && (psType->iBytes > 0);
}


//////////////////////////////////////////////////////////////////////////
//
// 4 for objects and data of 4 bytes or more, 2 for 2 bytes, otherwise 1.
//
//////////////////////////////////////////////////////////////////////////
int CCPPUnit::DataAlignment(SCPP65Type* psType, bool bClass)
{
	if ((psType->iBytes >= 4) || bClass)
	{
		return 4;
	}
	if (psType->iBytes == 2)
	{
		return 2;
	}
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
// A variable's data's alignment: DataAlignment, or more where its type or
// alignas asks it (VariableAlignment).
//
//////////////////////////////////////////////////////////////////////////
int CCPPUnit::VariableDataAlignment(CCPPVariable* pcVariable, bool bClass)
{
	int		iAlign;
	int		iAlignas;
	int		iVariable;

	iAlign = DataAlignment(pcVariable->mpsType, bClass);
	iVariable = VariableAlignment(pcVariable, &iAlignas);
	return (iVariable > iAlign) ? iVariable : iAlign;
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
// __attribute__((used)) (or __used__) on a declarator: the global is kept even if
// nothing uses it (its constructor may register it somewhere).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::HasUsedAttribute(CCPPSyntaxDeclarator* pcDeclarator)
{
	return HasAttribute(pcDeclarator, "used");
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the declarator has __attribute__((szName)) or ((__szName__)):
// after it, after a function's parameters (int f(int) __attribute__((noinline));),
// where it is among the qualifiers (const, volatile) or after the exception
// specification, or before the declaration among its specifiers
// (__attribute__((noinline)) static int f(int), as BaseLib's NO_INLINE).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::HasAttribute(CCPPSyntaxDeclarator* pcDeclarator, const char* szName)
{
	return FindAttribute(pcDeclarator, szName) != -1;
}


//////////////////////////////////////////////////////////////////////////
//
// HasAttribute's attribute: the token that names it, or -1 if the
// declarator has none of the name.
//
//////////////////////////////////////////////////////////////////////////
int32 CCPPUnit::FindAttribute(CCPPSyntaxDeclarator* pcDeclarator, const char* szName)
{
	CCPPSyntaxFunctionSuffix*	pcFunction;
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;
	CCPPSyntax*					pcDeclaration;
	int32						iAttribute;

	if (!pcDeclarator)
	{
		return -1;
	}
	pcFunction = pcDeclarator->GetFunctionSuffix();
	iAttribute = FindAttribute(&pcDeclarator->macpAttributes, szName);
	if ((iAttribute == -1) && pcFunction)
	{
		iAttribute = FindAttribute(&pcFunction->macpQualifiers, szName);
		iAttribute = (iAttribute != -1) ? iAttribute : FindAttribute(&pcFunction->macpAttributes, szName);
	}
	if (iAttribute != -1)
	{
		return iAttribute;
	}

	//The declaration the declarator is in (through its init-declarator).
	pcSpecifiers = NULL;
	for (pcDeclaration = pcDeclarator->mpcParent; pcDeclaration && !pcSpecifiers; pcDeclaration = pcDeclaration->mpcParent)
	{
		if (pcDeclaration->GetKind() == CPPS_SimpleDeclaration)
		{
			pcSpecifiers = ((CCPPSyntaxSimpleDeclaration*)pcDeclaration)->mpcSpecifiers;
		}
		else if (pcDeclaration->GetKind() == CPPS_FunctionDefinition)
		{
			pcSpecifiers = ((CCPPSyntaxFunctionDefinition*)pcDeclaration)->mpcSpecifiers;
		}
		else if ((pcDeclaration->GetKind() != CPPS_InitDeclarator) && (pcDeclaration->GetKind() != CPPS_Declarator))
		{
			break;
		}
	}
	return pcSpecifiers ? FindAttribute(&pcSpecifiers->macpSpecifiers, szName) : -1;
}


//////////////////////////////////////////////////////////////////////////
//
// The token that names the attribute szName, or __szName__, in the list
// (what else is in it, qualifiers, isn't looked at), or -1 if none does.
//
//////////////////////////////////////////////////////////////////////////
int32 CCPPUnit::FindAttribute(CArraySyntax* papcAttributes, const char* szName)
{
	size		i;
	int32		iToken;
	CCPPSyntax*	pcAttribute;
	CCPPToken*	pcToken;
	char*		szIdentifier;
	int			iLength;
	size		uiName;

	uiName = strlen(szName);
	for (i = 0; i < papcAttributes->NumElements(); i++)
	{
		pcAttribute = papcAttributes->GetPtr(i);
		for (iToken = pcAttribute->miFirstToken; (pcAttribute->GetKind() == CPPS_Attribute) && (iToken <= pcAttribute->miLastToken); iToken++)
		{
			pcToken = mpcTokeniser->Get(iToken);
			if (pcToken->IsIdentifier())
			{
				szIdentifier = mpcTokeniser->GetIdentifier(((CCTIdentifier*)pcToken)->miIdentifier, &iLength);
				if ((((size)iLength == uiName) && (memcmp(szIdentifier, szName, uiName) == 0)) ||
					(((size)iLength == uiName + 4) && (memcmp(szIdentifier, "__", 2) == 0) && (memcmp(szIdentifier + 2, szName, uiName) == 0) && (memcmp(szIdentifier + 2 + uiName, "__", 2) == 0)))
				{
					return iToken;
				}
			}
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// The token of the [[deprecated]] the symbol's declaration is marked with
// (a variable, a typedef, a member, a class, an enumeration, an
// enumerator), or -1.  [dcl.attr.deprecated] 2
//
//////////////////////////////////////////////////////////////////////////
int32 CCPPUnit::DeprecatedAttribute(CCPPSymbol* pcSymbol)
{
	CCPPSyntax*	pcDeclaration;

	pcDeclaration = pcSymbol->mpcDeclaration;
	if (!pcDeclaration)
	{
		return -1;
	}
	switch (pcDeclaration->GetKind())
	{
	case CPPS_InitDeclarator:
		return FindAttribute(((CCPPSyntaxInitDeclarator*)pcDeclaration)->mpcDeclarator, "deprecated");
	case CPPS_ClassSpecifier:
		return FindAttribute(&((CCPPSyntaxClassSpecifier*)pcDeclaration)->macpAttributes, "deprecated");
	case CPPS_ElaboratedTypeSpecifier:
		return FindAttribute(&((CCPPSyntaxElaboratedTypeSpecifier*)pcDeclaration)->macpAttributes, "deprecated");
	case CPPS_EnumSpecifier:
		return FindAttribute(&((CCPPSyntaxEnumSpecifier*)pcDeclaration)->macpAttributes, "deprecated");
	case CPPS_Enumerator:
		return FindAttribute(&((CCPPSyntaxEnumerator*)pcDeclaration)->macpAttributes, "deprecated");
	default:
		return -1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the source says deprecated anywhere: where it doesn't, which is
// nearly everywhere, no name is asked whether it is (WarnDeprecatedSymbol).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::SaysDeprecated(void)
{
	CCPPToken*	pcToken;
	int64		lliDeprecated;
	int64		lliUnderscored;
	size		i;

	if (miSaysDeprecated == -1)
	{
		miSaysDeprecated = 0;
		lliDeprecated = mpcTokeniser->GetIdentifierID((char*)"deprecated");
		lliUnderscored = mpcTokeniser->GetIdentifierID((char*)"__deprecated__");
		for (i = 0; (miSaysDeprecated == 0) && (i < mpcTokeniser->NumTokens()); i++)
		{
			pcToken = mpcTokeniser->Get((int32)i);
			if (pcToken->IsIdentifier() && ((((CCTIdentifier*)pcToken)->miIdentifier == lliDeprecated) || (((CCTIdentifier*)pcToken)->miIdentifier == lliUnderscored)))
			{
				miSaysDeprecated = 1;
			}
		}
	}
	return miSaysDeprecated == 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPErrors* CCPPUnit::GetErrors(void)
{
	//Their tokens are C++ token indices.
	return &mcErrors;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPErrors* CCPPUnit::GetWarnings(void)
{
	return &mcWarnings;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCPPUnit::NumErrors(void) const
{
	return (int)mcErrors.NumErrors();
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
// A check for a refactoring records where an enum's value would need a
// cast if it were an enum class, instead of refusing it there
// (CCPPSemanticPass::SetRecordedEnum).  True if it was recorded.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::RecordEnumCast(CCPPSyntax* pcNode, SCPP65Type* psFrom, SCPP65Type* psTo, uint8 uiKind)
{
	return mpcSemanticPass && mpcSemanticPass->RecordEnumCast(pcNode, psFrom, psTo, uiKind);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsRecordedEnum(SCPP65Type* psType)
{
	return mpcSemanticPass && mpcSemanticPass->IsRecordedEnum(psType);
}


//////////////////////////////////////////////////////////////////////////
//
// Errors are "File (line, column): message [token]", like the other stages'.
// In quiet mode (declarations, which only fail when what they declare is used) they
// are only noted.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::Error(CCPPSyntax* pcNode, const char* szMessage)
{
	if (mcContext.IsQuiet())
	{
		mcContext.SetFailed();
		return false;
	}
	return ErrorAt(pcNode ? pcNode->miFirstToken : -1, szMessage);
}


//////////////////////////////////////////////////////////////////////////
//
// A warning at pcNode's first token, unless that token already has one (the
// same expression is often looked at more than once).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::Warning(CCPPSyntax* pcNode, const char* szMessage)
{
	SPreprocessedToken*	psSource;
	CChars				szMessageText;
	CChars				szDetail;
	int					iToken;
	size				i;

	iToken = pcNode ? pcNode->miFirstToken : -1;
	if ((iToken < 0) || ((size)iToken >= mpcTokeniser->NumTokens()))
	{
		return;
	}
	for (i = 0; i < mcWarnings.NumErrors(); i++)
	{
		if (mcWarnings.GetToken(i) == iToken)
		{
			return;
		}
	}

	psSource = mpcTokeniser->GetSource(mpcTokeniser->Get(iToken));
	szMessageText.Init("Warning: ");
	szMessageText.Append(szMessage);
	AppendMacroUse(psSource, &szMessageText);
	szDetail.Init();
	psSource->Append(&szDetail);
	mpcTokeniser->mpcSource->AddError(&mcWarnings, psSource, iToken, szMessageText.Text(), szDetail.Text());
	szDetail.Kill();
	szMessageText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The warning that what pcNode names (szWhat: a function, a variable ...)
// is deprecated, with what its attribute says of it, if it says anything:
// [[deprecated("use G")]].  iAttribute: the attribute's token.
// [dcl.attr.deprecated] 1, 4
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::WarnDeprecated(CCPPSyntax* pcNode, const char* szWhat, int32 iAttribute)
{
	CChars		szText;
	CCPPToken*	pcOpen;
	CCTString*	pcString;

	szText.Init("The ");
	szText.Append(szWhat);
	szText.Append(" is deprecated");
	pcOpen = ((size)iAttribute + 2 < mpcTokeniser->NumTokens()) ? mpcTokeniser->Get(iAttribute + 1) : NULL;
	if (pcOpen && pcOpen->IsReservedWord() && (((CCTReservedWord*)pcOpen)->GetToken() == CT_OpenRoundBracket) && mpcTokeniser->Get(iAttribute + 2)->IsString())
	{
		pcString = (CCTString*)mpcTokeniser->Get(iAttribute + 2);
		szText.Append(": ");
		szText.Append(pcString->mszStart + 1, (size)(pcString->mszEnd - pcString->mszStart - 1));
	}
	Warning(pcNode, szText.Text());
	szText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// WarnDeprecated for a name that isn't a function's: a variable, a member,
// an enumerator, or a type (a class, an enumeration, a typedef) whose
// declaration is marked [[deprecated]], where it is named outside that
// declaration.  [dcl.attr.deprecated] 2, 4
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::WarnDeprecatedSymbol(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode)
{
	const char*	szWhat;
	int32		iAttribute;

	if (!pcSymbol || !pcNode || !SaysDeprecated())
	{
		return;
	}
	switch (pcSymbol->meKind)
	{
	case CPPSK_Variable:	szWhat = "variable"; break;
	case CPPSK_Field:		szWhat = "member"; break;
	case CPPSK_Enumerator:	szWhat = "enumerator"; break;
	case CPPSK_Class:		szWhat = "class"; break;
	case CPPSK_Enum:		szWhat = "enumeration"; break;
	case CPPSK_Typedef:		szWhat = "type"; break;
	default:				return;
	}
	iAttribute = DeprecatedAttribute(pcSymbol);
	if ((iAttribute == -1) || ((pcNode->miFirstToken >= pcSymbol->mpcDeclaration->miFirstToken) && (pcNode->miFirstToken <= pcSymbol->mpcDeclaration->miLastToken)))
	{
		return;
	}
	WarnDeprecated(pcNode, szWhat, iAttribute);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a warning containing szText is at one of pcNode's tokens.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::HasWarning(CCPPSyntax* pcNode, const char* szText)
{
	size	i;
	int		iToken;

	for (i = 0; i < mcWarnings.NumErrors(); i++)
	{
		iToken = mcWarnings.GetToken(i);
		if ((iToken >= pcNode->miFirstToken) && (iToken <= pcNode->miLastToken) && strstr(mcWarnings.Get(i), szText))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::ErrorAt(int iToken, const char* szMessage)
{
	SPreprocessedToken*	psSource;
	CChars				szDetail;
	CChars				szMessageText;

	if ((iToken < 0) || ((size)iToken >= mpcTokeniser->NumTokens()))
	{
		mcErrors.Add(NULL, -1, -1, -1, szMessage);
		return false;
	}

	psSource = mpcTokeniser->GetSource(mpcTokeniser->Get(iToken));
	szMessageText.Init(szMessage);
	AppendMacroUse(psSource, &szMessageText);
	szDetail.Init();
	psSource->Append(&szDetail);
	mpcTokeniser->mpcSource->AddError(&mcErrors, psSource, iToken, szMessageText.Text(), szDetail.Text());
	szDetail.Kill();
	szMessageText.Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A token made by a macro is reported where its characters are (often in a header's #define), so where the outermost
// macro was used is added: " (in AssertInt at TestImage.cpp (12, 2))".
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::AppendMacroUse(SPreprocessedToken* psSource, CChars* pszMessage)
{
	SMacroExpansion*		psExpansion;
	CPreprocessedTokens*	pcSource;
	const char*				szFile;
	int						iLine;

	if (!psSource || (psSource->iExpansion < 0))
	{
		return;
	}
	pcSource = mpcTokeniser->mpcSource;
	psExpansion = pcSource->GetExpansion((size)psSource->iExpansion);
	while (psExpansion->iParent >= 0)
	{
		psExpansion = pcSource->GetExpansion((size)psExpansion->iParent);
	}
	if (!psExpansion->pcFile)
	{
		return;
	}
	pszMessage->Append(" (in ");
	pszMessage->Append(psExpansion->szName, (size)psExpansion->uiNameLength);
	pszMessage->Append(" at ");
	szFile = psExpansion->pcFile->ShortName();
	iLine = psExpansion->iLine;
	pcSource->ReportedPosition(psExpansion->pcFile, &szFile, &iLine);
	pszMessage->Append(szFile);
	pszMessage->Append(" (");
	pszMessage->Append(iLine + 1);
	pszMessage->Append(", ");
	pszMessage->Append(psExpansion->iColumn + 1);
	pszMessage->Append("))");
}


//////////////////////////////////////////////////////////////////////////
//
// An error in the C++ the IR front end found (break outside a loop), reported
// in quiet mode too.  Returns false.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::ReportError(CCPPSyntax* pcNode, const char* szMessage)
{
	BeginLoud();
	Error(pcNode, szMessage);
	EndLoud();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// An error reported (as ReportError) unless the same one is already there
// for the place: for what is worked out once and asked about again, whose
// error must be there however often it is asked, and however often what
// was asked only quietly took its errors away.  Always false.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::ReportOnce(CCPPSyntax* pcNode, const char* szMessage)
{
	size	i;

	for (i = 0; pcNode && (i < mcErrors.NumErrors()); i++)
	{
		if ((mcErrors.GetToken(i) == pcNode->miFirstToken) && strstr(mcErrors.Get(i), szMessage))
		{
			return false;
		}
	}
	return ReportError(pcNode, szMessage);
}


//////////////////////////////////////////////////////////////////////////
//
// ReportError at a token.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::ReportErrorAt(int iToken, const char* szMessage)
{
	BeginLoud();
	ErrorAt(iToken, szMessage);
	EndLoud();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Declarations of things the generator cannot do yet are only errors when they
// are used.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::BeginQuiet(void)
{
	if (mcContext.BeginQuiet())
	{
		mpcStatistics->Count(CPPCC_QuietTrials);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Returns false if an error happened since BeginQuiet.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::EndQuiet(void)
{
	bool	bPassed;

	bPassed = mcContext.EndQuiet();
	if (!bPassed && !mcContext.IsQuiet())
	{
		mpcStatistics->Count(CPPCC_QuietTrialsFailed);
	}
	return bPassed;
}


//////////////////////////////////////////////////////////////////////////
//
// Something is tried that may fail, and whether it does is its own: EndTrial says
// whether it made an error, and the quiet code around it (if any) hasn't failed
// because it did.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::BeginTrial(void)
{
	if (mcContext.BeginTrial())
	{
		mpcStatistics->Count(CPPCC_QuietTrials);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::EndTrial(void)
{
	bool	bPassed;

	bPassed = mcContext.EndTrial();
	if (!bPassed && !mcContext.IsQuiet())
	{
		mpcStatistics->Count(CPPCC_QuietTrialsFailed);
	}
	return bPassed;
}


//////////////////////////////////////////////////////////////////////////
//
// What is an error where it is, in quiet mode too, is reported: until EndLoud
// nothing is quiet, and after it what was quiet is, having failed or not as it
// had.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::BeginLoud(void)
{
	mcContext.BeginLoud();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::EndLoud(void)
{
	mcContext.EndLoud();
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
	return pcClass->mpcSymbol && (pcClass->mpcSymbol->meKind == CPPSK_Class) && !GetComponentTemplate(pcClass);
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
			bResult &= TemplateDeclaration((CCPPSyntaxTemplateDeclaration*)pcDeclaration);
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
			bResult &= GlobalDeclaration((CCPPSyntaxSimpleDeclaration*)pcDeclaration);
			break;
		case CPPS_LinkageSpecification:
			//extern "C" int x; declares x, as if it were extern; extern "C" { int x; } defines it.  [dcl.link]
			pcLinkage = (CCPPSyntaxLinkageSpecification*)pcDeclaration;
			mcContext.EnterLinkage(IsCLinkage(pcLinkage), !pcLinkage->mbBraced);
			bResult &= Declarations(&pcLinkage->macpDeclarations);
			mcContext.LeaveLinkage();
			break;
		case CPPS_TemplateDeclaration:
			bResult &= TemplateDeclaration((CCPPSyntaxTemplateDeclaration*)pcDeclaration);
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
				bResult &= Error(pcDeclaration, "This declaration is not supported by the compiler yet");
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
// Records a function's signature (DeclareSignature) and, where its return
// type is written after it or left to its body (auto), works that out.
// Returns NULL after an error.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPUnit::DeclareFunction(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, bool bDefinition, CCPPClass* pcClass, bool bPure, CCPPTemplate* pcTemplate, CChars* pszArguments)
{
	CCPPFunction*	pcFunction;

	pcFunction = DeclareSignature(pcSpecifiers, pcDeclarator, bDefinition, pcClass, bPure, pcTemplate, pszArguments);
	if (pcFunction && (pcFunction->mpsReturn == &mcTypes.msUndeduced) && !ResolveReturn(pcFunction, pcSpecifiers, pcDeclarator))
	{
		return NULL;
	}
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// A function's type: its return type, parameters, whether it is variadic,
// and whether it is a member function that is const or volatile.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::FunctionType(CCPPFunction* pcFunction)
{
	bool	bMember;

	bMember = pcFunction->mpcClass && !pcFunction->mbStatic;
	return mcTypes.Function(pcFunction->mpsReturn, &pcFunction->mapsParameters, pcFunction->mbVariadic, bMember && pcFunction->mbConstThis, bMember && pcFunction->mbVolatileThis);
}


//////////////////////////////////////////////////////////////////////////
//
// What an exception specification that is written allows: noexcept,
// noexcept(true) and throw() nothing, noexcept(false) and Microsoft's
// throw(...) anything, throw(A, B) those types.  [except.spec] 2 and 3
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::WrittenExceptions(CCPPSyntaxExceptionSpecification* pcSpecification, CCPPExceptionSpec* pcDest)
{
	SCPP65Type*	psType;
	int64		lliValue;
	size		i;

	pcDest->SetAnything();
	if (pcSpecification->mbNoexcept)
	{
		if (!pcSpecification->mpcCondition || (Constant(pcSpecification->mpcCondition, &lliValue) && (lliValue != 0)))
		{
			pcDest->SetNothing();
		}
		return;
	}
	if (pcSpecification->mbAnything)
	{
		return;
	}
	pcDest->SetNothing();
	pcDest->mbDynamic = true;
	for (i = 0; i < pcSpecification->macpTypes.NumElements(); i++)
	{
		psType = mcDeclarators.GetTypeIdType((CCPPSyntaxTypeId*)pcSpecification->macpTypes.GetPtr(i));
		if (psType)
		{
			pcDest->AddType(ParameterType(psType)->Unqualified());
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What the exception specification a function is declared with allows.
// noexcept(e)'s e may name the function's parameters, as the return type
// written after them may: noexcept(noexcept(t.f())).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::DeclaredExceptions(CCPPFunction* pcFunction, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntaxExceptionSpecification* pcSpecification, CCPPExceptionSpec* pcDest)
{
	SCPPFunctionScope	sScope;

	if (!pcSpecification->mpcCondition)
	{
		WrittenExceptions(pcSpecification, pcDest);
		return;
	}
	EnterFunctionScope(&sScope, pcFunction);
	AddParameters(pcFunction, pcDeclarator);
	WrittenExceptions(pcSpecification, pcDest);
	LeaveFunctionScope(&sScope);
}


//////////////////////////////////////////////////////////////////////////
//
// The exception specification a function's declarator is written with,
// NULL for none.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxExceptionSpecification* CCPPUnit::WrittenSpecification(CCPPSyntaxFunctionSuffix* pcSuffix)
{
	if (!pcSuffix || !pcSuffix->mpcExceptionSpecification || (pcSuffix->mpcExceptionSpecification->GetKind() != CPPS_ExceptionSpecification))
	{
		return NULL;
	}
	return (CCPPSyntaxExceptionSpecification*)pcSuffix->mpcExceptionSpecification;
}


//////////////////////////////////////////////////////////////////////////
//
// A new function's exception specification, from its declarator.  One
// declared with none allows anything, but: an operator delete allows
// nothing ([except.spec] 15), and a destructor's is worked out from its
// class when it is asked for (ExceptionsOf).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::SetDeclaredExceptions(CCPPFunction* pcFunction, CCPPSyntaxDeclarator* pcDeclarator)
{
	CCPPSyntaxExceptionSpecification*	pcSpecification;

	pcSpecification = WrittenSpecification(pcDeclarator->GetFunctionSuffix());
	if (pcSpecification)
	{
		DeclaredExceptions(pcFunction, pcDeclarator, pcSpecification, &pcFunction->mcExceptions);
		pcFunction->mbExceptionsWritten = true;
	}
	else if ((pcFunction->mlliName == GetOperatorName(CT_delete, false)) || (pcFunction->mlliName == GetOperatorName(CT_delete, true)))
	{
		pcFunction->mcExceptions.SetNothing();
		pcFunction->mbExceptionsWritten = true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A function declared again: its exception specification must be
// compatible with the one it has ([except.spec] 3).  A declaration
// without one has what the function would have with none written, which
// for a destructor is its class's parts' ([class.dtor] 3).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckRedeclaredExceptions(CCPPFunction* pcFunction, CCPPSyntaxDeclarator* pcDeclarator)
{
	CCPPSyntaxExceptionSpecification*	pcSpecification;
	CCPPExceptionSpec					cDeclared;
	bool								bCompatible;

	if (pcDeclarator == pcFunction->mpcDeclarator)
	{
		return true;
	}
	pcSpecification = WrittenSpecification(pcDeclarator->GetFunctionSuffix());
	cDeclared.Init();
	if (pcSpecification)
	{
		DeclaredExceptions(pcFunction, pcDeclarator, pcSpecification, &cDeclared);
	}
	else if ((pcFunction->mlliName == GetOperatorName(CT_delete, false)) || (pcFunction->mlliName == GetOperatorName(CT_delete, true)))
	{
		cDeclared.SetNothing();
	}
	else if ((pcFunction->meKind == CPPFK_Destructor) || pcFunction->mbImplicit)
	{
		if (!pcFunction->mbExceptionsWritten || !pcFunction->mpcClass || !pcFunction->mpcClass->IsComplete())
		{
			cDeclared.Kill();
			return true;
		}
		cDeclared.SetNothing();
		ImplicitExceptions(pcFunction, &cDeclared);
	}
	bCompatible = cDeclared.IsCompatible(ExceptionsOf(pcFunction));
	cDeclared.Kill();
	if (!bCompatible)
	{
		//Declarations are made quietly (one that fails is only an error if it is used); this one is wrong where it is.
		BeginLoud();
		Error(pcDeclarator, "The function is declared again with a different exception specification");
		EndLoud();
	}
	return bCompatible;
}


//////////////////////////////////////////////////////////////////////////
//
// What a function may throw.  One declared with a specification has that.
// A destructor declared without one, and a function the compiler writes,
// allow what the functions they call allow ([except.spec] 14): it is
// worked out here, once the class is complete.
//
//////////////////////////////////////////////////////////////////////////
CCPPExceptionSpec* CCPPUnit::ExceptionsOf(CCPPFunction* pcFunction)
{
	if (pcFunction->mbExceptionsWritten || pcFunction->mbExceptionsKnown)
	{
		return &pcFunction->mcExceptions;
	}
	pcFunction->mbExceptionsKnown = true;
	if (!pcFunction->mpcClass || ((pcFunction->meKind != CPPFK_Destructor) && !pcFunction->mbImplicit))
	{
		return &pcFunction->mcExceptions;
	}
	pcFunction->mcExceptions.SetNothing();
	ImplicitExceptions(pcFunction, &pcFunction->mcExceptions);
	pcFunction->mbExceptionsKnown = pcFunction->mpcClass->IsComplete();
	return &pcFunction->mcExceptions;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsNoexcept(CCPPFunction* pcFunction)
{
	return ExceptionsOf(pcFunction)->IsNonThrowing();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function's label is weak, so that another object's definition
// takes its place: __attribute__((weak)) on its definition, or on the
// declaration it is known by (Libc65's replaceable operator new and delete
// say it where they are defined, so that a program's own, declared by the
// same header, isn't).  [replacement.functions]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsWeakFunction(CCPPFunction* pcFunction)
{
	return HasAttribute(pcFunction->mpcDeclarator, "weak") || (pcFunction->mpcDefinition && HasAttribute(pcFunction->mpcDefinition->mpcDeclarator, "weak"));
}


//////////////////////////////////////////////////////////////////////////
//
// Adds to pcDest what a function with no specification written allows
// because of what it does to its class's bases and fields: a destructor
// destroys them, a default constructor makes them, a copy or move
// constructor or operator= copies or moves them, and an inherited
// constructor calls the base's and makes the rest.  [except.spec] 14
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::ImplicitExceptions(CCPPFunction* pcFunction, CCPPExceptionSpec* pcDest)
{
	CCPPClass*			pcClass;
	SCPP65Type*			psParameter;
	ECPPSpecialMember	eMember;
	bool				bCopier;
	bool				bMove;

	pcClass = pcFunction->mpcClass;
	psParameter = (pcFunction->mapsParameters.NumElements() > 0) ? pcFunction->mapsParameters.GetPtr(0) : NULL;
	bCopier = psParameter && psParameter->IsReference() && psParameter->psBase->IsClass() && (psParameter->psBase->pcClass == pcClass);
	bMove = bCopier && psParameter->bRvalue;
	if (pcFunction->meKind == CPPFK_Destructor)
	{
		eMember = CPPSM_Destroy;
	}
	else if (pcFunction->meKind == CPPFK_Constructor)
	{
		eMember = bCopier ? (bMove ? CPPSM_Move : CPPSM_Copy) : CPPSM_DefaultConstruct;
		if (pcFunction->mpcInherited)
		{
			pcDest->Add(ExceptionsOf(pcFunction->mpcInherited));
		}
	}
	else if (bCopier && (pcFunction->mlliName == CPP_OPERATOR_NAME(CT_Assignment)))
	{
		eMember = bMove ? CPPSM_MoveAssign : CPPSM_Assign;
	}
	else if (pcFunction->mbDeleting)
	{
		//A deleting destructor lets out what the destructor it calls does.
		pcDest->Add(ExceptionsOf(pcClass->mpcDestructor));
		return;
	}
	else
	{
		//A closure's conversion to a pointer to a function, which does nothing that throws.
		return;
	}
	PartsExceptions(pcClass, eMember, pcDest);
}


//////////////////////////////////////////////////////////////////////////
//
// Adds to pcDest what doing eMember to each base and field of the class
// allows, and for making an object what its fields' default member
// initialisers do.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::PartsExceptions(CCPPClass* pcClass, ECPPSpecialMember eMember, CCPPExceptionSpec* pcDest)
{
	SCPPField*	psField;
	SCPP65Type*	psType;
	size		i;

	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		ClassExceptions(pcClass->macBases.Get(i)->pcClass, eMember, pcDest);
	}
	for (i = 0; i < pcClass->macVirtualBases.NumElements(); i++)
	{
		ClassExceptions(pcClass->macVirtualBases.Get(i)->pcClass, eMember, pcDest);
	}
	for (i = 0; i < pcClass->macFields.NumElements(); i++)
	{
		psField = pcClass->macFields.Get(i);
		psType = psField->psType;
		while (psType->IsArray())
		{
			psType = psType->psBase;
		}
		if (psType->IsClass() && (psType->pcClass != pcClass))
		{
			ClassExceptions(psType->pcClass, eMember, pcDest);
		}
		if ((eMember == CPPSM_DefaultConstruct) && psField->pcInitialiser && MayThrow(psField->pcInitialiser))
		{
			pcDest->SetAnything();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Adds to pcDest what doing eMember to an object of the class allows: what
// the function the class has for it allows, and when it has none (the
// compiler copies, makes or destroys the object part by part where it is
// needed) what its parts' do.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::ClassExceptions(CCPPClass* pcClass, ECPPSpecialMember eMember, CCPPExceptionSpec* pcDest)
{
	CCPPFunction*	pcFunction;
	CCPPFunction*	pcMember;
	size			i;

	pcFunction = NULL;
	switch (eMember)
	{
	case CPPSM_Destroy:
		if (pcClass->mpcDestructor)
		{
			pcDest->Add(ExceptionsOf(pcClass->mpcDestructor));
		}
		return;
	case CPPSM_DefaultConstruct:
		for (i = 0; i < pcClass->mapcMembers.NumElements(); i++)
		{
			pcMember = pcClass->mapcMembers.GetPtr(i);
			if ((pcMember->meKind == CPPFK_Constructor) && !pcMember->mbDeleted && !pcMember->mpcInherited && (pcMember->RequiredParameters() == 0))
			{
				pcFunction = pcMember;
				break;
			}
		}
		break;
	case CPPSM_Copy:
	case CPPSM_Move:
		pcFunction = pcClass->GetPartCopier(false, eMember == CPPSM_Move);
		break;
	default:
		pcFunction = pcClass->GetPartCopier(true, eMember == CPPSM_MoveAssign);
		break;
	}
	if (pcFunction)
	{
		pcDest->Add(ExceptionsOf(pcFunction));
	}
	else
	{
		PartsExceptions(pcClass, eMember, pcDest);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether doing eMember to an object of the class may throw.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::ClassMayThrow(CCPPClass* pcClass, ECPPSpecialMember eMember)
{
	CCPPExceptionSpec	cExceptions;
	bool				bThrows;

	cExceptions.Init();
	cExceptions.SetNothing();
	ClassExceptions(pcClass, eMember, &cExceptions);
	bThrows = !cExceptions.IsNonThrowing();
	cExceptions.Kill();
	return bThrows;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcTighter allows only what pcLooser allows: what an overriding
// function's must do against the function's it overrides, and a function's
// against the pointer's it is assigned to.  A class is allowed where a
// base of it is.  [except.spec] 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::AllowsOnlyWhat(CCPPExceptionSpec* pcTighter, CCPPExceptionSpec* pcLooser)
{
	SCPP65Type*	psType;
	SCPP65Type*	psAllowed;
	size		i;
	size		j;
	int			iOffset;
	bool		bAllowed;

	if (pcLooser->IsAnything() || pcTighter->IsNonThrowing())
	{
		return true;
	}
	if (pcTighter->IsAnything())
	{
		return false;
	}
	for (i = 0; i < pcTighter->mapsTypes.NumElements(); i++)
	{
		psType = pcTighter->mapsTypes.GetPtr(i);
		bAllowed = pcLooser->HasType(psType);
		for (j = 0; !bAllowed && (j < pcLooser->mapsTypes.NumElements()); j++)
		{
			psAllowed = pcLooser->mapsTypes.GetPtr(j);
			if (psType->IsPointer() && psAllowed->IsPointer() && !psType->IsMemberPointer() && !psAllowed->IsMemberPointer())
			{
				psType = psType->psBase->Unqualified();
				psAllowed = psAllowed->psBase->Unqualified();
			}
			bAllowed = psType->IsClass() && psAllowed->IsClass() && IsDerived(psType->pcClass, psAllowed->pcClass, &iOffset);
			psType = pcTighter->mapsTypes.GetPtr(i);
		}
		if (!bAllowed)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function's name is operator new, new[], delete or delete[],
// which as a class's members are static whether or not they are declared
// so.  [class.free] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsAllocationFunction(int64 lliName)
{
	return (lliName == GetOperatorName(CT_new, false)) || (lliName == GetOperatorName(CT_new, true)) || (lliName == GetOperatorName(CT_delete, false)) || (lliName == GetOperatorName(CT_delete, true));
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a qualified name (A::f) is defined in a namespace that encloses
// its declaration, as it must be: not namespace C { int A::f() { ... } }.
// [namespace.memdef] 2
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsDefinedWhereDeclared(CCPPSyntaxName* pcName)
{
	CCPPScope*	pcHere;

	if ((pcName->macpComponents.NumElements() < 2) || !pcName->mpcSymbol || !pcName->mpcSymbol->mpcScope)
	{
		return true;
	}
	pcHere = mcContext.GetNamespace();
	return !pcHere || pcHere->Encloses(pcName->mpcSymbol->mpcScope);
}


//////////////////////////////////////////////////////////////////////////
//
// The type of what a function declared auto f(...) -> type, auto f(...)
// { } or decltype(auto) f(...) { } returns.  Its parameters are in scope
// for the type written after it, and its locals too for the type its
// first return gives, which the others must give as well: they are made
// as they are to write the function, in a function scope of their own,
// as this may be asked while another function is being written.  A
// declaration with neither leaves the type until the definition.
// [dcl.fct] 2, [dcl.spec.auto] 2, 7-9
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::ResolveReturn(CCPPFunction* pcFunction, CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator)
{
	CCPPSyntaxFunctionSuffix*		pcSuffix;
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	CCPPSyntaxTypeId*				pcTrailing;
	SCPPFunctionScope				sScope;
	SCPP65Type*						psReturn;
	CArraySyntax					apcReturns;
	size							i;
	bool							bWritten;

	pcSuffix = pcDeclarator->GetFunctionSuffix();
	pcDefinition = (pcDeclarator->mpcParent && (pcDeclarator->mpcParent->GetKind() == CPPS_FunctionDefinition)) ? (CCPPSyntaxFunctionDefinition*)pcDeclarator->mpcParent : NULL;
	if (pcDefinition && (pcDefinition->meBody != CPPFB_Body))
	{
		pcDefinition = NULL;
	}

	//-> auto and -> decltype(auto) are the returns' type still, as the specifiers before the function's name make it.
	pcTrailing = pcSuffix->mpcTrailingReturn;
	bWritten = pcTrailing && !mcDeclarators.IsAuto(pcTrailing->mpcSpecifiers) && !mcDeclarators.IsDecltypeAuto(pcTrailing->mpcSpecifiers);
	if (!bWritten && !pcDefinition)
	{
		return true;
	}

	EnterFunctionScope(&sScope, pcFunction);
	AddParameters(pcFunction, pcDeclarator);
	if (pcTrailing && !bWritten)
	{
		pcSpecifiers = pcTrailing->mpcSpecifiers;
		pcDeclarator = pcTrailing->mpcDeclarator;
	}
	if (bWritten)
	{
		psReturn = mcDeclarators.GetTypeIdType(pcTrailing);
	}
	else
	{
		BeginQuiet();
		AllocateLocals(pcDefinition->mpcBody);
		EndQuiet();
		apcReturns.Init();
		FindReturns(pcDefinition->mpcBody, &apcReturns);
		psReturn = &mcTypes.msVoid;
		for (i = 0; psReturn && (i < apcReturns.NumElements()); i++)
		{
			psReturn = ReturnedType(pcSpecifiers, pcDeclarator, (CCPPSyntaxReturnStatement*)apcReturns.GetPtr(i));
			if (i == 0)
			{
				//Known from here on, so the function can call itself after its first return.
				pcFunction->mpsReturn = psReturn ? psReturn : &mcTypes.msUndeduced;
			}
			else if (psReturn && !psReturn->Same(pcFunction->mpsReturn))
			{
				Error(apcReturns.GetPtr(i), "Each return of a function declared auto must give the same type");
				psReturn = NULL;
			}
		}
		apcReturns.Kill();
	}
	LeaveFunctionScope(&sScope);
	if (psReturn)
	{
		pcFunction->mpsReturn = psReturn;
	}
	return psReturn != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type return value; gives a function declared auto (as it gives a
// variable declared so, with the function's & or *) or decltype(auto):
// by the specifiers and declarator before its name, or those after its ->
// (pcDeclarator is NULL when there is none there).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::ReturnedType(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntaxReturnStatement* pcReturn)
{
	SCPP65Type*	psType;

	if (!pcReturn->mpcValue)
	{
		return &mcTypes.msVoid;
	}
	if (mcDeclarators.IsDecltypeAuto(pcSpecifiers))
	{
		return mcDeclarators.DecltypeType(pcReturn->mpcValue, pcReturn);
	}
	psType = mcDeclarators.DeduceAuto(pcSpecifiers, pcDeclarator, pcReturn->mpcValue, pcReturn);
	if (psType && (pcSpecifiers->muiFlags & (CPPDS_CONST | CPPDS_VOLATILE)))
	{
		psType = mcTypes.Qualified(psType, psType->bConst || (pcSpecifiers->muiFlags & CPPDS_CONST), psType->bVolatile || (pcSpecifiers->muiFlags & CPPDS_VOLATILE));
	}
	return pcDeclarator ? ReturnDeclaratorType(psType, pcDeclarator) : psType;
}


//////////////////////////////////////////////////////////////////////////
//
// The return statements of a function's body, in order: not those of a
// class or a lambda inside it.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::FindReturns(CCPPSyntax* pcNode, CArraySyntax* papcReturns)
{
	size	i;

	if (!pcNode || (pcNode->GetKind() == CPPS_ClassSpecifier) || (pcNode->GetKind() == CPPS_LambdaExpression))
	{
		return;
	}
	if (pcNode->GetKind() == CPPS_ReturnStatement)
	{
		papcReturns->Add(pcNode);
		return;
	}
	for (i = 0; i < pcNode->NumChildren(); i++)
	{
		FindReturns(pcNode->GetChild(i), papcReturns);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a declarator that is only a name declares a function, by its
// specifiers naming a function's type: typedef int Binary(int, int);
// Binary Add;  [dcl.fct] 10
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsFunctionByType(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator)
{
	CCPPSyntaxDeclSpecifiers*	pcTypeSpecifiers;
	CCPPSyntaxDeclarator*		pcShape;

	return pcSpecifiers && pcDeclarator && pcDeclarator->IsNameAlone() && pcSpecifiers->NamesFunctionType(&pcTypeSpecifiers, &pcShape);
}


//////////////////////////////////////////////////////////////////////////
//
// What a function's declarator makes of the type before it: int (*Pick())(int)
// returns what the declarators around its name make of the int, and int*& f()
// a reference to a pointer.  NULL after an error.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::ReturnDeclaratorType(SCPP65Type* psReturn, CCPPSyntaxDeclarator* pcDeclarator)
{
	size		i;
	CCPPClass*	pcClass;

	if (psReturn && pcDeclarator->mpcNested)
	{
		psReturn = mcDeclarators.GetDeclaratorType(psReturn, pcDeclarator, pcDeclarator, NULL, pcDeclarator->GetFunctionSuffix());
	}
	for (i = 0; psReturn && !pcDeclarator->mpcNested && (i < pcDeclarator->NumPointers()); i++)
	{
		switch (pcDeclarator->GetPointer(i)->meKind)
		{
		case CPPP_Pointer:
			psReturn = QualifiedPointer(mcTypes.Pointer(psReturn), pcDeclarator->GetPointer(i));
			break;
		case CPPP_Reference:
		case CPPP_RValueReference:
			psReturn = mcTypes.Reference(psReturn, pcDeclarator->GetPointer(i)->meKind == CPPP_RValueReference);
			break;
		case CPPP_MemberPointer:
			//int B::* Get() returns a pointer to an int member of B.
			pcClass = pcDeclarator->GetPointer(i)->mpcClass ? GetNameClass(pcDeclarator->GetPointer(i)->mpcClass, pcDeclarator) : NULL;
			psReturn = pcClass ? mcTypes.MemberPointer(pcClass, psReturn) : NULL;
			if (!pcClass)
			{
				Error(pcDeclarator, "The class of a pointer to a member is not supported by the compiler yet");
			}
			break;
		default:
			Error(pcDeclarator, "This declarator is not supported by the compiler yet");
			psReturn = NULL;
			break;
		}
	}
	return psReturn;
}


//////////////////////////////////////////////////////////////////////////
//
// Starts a function scope inside whatever is being written: pcFunction's
// parameters and locals can be made (AddParameters, AllocateLocals) and
// its expressions typed, and LeaveFunctionScope puts back the function
// that was being written, with its own.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::EnterFunctionScope(SCPPFunctionScope* psScope, CCPPFunction* pcFunction)
{
	psScope->pcFunction = mcContext.GetFunction();
	memcpy(&psScope->apcLocals, &mapcLocals, sizeof(CArrayCPPVariables));
	memcpy(&psScope->cFunctionPacks, &mcFunctionPacks, sizeof(CCPPFunctionPacks));
	mapcLocals.Init();
	mcFunctionPacks.Init();
	BeginFunction(pcFunction);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::LeaveFunctionScope(SCPPFunctionScope* psScope)
{
	mcContext.EndFunction();
	FreeVariables(&mapcLocals);
	mapcLocals.Kill();
	mcFunctionPacks.Kill();
	memcpy(&mapcLocals, &psScope->apcLocals, sizeof(CArrayCPPVariables));
	memcpy(&mcFunctionPacks, &psScope->cFunctionPacks, sizeof(CCPPFunctionPacks));
	if (psScope->pcFunction)
	{
		mcContext.BeginFunction(psScope->pcFunction);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Records a function's signature, a member of pcClass if it is not NULL, or an
// instance of pcTemplate.  A function may be declared any number of times but
// defined once.  Functions with the same name and different parameters are
// overloads.  Returns NULL after an error.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPUnit::DeclareSignature(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, bool bDefinition, CCPPClass* pcClass, bool bPure, CCPPTemplate* pcTemplate, CChars* pszArguments)
{
	int64						lliName;
	CCPPFunction*				pcFunction;
	CCPPFunction*				pcOther;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxParameter*		pcParameter;
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;
	SCPP65Type*					psType;
	SCPP65Type*					psReturn;
	ECPPFunctionKind			eKind;
	CArrayCPP65Types			apsParameters;
	CArraySyntax				apcDefaults;
	SCPPBinding*				psPack;
	size						i;
	size						j;
	size						uiPack;
	uint32						uiFlags;
	bool						bConstThis;
	bool						bVolatileThis;
	bool						bUndeduced;
	bool						bSameParameters;
	SCPP65Type*					psAbstract;
	ECPPRefQualifier			eRefQualifier;
	bool						bFriend;
	CCPPScope*					pcNamespace;
	CCPPSyntaxDeclSpecifiers*	pcTypeSpecifiers;

	pcName = pcDeclarator->GetName();
	pcComponent = pcName ? pcName->GetLast() : NULL;
	if (!pcComponent || ((pcComponent->meKind == CPPNC_Conversion) && !pcClass) || (!pcClass && !pcTemplate && pcName->IsQualifiedByClass()))
	{
		Error(pcDeclarator, "Conversion functions outside classes and names qualified by what isn't a namespace are not supported by the compiler yet");
		return NULL;
	}
	lliName = GetFunctionName(pcComponent);
	if (lliName == -1)
	{
		Error(pcDeclarator, "This function name is not supported by the compiler yet");
		return NULL;
	}
	if (!pcClass && !pcTemplate && bDefinition && !IsDefinedWhereDeclared(pcName))
	{
		Error(pcDeclarator, "The function is defined in a namespace that doesn't enclose it");
		return NULL;
	}
	eKind = CPPFK_Function;
	if (pcComponent->meKind == CPPNC_Destructor)
	{
		eKind = CPPFK_Destructor;
	}
	else if (pcClass && (lliName == pcClass->mlliName))
	{
		eKind = CPPFK_Constructor;
	}

	//typedef int Binary(int, int); Binary Add; declares a function by the name of its type: the typedef's declarator
	//has its parameters, and the typedef's specifiers what it returns.  [dcl.fct] 10
	pcTypeSpecifiers = pcSpecifiers;
	if (!pcDeclarator->GetFunctionSuffix() && !(pcSpecifiers && pcSpecifiers->NamesFunctionType(&pcTypeSpecifiers, &pcDeclarator)))
	{
		Error(pcDeclarator, "This function declarator is not supported by the compiler yet");
		return NULL;
	}
	pcSuffix = pcDeclarator->GetFunctionSuffix();
	if (!pcDeclarator->mpcNested && (pcDeclarator->macpSuffixes.NumElements() != 1))
	{
		Error(pcDeclarator, "This function declarator is not supported by the compiler yet");
		return NULL;
	}
	bUndeduced = mcDeclarators.IsAuto(pcTypeSpecifiers) || mcDeclarators.IsDecltypeAuto(pcTypeSpecifiers);
	if (pcSuffix->mpcTrailingReturn && (!mcDeclarators.IsAuto(pcTypeSpecifiers) || pcDeclarator->mpcNested || (pcDeclarator->NumPointers() != 0)))
	{
		Error(pcDeclarator, "A function whose return type is written after it is declared auto, and nothing more");
		return NULL;
	}

	psReturn = &mcTypes.msVoid;
	if (lliName == CPP_CONVERSION_NAME)
	{
		//operator T () returns T.
		psReturn = pcComponent->mpcType ? mcDeclarators.GetTypeIdType(pcComponent->mpcType) : NULL;
		if (!psReturn)
		{
			return NULL;
		}
	}
	else if (eKind == CPPFK_Function)
	{
		//auto: the type is worked out once the function is known (ResolveReturn).
		psReturn = bUndeduced ? &mcTypes.msUndeduced : ReturnDeclaratorType(mcDeclarators.GetType(pcTypeSpecifiers, NULL, pcDeclarator, NULL), pcDeclarator);
		if (!psReturn || !mcDeclarators.IsReturnable(psReturn, pcDeclarator))
		{
			return NULL;
		}
	}
	if (!pcClass && (pcSuffix->IsConst() || pcSuffix->IsVolatile()))
	{
		//[dcl.fct] 6
		Error(pcDeclarator, pcSuffix->IsConst() ? "Only a member function is const" : "Only a member function is volatile");
		return NULL;
	}

	apsParameters.Init();
	apcDefaults.Init();
	bConstThis = pcClass && pcSuffix->IsConst();
	bVolatileThis = pcClass && pcSuffix->IsVolatile();
	eRefQualifier = pcClass ? RefQualifier(pcSuffix) : CPPRQ_None;
	for (i = 0; i < pcSuffix->macpParameters.NumElements(); i++)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		psPack = PackBinding(pcParameter);
		uiPack = psPack ? psPack->papsPack->NumElements() : 1;
		for (j = 0; j < uiPack; j++)
		{
			//Args ... args is a parameter for each of the pack's types.
			if (psPack)
			{
				psPack->psType = psPack->papsPack->GetPtr(j);
			}
			psType = mcDeclarators.GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter);
			if (psPack)
			{
				psPack->psType = NULL;
			}
			if (!psType || (psType->IsVoid() && !mcDeclarators.IsWholeVoid(pcSuffix, pcParameter)))
			{
				apcDefaults.Kill();
				apsParameters.Kill();
				return NULL;
			}
			if (psType->IsVoid())
			{
				break;
			}
			apsParameters.Add(ParameterType(psType));
			apcDefaults.Add(pcParameter->mpcDefault);
		}
	}
	if (!pcTemplate && (!CheckAllocationFunction(lliName, psReturn, &apsParameters, pcDeclarator) || !CheckOperatorSignature(pcName->GetLast(), lliName, &apsParameters, &apcDefaults, pcClass, pcDeclarator)))
	{
		apcDefaults.Kill();
		apsParameters.Kill();
		return NULL;
	}

	//A declaration of a function declared before.
	uiFlags = pcSpecifiers ? pcSpecifiers->muiFlags : 0;
	pcNamespace = NULL;
	if (!pcClass)
	{
		//A friend with a simple name is of the namespace its class is in, whatever the name finds.  [namespace.memdef] 3
		bFriend = pcName->IsSimple() && (uiFlags & CPPDS_FRIEND);
		pcNamespace = (pcName->mpcSymbol && !bFriend) ? pcName->mpcSymbol->mpcScope->GetNamespace() : mcContext.GetNamespace();
		if (!pcTemplate && ((lliName == GetOperatorName(CT_new, false)) || (lliName == GetOperatorName(CT_new, true)) || (lliName == GetOperatorName(CT_delete, false)) || (lliName == GetOperatorName(CT_delete, true))))
		{
			//An allocation or deallocation function is a class's or the global namespace's, and not static.  [basic.stc.dynamic] 1
			if (pcNamespace && pcNamespace->mpcParent)
			{
				ReportOnce(pcDeclarator, "An operator new or delete is declared in the global namespace or in a class");
			}
			else if (uiFlags & CPPDS_STATIC)
			{
				ReportOnce(pcDeclarator, "An operator new or delete in the global namespace can't be static");
			}
		}
	}
	for (i = 0; !pcTemplate && (i <= apsParameters.NumElements()); i++)
	{
		psAbstract = (i < apsParameters.NumElements()) ? apsParameters.GetPtr(i) : psReturn;
		if (psAbstract && psAbstract->IsClass() && psAbstract->pcClass->IsComplete() && psAbstract->pcClass->IsAbstract())
		{
			//[class.abstract] 3
			ReportOnce(pcDeclarator, "An abstract class is no function's parameter or result: no object of it can be made");
			break;
		}
	}
	for (pcOther = FirstFunctionOfName(lliName); pcOther; pcOther = pcOther->mpcNextOfName)
	{
		if ((pcOther->mpcClass != pcClass) || (pcOther->mpcNamespace != pcNamespace) || (pcOther->meKind != eKind) || (pcOther->mapsParameters.NumElements() != apsParameters.NumElements()) || (pcOther->mpcTemplate != pcTemplate) ||
			(pcOther->mbVariadic != pcSuffix->mbVariadic))
		{
			continue;
		}
		if (pcTemplate && !pcOther->mszArguments.Equals(pszArguments))
		{
			continue;
		}
		if ((pcOther->IsConversion() && !pcOther->mpsReturn->Equals(psReturn)) || (pcOther->mbConstThis != bConstThis) || (pcOther->mbVolatileThis != bVolatileThis) || (pcOther->meRefQualifier != eRefQualifier))
		{
			continue;
		}
		pcFunction = pcOther;
		for (j = 0; j < apsParameters.NumElements(); j++)
		{
			if (!pcOther->mapsParameters.GetPtr(j)->Same(apsParameters.GetPtr(j)))
			{
				pcFunction = NULL;
				break;
			}
		}
		if (!pcFunction)
		{
			continue;
		}
		if (!pcTemplate && !pcFunction->IsConversion() && !pcFunction->mpsReturn->Same(psReturn) && (pcFunction->mpsReturn != &mcTypes.msUndeduced) && (psReturn != &mcTypes.msUndeduced))
		{
			//What a function returns doesn't tell it from another: one declared twice returns one type.  [dcl.fct] 5, [over.load] 2
			ReportOnce(pcDeclarator, "The function is declared again with another return type");
		}
		for (j = 0; j < apcDefaults.NumElements(); j++)
		{
			if (apcDefaults.GetPtr(j) && pcFunction->mapcDefaults.GetPtr(j) && (apcDefaults.GetPtr(j) != pcFunction->mapcDefaults.GetPtr(j)) && !pcTemplate)
			{
				//[dcl.fct.default] 4
				Error(apcDefaults.GetPtr(j), "The parameter's default argument is given once: an earlier declaration has it");
			}
			if (apcDefaults.GetPtr(j))
			{
				pcFunction->mapcDefaults.SetPtr(j, apcDefaults.GetPtr(j));
			}
		}
		if (!pcTemplate)
		{
			CheckDefaultsFollow(pcFunction, pcDeclarator);
		}
		apcDefaults.Kill();
		apsParameters.Kill();
		pcFunction->mbVirtual |= (uiFlags & CPPDS_VIRTUAL) != 0;
		pcFunction->mbPure |= bPure;
		pcFunction->mbCLinkage |= mcContext.IsCLinkage() && !pcClass;
		pcFunction->mbExplicit |= (uiFlags & CPPDS_EXPLICIT) != 0;
		pcFunction->mbConstexpr |= (uiFlags & CPPDS_CONSTEXPR) != 0;
		CheckRedeclaredExceptions(pcFunction, pcDeclarator);
		return pcFunction;
	}

	if (mcContext.IsCLinkage() && !pcClass && !pcTemplate)
	{
		for (pcOther = FirstFunctionOfName(lliName); pcOther; pcOther = pcOther->mpcNextOfName)
		{
			if (!pcOther->mbCLinkage || pcOther->mpcClass)
			{
				continue;
			}
			bSameParameters = (pcOther->mapsParameters.NumElements() == apsParameters.NumElements()) && (pcOther->mbVariadic == pcSuffix->mbVariadic);
			for (j = 0; bSameParameters && (j < apsParameters.NumElements()); j++)
			{
				bSameParameters = pcOther->mapsParameters.GetPtr(j)->Same(apsParameters.GetPtr(j));
			}
			if (!bSameParameters)
			{
				//Two extern "C" functions of a name are one function, whatever namespace each is in.  [dcl.link] 6
				ReportOnce(pcDeclarator, "Two extern \"C\" functions of one name are one function: it can't be overloaded");
				break;
			}
		}
	}

	for (pcOther = pcClass ? FirstFunctionOfName(lliName) : NULL; pcOther; pcOther = pcOther->mpcNextOfName)
	{
		if ((pcOther->mpcClass != pcClass) || (pcOther->meKind != eKind) || pcOther->mpcTemplate || pcTemplate || ((pcOther->meRefQualifier == CPPRQ_None) == (eRefQualifier == CPPRQ_None)))
		{
			continue;
		}
		bSameParameters = (pcOther->mapsParameters.NumElements() == apsParameters.NumElements()) && (pcOther->mbVariadic == pcSuffix->mbVariadic);
		for (j = 0; bSameParameters && (j < apsParameters.NumElements()); j++)
		{
			bSameParameters = pcOther->mapsParameters.GetPtr(j)->Same(apsParameters.GetPtr(j));
		}
		if (bSameParameters)
		{
			//Overloads that take the same parameters are all declared & or && or none is.  [over.load] 2
			ReportOnce(pcDeclarator, "Where one overload taking these parameters is declared & or &&, each is");
			break;
		}
	}

	pcFunction = AddFunction(pcClass, lliName, eKind, psReturn);
	pcFunction->mpcNamespace = pcNamespace;
	pcFunction->mpcDeclarator = pcDeclarator;
	for (i = 0; i < apsParameters.NumElements(); i++)
	{
		pcFunction->mapsParameters.Add(apsParameters.GetPtr(i));
		pcFunction->mapcDefaults.Add(apcDefaults.GetPtr(i));
	}
	pcFunction->mbConstThis = bConstThis;
	pcFunction->mbVolatileThis = bVolatileThis;
	pcFunction->meRefQualifier = eRefQualifier;
	apcDefaults.Kill();
	apsParameters.Kill();
	if (!pcTemplate)
	{
		CheckDefaultsFollow(pcFunction, pcDeclarator);
	}
	pcFunction->mbVariadic = pcSuffix->mbVariadic;
	pcFunction->mbStatic = ((uiFlags & CPPDS_STATIC) != 0) || (pcClass && IsAllocationFunction(lliName));
	pcFunction->mbVirtual = (uiFlags & CPPDS_VIRTUAL) != 0;
	pcFunction->mbPure = bPure;
	pcFunction->mbInline = (uiFlags & (CPPDS_INLINE | CPPDS_CONSTEXPR)) != 0;
	pcFunction->mbConstexpr = (uiFlags & CPPDS_CONSTEXPR) != 0;
	pcFunction->mpcTemplate = pcTemplate;
	pcFunction->mbCLinkage = mcContext.IsCLinkage() && !pcClass;
	pcFunction->mbExplicit = (uiFlags & CPPDS_EXPLICIT) != 0;
	pcFunction->miDeprecated = (pcFunction->miDeprecated != -1) ? pcFunction->miDeprecated : FindAttribute(pcDeclarator, "deprecated");
	SetDeclaredExceptions(pcFunction, pcDeclarator);
	pcFunction->mbOverride |= HasVirtSpecifier(pcSuffix, mpcTokeniser->mlliOverride);
	pcFunction->mbFinal |= HasVirtSpecifier(pcSuffix, mpcTokeniser->mlliFinal);
	if (pszArguments)
	{
		pcFunction->mszArguments.Append(pszArguments);
	}
	if (pcTemplate)
	{
		pcTemplate->mapcInstances.Add(pcFunction);
	}
	if (eKind == CPPFK_Constructor)
	{
		pcClass->mbConstructors = true;
	}
	else if (eKind == CPPFK_Destructor)
	{
		pcClass->mpcDestructor = pcFunction;
	}
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// A function defined or called takes no parameter of an abstract class
// and returns no object of one (its declaration said so where the class
// was complete then; DeclareSignature).  An error at pcNode if it does.
// [class.abstract] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckAbstractSignature(CCPPFunction* pcFunction, CCPPSyntax* pcNode)
{
	SCPP65Type*	psType;
	size		i;

	for (i = 0; i <= pcFunction->mapsParameters.NumElements(); i++)
	{
		psType = (i < pcFunction->mapsParameters.NumElements()) ? pcFunction->mapsParameters.GetPtr(i) : pcFunction->mpsReturn;
		if (psType && psType->IsClass() && psType->pcClass->IsComplete() && psType->pcClass->IsAbstract())
		{
			return ReportOnce(pcNode, "An abstract class is no function's parameter or result: no object of it can be made");
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// What a variable's declaration must be, wherever it is (a local, a static
// local, a global; the semantic pass's check too): initialised where its
// type needs it to be (NeedsNoInitialiser), not copy-initialised through an
// explicit constructor (CopyInitialises), and what its initialiser, or none,
// can make (the type checker's Initialise: the one place that says so, of
// an object, a reference, a number and an array alike).  Its errors are
// reported, where it is quiet too.  [dcl.init]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckDeclared(SCPP65Type* psType, CCPPSyntaxInitDeclarator* pcInit, CCPPSyntax* pcInitialiser)
{
	SCPP65Type*						psElement;
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	bool							bExtern;
	bool							bResult;

	if (!pcInitialiser && !NeedsNoInitialiser(psType, pcInit))
	{
		return false;
	}
	if (psType->IsClass() && !mcBinder.CopyInitialises(psType->pcClass, pcInit))
	{
		return false;
	}
	psElement = psType;
	while (psElement->IsArray())
	{
		psElement = psElement->psBase;
	}
	pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcInit->mpcParent;
	bExtern = pcDeclaration && (pcDeclaration->GetKind() == CPPS_SimpleDeclaration) && pcDeclaration->mpcSpecifiers && (pcDeclaration->mpcSpecifiers->muiFlags & CPPDS_EXTERN);
	if (psElement->IsClass() && !bExtern && CompleteClass(psElement->pcClass, pcInit) && !mcBinder.ReachesDestructor(psElement->pcClass, pcInit))
	{
		//The object is destroyed where it ends.  [class.dtor] 11
		return false;
	}
	BeginLoud();
	bResult = mcTypeChecker.Initialise(psType, pcInitialiser, pcInit);
	EndLoud();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A class written in a source file (IsInSourceFile) with a member function
// declared override or final, or with a base, is laid out where it is
// defined, so what final and override say is checked whether or not
// anything uses the class (CheckOverrides: one that overrides nothing, one
// that isn't virtual, one that overrides a final function; ResolveBases: a
// base that is final); any other class is laid out when it is first needed
// whole, and one nothing uses never is.
//
// A header's class is not: it is checked in the unit that first needs it
// whole.  Every unit parses each class its headers so much as name, and
// laying all of those out in every unit was half the time of the
// declarations (DeclarationsSpeedPlan.txt).  Nor is any class when the unit
// was compiled before, which found what there is to find
// (SetCompiledBefore).
// [class.virtual] 4, 5, [class] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckVirtSpecifiers(CCPPSyntaxClassSpecifier* pcSpecifier)
{
	CCPPClass*	pcClass;

	if (mbCompiledBefore || !pcSpecifier->mpcSymbol || (pcSpecifier->mpcSymbol->meKind != CPPSK_Class) || !(pcSpecifier->HasVirtSpecifiedMember() || (pcSpecifier->macpBases.NumElements() > 0)) ||
		!IsInSourceFile(pcSpecifier))
	{
		return true;
	}
	pcClass = GetClass(pcSpecifier->mpcSymbol, pcSpecifier);
	if (!pcClass->IsComplete() && !pcClass->IsBeingMade())
	{
		mpcStatistics->Count(CPPCC_ClassLayoutsWhereDefined);
	}
	return CompleteClass(pcClass, pcSpecifier);
}


//////////////////////////////////////////////////////////////////////////
//
// True if the declarator declares an operator new, new[], delete or
// delete[].
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsAllocationName(CCPPSyntaxDeclarator* pcDeclarator)
{
	CCPPSyntaxName*	pcName;
	int64			lliName;

	pcName = pcDeclarator->GetName();
	lliName = (pcName && (pcName->GetLast()->meKind == CPPNC_Operator)) ? GetFunctionName(pcName->GetLast()) : -1;
	return (lliName != -1) && ((lliName == GetOperatorName(CT_new, false)) || (lliName == GetOperatorName(CT_new, true)) || (lliName == GetOperatorName(CT_delete, false)) || (lliName == GetOperatorName(CT_delete, true)));
}


//////////////////////////////////////////////////////////////////////////
//
// An error in a function's declaration: a member's is a class error, kept
// though the class is laid out quietly (ClassError).  False.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::OperatorError(CCPPClass* pcClass, CCPPSyntax* pcNode, const char* szMessage)
{
	return pcClass ? ClassError(pcNode->miFirstToken, szMessage) : Error(pcNode, szMessage);
}


//////////////////////////////////////////////////////////////////////////
//
// An operator function's parameters: none has a default, but operator()'s;
// one that is no class's member has a class or an enum among them (a
// reference to one too); postfix ++'s and --'s extra one is an int.  A
// literal operator's are one of the lists [over.literal] 3 gives.  An error
// (and false) if not.  [over.oper] 6, 8, [over.inc] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckOperatorSignature(CCPPSyntaxNameComponent* pcComponent, int64 lliName, CArrayCPP65Types* papsParameters, CArraySyntax* papcDefaults, CCPPClass* pcClass, CCPPSyntax* pcNode)
{
	SCPP65Type*	psType;
	SCPP65Type*	psFirst;
	size		i;
	bool		bClass;

	if (pcComponent->meKind == CPPNC_Literal)
	{
		psFirst = (papsParameters->NumElements() > 0) ? papsParameters->GetPtr(0)->Unqualified() : NULL;
		if (papsParameters->NumElements() == 1)
		{
			bClass = psFirst->Equals(mcTypes.Pointer(mcTypes.Const(&mcTypes.msChar))) || psFirst->Equals(&mcTypes.msUnsignedLongLong) || psFirst->Equals(&mcTypes.msLongDouble) ||
					 psFirst->Equals(&mcTypes.msChar) || psFirst->Equals(&mcTypes.msWChar) || psFirst->Equals(&mcTypes.msChar16) || psFirst->Equals(&mcTypes.msChar32);
		}
		else if (papsParameters->NumElements() == 2)
		{
			bClass = psFirst->IsPointer() && psFirst->psBase->bConst && (psFirst->psBase->Unqualified()->Equals(&mcTypes.msChar) || psFirst->psBase->Unqualified()->Equals(&mcTypes.msWChar) ||
					 psFirst->psBase->Unqualified()->Equals(&mcTypes.msChar16) || psFirst->psBase->Unqualified()->Equals(&mcTypes.msChar32)) && papsParameters->GetPtr(1)->Unqualified()->Equals(mcTypes.Size());
		}
		else
		{
			bClass = false;
		}
		return bClass || Error(pcNode, "A literal operator's parameters are one of: const char*; unsigned long long; long double; a character type; a pointer to const characters and a size_t");
	}
	if ((pcComponent->meKind != CPPNC_Operator) || (lliName == GetOperatorName(CT_new, false)) || (lliName == GetOperatorName(CT_new, true)) ||
		(lliName == GetOperatorName(CT_delete, false)) || (lliName == GetOperatorName(CT_delete, true)))
	{
		return true;
	}
	if (lliName != GetOperatorName(CT_OpenRoundBracket, false))
	{
		for (i = 0; i < papcDefaults->NumElements(); i++)
		{
			if (papcDefaults->GetPtr(i))
			{
				return OperatorError(pcClass, pcNode, "An operator function has no default arguments (but operator())");
			}
		}
	}
	if (!pcClass)
	{
		bClass = false;
		for (i = 0; !bClass && (i < papsParameters->NumElements()); i++)
		{
			psType = papsParameters->GetPtr(i)->WithoutReference();
			bClass = psType->IsClass() || (psType->pcEnum != NULL);
		}
		if (!bClass)
		{
			return Error(pcNode, "An operator function that is no class's member has a parameter of a class or an enum");
		}
	}
	if (((lliName == GetOperatorName(CT_Increment, false)) || (lliName == GetOperatorName(CT_Decrement, false))) && (papsParameters->NumElements() == (pcClass ? 1 : 2)) &&
		!papsParameters->GetPtr(papsParameters->NumElements() - 1)->Unqualified()->Equals(&mcTypes.msInt))
	{
		return OperatorError(pcClass, pcNode, "A postfix ++ or --'s second parameter is an int");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// What an operator new or delete must be to be an allocation or a
// deallocation function: operator new returns void* and its first
// parameter is a std::size_t; operator delete returns nothing and its first
// parameter is a void*.  True for any other function.
// [basic.stc.dynamic.allocation] 1, [basic.stc.dynamic.deallocation] 2
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckAllocationFunction(int64 lliName, SCPP65Type* psReturn, CArrayCPP65Types* papsParameters, CCPPSyntax* pcNode)
{
	SCPP65Type*	psVoidPointer;
	SCPP65Type*	psFirst;

	psVoidPointer = mcTypes.Pointer(&mcTypes.msVoid);
	psFirst = (papsParameters->NumElements() > 0) ? papsParameters->GetPtr(0)->Unqualified() : NULL;
	if ((lliName == GetOperatorName(CT_new, false)) || (lliName == GetOperatorName(CT_new, true)))
	{
		if (!psReturn->Equals(psVoidPointer))
		{
			return Error(pcNode, "operator new returns void*");
		}
		return (psFirst && psFirst->Equals(mcTypes.Size())) || Error(pcNode, "operator new's first parameter is a std::size_t");
	}
	if ((lliName == GetOperatorName(CT_delete, false)) || (lliName == GetOperatorName(CT_delete, true)))
	{
		if (!psReturn->IsVoid())
		{
			return Error(pcNode, "operator delete returns void");
		}
		return (psFirst && psFirst->Equals(psVoidPointer)) || Error(pcNode, "operator delete's first parameter is a void*");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// What = default and = delete ask of the function they define.  = default
// is of a special member function: a constructor that takes nothing or
// its own class (a copy or a move), a destructor, or an operator= that
// takes its own class; and it has no default argument.  = delete is where
// the function is first declared.  An error (and false) if not; true for
// any other definition.  [dcl.fct.def.default] 1, [dcl.fct.def.delete] 4
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckDefaultedOrDeleted(CCPPFunction* pcFunction, CCPPSyntaxFunctionDefinition* pcDefinition)
{
	SCPP65Type*	psParameter;
	size		i;
	bool		bOwnClass;

	if (pcDefinition->meBody == CPPFB_Delete)
	{
		return (pcFunction->mpcDeclarator == pcDefinition->mpcDeclarator) || pcFunction->mpcTemplate || ReportError(pcDefinition, "A function is deleted where it is first declared");
	}
	if (pcDefinition->meBody != CPPFB_Default)
	{
		return true;
	}
	psParameter = (pcFunction->mapsParameters.NumElements() == 1) ? pcFunction->mapsParameters.GetPtr(0)->WithoutReference() : NULL;
	bOwnClass = psParameter && psParameter->IsClass() && (psParameter->pcClass == pcFunction->mpcClass);
	if (!pcFunction->mpcClass || ((pcFunction->meKind == CPPFK_Constructor) && (pcFunction->mapsParameters.NumElements() != 0) && !bOwnClass) ||
		((pcFunction->meKind == CPPFK_Function) && !((pcFunction->mlliName == CPP_OPERATOR_NAME(CT_Assignment)) && bOwnClass)))
	{
		return ReportError(pcDefinition, "Only a constructor that takes nothing or its own class, a destructor or an operator= that takes its own class is = default");
	}
	for (i = 0; i < pcFunction->mapcDefaults.NumElements(); i++)
	{
		if (pcFunction->mapcDefaults.GetPtr(i))
		{
			return ReportError(pcDefinition, "A function that is = default has no default argument");
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a variable of psType may be defined with no initialiser: not a
// reference, which must be bound to something, and not a const object (or
// an array of them) that nothing would ever give a value: a number or a
// pointer, or an object of a class with fields and no constructor to fill
// them.  An error at pcNode (and false) if not.  [dcl.ref] 5, [dcl.init] 7
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::NeedsNoInitialiser(SCPP65Type* psType, CCPPSyntax* pcNode)
{
	SCPP65Type*	psElement;
	CCPPClass*	pcClass;

	if (psType->IsReference())
	{
		return ReportError(pcNode, "A reference must be initialised");
	}
	psElement = psType;
	while (psElement->IsArray())
	{
		psElement = psElement->psBase;
	}
	if (psElement->IsClass() && !psElement->pcClass->mbConstructors && HasFieldNeedingInitialiser(psElement->pcClass))
	{
		//[class.ctor] 4
		return ReportError(pcNode, "The class's default constructor is deleted: a reference or const member of it has no initialiser");
	}
	if (!psElement->bConst)
	{
		return true;
	}
	if (!psElement->IsClass())
	{
		return ReportError(pcNode, "A const object must be initialised");
	}
	pcClass = psElement->pcClass;
	if (!pcClass->mbConstructors && !pcClass->NeedsConstructor() && (pcClass->macFields.NumElements() > 0))
	{
		return ReportError(pcNode, "A const object of a class with no constructor must be initialised");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a field of the class has no default member initialiser and can't
// be made by default: a reference, or a const object of a type with no
// constructor of its own (a number, say).  [class.ctor] 4
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::HasFieldNeedingInitialiser(CCPPClass* pcClass)
{
	SCPPField*	psField;
	SCPP65Type*	psType;
	size		i;

	for (i = 0; i < pcClass->macFields.NumElements(); i++)
	{
		psField = pcClass->macFields.Get(i);
		if (!psField->pcSymbol || psField->pcInitialiser)
		{
			continue;
		}
		psType = psField->psType;
		while (psType->IsArray())
		{
			psType = psType->psBase;
		}
		if (psType->IsReference() || (psType->bConst && !(psType->IsClass() && psType->pcClass->mbConstructors)))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A constructor's member initialisers: each names a member or a base of
// its class, once; a delegating one (naming the class) is the only one;
// and a reference member, or a const one of a type with no constructor of
// its own, that no default member initialiser gives a value is named.
// [class.base.init] 2, 4, 6, 8
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckMemberInitialisers(CCPPFunction* pcFunction)
{
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	CArraySyntax*					papcInitialisers;
	CCPPSyntaxMemberInitializer*	pcMember;
	CCPPSyntaxMemberInitializer*	pcOther;
	CCPPClass*						pcClass;
	CCPPClass*						pcNamed;
	CCPPClass*						pcOtherNamed;
	SCPPField*						psField;
	SCPP65Type*						psType;
	int64							lliName;
	size							i;
	size							j;
	int								iOffset;
	bool							bDelegating;
	bool							bNamed;

	pcDefinition = pcFunction->mpcDefinition;
	pcClass = pcFunction->mpcClass;
	if ((pcFunction->meKind != CPPFK_Constructor) || !pcDefinition || !pcClass || (pcDefinition->meBody != CPPFB_Body) || !CompleteClass(pcClass, pcDefinition))
	{
		return true;
	}
	papcInitialisers = ExpandArguments(&pcDefinition->macpMemberInitializers);
	if (!papcInitialisers)
	{
		return true;
	}

	bDelegating = false;
	for (i = 0; i < papcInitialisers->NumElements(); i++)
	{
		pcMember = (CCPPSyntaxMemberInitializer*)papcInitialisers->GetPtr(i);
		if (pcMember->GetKind() != CPPS_MemberInitializer)
		{
			return true;
		}
		lliName = GetComponentName(pcMember->mpcName->GetLast());
		pcNamed = (pcMember->mpcName->mpcSymbol && (pcMember->mpcName->mpcSymbol->meKind == CPPSK_Field)) ? NULL : GetNameClass(pcMember->mpcName, pcMember);
		if (pcNamed == pcClass)
		{
			bDelegating = true;
			if (papcInitialisers->NumElements() != 1)
			{
				return Error(pcMember, "A constructor that names its own class in its member initialisers has no other");
			}
		}
		else if (!(pcNamed && IsDerived(pcClass, pcNamed, NULL)) && !pcClass->FindField(lliName, &iOffset))
		{
			return Error(pcMember, "The member initialiser names no member or base of the class");
		}
		for (j = 0; j < i; j++)
		{
			pcOther = (CCPPSyntaxMemberInitializer*)papcInitialisers->GetPtr(j);
			pcOtherNamed = (pcOther->mpcName->mpcSymbol && (pcOther->mpcName->mpcSymbol->meKind == CPPSK_Field)) ? NULL : GetNameClass(pcOther->mpcName, pcOther);
			if (pcNamed ? (pcOtherNamed == pcNamed) : (!pcOtherNamed && (GetComponentName(pcOther->mpcName->GetLast()) == lliName)))
			{
				return Error(pcMember, "The member or base is named twice in the member initialisers");
			}
		}
	}
	if (bDelegating || pcClass->mbUnion)
	{
		return true;
	}

	for (i = 0; i < pcClass->macFields.NumElements(); i++)
	{
		psField = pcClass->macFields.Get(i);
		if (!psField->pcSymbol || psField->pcInitialiser)
		{
			continue;
		}
		psType = psField->psType;
		if (!(psType->IsReference() || (psType->bConst && !(psType->IsClass() && psType->pcClass->mbConstructors))))
		{
			continue;
		}
		bNamed = false;
		for (j = 0; !bNamed && (j < papcInitialisers->NumElements()); j++)
		{
			pcMember = (CCPPSyntaxMemberInitializer*)papcInitialisers->GetPtr(j);
			bNamed = (pcMember->mpcName->mpcSymbol == psField->pcSymbol) || (GetComponentName(pcMember->mpcName->GetLast()) == psField->lliName);
		}
		if (!bNamed)
		{
			return Error(pcDefinition->mpcDeclarator, "The constructor leaves a reference or const member without a value: name it in the member initialisers");
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// After a parameter with a default argument every parameter has one, from
// this declaration or one before it: an error at pcNode if not.
// [dcl.fct.default] 4
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckDefaultsFollow(CCPPFunction* pcFunction, CCPPSyntax* pcNode)
{
	size	i;
	bool	bDefault;

	bDefault = false;
	for (i = 0; i < pcFunction->mapcDefaults.NumElements(); i++)
	{
		if (bDefault && !pcFunction->mapcDefaults.GetPtr(i))
		{
			return Error(pcNode, "A parameter after one with a default argument needs one too");
		}
		bDefault |= pcFunction->mapcDefaults.GetPtr(i) != NULL;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// int F(int a); in a block declares a function of the namespace around the
// block, as if it was declared there.  [basic.link] 6
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::DeclareBlockFunction(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator)
{
	BeginQuiet();
	if (!DeclareFunction(pcSpecifiers, pcDeclarator, false, NULL, false))
	{
		mapcFailed.Add(pcDeclarator);
	}
	EndQuiet();
}


//////////////////////////////////////////////////////////////////////////
//
// A new function.  Its overload number counts the functions with its name before it.
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
	if (!pcFunction->mbGenerated && !pcFunction->mbQueued && (pcFunction->mpcDefinition || pcFunction->mbImplicit || pcFunction->mpcCovariantOf) && !IsInstantiatedElsewhere(pcFunction))
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
// Function declarations, classes, typedefs, enums and global variables.  Global
// variables are data with constant initialisers (zero without one).  Global objects
// are zero and made by their R_Initialise before main, and destroyed (last first) by
// their R_Finalise after it.  A qualified name defines a static data member.  Function
// and extern declarations the generator cannot do are only errors when used.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::GlobalDeclaration(CCPPSyntaxSimpleDeclaration* pcDeclaration)
{
	size						i;
	CCPPSyntaxInitDeclarator*	pcInit;
	CCPPSyntax*					pcInitialiser;
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;
	CCPPVariable*				pcVariable;
	CCPPFunction*				pcFunction;
	SCPP65Type*					psType;
	SCPP65Type*					psElement;
	CChars						szLabel;
	CCPPSyntaxName*				pcName;
	CCPPClass*				pcClass;
	int64						lliName;
	bool						bResult;
	bool						bBss;
	bool						bInitialise;
	CChars						szWritten;
	CChars::Count				iDataLength;
	bool						bConstantData;
	const char*					szOptions;
	CChars						szBound;
	int							iBound;

	pcSpecifiers = pcDeclaration->mpcSpecifiers;
	if (pcSpecifiers && !CheckEnumerators(pcSpecifiers->mpcType))
	{
		return false;
	}
	if (pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_TYPEDEF))
	{
		return CheckTypedefs(pcDeclaration);
	}
	if (pcSpecifiers && pcSpecifiers->mpcType && (pcSpecifiers->mpcType->GetKind() == CPPS_ClassSpecifier))
	{
		//The class is laid out when it is first needed whole.
		BeginQuiet();
		mcDeclarators.GetBaseType(pcSpecifiers, pcDeclaration);
		EndQuiet();
		if (!ClassStaticAsserts((CCPPSyntaxClassSpecifier*)pcSpecifiers->mpcType) || !CheckVirtSpecifiers((CCPPSyntaxClassSpecifier*)pcSpecifiers->mpcType))
		{
			return false;
		}
		if ((pcDeclaration->macpDeclarators.NumElements() == 0) && !((CCPPSyntaxClassSpecifier*)pcSpecifiers->mpcType)->mpcName)
		{
			return AnonymousGlobal((CCPPSyntaxClassSpecifier*)pcSpecifiers->mpcType);
		}
	}

	bResult = true;
	for (i = 0; i < pcDeclaration->macpDeclarators.NumElements(); i++)
	{
		pcInit = (CCPPSyntaxInitDeclarator*)pcDeclaration->macpDeclarators.GetPtr(i);
		if (pcInit->mpcDeclarator->IsFunction() || IsFunctionByType(pcSpecifiers, pcInit->mpcDeclarator))
		{
			BeginQuiet();
			pcFunction = DeclareFunction(pcSpecifiers, pcInit->mpcDeclarator, false, NULL, false);
			EndQuiet();
			if (!pcFunction && IsAllocationName(pcInit->mpcDeclarator))
			{
				//What is wrong with an operator new or delete is said where it is declared: a program replaces the
				//library's by declaring its own, and a wrong one would be called by none of its news.
				bResult &= DeclareFunction(pcSpecifiers, pcInit->mpcDeclarator, false, NULL, false) != NULL;
			}
			else if (!pcFunction)
			{
				mapcFailed.Add(pcInit->mpcDeclarator);
			}
			continue;
		}

		lliName = GetName(pcInit->mpcDeclarator);
		pcName = pcInit->mpcDeclarator->GetName();
		pcVariable = pcInit->mpcSymbol ? (mpcDefining ? mpcDefining : GetVariable(pcInit->mpcSymbol)) : NULL;
		if (!pcVariable && pcInit->mpcSymbol && !mpcDefining)
		{
			//int x; after extern int x; is the same variable.
			pcVariable = RedeclaredVariable(pcInit->mpcSymbol);
		}
		if ((lliName == -1) && pcName && pcName->IsQualifiedByClass())
		{
			//int CClass::siStatic = 0; defines a static data member, so the class is laid out.  The member is the
			//class's own: each instance of a class template has one of the symbol (template<> int S<int>::k = 5;).
			pcClass = mpcDefining ? mpcDefining->mpcClass : GetComponentClass(pcName->GetQualifier(), NULL, pcInit);
			if (pcClass)
			{
				CompleteClass(pcClass, pcInit);
				pcVariable = (pcInit->mpcSymbol && !mpcDefining) ? OwnStatic(pcClass, pcInit->mpcSymbol) : pcVariable;
			}
		}
		if (!pcInit->mpcSymbol || ((lliName == -1) && pcName && pcName->IsQualifiedByClass() && !(pcVariable && pcVariable->mpcClass)))
		{
			bResult &= Error(pcInit, "Only variables with simple names are supported by the compiler yet");
			continue;
		}
		bResult &= CheckPointerExceptions(pcInit);

		//extern without an initialiser only declares; extern int x = 1; defines x.  [basic.def]
		if (((pcSpecifiers->muiFlags & CPPDS_EXTERN) || mcContext.IsLinkageExtern()) && !pcInit->mpcInitializer)
		{
			BeginQuiet();
			psType = mcDeclarators.GetType(pcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
			EndQuiet();
			if (psType && pcVariable && !SameVariableType(pcVariable->mpsType, psType))
			{
				bResult &= Error(pcInit, "The variable is declared again with another type");
				continue;
			}
			if (psType && !pcVariable)
			{
				pcVariable = AddVariable(&mapcGlobals, pcInit->mpcSymbol, psType, CPPVS_Global, 0);
			}
			if (pcVariable)
			{
				pcVariable->mbRestrict |= IsRestrictPointer(pcInit->mpcDeclarator);
			}
			continue;
		}

		//const int CClass::k; has the initialiser the class gave it, and const int CClass::k = 5; is what k is from here.
		pcInitialiser = pcInit->mpcInitializer;
		if (!pcInitialiser && pcVariable)
		{
			pcInitialiser = pcVariable->mpcInClassInitialiser;
		}
		else if (pcVariable && pcVariable->mpcClass)
		{
			pcVariable->mpcOutsideInitialiser = pcInitialiser;
		}

		psType = mcDeclarators.GetType(pcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInitialiser);
		if (!psType)
		{
			bResult = false;
			continue;
		}
		if (psType->IsReference())
		{
			pcInitialiser = BoundInitialiser(psType, pcInitialiser);
		}
		psElement = psType->IsArray() ? psType->psBase : psType;
		if (psElement->IsClass() && !CompleteObjectClass(psElement->pcClass, pcInit))
		{
			bResult = false;
			continue;
		}

		if (pcVariable && !pcVariable->mpcClass && !SameVariableType(pcVariable->mpsType, psType))
		{
			bResult &= Error(pcInit, "The variable is declared again with another type");
			continue;
		}
		if (!pcVariable)
		{
			pcVariable = AddVariable(&mapcGlobals, pcInit->mpcSymbol, psType, CPPVS_Global, 0);
		}
		pcVariable->mbInternal |= !pcVariable->mpcClass && ((pcSpecifiers->muiFlags & CPPDS_STATIC) != 0);
		pcVariable->mbRestrict |= IsRestrictPointer(pcInit->mpcDeclarator);
		if (pcVariable->mbDefined)
		{
			bResult &= Error(pcInit, "The variable is defined twice");
			continue;
		}
		pcVariable->mbDefined = true;
		pcVariable->mpsType = psType;
		CompleteDeclaredArrays(pcVariable);
		if (!CheckDeclared(psType, pcInit, pcInitialiser))
		{
			bResult = false;
			continue;
		}
		if ((pcSpecifiers->muiFlags & CPPDS_CONSTEXPR) && !mpcConstants->CheckVariable(pcVariable, pcInit))
		{
			bResult = false;
			continue;
		}

		szLabel.Init();
		AppendVariableLabel(&szLabel, pcVariable);
		bConstantData = (pcSpecifiers->muiFlags & CPPDS_CONSTEXPR) != 0;
		bBss = !bConstantData && IsBss(pcInitialiser, psType);
		szOptions = mpcDefining ? ", comdat" : (HasUsedAttribute(pcInit->mpcDeclarator) ? ", keep" : NULL);
		szBound.Init();
		if (psType->IsReference() && !BoundTemporary(psType, pcInitialiser, &szLabel, !pcVariable->mbInternal, szOptions, &szBound, &iBound, &bResult))
		{
			szBound.Kill();
			szLabel.Kill();
			continue;
		}
		mcData.BeginData(&szLabel, VariableDataAlignment(pcVariable, psElement->IsClass()), !pcVariable->mbInternal, szOptions, bBss);

		//GlobalInitialiser writes the constant data.  What isn't constant is worked out by the global's own R_Initialise,
		//and an object is destroyed by its R_Finalise: the IR front end writes them (GlobalStartUpRoutines).
		mcContext.NoThis();
		if (bConstantData && mpcConstants->AppendData(pcVariable, &mcData))
		{
			//A constexpr variable's value, worked out when it was defined: its data, with nothing to run.
		}
		else if (szBound.Length() > 0)
		{
			//A reference to the temporary it is bound to (BoundTemporary).
			mcData.AppendAddress(&szBound, iBound, psType->iBytes);
		}
		else if (bBss)
		{
			mcData.AppendSpace(psType->iBytes);
			bResult &= GlobalStartUpRoutines(psType, pcInitialiser, &szLabel, psElement->IsClass(), psElement->IsClass() && (psElement->pcClass->mpcDestructor != NULL), (pcSpecifiers->muiFlags & CPPDS_THREAD_LOCAL) != 0);
		}
		else if (!psElement->IsClass() && !psType->IsReference())
		{
			bInitialise = false;
			iDataLength = mcData.GetText()->Length();
			bResult &= GlobalInitialiser(psType, pcInitialiser, pcInit, &bInitialise);
			if (bInitialise && bResult && !(pcSpecifiers->muiFlags & CPPDS_THREAD_LOCAL) && psElement->IsPointer())
			{
				//A pointer to an object of static storage, or a table of them, is a constant the interpreter works out.
				szWritten.Init();
				szWritten.AppendSubString(mcData.GetText(), iDataLength, mcData.GetText()->Length());
				mcData.GetText()->SetLength(iDataLength);
				bInitialise = !mpcConstants->AppendConstantInitialisation(pcVariable, &mcData);
				if (bInitialise)
				{
					mcData.GetText()->Append(&szWritten);
				}
				szWritten.Kill();
			}
			bResult &= GlobalStartUpRoutines(psType, pcInitialiser, &szLabel, bInitialise, false, false);
		}
		else if (!(pcSpecifiers->muiFlags & CPPDS_THREAD_LOCAL) && mpcConstants->AppendConstantInitialisation(pcVariable, &mcData))
		{
			//An object made by a constexpr constructor from constants, or a reference bound to an object of static
			//storage: initialised before anything runs, as data, though not constexpr.  [basic.start.init] 2
			bResult &= GlobalStartUpRoutines(psType, pcInitialiser, &szLabel, false, psElement->IsClass() && (psElement->pcClass->mpcDestructor != NULL), false);
		}
		else
		{
			mcData.AppendZeros(psType->iBytes);
			bResult &= GlobalStartUpRoutines(psType, pcInitialiser, &szLabel, true, psElement->IsClass() && (psElement->pcClass->mpcDestructor != NULL), (pcSpecifiers->muiFlags & CPPDS_THREAD_LOCAL) != 0);
		}
		mcData.NextStartUp();
		szBound.Kill();
		szLabel.Kill();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Appends the data for a global of psType: zero, a constant, a list of constants or
// a string.  A char pointer initialised with a string literal is the literal's
// address.  A value that is not a constant is zero data, worked out by the global's
// R_Initialise: *pbInitialise is set when there is one.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::GlobalInitialiser(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode, bool* pbInitialise)
{
	int64						lliValue;
	CArraySyntax*				papcElements;
	size						i;
	CArrayUint8					auiBytes;
	bool						bResult;
	double						dValue;

	pcInitialiser = BracedFor(psType, pcInitialiser);
	if (pcInitialiser && (pcInitialiser->GetKind() == CPPS_ParenthesisedInitializer))
	{
		papcElements = &((CCPPSyntaxParenthesisedInitializer*)pcInitialiser)->macpElements;
		pcInitialiser = (papcElements->NumElements() == 1) ? papcElements->GetPtr(0) : pcInitialiser;
	}
	if (pcInitialiser && (pcInitialiser->GetKind() == CPPS_InitializerList) && psType->IsScalar())
	{
		papcElements = ListElements(pcInitialiser);
		pcInitialiser = (papcElements->NumElements() == 1) ? papcElements->GetPtr(0) : NULL;
		if (pcInitialiser && !CheckBracedValue(pcInitialiser, psType))
		{
			return false;
		}
	}

	if (psType->IsArray())
	{
		bResult = true;
		pcInitialiser = StringInitialiser(pcInitialiser, psType->psBase);
		if (!pcInitialiser)
		{
			i = 0;
		}
		else if (IsStringLiteral(pcInitialiser) && (psType->psBase->iBytes == StringElementBytes(pcInitialiser)))
		{
			auiBytes.Init();
			bResult = DecodeString(pcInitialiser, &auiBytes);
			for (i = 0; i < auiBytes.NumElements() / psType->psBase->iBytes; i++)
			{
				mcData.AppendData(psType->psBase->iBytes, StringElement(&auiBytes, psType->psBase->iBytes, i));
			}
			auiBytes.Kill();
			if (i > (size)psType->iCount)
			{
				return Error(pcInitialiser, "The string is longer than the array");
			}
		}
		else if (pcInitialiser->GetKind() == CPPS_InitializerList)
		{
			papcElements = ListElements(pcInitialiser);
			if (papcElements->NumElements() > (size)psType->iCount)
			{
				return Error(pcInitialiser, "Too many initialisers for the array");
			}
			for (i = 0; i < papcElements->NumElements(); i++)
			{
				bResult &= GlobalInitialiser(psType->psBase, papcElements->GetPtr(i), pcNode, pbInitialise);
			}
		}
		else
		{
			return Error(pcInitialiser, "An array needs a list of initialisers");
		}

		for (; i < (size)psType->iCount; i++)
		{
			GlobalInitialiser(psType->psBase, NULL, pcNode, pbInitialise);
		}
		return bResult;
	}

	if (!psType->IsScalar())
	{
		return Error(pcNode, "This type of global variable is not supported by the compiler yet");
	}
	lliValue = 0;
	if (pcInitialiser && psType->IsFloat() && FloatConstant(pcInitialiser, &dValue))
	{
		mcData.AppendData(psType->iBytes, FloatBits(psType, dValue));
		return true;
	}
	if (pcInitialiser && psType->IsPointer() && IsStringLiteral(pcInitialiser) && (psType->psBase->iBytes == StringElementBytes(pcInitialiser)))
	{
		auiBytes.Init();
		bResult = DecodeString(pcInitialiser, &auiBytes);
		if (bResult)
		{
			//AddString ends it with one zero byte.
			auiBytes.GrowToAtLeastNumElements(auiBytes.NumElements() + psType->psBase->iBytes - 1, true, 0);
			mcData.GetText()->Append("\t\t.long Str")->Append(mcData.AddString((char*)auiBytes.GetData(), auiBytes.NumElements()))->AppendNewLine();
		}
		auiBytes.Kill();
		return bResult;
	}
	if (pcInitialiser && (psType->IsFloat() || !Constant(pcInitialiser, &lliValue)))
	{
		//Not a constant: R_Initialise works it out before main, or for a static local its declaration the first time it
		//is reached.  [basic.start.init], [stmt.dcl] 4
		mcData.AppendData(psType->iBytes, 0);
		if (!mcTypeChecker.ImplicitConvert(mcTypeChecker.ValueFor(pcInitialiser, psType, false), psType, pcInitialiser))
		{
			return false;
		}
		*pbInitialise = true;
		return true;
	}
	if (pcInitialiser)
	{
		ConversionOverflow(pcInitialiser, psType);
	}
	mcData.AppendData(psType->iBytes, lliValue);
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
// extern int a[]; declared an array of unknown bound that the definition
// of pcDefined, int a[3], has now completed: the earlier declaration's
// variable, which what was written before the definition names, is of the
// same array.  [dcl.array] 3
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::CompleteDeclaredArrays(CCPPVariable* pcDefined)
{
	CCPPSymbol*		pcSymbol;
	CCPPVariable*	pcDeclared;

	if (!pcDefined->mpsType->IsArray() || !pcDefined->mpcSymbol->mpcScope)
	{
		return;
	}
	for (pcSymbol = pcDefined->mpcSymbol->mpcScope->FindHere(pcDefined->mpcSymbol->mlliName); pcSymbol; pcSymbol = pcSymbol->mpcNext)
	{
		pcDeclared = (pcSymbol != pcDefined->mpcSymbol) ? GetVariable(pcSymbol) : NULL;
		if (pcDeclared && pcDeclared->mpsType->IsArray() && (pcDeclared->mpsType->iCount == 0) && pcDeclared->mpsType->psBase->Same(pcDefined->mpsType->psBase))
		{
			pcDeclared->mpsType = pcDefined->mpsType;
		}
	}
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

	for (i = 0; i < mapcLocals.NumElements(); i++)
	{
		pcVariable = mapcLocals.GetPtr(i);
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
// The variable an earlier declaration of a namespace scope variable's name
// in its scope made: extern int x; int x; is one variable, which can be
// defined once.  NULL if there is none, and in C, where int x; int x; are
// tentative definitions of one.  [basic.def.odr] 1, [basic.link] 9
//
//////////////////////////////////////////////////////////////////////////
CCPPVariable* CCPPUnit::RedeclaredVariable(CCPPSymbol* pcSymbol)
{
	CCPPSymbol*		pcOther;
	CCPPVariable*	pcVariable;

	if (IsCSource() || (pcSymbol->meKind != CPPSK_Variable) || !pcSymbol->mpcScope || (pcSymbol->mpcScope->meKind != CPPSC_Namespace))
	{
		return NULL;
	}
	for (pcOther = pcSymbol->mpcScope->FindHere(pcSymbol->mlliName); pcOther; pcOther = pcOther->mpcNext)
	{
		if ((pcOther != pcSymbol) && (pcOther->mpcScope == pcSymbol->mpcScope) && (pcOther->meKind == CPPSK_Variable))
		{
			pcVariable = GetVariable(pcOther);
			if (pcVariable && (pcVariable->meStorage == CPPVS_Global) && !pcVariable->mpcClass)
			{
				return pcVariable;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether two declarations of a variable give it one type: the same, but
// for an array's bound, which one may leave out.  [basic.link] 10
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::SameVariableType(SCPP65Type* psFirst, SCPP65Type* psSecond)
{
	if (!psFirst || !psSecond || psFirst->Same(psSecond))
	{
		return true;
	}
	if (psFirst->IsArray() && psSecond->IsArray())
	{
		return psFirst->psBase->Same(psSecond->psBase);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A parameter's type in its function's type: an array is a pointer to its
// element, a function a pointer to it, and const and volatile on the
// parameter itself go (f(const int) is f(int)).  [dcl.fct] 5
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::ParameterType(SCPP65Type* psType)
{
	if (psType->IsArray())
	{
		return mcTypes.Pointer(psType->psBase);
	}
	if (psType->IsFunction())
	{
		return mcTypes.Pointer(psType);
	}
	return psType->Unqualified();
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
// char a[] = { "string" } is char a[] = "string": the string, when the initialiser of an array of psElement is a
// list of just a string and the elements are characters.  Otherwise pcInitialiser.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPUnit::StringInitialiser(CCPPSyntax* pcInitialiser, SCPP65Type* psElement)
{
	CArraySyntax*	papcElements;

	if (pcInitialiser && (pcInitialiser->GetKind() == CPPS_InitializerList) && psElement->IsInteger())
	{
		papcElements = ListElements(pcInitialiser);
		if ((papcElements->NumElements() == 1) && IsStringLiteral(papcElements->GetPtr(0)) && (StringElementBytes(papcElements->GetPtr(0)) == psElement->iBytes))
		{
			return papcElements->GetPtr(0);
		}
	}
	return pcInitialiser;
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
		pcClass = GetComponentClass(pcName->GetQualifier(), NULL, pcDefinition);
		if (!pcClass)
		{
			return Error(pcDefinition, "The function's class has not been defined");
		}
		if (!CompleteClass(pcClass, pcDefinition))
		{
			return false;
		}
	}

	if (pcDefinition->meBody == CPPFB_Skipped)
	{
		//A body skipped in a system header: only a declaration.
		BeginQuiet();
		DeclareFunction(pcDefinition->mpcSpecifiers, pcDefinition->mpcDeclarator, false, pcClass, false);
		EndQuiet();
		return true;
	}
	pcFunction = DeclareFunction(pcDefinition->mpcSpecifiers, pcDefinition->mpcDeclarator, true, pcClass, false);
	if (!pcFunction)
	{
		return false;
	}
	if (pcFunction->mpcDefinition || pcFunction->mbGenerated)
	{
		return Error(pcDefinition, "The function is defined twice");
	}
	if (!CheckDefaultedOrDeleted(pcFunction, pcDefinition))
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

	uiErrors = mcErrors.NumErrors();
	GenerateFunction(pcFunction);
	return mcErrors.NumErrors() == uiErrors;
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
		CheckFunction(pcFunction);
		mcContext.LeaveBindings();
		return;
	}

	//Each function is a section of its own.  Inline, template and generated functions are written by every unit
	//that needs them, each in a COMDAT section the linker keeps one of.
	szLabel.Init();
	mcMangler.AppendFunctionLabel(&szLabel, pcFunction);
	bShared = IsSharedFunction(pcFunction);
	if (bShared && !Emit(&szLabel))
	{
		szLabel.Kill();
		return;
	}
	CCPPUnitData::AppendSection(&mszCode, "text", &szLabel, bShared ? ", comdat" : NULL);
	if (!pcFunction->IsUnitFunction())
	{
		mszCode.Append(IsWeakFunction(pcFunction) ? "\t\t.weak " : "\t\t.global ")->Append(&szLabel)->AppendNewLine();
	}
	szLabel.Kill();
	mcContext.EnterBindings(pcFunction->mpcBindings);

	if (pcFunction->mpcCovariantOf)
	{
		//A covariant thunk (CCPPVtable::EntryFor).
		BeginFunction(pcFunction);
		if (!WriteBody(pcFunction))
		{
			szLabel.Init("A covariant result the IR front end can't write a thunk for is not supported on this target yet: ");
			mcMangler.AppendFunctionLabel(&szLabel, pcFunction);
			AppendUnsupported(&szLabel);
			ErrorAt(-1, szLabel.Text());
			szLabel.Kill();
			EndFunction(pcFunction);
		}
	}
	else if (pcFunction->mbImplicit)
	{
		//An implicit default constructor, an inherited one or a destructor, or a copy defaulted after its class.
		BeginFunction(pcFunction);
		if (((pcFunction->mapsParameters.NumElements() != 0) && !pcFunction->mpcInherited && !pcFunction->IsCopy()) || !WriteBody(pcFunction))
		{
			szLabel.Init("An implicit copy constructor or assignment (or one the IR front end can't write) is not supported on this target yet: ");
			mcMangler.AppendFunctionLabel(&szLabel, pcFunction);
			if (pcFunction->mapsParameters.NumElements() == 0)
			{
				AppendUnsupported(&szLabel);
			}
			ErrorAt(-1, szLabel.Text());
			szLabel.Kill();
			EndFunction(pcFunction);
		}
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
		return Error(pcDefinition, "Only constructors have member initialisers");
	}

	BeginFunction(pcFunction);
	AddParameters(pcFunction);
	if (!CheckAbstractSignature(pcFunction, pcDefinition->mpcDeclarator) || !CheckMemberInitialisers(pcFunction) || !AllocateLocals(pcDefinition->mpcBody) || !CheckTryJumps(pcDefinition->mpcBody))
	{
		EndFunction(pcFunction);
		return false;
	}

	//The IR front end writes the whole function.  An error it reported (ReportTypeOf: C++ that is wrong) is the reason
	//when it can't; else it is C++ the front end doesn't do yet.
	uiErrors = mcErrors.NumErrors();
	if (WriteBody(pcFunction))
	{
		return true;
	}
	if (mcErrors.NumErrors() > uiErrors)
	{
		mpcIRFrontEnd->ClearUnsupported();
	}
	else
	{
		szMessage.Init("This function uses C++ the ");
		szMessage.Append(mpcTarget->GetConfigurationName());
		szMessage.Append(" compiler doesn't support yet");
		AppendUnsupported(&szMessage);
		Error(pcDefinition, szMessage.Text());
		szMessage.Kill();
	}
	EndFunction(pcFunction);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The parameters of the function BeginFunction started are its first
// locals.  Arguments take their own size on the stack, the first lowest,
// after the hidden result address of a function that returns an object.
// Object arguments are the function's to destroy.  A function parameter
// pack is a parameter for each of its types, each with its own symbol (see
// ExpandPackCall).  They are named as pcDeclarator names them, or as the
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
				mcFunctionPacks.Add(pcParameter->mpcSymbol);
			}
		}
		for (k = 0; k < uiPack; k++)
		{
			psType = pcFunction->mapsParameters.GetPtr(j);
			pcSymbol = (bPack && pcParameter->mpcSymbol) ? FunctionPackElement(pcFunction, pcParameter->mpcSymbol, k) : pcParameter->mpcSymbol;
			if (pcSymbol)
			{
				pcVariable = AddVariable(&mapcLocals, pcSymbol, ParameterVariableType(psType, pcParameter), CPPVS_Parameter, (int)j);
				pcVariable->mbRestrict = !bPack && pcParameter->mpcDeclarator && IsRestrictPointer(pcParameter->mpcDeclarator);
				if (psType->IsClass())
				{
					//Laid out now, as the IR front end needs it.
					CompleteClass(psType->pcClass, pcParameter);
				}
			}
			j++;
		}
	}

	//A lambda's body expands the parameter packs of the functions the lambda is in too, into the elements it captured
	//(CapturePack).
	psLambda = BodyLambda(pcFunction);
	if (psLambda)
	{
		mcFunctionPacks.AddAll(&psLambda->cFunctionPacks);
	}
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
// Starts generating a function's body.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::BeginFunction(CCPPFunction* pcFunction)
{
	mcContext.BeginFunction(pcFunction);
	FreeVariables(&mapcLocals);
	mcFunctionPacks.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// Leaves the function BeginFunction started, as the IR front end leaves a
// function it writes.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::EndFunction(CCPPFunction* pcFunction)
{
	//Only a function the IR front end couldn't write ends here (it ends the ones it writes itself): nothing is written.
	mcContext.EndFunction();
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

	if ((pcNode->GetKind() == CPPS_InitDeclarator) && pcNode->mpcParent && (pcNode->mpcParent->GetKind() == CPPS_SimpleDeclaration))
	{
		pcInit = (CCPPSyntaxInitDeclarator*)pcNode;
		pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcNode->mpcParent;
		if (pcInit->mpcSymbol && (pcInit->mpcSymbol->meKind == CPPSK_Variable))
		{
			if (pcDeclaration->mpcSpecifiers->muiFlags & CPPDS_EXTERN)
			{
				return ExternLocal(pcInit, pcDeclaration->mpcSpecifiers);
			}
			if (pcDeclaration->mpcSpecifiers->muiFlags & (CPPDS_STATIC | CPPDS_THREAD_LOCAL))
			{
				//thread_local in a block is static too; there is one thread.  [dcl.stc] 3
				return StaticLocal(pcInit, pcDeclaration->mpcSpecifiers);
			}
			psType = mcDeclarators.GetType(pcDeclaration->mpcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
			if (!psType)
			{
				return false;
			}
			if (!psType->IsScalar() && !psType->IsArray() && !psType->IsClass() && !psType->IsReference())
			{
				return Error(pcNode, "This type of local variable is not supported by the compiler yet");
			}
			if ((psType->IsArray() ? psType->psBase : psType)->IsClass() && !CompleteObjectClass((psType->IsArray() ? psType->psBase : psType)->pcClass, pcNode))
			{
				return false;
			}

			AddVariable(&mapcLocals, pcInit->mpcSymbol, psType, CPPVS_Local, 0);
		}
	}

	if ((pcNode->GetKind() == CPPS_RangeForStatement) && !RangeForLocals((CCPPSyntaxRangeForStatement*)pcNode))
	{
		return false;
	}
	if ((pcNode->GetKind() == CPPS_CatchHandler) && !HandlerLocal((CCPPSyntaxCatchHandler*)pcNode))
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

	pcClass = GetUnnamedClass(pcSpecifier);
	if (!pcClass || !CompleteClass(pcClass, pcSpecifier))
	{
		return Error(pcSpecifier, "This anonymous class is not supported by the compiler yet");
	}

	for (i = 0; i < pcClass->macFields.NumElements(); i++)
	{
		psField = pcClass->macFields.Get(i);
		if (!psField->pcSymbol)
		{
			return Error(pcSpecifier, "Anonymous classes in anonymous local classes are not supported by the compiler yet");
		}
		if (psField->IsBitField())
		{
			return Error(pcSpecifier, "Bit-fields in anonymous local classes are not supported by the compiler yet");
		}
		pcVariable = AddVariable(&mapcLocals, psField->pcSymbol, psField->psType, CPPVS_Local, psField->iOffset);
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
				BeginQuiet();
				psType = mcDeclarators.GetType(pcDeclaration->mpcSpecifiers, pcInit->mpcDeclarator, pcInit, NULL);
				EndQuiet();
				if (!psType)
				{
					break;
				}
			}
			if (pcOther->meKind == CPPSK_Typedef)
			{
				BeginQuiet();
				psOther = mcDeclarators.GetSymbolType(pcOther, pcInit);
				EndQuiet();
				if (psOther && !psOther->Same(psType))
				{
					bResult &= Error(pcInit, "The typedef is declared again for another type");
				}
			}
			else if (!((psType->IsClass() && psType->pcClass->mpcSymbol == pcOther) || (psType->pcEnum == pcOther)))
			{
				bResult &= Error(pcInit, "A typedef of a class's or an enum's name in its scope names that class or enum");
			}
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// An anonymous union at namespace scope: one object, zero, whose label is
// its first member's, and a global variable for each member at its offset
// in it, internal as the union is (static or in an unnamed namespace).
// [class.union] 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::AnonymousGlobal(CCPPSyntaxClassSpecifier* pcSpecifier)
{
	CCPPClass*		pcClass;
	SCPPField*		psField;
	CCPPVariable*	pcVariable;
	CCPPVariable*	pcObject;
	CChars			szLabel;
	size			i;

	pcClass = GetUnnamedClass(pcSpecifier);
	if (!pcClass || !CompleteClass(pcClass, pcSpecifier) || !pcClass->mbUnion)
	{
		return Error(pcSpecifier, "This anonymous class is not supported by the compiler yet");
	}
	pcObject = NULL;
	for (i = 0; i < pcClass->macFields.NumElements(); i++)
	{
		psField = pcClass->macFields.Get(i);
		if (!psField->pcSymbol || psField->IsBitField())
		{
			return Error(pcSpecifier, "This anonymous union's member is not supported by the compiler yet");
		}
		pcVariable = AddVariable(&mapcGlobals, psField->pcSymbol, psField->psType, CPPVS_Global, psField->iOffset);
		pcVariable->mpcAnonymous = pcClass;
		pcVariable->mbInternal = true;
		pcVariable->mbDefined = true;
		pcObject = pcObject ? pcObject : pcVariable;
		pcVariable->mpcAnonymousObject = pcObject;
	}
	if (!pcObject)
	{
		return true;
	}
	szLabel.Init();
	AppendVariableLabel(&szLabel, pcObject);
	mcData.BeginData(&szLabel, DataAlignment(pcClass->mpsType, true), false, NULL, IsBss(NULL, pcClass->mpsType));
	if (IsBss(NULL, pcClass->mpsType))
	{
		mcData.AppendSpace(pcClass->mpsType->iBytes);
	}
	else
	{
		mcData.AppendZeros(pcClass->mpsType->iBytes);
	}
	mcData.NextStartUp();
	szLabel.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// extern int k; in a block declares the variable of the namespace it is in,
// defined elsewhere (before or after the function, or in another unit): a
// variable of the function's with the global's label.  [basic.link] 6
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::ExternLocal(CCPPSyntaxInitDeclarator* pcInit, CCPPSyntaxDeclSpecifiers* pcSpecifiers)
{
	SCPP65Type*		psType;
	CCPPVariable*	pcGlobal;

	if (pcInit->mpcInitializer)
	{
		return Error(pcInit, "An extern variable declared in a block can't be initialised there");
	}
	psType = mcDeclarators.GetType(pcSpecifiers, pcInit->mpcDeclarator, pcInit, NULL);
	if (!psType)
	{
		return false;
	}
	pcGlobal = NamespaceVariable(pcInit->mpcSymbol);
	if (pcGlobal && !SameVariableType(pcGlobal->mpsType, psType))
	{
		//It names the variable of the namespace it is in, which has one type.  [basic.link] 6, 10
		return Error(pcInit, "The variable is declared again with another type");
	}
	AddVariable(&mapcLocals, pcInit->mpcSymbol, psType, CPPVS_Global, 0);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The variable of the innermost namespace a block's extern declaration is
// in that it names, if that namespace declares one.  [basic.link] 6
//
//////////////////////////////////////////////////////////////////////////
CCPPVariable* CCPPUnit::NamespaceVariable(CCPPSymbol* pcSymbol)
{
	CCPPScope*		pcNamespace;
	CCPPSymbol*		pcOther;
	CCPPVariable*	pcVariable;

	pcNamespace = pcSymbol->mpcScope ? pcSymbol->mpcScope->GetEnclosingNamespace() : NULL;
	if (!pcNamespace)
	{
		return NULL;
	}
	for (pcOther = pcNamespace->FindHere(pcSymbol->mlliName); pcOther; pcOther = pcOther->mpcNext)
	{
		if ((pcOther->mpcScope == pcNamespace) && (pcOther->meKind == CPPSK_Variable))
		{
			pcVariable = GetVariable(pcOther);
			if (pcVariable)
			{
				return pcVariable;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A static local variable is data with its own label, initialised in the
// data.  An object, a reference, or a variable whose initialiser isn't a
// constant is initialised the first time its declaration is reached
// (CCPP65IRFrontEnd::Declaration), with a guard byte beside it that says
// whether it has been (mbGuarded).  An object whose class needs a
// destructor is destroyed when the program ends: once made it is pushed
// on an exit list (the thread's for a thread_local), with the R_Finalise
// WriteExitRoutines writes.  [stmt.dcl] 4, [basic.start.term] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::StaticLocal(CCPPSyntaxInitDeclarator* pcInit, CCPPSyntaxDeclSpecifiers* pcSpecifiers)
{
	CCPPVariable*		pcVariable;
	CCPPVariable*		pcKept;
	SCPP65Type*			psType;
	SCPP65Type*			psElement;
	CChars				szLabel;
	bool				bResult;
	bool				bInitialise;
	bool				bObject;
	CCPPFunction*		pcShared;
	const char*			szOptions;
	size				uiData;
	bool				bFirst;
	size				i;
	int					iNumber;

	psType = mcDeclarators.GetType(pcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
	if (!psType)
	{
		return false;
	}
	psElement = psType->IsArray() ? psType->psBase : psType;
	bObject = psElement->IsClass() || psType->IsReference();
	if (psElement->IsClass() && !CompleteClass(psElement->pcClass, pcInit))
	{
		return false;
	}

	//In a function every unit may write it is one object: labelled for the function and its number there, its data COMDAT.
	pcShared = IsSharedFunction(mcContext.GetFunction()) ? mcContext.GetFunction() : NULL;
	iNumber = miStatics;
	if (pcShared)
	{
		iNumber = 0;
		for (i = 0; i < mapcLocals.NumElements(); i++)
		{
			iNumber += (mapcLocals.GetPtr(i)->meStorage == CPPVS_Static) ? 1 : 0;
		}
	}
	szOptions = pcShared ? ", comdat" : NULL;
	pcVariable = AddVariable(&mapcLocals, pcInit->mpcSymbol, psType, CPPVS_Static, iNumber);
	miStatics++;
	pcVariable->mbDefined = true;
	pcVariable->mbThreadLocal = (pcSpecifiers->muiFlags & CPPDS_THREAD_LOCAL) != 0;
	pcVariable->mpcSharedIn = pcShared;
	pcKept = EnclosingStatic(pcInit->mpcSymbol);
	if (!pcKept)
	{
		pcKept = AddVariable(&mapcStaticLocals, pcInit->mpcSymbol, psType, CPPVS_Static, iNumber);
	}
	pcKept->miOffset = iNumber;
	pcKept->mpsType = psType;
	pcKept->mbDefined = true;
	pcKept->mbThreadLocal = pcVariable->mbThreadLocal;
	pcKept->mpcSharedIn = pcShared;
	szLabel.Init();
	AppendVariableLabel(&szLabel, pcVariable);
	//A function written again in the unit has its static locals again: a shared one's data is the unit's once.
	bFirst = !pcShared || Emit(&szLabel);
	uiData = mcData.GetText()->Length();
	mcData.BeginData(&szLabel, VariableDataAlignment(pcVariable, psElement->IsClass()), pcShared != NULL, szOptions, bObject || IsBss(pcInit->mpcInitializer, psType));
	szLabel.Kill();
	bResult = true;
	bInitialise = false;
	if (bObject || IsBss(pcInit->mpcInitializer, psType))
	{
		mcData.AppendSpace(psType->iBytes);
	}
	else
	{
		mcContext.EnterStaticLocal();
		bResult = GlobalInitialiser(psType, pcInit->mpcInitializer, pcInit, &bInitialise);
		mcContext.LeaveStaticLocal();
	}

	pcVariable->mbGuarded = bResult && (bObject || bInitialise);
	if (pcVariable->mbGuarded)
	{
		szLabel.Init();
		AppendGuardLabel(&szLabel, pcVariable);
		mcData.BeginData(&szLabel, 1, pcShared != NULL, szOptions, true);
		mcData.AppendSpace(1);
		szLabel.Kill();
	}
	if (pcVariable->mbGuarded && psElement->IsClass() && psElement->pcClass->NeedsDestructor())
	{
		pcVariable->mbExitNode = true;
		szLabel.Init();
		AppendVariableLabel(&szLabel, pcVariable);
		if (bFirst)
		{
			maszExitFinalisers.Add(szLabel.Text());
			mapsExitFinalisers.Add(psType);
			maiExitShared.Add(pcShared != NULL);
		}
		szLabel.Append("__ExitNode");
		mcData.BeginData(&szLabel, 4, pcShared != NULL, szOptions, true);
		mcData.AppendSpace(2 * mcTypes.Pointer(&mcTypes.msVoid)->iBytes);
		szLabel.Kill();
		UseExitLists();
	}
	if (!bFirst)
	{
		mcData.GetText()->SetLength(uiData);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The label of a guarded static local's guard (G, its number, _ and its
// name), as its own is S, its number, _ and its name.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::AppendGuardLabel(CChars* psz, CCPPVariable* pcVariable)
{
	char*	szName;
	int		iLength;

	szName = mpcTokeniser->GetIdentifier(pcVariable->mpcSymbol->mlliName, &iLength);
	if (pcVariable->mpcSharedIn)
	{
		mcMangler.AppendFunctionLabel(psz, pcVariable->mpcSharedIn);
		psz->Append("__");
	}
	psz->Append('G');
	psz->Append(pcVariable->miOffset);
	psz->Append('_');
	psz->Append(szName, iLength);
}


//////////////////////////////////////////////////////////////////////////
//
// The label of a global, a static data member or a static local (S, its
// number, _ and its name).  A global is used once its label is written.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::AppendVariableLabel(CChars* psz, CCPPVariable* pcVariable)
{
	char*	szName;
	int		iLength;

	if (pcVariable->meStorage == CPPVS_Static)
	{
		szName = mpcTokeniser->GetIdentifier(pcVariable->mpcSymbol->mlliName, &iLength);
		if (pcVariable->mpcSharedIn)
		{
			//A shared function's: its label, __S and its number in the function.
			mcMangler.AppendFunctionLabel(psz, pcVariable->mpcSharedIn);
			psz->Append("__");
		}
		psz->Append('S');
		psz->Append(pcVariable->miOffset);
		psz->Append('_');
		psz->Append(szName, iLength);
		return;
	}
	pcVariable->mbUsed = true;
	mcMangler.AppendGlobalLabel(psz, pcVariable);
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
			bResult &= CheckEnumerators(((CCPPSyntaxSimpleDeclaration*)pcMember)->mpcSpecifiers->mpcType);
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// An enum's enumerators when it is declared (pcType, a declaration's
// type, may be anything else): a value given that is definitely no
// constant (it overflows, divides by zero, takes too long ...) is an error
// even if nothing uses it.  One the compiler can't work out yet is left to
// where it is used.  [dcl.enum] 2
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckEnumerators(CCPPSyntax* pcType)
{
	CCPPSyntaxEnumSpecifier*	pcSpecifier;
	CCPPSyntaxEnumerator*		pcEnumerator;
	const char*					szReason;
	int64						lliValue;
	size						uiErrors;
	size						i;
	bool						bConstant;
	bool						bResult;

	if (!pcType || (pcType->GetKind() != CPPS_EnumSpecifier) || !((CCPPSyntaxEnumSpecifier*)pcType)->mbBody)
	{
		return true;
	}
	pcSpecifier = (CCPPSyntaxEnumSpecifier*)pcType;
	bResult = true;
	for (i = 0; i < pcSpecifier->macpEnumerators.NumElements(); i++)
	{
		pcEnumerator = (CCPPSyntaxEnumerator*)pcSpecifier->macpEnumerators.GetPtr(i);
		if (!pcEnumerator->mpcValue)
		{
			continue;
		}
		BeginQuiet();
		bConstant = Constant(pcEnumerator->mpcValue, &lliValue);
		EndQuiet();
		szReason = mpcConstants->GetReason();
		if (!bConstant && (szReason[0] != '\0') && (strncmp(szReason, "It uses what", 12) != 0) && (strncmp(szReason, "Its initialiser uses what", 25) != 0))
		{
			bResult &= ConstantError(pcEnumerator->mpcValue, "An enumerator's value must be a constant");
		}
	}
	if (bResult && pcSpecifier->mpcSymbol && pcSpecifier->mpcBase)
	{
		//A fixed underlying type holds each value, whether or not the enum is used (EnumType says where one doesn't).
		//[dcl.enum] 5
		uiErrors = mcErrors.NumErrors();
		EnumType(pcSpecifier->mpcSymbol, pcSpecifier);
		bResult = mcErrors.NumErrors() == uiErrors;
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A static_assert whose condition is a constant that is false is an error.  One the generator cannot work out
// (sizeof a member function pointer ...) is ignored.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::StaticAssert(CCPPSyntaxStaticAssert* pcAssert)
{
	int64	lliValue;
	size	uiErrors;
	bool	bConstant;

	uiErrors = mcErrors.NumErrors();
	mpcConstants->ClearReason();
	BeginQuiet();
	bConstant = Constant(pcAssert->mpcCondition, &lliValue);
	EndQuiet();
	if (bConstant && (lliValue == 0))
	{
		return Error(pcAssert, "The static_assert failed");
	}
	if (!bConstant && (mcErrors.NumErrors() == uiErrors))
	{
		BeginQuiet();
		bConstant = mpcConstants->Value(pcAssert->mpcCondition, &mcTypes.msBool, &lliValue);
		EndQuiet();
		if (bConstant && (lliValue == 0))
		{
			return Error(pcAssert, "The static_assert failed");
		}
	}
	if (!bConstant && (mpcConstants->GetReason()[0] != '\0') && (strncmp(mpcConstants->GetReason(), "Its initialiser uses", 20) != 0))
	{
		//What can never be a constant (not what the compiler can't work out yet).  [dcl.dcl] 4
		return ConstantError(pcAssert->mpcCondition, "A static_assert's expression is a constant");
	}
	return mcErrors.NumErrors() == uiErrors;
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
	ErrorAt(-1, szMessage);
}


CCPP65Types* CCPPUnit::GetTypes(void) { return &mcTypes; }
CCPPContext* CCPPUnit::GetContext(void) { return &mcContext; }
CCPPBinder* CCPPUnit::GetBinder(void) { return &mcBinder; }
CCPPConstants* CCPPUnit::GetConstants(void) const { return mpcConstants; }
CCPPDeclarators* CCPPUnit::GetDeclarators(void) { return &mcDeclarators; }
CCPPMangler* CCPPUnit::GetMangler(void) { return &mcMangler; }
CCPPUnitData* CCPPUnit::GetData(void) { return &mcData; }
CCPP65TypeChecker* CCPPUnit::GetTypeChecker(void) { return &mcTypeChecker; }
CChars* CCPPUnit::GetCode(void) { return &mszCode; }
CArrayCPPTemplates* CCPPUnit::GetTemplates(void) { return &mapcTemplates; }
CArrayCPPVariables* CCPPUnit::GetLocals(void) { return &mapcLocals; }
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
bool CCPPUnit::IsUnevaluated(void) const { return miUnevaluated > 0; }
