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
#include "CTString.h"
#include "CppTokeniser.h"
#include "CPPSyntaxTree.h"
#include "CPP65IRFrontEnd.h"
#include "CPP65Generator.h"


//////////////////////////////////////////////////////////////////////////
//
// Small functions of other units the IR front end's inliner may use; this
// unit's are added to pcImports when it is finished (CCPP65IRFrontEnd::
// FinishUnit), in its turn uiTurn (CIRImports::BeginTurn).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::SetImports(CIRImports* pcImports, size uiTurn)
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
bool CCPP65Generator::MissedImports(void)
{
	return mbMissedImports;
}


//////////////////////////////////////////////////////////////////////////
//
// After GenerateBeforeTurn: the functions the IR front end looked for in
// the imports and didn't find, NULL if it has none.
//
//////////////////////////////////////////////////////////////////////////
CArrayChars* CCPP65Generator::GetNotImported(void)
{
	return mpcIRFrontEnd->GetNotImported();
}


//////////////////////////////////////////////////////////////////////////
//
// A profile of the program's functions: the W65C816 back end gives those it
// measured worth it page aligned frames (CIR816Backend::AlignFrame).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::SetFrameProfile(CAsm65816Profile* pcProfile)
{
	mpcFrameProfile = pcProfile;
}


//////////////////////////////////////////////////////////////////////////
//
// The code the IR front end writes gets a comment with the C++ source line
// before the code for it ("; File.cpp(12): text"), for a listing.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::SetListing(bool bListing)
{
	mbListing = bListing;
}


//////////////////////////////////////////////////////////////////////////
//
// The arena the W65C816 back end's working arrays and strings are in while
// it writes a function (CCPP65IRFrontEnd::GenerateCode, CIR816Backend::
// SetScratch).
//
//////////////////////////////////////////////////////////////////////////
CScratchPadAllocator* CCPP65Generator::GetBackendScratch(void)
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
void CCPP65Generator::SetSpillStore(CSpillStore* pcStore, uint32 uiUnit)
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
void CCPP65Generator::SetStatistics(CCPPCompileStatistics* pcStatistics)
{
	mpcStatistics = pcStatistics ? pcStatistics : &mcOwnStatistics;
	mpcIRFrontEnd->SetStatistics(mpcStatistics);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPCompileStatistics* CCPP65Generator::GetStatistics(void)
{
	return mpcStatistics;
}


//////////////////////////////////////////////////////////////////////////
//
// How the unit is compiled where there is a choice: a builder's options, or
// the defaults for NULL.  Before anything is generated.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::SetOptions(CCPPCompileOptions* pcOptions)
{
	mpcOptions = pcOptions ? pcOptions : GetDefaultCPPCompileOptions();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPCompileOptions* CCPP65Generator::GetOptions(void)
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
void CCPP65Generator::SetLineTable(bool bLineTable, int64 lliCompileTime)
{
	mbLineTable = bLineTable;
	mlliCompileTime = lliCompileTime;
}


//////////////////////////////////////////////////////////////////////////
//
// Floating point in any code the unit writes (a float or double value,
// variable, literal or conversion) is an error at its statement.  Only
// what is written: a declaration that is never used (a header's float
// function) is not.  See CCPP65Builder::SetNoFloats.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::SetNoFloats(bool bNoFloats)
{
	mbNoFloats = bNoFloats;
}


//////////////////////////////////////////////////////////////////////////
//
// The functions the unit calls that were neither its own nor in the
// imports when it was finished (a later unit may export them).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::GetMissingCallees(CArrayChars* paszNames)
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
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::Init(CCPPSyntaxTree* pcTree, ECPPTarget eTarget)
{
	mpcTree = pcTree;
	mpcTokeniser = pcTree->GetTokeniser();
	mpcTarget = GetCPPTarget(eTarget);
	mcTypes.Init(mpcTarget->GetIntBytes(), mpcTarget->GetPointerBytes());

	mszCode.Init();
	mszData.Init();
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
	mlliCompileTime = 0;
	mcMemorySpillStore.Init();
	mpcSpillStore = &mcMemorySpillStore;
	muiSpillUnit = 0;
	mauiCodePieces.Init();
	mpcSemanticPass = NULL;
	mcOwnStatistics.Init();
	mpcStatistics = &mcOwnStatistics;
	mpcOptions = GetDefaultCPPCompileOptions();

	//Every function body is written as IR (and generated by the target's IR back end).
	mpcIRFrontEnd = NewMalloc<CCPP65IRFrontEnd>();
	mpcIRFrontEnd->Init(this);
	mcTypeChecker.Init(this);
	mcErrors.Init();
	mcWarnings.Init();

	mapcGlobals.Init();
	mapcFunctions.Init();
	mcFunctionsOfNames.Init(true);
	mpcDeclaringNamespace = NULL;
	mapcUnnamedNamespaces.Init();
	mapcClasses.Init();
	mapcTemplates.Init();
	mapcAllBindings.Init();
	mapapsPacks.Init();
	mapcPackElements.Init();
	mapcPackSymbols.Init();
	mpcPackParameter = NULL;
	mapcQueue.Init();
	muiQueueNext = 0;
	mapcFailed.Init();
	mpcBindings = NULL;
	mcTypeOfs.Init(true);
	maszEmitted.Init();
	miQuiet = 0;
	miWarningChecks = 0;
	mapcConstantTypes.Init();
	mbQuietFailed = false;
	maszStrings.Init();
	miStartUps = 0;
	mbDataBss = false;
	mbPureVirtual = false;

	mapcLocals.Init();
	mpcFunction = NULL;
	miStatics = 0;
	mbStaticLocal = false;
	mbLinkageExtern = false;
	mbCLinkage = false;
	mbThis = false;
	miLabels = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::Kill(void)
{
	size				i;
	size				j;
	CCPPFunction*		pcFunction;
	CCPPClass*		pcClass;
	CCPPTemplateArguments*		pcBindings;
	CArrayCPP65Types*	papsPack;

	mcTypeChecker.Kill();
	SafeKill(mpcIRFrontEnd);
	FreeVariables(&mapcLocals);
	mapcLocals.Kill();
	mcBackendScratch.Kill();

	for (i = 0; i < maszStrings.NumElements(); i++)
	{
		maszStrings.Get(i)->Kill();
	}
	maszStrings.Kill();
	for (i = 0; i < mapcClasses.NumElements(); i++)
	{
		pcClass = mapcClasses.GetPtr(i);
		pcClass->mszArguments.Kill();
		for (j = 0; j < pcClass->macVtables.NumElements(); j++)
		{
			pcClass->macVtables.Get(j)->maiAdjust.Kill();
			pcClass->macVtables.Get(j)->mapcFunctions.Kill();
		}
		pcClass->macVtables.Kill();
		pcClass->mapcVirtuals.Kill();
		pcClass->mapcMembers.Kill();
		pcClass->macFields.Kill();
		pcClass->macUsings.Kill();
		pcClass->macBases.Kill();
		SafeFree(pcClass);
	}
	mapcClasses.Kill();
	for (i = 0; i < mapcFunctions.NumElements(); i++)
	{
		pcFunction = mapcFunctions.GetPtr(i);
		pcFunction->mszArguments.Kill();
		pcFunction->mapcDefaults.Kill();
		pcFunction->mapsParameters.Kill();
		SafeFree(pcFunction);
	}
	mapcUnnamedNamespaces.Kill();
	mcFunctionsOfNames.Kill();
	mapcFunctions.Kill();
	for (i = 0; i < mapcTemplates.NumElements(); i++)
	{
		mapcTemplates.GetPtr(i)->mapcInstances.Kill();
		mapcTemplates.GetPtr(i)->mszArguments.Kill();
		free(mapcTemplates.GetPtr(i));
	}
	mapcTemplates.Kill();
	for (i = 0; i < mapcAllBindings.NumElements(); i++)
	{
		pcBindings = mapcAllBindings.GetPtr(i);
		pcBindings->macBindings.Kill();
		SafeFree(pcBindings);
	}
	mapcAllBindings.Kill();
	for (i = 0; i < mapapsPacks.NumElements(); i++)
	{
		papsPack = mapapsPacks.GetPtr(i);
		SafeKill(papsPack);
	}
	mapapsPacks.Kill();
	mapcPackElements.Kill();
	for (i = 0; i < mapcPackSymbols.NumElements(); i++)
	{
		free(mapcPackSymbols.GetPtr(i));
	}
	mapcPackSymbols.Kill();
	mapcConstantTypes.Kill();
	mapcQueue.Kill();
	mapcFailed.Kill();
	mcTypeOfs.Kill();
	maszEmitted.Kill();
	FreeVariables(&mapcGlobals);
	mapcGlobals.Kill();

	mcWarnings.Kill();
	mcErrors.Kill();
	maszRuntime.Kill();
	mszThunks.Kill();
	mszData.Kill();
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
bool CCPP65Generator::Generate(CChars* pszDest)
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
bool CCPP65Generator::Generate(CCPP65Assembly* pcDest)
{
	GenerateBeforeTurn();
	return GenerateAfterTurn(pcDest);
}


//////////////////////////////////////////////////////////////////////////
//
// The first part of Generate, which can be done before the unit's turn with
// the imports (SetImports): the functions are generated and inlined, with
// the imports exported so far.  GenerateAfterTurn does the rest, in the
// unit's turn.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::GenerateBeforeTurn(void)
{
	CCPPSyntaxTranslationUnit*	pcRoot;

	mpcStatistics->Begin(CPPCP_Declarations);
	pcRoot = mpcTree->GetRoot();
	if (pcRoot)
	{
		Declarations(&pcRoot->macpDeclarations);
	}

	//Functions generated when they are first called, which may call others.
	GenerateQueued();
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
bool CCPP65Generator::GenerateAfterTurn(CCPP65Assembly* pcDest)
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
	AppendStrings(&szStrings);
	pszText = pcDest->GetText();
	pszText->Append(mszCode.Text(), mszCode.Length());
	pszText->Append(mszThunks.Text(), mszThunks.Length());
	pszText->Append(mszData.Text(), mszData.Length());
	pszText->Append(szStrings.Text(), szStrings.Length());
	szStrings.Kill();
	pcDest->SetPieces(&mauiCodePieces);
	return mcErrors.NumErrors() == 0;
}


//////////////////////////////////////////////////////////////////////////
//
// A start up routine's entry in the init (or fini) table, kept only while the global's data section is.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendStartUpEntry(const char* szTable, CChars* pszLabel, CChars* pszData)
{
	CChars	szOptions;

	szOptions.Init(mbDataBss ? ", associated bss." : ", associated data.");
	szOptions.Append(pszData);
	AppendSection(&mszData, szTable, pszLabel, szOptions.Text());

	//Aligned, so an entry never crosses the end of a W65C816 bank (the linker would move it on, leaving a gap the start
	//up would read as an entry).
	mszData.Append("\t\t.align 4\n");
	mszData.Append("\t\t.long ");
	mszData.Append(pszLabel);
	mszData.AppendNewLine();
	szOptions.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A global's R_Initialise and R_Finalise are written by the IR front end (CCPP65IRFrontEnd::GlobalRoutine).  bConstruct when it needs constructing, bDestroy when it needs
// destroying.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::GlobalStartUpRoutines(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CChars* pszData, bool bConstruct, bool bDestroy)
{
	CChars	szLabel;
	bool	bResult;
	int		i;

	bResult = true;
	for (i = 0; i < 2; i++)
	{
		if (!((i == 0) ? bConstruct : bDestroy))
		{
			continue;
		}
		szLabel.Init((i == 0) ? "R_Initialise" : "R_Finalise");
		szLabel.Append(miStartUps);
		AppendSection(&mszCode, "text", &szLabel, NULL);
		if (mpcIRFrontEnd->GlobalRoutine(szLabel.Text(), pszData->Text(), psType, pcInitialiser, i == 1))
		{
			AppendStartUpEntry((i == 0) ? "init" : "fini", &szLabel, pszData);
		}
		else
		{
			bResult = ErrorAt(-1, "This global's construction or destruction uses C++ the IR front end doesn't support yet");
		}
		szLabel.Kill();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A whole program from one translation unit, as one object: the target's start up
// (CCPPTarget::AppendStartUp), then the translation unit.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::GenerateProgram(CChars* pszDest)
{
	mpcTarget->AppendStartUp(pszDest);
	return Generate(pszDest);
}


//////////////////////////////////////////////////////////////////////////
//
// ".section kind.name" and szOptions (", comdat" for code or data every unit that
// uses it writes: inline, template and generated functions, vtables, thunks and
// runtime routines, so the linker keeps one copy).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendSection(CChars* psz, const char* szKind, CChars* pszName, const char* szOptions)
{
	psz->Append("\t\t.section ");
	psz->Append(szKind);
	psz->Append('.');
	psz->Append(pszName);
	if (szOptions)
	{
		psz->Append(szOptions);
	}
	psz->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// A data item's section (data.label, or bss.label if bBss, and szOptions),
// alignment, .global (if bGlobal) and label.  Its data follows (only .space
// in bss).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::BeginData(CChars* pszLabel, int iAlignment, bool bGlobal, const char* szOptions, bool bBss)
{
	AppendSection(&mszData, bBss ? "bss" : "data", pszLabel, szOptions);
	mbDataBss = bBss;
	if (iAlignment > 1)
	{
		mszData.Append("\t\t.align ");
		mszData.Append(iAlignment);
		mszData.AppendNewLine();
	}
	if (bGlobal)
	{
		AppendGlobal(&mszData, pszLabel);
	}
	mszData.Append(pszLabel);
	mszData.Append(':');
	mszData.AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// A global or static local with no initialiser starts as zeros: on a
// target whose start up clears bss (CCPPTarget::HasZeroedBss) it goes
// there, and takes no space in the program's image.  An object is cleared
// there too, before its constructor runs from the init table.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsBss(CCPPSyntax* pcInitialiser, SCPP65Type* psType)
{
	return mpcTarget->HasZeroedBss() && !pcInitialiser && !psType->IsReference() && (psType->iBytes > 0);
}


//////////////////////////////////////////////////////////////////////////
//
// iBytes of a bss item.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendSpace(int iBytes)
{
	mszData.Append("\t\t.space ");
	mszData.Append(iBytes);
	mszData.AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// 4 for objects and data of 4 bytes or more, 2 for 2 bytes, otherwise 1.
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::DataAlignment(SCPP65Type* psType, bool bClass)
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
// True if the declarator declares a pointer that is itself __restrict
// (T* __restrict p): its outermost pointer is qualified.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsRestrictPointer(CCPPSyntaxDeclarator* pcDeclarator)
{
	CCPPSyntaxPointerOperator*	pcPointer;
	CCPPSyntax*					pcQualifier;
	ECppTokens					eKeyword;
	size						i;

	if (pcDeclarator->mpcNested || (pcDeclarator->macpSuffixes.NumElements() != 0) || (pcDeclarator->NumPointers() == 0))
	{
		return false;
	}
	pcPointer = pcDeclarator->GetPointer(pcDeclarator->NumPointers() - 1);
	if (pcPointer->meKind != CPPP_Pointer)
	{
		return false;
	}
	for (i = 0; i < pcPointer->macpQualifiers.NumElements(); i++)
	{
		pcQualifier = pcPointer->macpQualifiers.GetPtr(i);
		if (pcQualifier->GetKind() == CPPS_KeywordSpecifier)
		{
			eKeyword = ((CCPPSyntaxKeywordSpecifier*)pcQualifier)->meKeyword;
			if ((eKeyword == CT___restrict) || (eKeyword == CT___restrict__))
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// psType with a pointer operator's qualifiers (T* const p, T* volatile p):
// the pointer itself is const or volatile.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::QualifiedPointer(SCPP65Type* psType, CCPPSyntaxPointerOperator* pcPointer)
{
	CCPPSyntax*	pcQualifier;
	ECppTokens	eKeyword;
	size		i;
	bool		bConst;
	bool		bVolatile;

	bConst = false;
	bVolatile = false;
	for (i = 0; i < pcPointer->macpQualifiers.NumElements(); i++)
	{
		pcQualifier = pcPointer->macpQualifiers.GetPtr(i);
		if (pcQualifier->GetKind() == CPPS_KeywordSpecifier)
		{
			eKeyword = ((CCPPSyntaxKeywordSpecifier*)pcQualifier)->meKeyword;
			bConst |= (eKeyword == CT_const) || (eKeyword == CT___const__);
			bVolatile |= (eKeyword == CT_volatile) || (eKeyword == CT___volatile__);
		}
	}
	return mcTypes.Qualified(psType, bConst, bVolatile);
}


//////////////////////////////////////////////////////////////////////////
//
// __attribute__((used)) (or __used__) on a declarator: the global is kept even if
// nothing uses it (its constructor may register it somewhere).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::HasUsedAttribute(CCPPSyntaxDeclarator* pcDeclarator)
{
	size		i;
	int32		iToken;
	CCPPSyntax*	pcAttribute;
	CCPPToken*	pcToken;
	char*		szName;
	int			iLength;

	for (i = 0; i < pcDeclarator->macpAttributes.NumElements(); i++)
	{
		pcAttribute = pcDeclarator->macpAttributes.GetPtr(i);
		for (iToken = pcAttribute->miFirstToken; iToken <= pcAttribute->miLastToken; iToken++)
		{
			pcToken = mpcTokeniser->Get(iToken);
			if (pcToken->IsIdentifier())
			{
				szName = mpcTokeniser->GetIdentifier(((CCTIdentifier*)pcToken)->miIdentifier, &iLength);
				if (((iLength == 4) && (memcmp(szName, "used", 4) == 0)) || ((iLength == 8) && (memcmp(szName, "__used__", 8) == 0)))
				{
					return true;
				}
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The label is visible to the other objects.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendGlobal(CChars* psz, CChars* pszLabel)
{
	psz->Append("\t\t.global ");
	psz->Append(pszLabel);
	psz->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// A static function, a member of a class without a name, or what is in a
// namespace without a name is the unit's own: its label is local to the object.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsUnitFunction(CCPPFunction* pcFunction)
{
	CCPPClass*	pcClass;

	pcClass = pcFunction->mpcClass;
	if (pcClass)
	{
		return (pcClass->mlliName < 0) || (pcClass->mpcSymbol && pcClass->mpcSymbol->mpcScope->IsInUnnamedNamespace());
	}
	return pcFunction->mbStatic || (pcFunction->mpcNamespace && pcFunction->mpcNamespace->IsInUnnamedNamespace());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPErrors* CCPP65Generator::GetErrors(void)
{
	//Their tokens are C++ token indices.
	return &mcErrors;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPErrors* CCPP65Generator::GetWarnings(void)
{
	return &mcWarnings;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::NumErrors(void)
{
	return (int)mcErrors.NumErrors();
}


//////////////////////////////////////////////////////////////////////////
//
// The labels of functions and global variables that were used but not defined in
// the translation unit.  They must come from somewhere else.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::GetExternals(CArrayChars* paszLabels)
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
			AppendGlobalLabel(&szLabel, pcVariable);
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
			AppendFunctionLabel(&szLabel, pcFunction);
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
// Errors are "File (line, column): message [token]", like the other stages'.
// In quiet mode (declarations, which only fail when what they declare is used) they
// are only noted.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::Error(CCPPSyntax* pcNode, const char* szMessage)
{
	if (miQuiet > 0)
	{
		mbQuietFailed = true;
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
void CCPP65Generator::Warning(CCPPSyntax* pcNode, const char* szMessage)
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
// Whether a warning containing szText is at one of pcNode's tokens.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::HasWarning(CCPPSyntax* pcNode, const char* szText)
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
bool CCPP65Generator::ErrorAt(int iToken, const char* szMessage)
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
void CCPP65Generator::AppendMacroUse(SPreprocessedToken* psSource, CChars* pszMessage)
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
bool CCPP65Generator::ReportError(CCPPSyntax* pcNode, const char* szMessage)
{
	int		iQuiet;
	bool	bQuietFailed;

	iQuiet = miQuiet;
	bQuietFailed = mbQuietFailed;
	miQuiet = 0;
	Error(pcNode, szMessage);
	miQuiet = iQuiet;
	mbQuietFailed = bQuietFailed;
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// ReportError at a token.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::ReportErrorAt(int iToken, const char* szMessage)
{
	int		iQuiet;
	bool	bQuietFailed;

	iQuiet = miQuiet;
	bQuietFailed = mbQuietFailed;
	miQuiet = 0;
	ErrorAt(iToken, szMessage);
	miQuiet = iQuiet;
	mbQuietFailed = bQuietFailed;
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Declarations of things the generator cannot do yet are only errors when they
// are used.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::BeginQuiet(void)
{
	if (miQuiet == 0)
	{
		mbQuietFailed = false;
		mpcStatistics->Count(CPPCC_QuietTrials);
	}
	miQuiet++;
}


//////////////////////////////////////////////////////////////////////////
//
// Returns false if an error happened since BeginQuiet.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::EndQuiet(void)
{
	miQuiet--;
	if ((miQuiet == 0) && mbQuietFailed)
	{
		mpcStatistics->Count(CPPCC_QuietTrialsFailed);
	}
	return !mbQuietFailed;
}


//////////////////////////////////////////////////////////////////////////
//
// Starts an instruction line.  The caller appends the rest and the new line.
//
//////////////////////////////////////////////////////////////////////////
CChars* CCPP65Generator::Line(const char* szText)
{
	mszCode.Append("\t\t");
	return mszCode.Append(szText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendName(CChars* psz, int64 lliName)
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
int64 CCPP65Generator::GetName(CCPPSyntaxDeclarator* pcDeclarator)
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
bool CCPP65Generator::IsOutsideMemberTemplate(CCPPSyntax* pcDeclaration)
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
bool CCPP65Generator::Declarations(CArraySyntax* papcDeclarations)
{
	size								i;
	CCPPSyntax*							pcDeclaration;
	CCPPSyntaxLinkageSpecification*		pcLinkage;
	bool								bResult;
	bool								bLinkageExtern;
	bool								bCLinkage;
	CCPPScope*							pcEnclosing;

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
			bLinkageExtern = mbLinkageExtern;
			bCLinkage = mbCLinkage;
			mbLinkageExtern = !pcLinkage->mbBraced;
			mbCLinkage = IsCLinkage(pcLinkage);
			bResult &= Declarations(&pcLinkage->macpDeclarations);
			mbLinkageExtern = bLinkageExtern;
			mbCLinkage = bCLinkage;
			break;
		case CPPS_TemplateDeclaration:
			bResult &= TemplateDeclaration((CCPPSyntaxTemplateDeclaration*)pcDeclaration);
			break;
		case CPPS_Namespace:
			pcEnclosing = mpcDeclaringNamespace;
			mpcDeclaringNamespace = ((CCPPSyntaxNamespace*)pcDeclaration)->mpcScope;
			bResult &= Declarations(&((CCPPSyntaxNamespace*)pcDeclaration)->macpDeclarations);
			mpcDeclaringNamespace = pcEnclosing;
			break;
		case CPPS_EmptyDeclaration:
		case CPPS_StaticAssert:
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
bool CCPP65Generator::IsScopeDeclaration(CCPPSyntax* pcDeclaration)
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
bool CCPP65Generator::IsCLinkage(CCPPSyntaxLinkageSpecification* pcLinkage)
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
// Records a function's signature, a member of pcClass if it is not NULL, or an
// instance of pcTemplate.  A function may be declared any number of times but
// defined once.  Functions with the same name and different parameters are
// overloads.  Returns NULL after an error.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPP65Generator::DeclareFunction(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, bool bDefinition, CCPPClass* pcClass, bool bPure, CCPPTemplate* pcTemplate, CChars* pszArguments)
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
	bool						bFriend;
	CCPPScope*					pcNamespace;

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
	eKind = CPPFK_Function;
	if (pcComponent->meKind == CPPNC_Destructor)
	{
		eKind = CPPFK_Destructor;
	}
	else if (pcClass && (lliName == pcClass->mlliName))
	{
		eKind = CPPFK_Constructor;
	}

	pcSuffix = pcDeclarator->GetFunctionSuffix();
	if ((!pcDeclarator->mpcNested && (pcDeclarator->macpSuffixes.NumElements() != 1)) || pcSuffix->mpcTrailingReturn)
	{
		Error(pcDeclarator, "This function declarator is not supported by the compiler yet");
		return NULL;
	}

	psReturn = &mcTypes.msVoid;
	if (lliName == CPP65_CONVERSION_NAME)
	{
		//operator T () returns T.
		psReturn = pcComponent->mpcConversionType ? GetTypeIdType(pcComponent->mpcConversionType) : NULL;
		if (!psReturn)
		{
			return NULL;
		}
	}
	else if (eKind == CPPFK_Function)
	{
		psReturn = GetType(pcSpecifiers, NULL, pcDeclarator, NULL);
		if (psReturn && pcDeclarator->mpcNested)
		{
			//int (*Pick())(int) returns what the declarators around its name make of the int.
			psReturn = GetDeclaratorType(psReturn, pcDeclarator, pcDeclarator, NULL, pcSuffix);
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
				psReturn = mcTypes.Reference(psReturn);
				break;
			default:
				Error(pcDeclarator, "Pointers to members are not supported by the compiler yet");
				psReturn = NULL;
				break;
			}
		}
		if (!psReturn)
		{
			return NULL;
		}
	}

	apsParameters.Init();
	apcDefaults.Init();
	bConstThis = pcClass && IsConstSuffix(pcSuffix);
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
			psType = GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter);
			if (psPack)
			{
				psPack->psType = NULL;
			}
			if (!psType)
			{
				apcDefaults.Kill();
				apsParameters.Kill();
				return NULL;
			}
			if (psType->IsVoid() && (pcSuffix->macpParameters.NumElements() == 1))
			{
				break;
			}
			apsParameters.Add(ParameterType(psType));
			apcDefaults.Add(pcParameter->mpcDefault);
		}
	}

	//A declaration of a function declared before.
	uiFlags = pcSpecifiers ? pcSpecifiers->muiFlags : 0;
	pcNamespace = NULL;
	if (!pcClass)
	{
		//A friend with a simple name is of the namespace its class is in, whatever the name finds.  [namespace.memdef] 3
		bFriend = pcName->IsSimple() && (uiFlags & CPPDS_FRIEND);
		pcNamespace = (pcName->mpcSymbol && !bFriend) ? NamespaceOf(pcName->mpcSymbol->mpcScope) : mpcDeclaringNamespace;
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
		if (((lliName == CPP65_CONVERSION_NAME) && !pcOther->mpsReturn->Equals(psReturn)) || (pcOther->mbConstThis != bConstThis))
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
		for (j = 0; j < apcDefaults.NumElements(); j++)
		{
			if (apcDefaults.GetPtr(j))
			{
				pcFunction->mapcDefaults.SetPtr(j, apcDefaults.GetPtr(j));
			}
		}
		apcDefaults.Kill();
		apsParameters.Kill();
		pcFunction->mbVirtual |= (uiFlags & CPPDS_VIRTUAL) != 0;
		pcFunction->mbPure |= bPure;
		pcFunction->mbCLinkage |= mbCLinkage && !pcClass;
		pcFunction->mbExplicit |= (uiFlags & CPPDS_EXPLICIT) != 0;
		return pcFunction;
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
	apcDefaults.Kill();
	apsParameters.Kill();
	pcFunction->mbVariadic = pcSuffix->mbVariadic;
	pcFunction->mbStatic = (uiFlags & CPPDS_STATIC) != 0;
	pcFunction->mbVirtual = (uiFlags & CPPDS_VIRTUAL) != 0;
	pcFunction->mbPure = bPure;
	pcFunction->mbInline = (uiFlags & (CPPDS_INLINE | CPPDS_CONSTEXPR)) != 0;
	pcFunction->mpcTemplate = pcTemplate;
	pcFunction->mbCLinkage = mbCLinkage && !pcClass;
	pcFunction->mbExplicit = (uiFlags & CPPDS_EXPLICIT) != 0;
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
// int F(int a); in a block declares a function of the namespace around the
// block, as if it was declared there.  [basic.link] 6
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::DeclareBlockFunction(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator)
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
CCPPFunction* CCPP65Generator::AddFunction(CCPPClass* pcClass, int64 lliName, ECPPFunctionKind eKind, SCPP65Type* psReturn)
{
	CCPPFunction*			pcFunction;
	SCPPFunctionsOfName*	psOfName;
	SCPPFunctionsOfName	sOfName;

	pcFunction = NewMalloc<CCPPFunction>();
	pcFunction->mlliName = lliName;
	pcFunction->mpsReturn = psReturn;
	pcFunction->mapsParameters.Init();
	pcFunction->mbDefined = false;
	pcFunction->mpcClass = pcClass;
	pcFunction->mpcNamespace = NULL;
	pcFunction->meKind = eKind;
	pcFunction->mbStatic = false;
	pcFunction->mbVirtual = false;
	pcFunction->mbPure = false;
	pcFunction->miSlot = -1;
	pcFunction->mbCLinkage = false;
	pcFunction->mbExplicit = false;
	pcFunction->mbConstThis = false;
	pcFunction->mbImplicit = false;
	pcFunction->mpcInherited = NULL;
	pcFunction->mapcDefaults.Init();
	pcFunction->mbVariadic = false;
	pcFunction->mpcDefinition = NULL;
	pcFunction->mpcDeclarator = NULL;
	pcFunction->mpcBindings = pcClass ? pcClass->mpcBindings : NULL;
	pcFunction->mbInline = false;
	pcFunction->mbQueued = false;
	pcFunction->mbGenerated = false;
	pcFunction->mbCalled = false;
	pcFunction->mpcTemplate = NULL;
	pcFunction->mszArguments.Init();
	pcFunction->mpcNextOfName = NULL;
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
CCPPFunction* CCPP65Generator::FirstFunctionOfName(int64 lliName)
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
bool CCPP65Generator::GenerateQueued(void)
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
// yet.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::RequireFunction(CCPPFunction* pcFunction)
{
	pcFunction->mbCalled = true;
	if (!pcFunction->mbGenerated && !pcFunction->mbQueued && (pcFunction->mpcDefinition || pcFunction->mbImplicit))
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
bool CCPP65Generator::Emit(CChars* pszLabel)
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
bool CCPP65Generator::SameParameters(CCPPFunction* pcFunction, CCPPFunction* pcOther)
{
	size	i;

	if ((pcFunction->mapsParameters.NumElements() != pcOther->mapsParameters.NumElements()) || (pcFunction->mbVariadic != pcOther->mbVariadic))
	{
		return false;
	}
	for (i = 0; i < pcFunction->mapsParameters.NumElements(); i++)
	{
		if (!pcFunction->mapsParameters.GetPtr(i)->Same(pcOther->mapsParameters.GetPtr(i)))
		{
			return false;
		}
	}
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
bool CCPP65Generator::GlobalDeclaration(CCPPSyntaxSimpleDeclaration* pcDeclaration)
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

	pcSpecifiers = pcDeclaration->mpcSpecifiers;
	if (pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_TYPEDEF))
	{
		return true;
	}
	if (pcSpecifiers && pcSpecifiers->mpcType && (pcSpecifiers->mpcType->GetKind() == CPPS_ClassSpecifier))
	{
		//The class is laid out when it is first needed whole.
		BeginQuiet();
		GetBaseType(pcSpecifiers, pcDeclaration);
		EndQuiet();
	}

	bResult = true;
	for (i = 0; i < pcDeclaration->macpDeclarators.NumElements(); i++)
	{
		pcInit = (CCPPSyntaxInitDeclarator*)pcDeclaration->macpDeclarators.GetPtr(i);
		if (pcInit->mpcDeclarator->IsFunction())
		{
			BeginQuiet();
			pcFunction = DeclareFunction(pcSpecifiers, pcInit->mpcDeclarator, false, NULL, false);
			EndQuiet();
			if (!pcFunction)
			{
				mapcFailed.Add(pcInit->mpcDeclarator);
			}
			continue;
		}

		lliName = GetName(pcInit->mpcDeclarator);
		pcName = pcInit->mpcDeclarator->GetName();
		if ((lliName == -1) && pcName && pcName->IsQualifiedByClass())
		{
			//int CClass::siStatic = 0; defines a static data member, so the class is laid out.
			pcClass = GetComponentClass(pcName->GetQualifier(), NULL, pcInit);
			if (pcClass)
			{
				CompleteClass(pcClass, pcInit);
			}
		}
		pcVariable = pcInit->mpcSymbol ? GetVariable(pcInit->mpcSymbol) : NULL;
		if (!pcInit->mpcSymbol || ((lliName == -1) && pcName && pcName->IsQualifiedByClass() && !(pcVariable && pcVariable->mpcClass)))
		{
			bResult &= Error(pcInit, "Only variables with simple names are supported by the compiler yet");
			continue;
		}

		//extern without an initialiser only declares; extern int x = 1; defines x.  [basic.def]
		if (((pcSpecifiers->muiFlags & CPPDS_EXTERN) || mbLinkageExtern) && !pcInit->mpcInitializer)
		{
			BeginQuiet();
			psType = GetType(pcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
			EndQuiet();
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

		//const int CClass::k; has the initialiser the class gave it.
		pcInitialiser = pcInit->mpcInitializer;
		if (!pcInitialiser && pcVariable)
		{
			pcInitialiser = pcVariable->mpcInClassInitialiser;
		}

		psType = GetType(pcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInitialiser);
		if (!psType)
		{
			bResult = false;
			continue;
		}
		psElement = psType->IsArray() ? psType->psBase : psType;
		if (psElement->IsClass() && !CompleteClass(psElement->pcClass, pcInit))
		{
			bResult = false;
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

		szLabel.Init();
		AppendVariableLabel(&szLabel, pcVariable);
		bBss = IsBss(pcInitialiser, psType);
		BeginData(&szLabel, DataAlignment(psType, psElement->IsClass()), !pcVariable->mbInternal, HasUsedAttribute(pcInit->mpcDeclarator) ? ", keep" : NULL, bBss);

		//GlobalInitialiser writes the constant data.  What isn't constant is worked out by the global's own R_Initialise,
		//and an object is destroyed by its R_Finalise: the IR front end writes them (GlobalStartUpRoutines).
		mbThis = false;
		if (bBss)
		{
			AppendSpace(psType->iBytes);
			bResult &= GlobalStartUpRoutines(psType, pcInitialiser, &szLabel, psElement->IsClass(), psElement->IsClass() && (psElement->pcClass->mpcDestructor != NULL));
		}
		else if (!psElement->IsClass() && !psType->IsReference())
		{
			bInitialise = false;
			bResult &= GlobalInitialiser(psType, pcInitialiser, pcInit, &bInitialise);
			bResult &= GlobalStartUpRoutines(psType, pcInitialiser, &szLabel, bInitialise, false);
		}
		else
		{
			AppendZeros(psType->iBytes);
			bResult &= GlobalStartUpRoutines(psType, pcInitialiser, &szLabel, true, psElement->IsClass() && (psElement->pcClass->mpcDestructor != NULL));
		}
		miStartUps++;
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
bool CCPP65Generator::GlobalInitialiser(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode, bool* pbInitialise)
{
	int64						lliValue;
	CCPPSyntaxInitializerList*	pcList;
	CArraySyntax*				papcElements;
	size						i;
	CArrayUint8					auiBytes;
	bool						bResult;
	double						dValue;

	if (pcInitialiser && (pcInitialiser->GetKind() == CPPS_ParenthesisedInitializer))
	{
		papcElements = &((CCPPSyntaxParenthesisedInitializer*)pcInitialiser)->macpElements;
		pcInitialiser = (papcElements->NumElements() == 1) ? papcElements->GetPtr(0) : pcInitialiser;
	}
	if (pcInitialiser && (pcInitialiser->GetKind() == CPPS_InitializerList) && psType->IsScalar())
	{
		papcElements = &((CCPPSyntaxInitializerList*)pcInitialiser)->macpElements;
		pcInitialiser = (papcElements->NumElements() == 1) ? papcElements->GetPtr(0) : NULL;
	}

	if (psType->IsArray())
	{
		bResult = true;
		pcInitialiser = StringInitialiser(pcInitialiser, psType->psBase);
		if (!pcInitialiser)
		{
			i = 0;
		}
		else if ((pcInitialiser->GetKind() == CPPS_StringExpression) && (psType->psBase->iBytes == StringElementBytes(pcInitialiser)))
		{
			auiBytes.Init();
			bResult = DecodeString(pcInitialiser, &auiBytes);
			for (i = 0; i < auiBytes.NumElements() / psType->psBase->iBytes; i++)
			{
				AppendData(psType->psBase, StringElement(&auiBytes, psType->psBase->iBytes, i));
			}
			auiBytes.Kill();
			if (i > (size)psType->iCount)
			{
				return Error(pcInitialiser, "The string is longer than the array");
			}
		}
		else if (pcInitialiser->GetKind() == CPPS_InitializerList)
		{
			pcList = (CCPPSyntaxInitializerList*)pcInitialiser;
			if (pcList->macpElements.NumElements() > (size)psType->iCount)
			{
				return Error(pcInitialiser, "Too many initialisers for the array");
			}
			for (i = 0; i < pcList->macpElements.NumElements(); i++)
			{
				bResult &= GlobalInitialiser(psType->psBase, pcList->macpElements.GetPtr(i), pcNode, pbInitialise);
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
		AppendData(psType, FloatBits(psType, dValue));
		return true;
	}
	if (pcInitialiser && psType->IsPointer() && (pcInitialiser->GetKind() == CPPS_StringExpression) && (psType->psBase->iBytes == StringElementBytes(pcInitialiser)))
	{
		auiBytes.Init();
		bResult = DecodeString(pcInitialiser, &auiBytes);
		if (bResult)
		{
			//AddString ends it with one zero byte.
			auiBytes.GrowToAtLeastNumElements(auiBytes.NumElements() + psType->psBase->iBytes - 1, true, 0);
			mszData.Append("\t\t.long Str")->Append(AddString((char*)auiBytes.GetData(), auiBytes.NumElements()))->AppendNewLine();
		}
		auiBytes.Kill();
		return bResult;
	}
	if (pcInitialiser && (psType->IsFloat() || !Constant(pcInitialiser, &lliValue)))
	{
		if (mbStaticLocal)
		{
			return Error(pcInitialiser, "A static local variable needs a constant initialiser in the compiler yet");
		}
		//Not a constant: R_Initialise works it out before main.  [basic.start.init]
		AppendData(psType, 0);
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
	AppendData(psType, lliValue);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendData(SCPP65Type* psType, int64 lliValue)
{
	switch (psType->iBytes)
	{
	case 1:
		mszData.Append("\t\t.byte ");
		mszData.Append((int64)(lliValue & 0xFF));
		break;
	case 2:
		mszData.Append("\t\t.word ");
		mszData.Append((int64)(lliValue & 0xFFFF));
		break;
	case 8:
		mszData.Append("\t\t.quad ");
		mszData.Append(lliValue);
		break;
	case 16:
		mszData.Append("\t\t.quad ");
		mszData.Append(lliValue);
		mszData.AppendNewLine();
		mszData.Append("\t\t.quad 0");
		break;
	default:
		mszData.Append("\t\t.long ");
		mszData.Append((int64)(lliValue & 0xFFFFFFFF));
		break;
	}
	mszData.AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendZeros(int iBytes)
{
	int		i;

	for (i = 0; i + 4 <= iBytes; i += 4)
	{
		mszData.Append("\t\t.long 0");
		mszData.AppendNewLine();
	}
	for (; i < iBytes; i++)
	{
		mszData.Append("\t\t.byte 0");
		mszData.AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// _name, or _Class__name for a static data member.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendGlobalLabel(CChars* psz, CCPPVariable* pcVariable)
{
	CChars	szName;
	char*	szIdentifier;
	int		iLength;
	int64	lliName;

	lliName = pcVariable->mpcSymbol->mlliName;
	szIdentifier = mpcTokeniser->GetIdentifier(lliName, &iLength);
	if (pcVariable->mpcClass)
	{
		szName.Init(szIdentifier, 0, iLength);
		AppendClassLabel(psz, pcVariable->mpcClass, szName.Text());
		szName.Kill();
		return;
	}
	psz->Append('_');
	if (!pcVariable->mbCLinkage)
	{
		AppendScopeNames(psz, NamespaceOf(pcVariable->mpcSymbol->mpcScope));
	}
	psz->Append(szIdentifier, iLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPVariable* CCPP65Generator::AddVariable(CArrayCPPVariables* papcVariables, CCPPSymbol* pcSymbol, SCPP65Type* psType, ECPPStorage eStorage, int iOffset)
{
	CCPPVariable*	pcVariable;

	pcVariable = NewMalloc<CCPPVariable>();
	pcVariable->mpcSymbol = pcSymbol;
	pcVariable->mpsType = psType;
	pcVariable->meStorage = eStorage;
	pcVariable->miOffset = iOffset;
	pcVariable->mpcClass = NULL;
	pcVariable->mpcInClassInitialiser = NULL;
	pcVariable->mbDefined = false;
	pcVariable->mbUsed = false;
	pcVariable->mbInternal = pcSymbol && pcSymbol->mpcScope && pcSymbol->mpcScope->IsInUnnamedNamespace();
	pcVariable->mbCLinkage = mbCLinkage;
	pcVariable->mbRestrict = false;
	pcVariable->mpcAnonymous = NULL;
	papcVariables->Add(pcVariable);
	return pcVariable;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::FreeVariables(CArrayCPPVariables* papcVariables)
{
	size	i;

	for (i = 0; i < papcVariables->NumElements(); i++)
	{
		free(papcVariables->GetPtr(i));
	}
	papcVariables->ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// The local, parameter or global a symbol names.  Locals are searched first.
//
//////////////////////////////////////////////////////////////////////////
CCPPVariable* CCPP65Generator::GetVariable(CCPPSymbol* pcSymbol)
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
// The type a declarator declares.  pcDeclarator may be NULL for the base type.  An
// array without a size gets it from pcInitialiser.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::GetType(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntax* pcNode, CCPPSyntax* pcInitialiser)
{
	SCPP65Type*	psType;

	psType = GetBaseType(pcSpecifiers, pcNode);
	if (psType && pcSpecifiers && (pcSpecifiers->muiFlags & (CPPDS_CONST | CPPDS_VOLATILE)))
	{
		//The base type's own qualifiers stay (a typedef of const int).
		psType = mcTypes.Qualified(psType, psType->bConst || (pcSpecifiers->muiFlags & CPPDS_CONST), psType->bVolatile || (pcSpecifiers->muiFlags & CPPDS_VOLATILE));
	}
	if (!psType || !pcDeclarator)
	{
		return psType;
	}
	return GetDeclaratorType(psType, pcDeclarator, pcNode, pcInitialiser);
}


//////////////////////////////////////////////////////////////////////////
//
// Applies a declarator to a type: the pointer and reference operators first, then
// the array and function suffixes from the last, then a nested declarator, as in
// int (*pf)(int) (a pointer to a function).  [dcl.meaning]  A parameter's array
// without a size (char* argv[]) is a pointer.  [dcl.fct]  It stops at pcFunction
// if that is given, for the type a function with that suffix returns.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::GetDeclaratorType(SCPP65Type* psType, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntax* pcNode, CCPPSyntax* pcInitialiser, CCPPSyntaxFunctionSuffix* pcFunction)
{
	CCPPSyntaxPointerOperator*	pcPointer;
	CCPPSyntax*					pcSuffix;
	CCPPSyntaxArraySuffix*		pcArray;
	CCPPClass*				pcClass;
	size						i;
	int64						lliCount;

	for (i = 0; i < pcDeclarator->NumPointers(); i++)
	{
		pcPointer = pcDeclarator->GetPointer(i);
		switch (pcPointer->meKind)
		{
		case CPPP_Pointer:
			psType = QualifiedPointer(mcTypes.Pointer(psType), pcPointer);
			break;
		case CPPP_Reference:
		case CPPP_RValueReference:
			psType = mcTypes.Reference(psType);
			break;
		case CPPP_MemberPointer:
			pcClass = pcPointer->mpcClass ? GetNameClass(pcPointer->mpcClass, pcNode) : NULL;
			if (!pcClass)
			{
				Error(pcNode, "The class of a pointer to a member is not supported by the compiler yet");
				return NULL;
			}
			psType = mcTypes.MemberPointer(pcClass, psType);
			break;
		default:
			Error(pcNode, "This declarator is not supported by the compiler yet");
			return NULL;
		}
	}

	for (i = pcDeclarator->macpSuffixes.NumElements(); i > 0; i--)
	{
		pcSuffix = pcDeclarator->macpSuffixes.GetPtr(i - 1);
		if (pcSuffix == pcFunction)
		{
			return psType;
		}
		if (pcSuffix->GetKind() == CPPS_FunctionSuffix)
		{
			psType = GetFunctionType(psType, (CCPPSyntaxFunctionSuffix*)pcSuffix, pcNode);
			if (!psType)
			{
				return NULL;
			}
			continue;
		}

		pcArray = (CCPPSyntaxArraySuffix*)pcSuffix;
		if (!pcArray->mpcSize && (i == 1) && !pcDeclarator->mpcNested && pcDeclarator->mpcParent && (pcDeclarator->mpcParent->GetKind() == CPPS_Parameter))
		{
			psType = mcTypes.Pointer(psType);
			continue;
		}
		if (!pcArray->mpcSize)
		{
			lliCount = ((i == 1) && !pcDeclarator->mpcNested) ? GetInitialiserCount(StringInitialiser(pcInitialiser, psType)) : 0;
			if (lliCount <= 0)
			{
				Error(pcNode, "An array needs a size or an initialiser");
				return NULL;
			}
		}
		else if (!Constant(pcArray->mpcSize, &lliCount) || (lliCount <= 0))
		{
			Error(pcArray->mpcSize, "An array size must be a positive constant");
			return NULL;
		}
		if (psType->IsClass() && !CompleteClass(psType->pcClass, pcNode))
		{
			return NULL;
		}
		psType = mcTypes.Array(psType, (int)lliCount);
	}

	if (pcDeclarator->mpcNested)
	{
		return GetDeclaratorType(psType, pcDeclarator->mpcNested, pcNode, pcInitialiser, pcFunction);
	}
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// A function type: what it returns and its parameters' types.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::GetFunctionType(SCPP65Type* psReturn, CCPPSyntaxFunctionSuffix* pcSuffix, CCPPSyntax* pcNode)
{
	CCPPSyntaxParameter*	pcParameter;
	CArrayCPP65Types		apsParameters;
	SCPP65Type*				psType;
	size					i;

	apsParameters.Init();
	for (i = 0; i < pcSuffix->macpParameters.NumElements(); i++)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		psType = GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter);
		if (!psType)
		{
			apsParameters.Kill();
			return NULL;
		}
		if (psType->IsVoid() && (pcSuffix->macpParameters.NumElements() == 1))
		{
			break;
		}
		apsParameters.Add(ParameterType(psType));
	}
	psType = mcTypes.Function(psReturn, &apsParameters, pcSuffix->mbVariadic);
	apsParameters.Kill();
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// A parameter's type in its function's type: an array is a pointer to its
// element, a function a pointer to it, and const and volatile on the
// parameter itself go (f(const int) is f(int)).  [dcl.fct] 5
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::ParameterType(SCPP65Type* psType)
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
SCPP65Type* CCPP65Generator::ParameterVariableType(SCPP65Type* psType, CCPPSyntaxParameter* pcParameter)
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
CCPPSyntax* CCPP65Generator::StringInitialiser(CCPPSyntax* pcInitialiser, SCPP65Type* psElement)
{
	CArraySyntax*	papcElements;

	if (pcInitialiser && (pcInitialiser->GetKind() == CPPS_InitializerList) && psElement->IsInteger())
	{
		papcElements = &((CCPPSyntaxInitializerList*)pcInitialiser)->macpElements;
		if ((papcElements->NumElements() == 1) && (papcElements->GetPtr(0)->GetKind() == CPPS_StringExpression) && (StringElementBytes(papcElements->GetPtr(0)) == psElement->iBytes))
		{
			return papcElements->GetPtr(0);
		}
	}
	return pcInitialiser;
}


//////////////////////////////////////////////////////////////////////////
//
// The number of elements an initialiser gives an array without a size: the
// elements of a list, or a string's characters and its zero.  0 if it gives none.
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::GetInitialiserCount(CCPPSyntax* pcInitialiser)
{
	CArrayUint8		auiBytes;
	int				iCount;

	if (!pcInitialiser)
	{
		return 0;
	}
	if (pcInitialiser->GetKind() == CPPS_InitializerList)
	{
		return (int)((CCPPSyntaxInitializerList*)pcInitialiser)->macpElements.NumElements();
	}
	if (pcInitialiser->GetKind() == CPPS_StringExpression)
	{
		auiBytes.Init();
		iCount = DecodeString(pcInitialiser, &auiBytes) ? ((int)auiBytes.NumElements() / StringElementBytes(pcInitialiser)) + 1 : 0;
		auiBytes.Kill();
		return iCount;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The type the specifiers name.  long is 4 bytes, as on Windows.  char is signed.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::GetBaseType(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntax* pcNode)
{
	CCPPSymbol*					pcMember;
	CCPPClass*				pcMemberClass;
	CCPPTemplateArguments*				pcSavedBindings;
	SCPP65Type*					psMemberType;
	uint32								uiKeywords;
	bool								bSigned;
	CCPPSyntaxTypeSpecifier*			pcType;
	CCPPSyntaxName*						pcName;
	CCPPSyntaxClassSpecifier*			pcClassSpecifier;
	CCPPSyntaxElaboratedTypeSpecifier*	pcElaborated;
	CCPPSymbol*							pcSymbol;
	CCPPClass*						pcClass;
	SCPPBinding*						psBinding;

	if (!pcSpecifiers)
	{
		return &mcTypes.msInt;
	}

	pcType = pcSpecifiers->mpcType;
	if (pcType)
	{
		if (pcType->GetKind() == CPPS_EnumSpecifier)
		{
			return EnumType(((CCPPSyntaxEnumSpecifier*)pcType)->mpcSymbol, (CCPPSyntaxEnumSpecifier*)pcType);
		}
		if (pcType->GetKind() == CPPS_ClassSpecifier)
		{
			//The class is laid out when it is first needed whole.
			pcClassSpecifier = (CCPPSyntaxClassSpecifier*)pcType;
			pcClass = pcClassSpecifier->mpcSymbol ? GetClass(pcClassSpecifier->mpcSymbol, pcNode) : GetUnnamedClass(pcClassSpecifier);
			if (!pcClass->mpcSpecifier)
			{
				pcClass->mpcSpecifier = pcClassSpecifier;
			}
			return pcClass->mpsType;
		}
		if (pcType->GetKind() == CPPS_ElaboratedTypeSpecifier)
		{
			pcElaborated = (CCPPSyntaxElaboratedTypeSpecifier*)pcType;
			if (pcElaborated->meKey == CT_enum)
			{
				pcSymbol = pcElaborated->mpcName->mpcSymbol;
				return (pcSymbol && (pcSymbol->meKind == CPPSK_Enum)) ? EnumType(pcSymbol, NULL) : &mcTypes.msInt;
			}
			//struct timsort names the class even when a function timsort was declared first.
			pcSymbol = pcElaborated->mpcName->mpcSymbol;
			while (pcSymbol && (pcSymbol->meKind != CPPSK_Class) && (pcSymbol->meKind != CPPSK_ClassTemplate))
			{
				pcSymbol = pcSymbol->mpcNext;
			}
			pcClass = GetComponentClass(pcElaborated->mpcName->GetLast(), pcSymbol ? pcSymbol : pcElaborated->mpcName->mpcSymbol, pcNode);
			if (pcClass)
			{
				return pcClass->mpsType;
			}
		}
		else if (pcType->GetKind() == CPPS_NamedTypeSpecifier)
		{
			pcName = ((CCPPSyntaxNamedTypeSpecifier*)pcType)->mpcName;
			pcMember = QualifiedMember(pcName, CPPSK_Typedef, pcNode, &pcMemberClass);
			if (pcMember)
			{
				pcSavedBindings = mpcBindings;
				mpcBindings = pcMemberClass->mpcBindings;
				psMemberType = GetSymbolType(pcMember, pcNode);
				mpcBindings = pcSavedBindings;
				return psMemberType;
			}
			if (pcName->mpcSymbol)
			{
				switch (pcName->mpcSymbol->meKind)
				{
				case CPPSK_Enum:
					return EnumType(pcName->mpcSymbol, NULL);
				case CPPSK_Typedef:
					return GetSymbolType(pcName->mpcSymbol, pcNode);
				case CPPSK_TypeParameter:
					psBinding = FindBinding(pcName->mpcSymbol);
					if (!psBinding || !psBinding->psType)
					{
						Error(pcNode, "A template parameter used outside an instantiation");
						return NULL;
					}
					return psBinding->psType;
				case CPPSK_Class:
				case CPPSK_ClassTemplate:
					pcClass = GetNameClass(pcName, pcNode);
					return pcClass ? pcClass->mpsType : NULL;
				case CPPSK_BuiltinType:
					if (mpcSemanticPass)
					{
						//Only checking: the host's __builtin_va_list, a char* as MSVC's va_list is.
						return mcTypes.Pointer(&mcTypes.msChar);
					}
					break;
				default:
					break;
				}
			}
		}
		Error(pcNode, "This type is not supported by the compiler yet");
		return NULL;
	}

	//long double is double, as MSVC.
	uiKeywords = pcSpecifiers->muiTypeKeywords;
	bSigned = (uiKeywords & CPPTK_UNSIGNED) == 0;
	if (uiKeywords & CPPTK_FLOAT)
	{
		return mcTypes.Float(4);
	}
	if (uiKeywords & CPPTK_DOUBLE)
	{
		return (pcSpecifiers->miLongs >= 1) ? mcTypes.LongDouble() : mcTypes.Float(8);
	}
	if ((pcSpecifiers->miLongs >= 2) || (uiKeywords & CPPTK_INT64))
	{
		return mcTypes.Integer(8, bSigned);
	}
	if (uiKeywords & CPPTK_AUTO)
	{
		Error(pcSpecifiers, "auto is not supported by the compiler yet");
		return NULL;
	}
	if (uiKeywords & CPPTK_VOID)
	{
		return &mcTypes.msVoid;
	}
	if (uiKeywords & CPPTK_BOOL)
	{
		return &mcTypes.msBool;
	}
	if (uiKeywords & (CPPTK_CHAR | CPPTK_INT8))
	{
		return mcTypes.Integer(1, bSigned);
	}
	if (uiKeywords & (CPPTK_WCHAR_T | CPPTK_CHAR16_T))
	{
		return mcTypes.Integer(2, false);
	}
	if (uiKeywords & CPPTK_CHAR32_T)
	{
		return mcTypes.Integer(4, false);
	}
	if (uiKeywords & (CPPTK_SHORT | CPPTK_INT16))
	{
		return mcTypes.Integer(2, bSigned);
	}
	if ((uiKeywords & CPPTK_INT32) || (pcSpecifiers->miLongs == 1))
	{
		return mcTypes.Long(bSigned);
	}
	if (uiKeywords & (CPPTK_INT | CPPTK_SIGNED | CPPTK_UNSIGNED))
	{
		return bSigned ? &mcTypes.msInt : &mcTypes.msUnsignedInt;
	}
	Error(pcNode, "This type is not supported by the compiler yet");
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::GetTypeIdType(CCPPSyntaxTypeId* pcTypeId)
{
	return GetType(pcTypeId->mpcSpecifiers, pcTypeId->mpcDeclarator, pcTypeId);
}


//////////////////////////////////////////////////////////////////////////
//
// The type a typedef, variable or parameter symbol was declared with.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::GetSymbolType(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode)
{
	CCPPSyntax*					pcDeclaration;
	CCPPSyntaxInitDeclarator*	pcInit;
	CCPPSyntaxParameter*		pcParameter;

	pcDeclaration = pcSymbol->mpcDeclaration;
	if (pcDeclaration && (pcDeclaration->GetKind() == CPPS_InitDeclarator) && pcDeclaration->mpcParent && (pcDeclaration->mpcParent->GetKind() == CPPS_SimpleDeclaration))
	{
		pcInit = (CCPPSyntaxInitDeclarator*)pcDeclaration;
		return GetType(((CCPPSyntaxSimpleDeclaration*)pcDeclaration->mpcParent)->mpcSpecifiers, pcInit->mpcDeclarator, pcNode, pcInit->mpcInitializer);
	}
	if (pcDeclaration && (pcDeclaration->GetKind() == CPPS_Parameter))
	{
		pcParameter = (CCPPSyntaxParameter*)pcDeclaration;
		return GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcNode);
	}
	if (pcDeclaration && (pcDeclaration->GetKind() == CPPS_AliasDeclaration))
	{
		return GetTypeIdType(((CCPPSyntaxAliasDeclaration*)pcDeclaration)->mpcType);
	}
	Error(pcNode, "This declaration is not supported by the compiler yet");
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A function definition outside a class, a member of pcClass if it is not NULL (or
// of the class its qualified name names).  Inline and constexpr functions are
// generated when they are called; others now.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::Function(CCPPSyntaxFunctionDefinition* pcDefinition, CCPPClass* pcClass)
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
	SetDefinition(pcFunction, pcDefinition);
	if (!pcFunction->mpcDefinition && !pcFunction->mbImplicit)
	{
		//A body skipped in a system header.
		return true;
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
bool CCPP65Generator::WriteBody(CCPPFunction* pcFunction)
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
void CCPP65Generator::GenerateFunction(CCPPFunction* pcFunction)
{
	CCPPTemplateArguments*	pcSaved;
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
		pcSaved = mpcBindings;
		mpcBindings = pcFunction->mpcBindings;
		CheckFunction(pcFunction);
		mpcBindings = pcSaved;
		return;
	}

	//Each function is a section of its own.  Inline, template and generated functions are written by every unit
	//that needs them, each in a COMDAT section the linker keeps one of.
	szLabel.Init();
	AppendFunctionLabel(&szLabel, pcFunction);
	bShared = !IsUnitFunction(pcFunction) && (pcFunction->mbInline || pcFunction->mbImplicit || pcFunction->mpcTemplate);
	if (bShared && !Emit(&szLabel))
	{
		szLabel.Kill();
		return;
	}
	AppendSection(&mszCode, "text", &szLabel, bShared ? ", comdat" : NULL);
	if (!IsUnitFunction(pcFunction))
	{
		AppendGlobal(&mszCode, &szLabel);
	}
	szLabel.Kill();
	pcSaved = mpcBindings;
	mpcBindings = pcFunction->mpcBindings;

	if (pcFunction->mbImplicit)
	{
		//An implicit default constructor, an inherited one or a destructor.
		BeginFunction(pcFunction);
		if (((pcFunction->mapsParameters.NumElements() != 0) && !pcFunction->mpcInherited) || !WriteBody(pcFunction))
		{
			szLabel.Init("An implicit copy constructor or assignment (or one the IR front end can't write) is not supported on this target yet: ");
			AppendFunctionLabel(&szLabel, pcFunction);
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
	mpcBindings = pcSaved;
}


//////////////////////////////////////////////////////////////////////////
//
// The code for a function's definition: its parameters and locals get their places,
// a constructor makes its bases and fields first, a destructor sets its vtable
// pointer first (EndFunction destroys its fields and bases), then the body.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::GenerateDefinition(CCPPFunction* pcFunction)
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
	if (!AllocateLocals(pcDefinition->mpcBody))
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
// ExpandPackCall).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AddParameters(CCPPFunction* pcFunction)
{
	CCPPSyntaxFunctionSuffix*		pcSuffix;
	CCPPSyntaxParameter*			pcParameter;
	SCPP65Type*						psType;
	CCPPVariable*					pcVariable;
	SCPPBinding*					psPack;
	CCPPSymbol*						pcSymbol;
	size							i;
	size							j;
	size							k;
	size							uiPack;

	pcSuffix = pcFunction->mpcDefinition->mpcDeclarator->GetFunctionSuffix();
	j = 0;
	for (i = 0; i < pcSuffix->macpParameters.NumElements(); i++)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		psPack = PackBinding(pcParameter);
		uiPack = psPack ? psPack->papsPack->NumElements() : ((j < pcFunction->mapsParameters.NumElements()) ? 1 : 0);
		if (psPack)
		{
			mpcPackParameter = pcParameter->mpcSymbol;
		}
		for (k = 0; k < uiPack; k++)
		{
			psType = pcFunction->mapsParameters.GetPtr(j);
			pcSymbol = (psPack && pcParameter->mpcSymbol) ? NewPackElement(pcParameter->mpcSymbol) : pcParameter->mpcSymbol;
			if (pcSymbol)
			{
				pcVariable = AddVariable(&mapcLocals, pcSymbol, ParameterVariableType(psType, pcParameter), CPPVS_Parameter, (int)j);
				pcVariable->mbRestrict = !psPack && pcParameter->mpcDeclarator && IsRestrictPointer(pcParameter->mpcDeclarator);
				if (psType->IsClass())
				{
					//Laid out now, as the IR front end needs it.
					CompleteClass(psType->pcClass, pcParameter);
				}
			}
			j++;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Why the IR front end couldn't write a function (CCPP65IRFrontEnd::
// AppendUnsupported).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendUnsupported(CChars* psz)
{
	mpcIRFrontEnd->AppendUnsupported(psz);
}


//////////////////////////////////////////////////////////////////////////
//
// Starts generating a function's body.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::BeginFunction(CCPPFunction* pcFunction)
{
	mpcFunction = pcFunction;
	FreeVariables(&mapcLocals);

	mbThis = pcFunction->mpcClass && !pcFunction->mbStatic;
	mpcPackParameter = NULL;
	mapcPackElements.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// Leaves the function BeginFunction started, as the IR front end leaves a
// function it writes.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::EndFunction(CCPPFunction* pcFunction)
{
	//Only a function the IR front end couldn't write ends here (it ends the ones it writes itself): nothing is written.
	mpcFunction = NULL;
	mbThis = false;
}


//////////////////////////////////////////////////////////////////////////
//
// Gives every local variable in the function its own place in the frame before the
// body is generated, so the frame size is known.  Places are not shared between
// blocks yet.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::AllocateLocals(CCPPSyntax* pcNode)
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
			if ((pcDeclaration->mpcSpecifiers->muiFlags & (CPPDS_STATIC | CPPDS_EXTERN | CPPDS_THREAD_LOCAL)) == CPPDS_STATIC)
			{
				return StaticLocal(pcInit, pcDeclaration->mpcSpecifiers);
			}
			if (pcDeclaration->mpcSpecifiers->muiFlags & (CPPDS_STATIC | CPPDS_EXTERN | CPPDS_THREAD_LOCAL))
			{
				return Error(pcNode, "Extern and thread local variables are not supported by the compiler yet");
			}
			psType = GetType(pcDeclaration->mpcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
			if (!psType)
			{
				return false;
			}
			if (!psType->IsScalar() && !psType->IsArray() && !psType->IsClass() && !psType->IsReference())
			{
				return Error(pcNode, "This type of local variable is not supported by the compiler yet");
			}
			if ((psType->IsArray() ? psType->psBase : psType)->IsClass() && !CompleteClass((psType->IsArray() ? psType->psBase : psType)->pcClass, pcNode))
			{
				return false;
			}

			AddVariable(&mapcLocals, pcInit->mpcSymbol, psType, CPPVS_Local, 0);
		}
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
bool CCPP65Generator::AnonymousLocal(CCPPSyntaxClassSpecifier* pcSpecifier)
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
		pcVariable = AddVariable(&mapcLocals, psField->pcSymbol, psField->psType, CPPVS_Local, psField->iOffset);
		pcVariable->mpcAnonymous = pcClass;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A static local variable is data with its own label, initialised in the data.  Its
// initialiser must be constant; one that is not (it would be worked out the first
// time the declaration is reached) is not supported yet, nor are objects.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::StaticLocal(CCPPSyntaxInitDeclarator* pcInit, CCPPSyntaxDeclSpecifiers* pcSpecifiers)
{
	CCPPVariable*		pcVariable;
	SCPP65Type*			psType;
	CChars				szLabel;
	bool				bResult;
	bool				bInitialise;
	bool				bThis;

	psType = GetType(pcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
	if (!psType)
	{
		return false;
	}
	if ((psType->IsArray() ? psType->psBase : psType)->IsClass() || psType->IsReference())
	{
		return Error(pcInit, "Static local objects and references are not supported by the compiler yet");
	}

	pcVariable = AddVariable(&mapcLocals, pcInit->mpcSymbol, psType, CPPVS_Static, miStatics);
	miStatics++;
	pcVariable->mbDefined = true;
	szLabel.Init();
	AppendVariableLabel(&szLabel, pcVariable);
	BeginData(&szLabel, DataAlignment(psType, false), false, NULL, IsBss(pcInit->mpcInitializer, psType));
	szLabel.Kill();
	if (IsBss(pcInit->mpcInitializer, psType))
	{
		AppendSpace(psType->iBytes);
		return true;
	}

	bThis = mbThis;
	mbThis = false;
	mbStaticLocal = true;
	bResult = GlobalInitialiser(psType, pcInit->mpcInitializer, pcInit, &bInitialise);
	mbStaticLocal = false;
	mbThis = bThis;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The label of a global, a static data member or a static local (S, its
// number, _ and its name).  A global is used once its label is written.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendVariableLabel(CChars* psz, CCPPVariable* pcVariable)
{
	char*	szName;
	int		iLength;

	if (pcVariable->meStorage == CPPVS_Static)
	{
		szName = mpcTokeniser->GetIdentifier(pcVariable->mpcSymbol->mlliName, &iLength);
		psz->Append('S');
		psz->Append(pcVariable->miOffset);
		psz->Append('_');
		psz->Append(szName, iLength);
		return;
	}
	pcVariable->mbUsed = true;
	AppendGlobalLabel(psz, pcVariable);
}


//////////////////////////////////////////////////////////////////////////
//
// A static_assert whose condition is a constant that is false is an error.  One the generator cannot work out
// (sizeof a member function pointer ...) is ignored.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::StaticAssert(CCPPSyntaxStaticAssert* pcAssert)
{
	int64	lliValue;
	size	uiErrors;
	bool	bConstant;

	uiErrors = mcErrors.NumErrors();
	BeginQuiet();
	bConstant = Constant(pcAssert->mpcCondition, &lliValue);
	EndQuiet();
	if (bConstant && (lliValue == 0))
	{
		return Error(pcAssert, "The static_assert failed");
	}
	return mcErrors.NumErrors() == uiErrors;
}


//////////////////////////////////////////////////////////////////////////
//
// String literals are Str and a number, zero terminated.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendStrings(CChars* pszDest)
{
	size	i;
	CChars	szLabel;

	for (i = 0; i < maszStrings.NumElements(); i++)
	{
		szLabel.Init("Str");
		szLabel.Append((int)i);
		AppendSection(pszDest, "rodata", &szLabel, NULL);
		szLabel.Kill();
		pszDest->Append("Str");
		pszDest->Append((int)i);
		pszDest->Append(':');
		pszDest->AppendNewLine();
		pszDest->Append("\t\t.byte ");
		pszDest->Append(maszStrings.Get(i));
		pszDest->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Runtime routines the generated code calls.  Only the ones used are added:
// R_Pure, which a pure virtual function's vtable entry is, and the target's
// own (CCPPTarget::AppendRuntime).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendRuntime(void)
{
	if (mbPureVirtual && BeginRuntime("R_Pure"))
	{
		//A pure virtual function was called.
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
CArrayChars* CCPP65Generator::GetRuntimeCalls(void)
{
	return &maszRuntime;
}


//////////////////////////////////////////////////////////////////////////
//
// A runtime routine's COMDAT section and label (any unit may write it), unless
// the unit has written it already: then false.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::BeginRuntime(const char* szName)
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
	AppendSection(&mszCode, "text", &szLabel, ", comdat, resident");
	AppendGlobal(&mszCode, &szLabel);
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
CChars* CCPP65Generator::GetRuntimeCode(void)
{
	return &mszCode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::RuntimeError(const char* szMessage)
{
	ErrorAt(-1, szMessage);
}
