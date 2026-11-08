/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

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
#include "IR816Backend.h"
#include "CPP65Generator.h"


bool	gbCPP65DefaultWarnNarrowing = false;
bool	gbW65C816Assembly = CPP65_W65C816_ASSEMBLY;		//SetW65C816Assembly.


//////////////////////////////////////////////////////////////////////////
//
// Whether generators made from now on warn about narrowing conversions: an
// implicit conversion of a non constant integer to a narrower integer type
// ("Integer conversion narrows").  Off by default: code written for a 32
// bit int and size has many that are correct.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::SetDefaultWarnNarrowing(bool bWarnNarrowing)
{
	gbCPP65DefaultWarnNarrowing = bWarnNarrowing;
}


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
// Whether the W65C816 code may read Garth Wilson's product table
// (MathMult) in place.  See CCPP65Builder::SetMathTables.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::SetMathTables(bool bMathTables)
{
	mbMathTables = bMathTables;
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
	mpsTarget = GetCPPTarget(eTarget);
	mcTypes.Init(mpsTarget->iIntBytes, mpsTarget->iPointerBytes);

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
	mbMathTables = true;
	mlliCompileTime = 0;
	mcMemorySpillStore.Init();
	mpcSpillStore = &mcMemorySpillStore;
	muiSpillUnit = 0;
	mauiCodePieces.Init();
	mpcSemanticPass = NULL;

	//Every function body is written as IR (and generated by the target's IR back end).
	mpcIRFrontEnd = NewMalloc<CCPP65IRFrontEnd>();
	mpcIRFrontEnd->Init(this);
	mcTypeChecker.Init(this);
	mcErrors.Init();
	mcWarnings.Init();

	mapsGlobals.Init();
	mapsFunctions.Init();
	mcFunctionsOfNames.Init(true);
	mapsClasses.Init();
	mapsTemplates.Init();
	mapsAllBindings.Init();
	mapapsPacks.Init();
	mapcPackElements.Init();
	mapcPackSymbols.Init();
	mpcPackParameter = NULL;
	mapsQueue.Init();
	mapcFailed.Init();
	mpsBindings = NULL;
	mcTypeOfs.Init(true);
	maszEmitted.Init();
	miQuiet = 0;
	miWarningChecks = 0;
	miConstantTypes = 0;
	mbWarnNarrowing = gbCPP65DefaultWarnNarrowing;
	mbQuietFailed = false;
	maszStrings.Init();
	miStartUps = 0;
	mbDataBss = false;
	mbPureVirtual = false;

	mapsLocals.Init();
	mpsFunction = NULL;
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
	SCPP65Function*		psFunction;
	SCPP65Class*		psClass;
	SCPP65Bindings*		psBindings;
	CArrayCPP65Types*	papsPack;

	mcTypeChecker.Kill();
	SafeKill(mpcIRFrontEnd);
	FreeVariables(&mapsLocals);
	mapsLocals.Kill();
	mcBackendScratch.Kill();

	for (i = 0; i < maszStrings.NumElements(); i++)
	{
		maszStrings.Get(i)->Kill();
	}
	maszStrings.Kill();
	for (i = 0; i < mapsClasses.NumElements(); i++)
	{
		psClass = mapsClasses.GetPtr(i);
		psClass->mszArguments.Kill();
		for (j = 0; j < psClass->macVtables.NumElements(); j++)
		{
			psClass->macVtables.Get(j)->maiAdjust.Kill();
			psClass->macVtables.Get(j)->mapsFunctions.Kill();
		}
		psClass->macVtables.Kill();
		psClass->mapsVirtuals.Kill();
		psClass->macFields.Kill();
		psClass->macUsings.Kill();
		psClass->macBases.Kill();
		SafeFree(psClass);
	}
	mapsClasses.Kill();
	for (i = 0; i < mapsFunctions.NumElements(); i++)
	{
		psFunction = mapsFunctions.GetPtr(i);
		psFunction->mszArguments.Kill();
		psFunction->mapcDefaults.Kill();
		psFunction->mapsParameters.Kill();
		SafeFree(psFunction);
	}
	mcFunctionsOfNames.Kill();
	mapsFunctions.Kill();
	for (i = 0; i < mapsTemplates.NumElements(); i++)
	{
		mapsTemplates.GetPtr(i)->mszArguments.Kill();
		free(mapsTemplates.GetPtr(i));
	}
	mapsTemplates.Kill();
	for (i = 0; i < mapsAllBindings.NumElements(); i++)
	{
		psBindings = mapsAllBindings.GetPtr(i);
		psBindings->macBindings.Kill();
		SafeFree(psBindings);
	}
	mapsAllBindings.Kill();
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
	mapsQueue.Kill();
	mapcFailed.Kill();
	mcTypeOfs.Kill();
	maszEmitted.Kill();
	FreeVariables(&mapsGlobals);
	mapsGlobals.Kill();

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
	SCPP65Function*				psFunction;

	pcRoot = mpcTree->GetRoot();
	if (pcRoot)
	{
		Declarations(&pcRoot->macpDeclarations);
	}

	//Functions generated when they are first called, which may call others.
	while (mapsQueue.NumElements() > 0)
	{
		psFunction = mapsQueue.GetPtr(0);
		mapsQueue.RemoveAt(0);
		GenerateFunction(psFunction);
	}

	//The IR front end inlines across the unit.
	mpcIRFrontEnd->InlineUnit();
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

	//If the IR front end missed a function another unit exported, the unit is generated again (MissedImports).
	if (!mpcIRFrontEnd->FinishUnit())
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
// A whole program from one translation unit, as one object: the start up (see
// AppendStartUp), then the translation unit.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::GenerateProgram(CChars* pszDest)
{
	AppendStartUp(pszDest);
	return Generate(pszDest);
}


//////////////////////////////////////////////////////////////////////////
//
// The start up sets the stack to 001F:FFFF (the first 4 bytes pushed are at
// 001F:FFFC), W to the pseudo registers at 0000:0000 and the registers to 32 bits,
// calls the init table (each global's R_Initialise, in link order), calls main, calls
// the fini table backwards and stops with main's result in A.  Its object must be
// linked first so it is at the start of the text, 0001:0000; the reset vector is left
// to the loader.
//
// After main it calls Libc65's __RunAtExit (the atexit functions, last first) when
// stdlib.cpp is linked: the reference is weak, so it doesn't pull stdlib.cpp in.
//
// It also defines C's _Exit(int iStatus), which stops at once with iStatus in A.
// Libc65's exit runs the atexit functions and the fini table and then calls it.
//
// It is kept, and everything it reaches, when the linker leaves out what is unused, and
// it is resident when functions are loaded lazily.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendStartUp(CChars* pszDest, ECPPTarget eTarget)
{
	if (eTarget == CPPT_W65C816)
	{
		AppendStartUp816(pszDest);
		return;
	}
	pszDest->Append("\t\t.section text, keep, resident")->AppendNewLine();
	pszDest->Append("Start:")->AppendNewLine();
	pszDest->Append("\t\tcpy S, 0x")->Append((uint32)CPP65_STACK_TOP, 16)->AppendNewLine();
	pszDest->Append("\t\tcpy W, 0")->AppendNewLine();
	pszDest->Append("\t\twtth (A, B), 32")->AppendNewLine();
	pszDest->Append("\t\twtth (X, Y), 32")->AppendNewLine();
	pszDest->Append("\t\tcpy Y, __init_start")->AppendNewLine();
	pszDest->Append("StartInitialise:")->AppendNewLine();
	pszDest->Append("\t\tcmp Y, __init_end")->AppendNewLine();
	pszDest->Append("\t\tbras c, StartMain")->AppendNewLine();
	pszDest->Append("\t\tpsh Y")->AppendNewLine();
	pszDest->Append("\t\tjsr [Y]")->AppendNewLine();
	pszDest->Append("\t\tpul Y")->AppendNewLine();
	pszDest->Append("\t\tadd Y, 4")->AppendNewLine();
	pszDest->Append("\t\tbra StartInitialise")->AppendNewLine();
	pszDest->Append("StartMain:")->AppendNewLine();
	pszDest->Append("\t\tjsr _main")->AppendNewLine();
	pszDest->Append("\t\tpsh A")->AppendNewLine();
	pszDest->Append("\t\t.weak ___RunAtExit")->AppendNewLine();
	pszDest->Append("\t\tcpy Y, ___RunAtExit")->AppendNewLine();
	pszDest->Append("\t\tcmp Y, 0")->AppendNewLine();
	pszDest->Append("\t\tbras z, StartAtExitDone")->AppendNewLine();
	pszDest->Append("\t\tjsr Y")->AppendNewLine();
	pszDest->Append("StartAtExitDone:")->AppendNewLine();
	pszDest->Append("\t\tcpy Y, __fini_end")->AppendNewLine();
	pszDest->Append("StartFinalise:")->AppendNewLine();
	pszDest->Append("\t\tcmp Y, __fini_start")->AppendNewLine();
	pszDest->Append("\t\tbras z, StartStop")->AppendNewLine();
	pszDest->Append("\t\tsub Y, 4")->AppendNewLine();
	pszDest->Append("\t\tpsh Y")->AppendNewLine();
	pszDest->Append("\t\tjsr [Y]")->AppendNewLine();
	pszDest->Append("\t\tpul Y")->AppendNewLine();
	pszDest->Append("\t\tbra StartFinalise")->AppendNewLine();
	pszDest->Append("StartStop:")->AppendNewLine();
	pszDest->Append("\t\tpul A")->AppendNewLine();
	pszDest->Append("\t\tstp")->AppendNewLine();
	pszDest->Append("\t\t.global __Exit")->AppendNewLine();
	pszDest->Append("__Exit:")->AppendNewLine();
	pszDest->Append("\t\tcpy V, S")->AppendNewLine();
	pszDest->Append("\t\tcpy A, [5 + V]")->AppendNewLine();
	pszDest->Append("\t\tstp")->AppendNewLine();
	pszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// The W65C816's start up.  Bank 0 only holds the stack, the direct page, the vectors and their stubs (the user,
// 2026-10-25), so the reset vector goes to a jml stub there and the start up is in bank 1 onwards.  It switches to
// native mode with 16 bit registers, sets S to the target's stack top and D to 0, clears bss, calls the init table's routines
// (each global's R_Initialise, in link order), calls main, Libc65's __RunAtExit when stdlib.cpp is linked (a weak
// reference), the fini table's routines last first, and stops with main's result in A.  The table walks keep a far
// pointer at 0000:0000 (the direct page while D is 0).  _Exit(int) stops with its argument in A (__Exit.a, the
// register entry the compiler's callers use, and __Exit, the stack entry).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendStartUp816(CChars* pszDest)
{
	pszDest->Append("\t\t.section rodata.ResetVector, at 0xFFFC, keep\n");
	pszDest->Append("\t\t.word ResetStub\n");
	pszDest->Append("\t\t.section text.ResetStub, at 0xFF00, keep\n");
	pszDest->Append("ResetStub:\n");
	pszDest->Append("\t\tjml Start\n");
	pszDest->Append("\t\t.section text, keep\n");
	pszDest->Append("Start:\n");
	pszDest->Append("\t\tclc\n");
	pszDest->Append("\t\txce\n");
	pszDest->Append("\t\trep (m, x)\n");
	pszDest->Append("\t\tlda 0x")->Append(GetCPPTarget(CPPT_W65C816)->uiStackTop, 16)->AppendNewLine();
	pszDest->Append("\t\ttcs\n");
	pszDest->Append("\t\tlda 0\n");
	pszDest->Append("\t\ttcd\n");

	//bss (globals with no initialiser) isn't in the program's image.  Up to 64KB is cleared a word at a time (a long
	//address plus X crosses banks; an odd size clears the byte after it too, where nothing is yet), more a byte at a time.
	pszDest->Append("\t\tlda __bss_end & 0xFFFF\n");
	pszDest->Append("\t\tsec\n");
	pszDest->Append("\t\tsbc __bss_start & 0xFFFF\n");
	pszDest->Append("\t\ttay\n");
	pszDest->Append("\t\tlda __bss_end >> 16\n");
	pszDest->Append("\t\tsbc __bss_start >> 16\n");
	pszDest->Append("\t\tbne StartClearLarge\n");
	pszDest->Append("\t\ttya\n");
	pszDest->Append("\t\tinc A\n");
	pszDest->Append("\t\tlsr A\n");
	pszDest->Append("\t\tbeq StartCleared\n");
	pszDest->Append("\t\ttay\n");
	pszDest->Append("\t\tldx 0\n");
	pszDest->Append("\t\tlda 0\n");
	pszDest->Append("StartClearWord:\n");
	pszDest->Append("\t\tsta [__bss_start + X]:24\n");
	pszDest->Append("\t\tinx\n");
	pszDest->Append("\t\tinx\n");
	pszDest->Append("\t\tdey\n");
	pszDest->Append("\t\tbne StartClearWord\n");
	pszDest->Append("\t\tbra StartCleared\n");
	pszDest->Append("StartClearLarge:\n");
	pszDest->Append("\t\tlda __bss_start & 0xFFFF\n");
	pszDest->Append("\t\tsta [D + 0]\n");
	pszDest->Append("\t\tlda __bss_start >> 16\n");
	pszDest->Append("\t\tsta [D + 2]\n");
	pszDest->Append("StartClear:\n");
	pszDest->Append("\t\tlda [D + 0]\n");
	pszDest->Append("\t\tcmp __bss_end & 0xFFFF\n");
	pszDest->Append("\t\tbne StartClearByte\n");
	pszDest->Append("\t\tlda [D + 2]\n");
	pszDest->Append("\t\tcmp __bss_end >> 16\n");
	pszDest->Append("\t\tbeq StartCleared\n");
	pszDest->Append("StartClearByte:\n");
	pszDest->Append("\t\tsep m\n");
	pszDest->Append("\t\tlda 0\n");
	pszDest->Append("\t\tsta [[D + 0]:24]\n");
	pszDest->Append("\t\trep m\n");
	pszDest->Append("\t\tinc [D + 0]\n");
	pszDest->Append("\t\tbne StartClear\n");
	pszDest->Append("\t\tinc [D + 2]\n");
	pszDest->Append("\t\tbra StartClear\n");
	pszDest->Append("StartCleared:\n");
	pszDest->Append("\t\tlda __init_start & 0xFFFF\n");
	pszDest->Append("\t\tsta [D + 0]\n");
	pszDest->Append("\t\tlda __init_start >> 16\n");
	pszDest->Append("\t\tsta [D + 2]\n");
	pszDest->Append("StartInitialise:\n");
	pszDest->Append("\t\tlda [D + 0]\n");
	pszDest->Append("\t\tcmp __init_end & 0xFFFF\n");
	pszDest->Append("\t\tbne StartInitialiseCall\n");
	pszDest->Append("\t\tlda [D + 2]\n");
	pszDest->Append("\t\tcmp __init_end >> 16\n");
	pszDest->Append("\t\tbeq StartMain\n");
	pszDest->Append("StartInitialiseCall:\n");
	pszDest->Append("\t\tjsr StartCallEntry\n");
	pszDest->Append("\t\tclc\n");
	pszDest->Append("\t\tlda [D + 0]\n");
	pszDest->Append("\t\tadc 4\n");
	pszDest->Append("\t\tsta [D + 0]\n");
	pszDest->Append("\t\tlda [D + 2]\n");
	pszDest->Append("\t\tadc 0\n");
	pszDest->Append("\t\tsta [D + 2]\n");
	pszDest->Append("\t\tbra StartInitialise\n");
	pszDest->Append("StartMain:\n");
	pszDest->Append("\t\tjsl _main\n");
	pszDest->Append("\t\tpha\n");
	pszDest->Append("\t\t.weak ___RunAtExit\n");
	pszDest->Append("\t\tlda ___RunAtExit & 0xFFFF\n");
	pszDest->Append("\t\tora ___RunAtExit >> 16\n");
	pszDest->Append("\t\tbeq StartAtExitDone\n");
	pszDest->Append("\t\tjsl ___RunAtExit\n");
	pszDest->Append("StartAtExitDone:\n");
	pszDest->Append("\t\tlda __fini_end & 0xFFFF\n");
	pszDest->Append("\t\tsta [D + 0]\n");
	pszDest->Append("\t\tlda __fini_end >> 16\n");
	pszDest->Append("\t\tsta [D + 2]\n");
	pszDest->Append("StartFinalise:\n");
	pszDest->Append("\t\tlda [D + 0]\n");
	pszDest->Append("\t\tcmp __fini_start & 0xFFFF\n");
	pszDest->Append("\t\tbne StartFinaliseCall\n");
	pszDest->Append("\t\tlda [D + 2]\n");
	pszDest->Append("\t\tcmp __fini_start >> 16\n");
	pszDest->Append("\t\tbeq StartStop\n");
	pszDest->Append("StartFinaliseCall:\n");
	pszDest->Append("\t\tsec\n");
	pszDest->Append("\t\tlda [D + 0]\n");
	pszDest->Append("\t\tsbc 4\n");
	pszDest->Append("\t\tsta [D + 0]\n");
	pszDest->Append("\t\tlda [D + 2]\n");
	pszDest->Append("\t\tsbc 0\n");
	pszDest->Append("\t\tsta [D + 2]\n");
	pszDest->Append("\t\tjsr StartCallEntry\n");
	pszDest->Append("\t\tbra StartFinalise\n");
	pszDest->Append("StartStop:\n");
	pszDest->Append("\t\tpla\n");
	pszDest->Append("\t\tstp\n");

	//Calls the routine the table entry [D + 0] points at: the return address and the routine - 1 are pushed, and rtl
	//goes to it.
	pszDest->Append("StartCallEntry:\n");
	pszDest->Append("\t\t.width (A, X), 16\n");
	pszDest->Append("\t\tldy 0\n");
	pszDest->Append("\t\tlda [[D + 0]:24 + Y]\n");
	pszDest->Append("\t\tsta [D + 4]\n");
	pszDest->Append("\t\tldy 2\n");
	pszDest->Append("\t\tlda [[D + 0]:24 + Y]\n");
	pszDest->Append("\t\tsta [D + 6]\n");
	pszDest->Append("\t\tphk\n");
	pszDest->Append("\t\tper StartCallReturn - 1\n");
	pszDest->Append("\t\tsep m\n");
	pszDest->Append("\t\tlda [D + 6]\n");
	pszDest->Append("\t\tpha\n");
	pszDest->Append("\t\trep m\n");
	pszDest->Append("\t\tlda [D + 4]\n");
	pszDest->Append("\t\tdec A\n");
	pszDest->Append("\t\tpha\n");
	pszDest->Append("\t\trtl\n");
	pszDest->Append("StartCallReturn:\n");
	pszDest->Append("\t\trts\n");
	//_Exit's stack entry, and its register entry (iStatus in A, as the compiler's callers pass it).
	pszDest->Append("\t\t.global __Exit, __Exit.a\n");
	pszDest->Append("__Exit:\n");
	pszDest->Append("\t\t.width (A, X), 16\n");
	pszDest->Append("\t\tlda [S + 4]\n");
	pszDest->Append("__Exit.a:\n");
	pszDest->Append("\t\tstp\n");
	pszDest->AppendNewLine();
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
// A global or static local with no initialiser starts as zeros: on the
// W65C816 it goes in bss, which takes no space in the program's image and
// is cleared by the start up.  (The 65C64's start up and lazy loading don't
// clear bss, so it keeps its zeros in data.)  An object is cleared there
// too, before its constructor runs from the init table.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsBss(CCPPSyntax* pcInitialiser, SCPP65Type* psType)
{
	return (mpsTarget->eTarget == CPPT_W65C816) && !pcInitialiser && !psType->IsReference() && (psType->iBytes > 0);
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
// A static function or a member of a class without a name is the unit's own: its
// label is local to the object.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsUnitFunction(SCPP65Function* psFunction)
{
	if (psFunction->psClass)
	{
		return psFunction->psClass->lliName < 0;
	}
	return psFunction->bStatic;
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
	SCPP65Function*	psFunction;
	SCPP65Variable*	psVariable;
	CChars			szLabel;
	size			i;

	for (i = 0; i < mapsGlobals.NumElements(); i++)
	{
		psVariable = mapsGlobals.GetPtr(i);
		if (psVariable->bUsed && !psVariable->bDefined)
		{
			szLabel.Init();
			AppendGlobalLabel(&szLabel, psVariable->psClass, psVariable->pcSymbol->mlliName);
			paszLabels->AddIfUnique(szLabel);
			szLabel.Kill();
		}
	}

	for (i = 0; i < mapsFunctions.NumElements(); i++)
	{
		psFunction = mapsFunctions.GetPtr(i);
		if (psFunction->bCalled && !psFunction->bGenerated)
		{
			szLabel.Init();
			AppendFunctionLabel(&szLabel, psFunction);
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
// Whether W65C816 builds use the units' X816.asm (and define
// __W65C816_ASSEMBLY__, which leaves out the C++ they replace) or compile
// the C++.  CPP65_W65C816_ASSEMBLY to start with.  Set it before building.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::SetW65C816Assembly(bool bAssembly)
{
	gbW65C816Assembly = bAssembly;
	CIR816Backend::SetSoftFloatInFrame(bAssembly);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsW65C816Assembly(void)
{
	return gbW65C816Assembly;
}


//////////////////////////////////////////////////////////////////////////
//
// The macros the target predefines, for the configuration code is preprocessed
// with.  __65C64__ or __W65C816__ is the processor (Libc65 uses __65C64__ for
// what its 65C64 assembly does), and __W65C816_ASSEMBLY__ when W65C816 builds
// use the X816.asm units (SetW65C816Assembly); __LIBC65__ is on both: the Libc65 headers and
// devices, which BaseLib's Define.h and its _65C64.cpp files are written for.
// The host's macros (_MSC_VER, _WIN32) are not defined, so its headers and
// _Windows.cpp files are left out.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AddDefines(CConfig* pcConfig, ECPPTarget eTarget)
{
	if (eTarget == CPPT_Host)
	{
		//The host's project defines its own macros.
		return;
	}
	pcConfig->AddDefine((char*)GetCPPTarget(eTarget)->szDefine);
	pcConfig->AddDefine("__LIBC65__=1");
	if ((eTarget == CPPT_W65C816) && gbW65C816Assembly)
	{
		pcConfig->AddDefine("__W65C816_ASSEMBLY__=1");
	}
	pcConfig->mbKeepPragmaPack = true;
	pcConfig->mbHostDefines = false;
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
	mcWarnings.Add(psSource->pcFile ? psSource->pcFile->ShortName() : NULL, psSource->iLine, psSource->iColumn, iToken, szMessageText.Text(), szDetail.Text());
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
	mcErrors.Add(psSource->pcFile ? psSource->pcFile->ShortName() : NULL, psSource->iLine, psSource->iColumn, iToken, szMessageText.Text(), szDetail.Text());
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
	pszMessage->Append(psExpansion->pcFile->ShortName());
	pszMessage->Append(" (");
	pszMessage->Append(psExpansion->iLine + 1);
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
	if (!pcName || (pcName->macpComponents.NumElements() != 2))
	{
		return false;
	}
	pcClass = (CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(0);
	return pcClass->mpcSymbol && (pcClass->mpcSymbol->meKind == CPPSK_Class) && !GetTemplateByName(GetComponentName(pcClass));
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
			if (!mpcSemanticPass)
			{
				bResult &= Error(pcDeclaration, "This declaration is not supported by the compiler yet");
				break;
			}
			//Checking looks at what it can in a namespace (qualified names aren't supported yet).
			bResult &= Declarations(&((CCPPSyntaxNamespace*)pcDeclaration)->macpDeclarations);
			break;
		case CPPS_AliasDeclaration:
		case CPPS_UsingDeclaration:
		case CPPS_UsingDirective:
			break;
		case CPPS_EmptyDeclaration:
		case CPPS_StaticAssert:
		case CPPS_LazyDeclaration:		//A header declaration the unit never used, so never parsed.
			break;
		default:
			bResult &= Error(pcDeclaration, "This declaration is not supported by the compiler yet");
			break;
		}
	}
	return bResult;
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
// Records a function's signature, a member of psClass if it is not NULL, or an
// instance of psTemplate.  A function may be declared any number of times but
// defined once.  Functions with the same name and different parameters are
// overloads.  Returns NULL after an error.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Function* CCPP65Generator::DeclareFunction(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, bool bDefinition, SCPP65Class* psClass, bool bPure, SCPP65Template* psTemplate, CChars* pszArguments)
{
	int64						lliName;
	SCPP65Function*				psFunction;
	SCPP65Function*				psOther;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxParameter*		pcParameter;
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;
	SCPP65Type*					psType;
	SCPP65Type*					psReturn;
	ECPP65FunctionKind			eKind;
	CArrayCPP65Types			apsParameters;
	CArraySyntax				apcDefaults;
	SCPP65Binding*				psPack;
	size						i;
	size						j;
	size						uiPack;
	uint32						uiFlags;
	bool						bConstThis;

	pcName = pcDeclarator->GetName();
	pcComponent = pcName ? pcName->GetLast() : NULL;
	if (!pcComponent || ((pcComponent->meKind == CPPNC_Conversion) && !psClass) || (!psClass && !psTemplate && (pcName->mbGlobal || (pcName->macpComponents.NumElements() != 1))))
	{
		Error(pcDeclarator, "Conversion functions outside classes and qualified names are not supported by the compiler yet");
		return NULL;
	}
	lliName = GetFunctionName(pcComponent);
	if (lliName == -1)
	{
		Error(pcDeclarator, "This function name is not supported by the compiler yet");
		return NULL;
	}
	eKind = CPP65FK_Function;
	if (pcComponent->meKind == CPPNC_Destructor)
	{
		eKind = CPP65FK_Destructor;
	}
	else if (psClass && (lliName == psClass->lliName))
	{
		eKind = CPP65FK_Constructor;
	}

	pcSuffix = pcDeclarator->GetFunctionSuffix();
	if ((pcDeclarator->macpSuffixes.NumElements() != 1) || pcDeclarator->mpcNested || pcSuffix->mpcTrailingReturn)
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
	else if (eKind == CPP65FK_Function)
	{
		psReturn = GetType(pcSpecifiers, NULL, pcDeclarator, NULL);
		for (i = 0; psReturn && (i < pcDeclarator->NumPointers()); i++)
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
	bConstThis = psClass && IsConstSuffix(pcSuffix);
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
	for (psOther = FirstFunctionOfName(lliName); psOther; psOther = psOther->psNextOfName)
	{
		if ((psOther->psClass != psClass) || (psOther->eKind != eKind) || (psOther->mapsParameters.NumElements() != apsParameters.NumElements()) || (psOther->psTemplate != psTemplate) ||
			(psOther->bVariadic != pcSuffix->mbVariadic))
		{
			continue;
		}
		if (psTemplate && !psOther->mszArguments.Equals(pszArguments))
		{
			continue;
		}
		if (((lliName == CPP65_CONVERSION_NAME) && !psOther->psReturn->Equals(psReturn)) || (psOther->bConstThis != bConstThis))
		{
			continue;
		}
		psFunction = psOther;
		for (j = 0; j < apsParameters.NumElements(); j++)
		{
			if (!psOther->mapsParameters.GetPtr(j)->Same(apsParameters.GetPtr(j)))
			{
				psFunction = NULL;
				break;
			}
		}
		if (!psFunction)
		{
			continue;
		}
		for (j = 0; j < apcDefaults.NumElements(); j++)
		{
			if (apcDefaults.GetPtr(j))
			{
				psFunction->mapcDefaults.SetPtr(j, apcDefaults.GetPtr(j));
			}
		}
		apcDefaults.Kill();
		apsParameters.Kill();
		psFunction->bVirtual |= (uiFlags & CPPDS_VIRTUAL) != 0;
		psFunction->bPure |= bPure;
		psFunction->bCLinkage |= mbCLinkage && !psClass;
		psFunction->bExplicit |= (uiFlags & CPPDS_EXPLICIT) != 0;
		return psFunction;
	}

	psFunction = AddFunction(psClass, lliName, eKind, psReturn);
	psFunction->pcDeclarator = pcDeclarator;
	for (i = 0; i < apsParameters.NumElements(); i++)
	{
		psFunction->mapsParameters.Add(apsParameters.GetPtr(i));
		psFunction->mapcDefaults.Add(apcDefaults.GetPtr(i));
	}
	psFunction->bConstThis = bConstThis;
	apcDefaults.Kill();
	apsParameters.Kill();
	psFunction->bVariadic = pcSuffix->mbVariadic;
	psFunction->bStatic = (uiFlags & CPPDS_STATIC) != 0;
	psFunction->bVirtual = (uiFlags & CPPDS_VIRTUAL) != 0;
	psFunction->bPure = bPure;
	psFunction->bInline = (uiFlags & (CPPDS_INLINE | CPPDS_CONSTEXPR)) != 0;
	psFunction->psTemplate = psTemplate;
	psFunction->bCLinkage = mbCLinkage && !psClass;
	psFunction->bExplicit = (uiFlags & CPPDS_EXPLICIT) != 0;
	if (pszArguments)
	{
		psFunction->mszArguments.Append(pszArguments);
	}
	if (eKind == CPP65FK_Constructor)
	{
		psClass->bConstructors = true;
	}
	else if (eKind == CPP65FK_Destructor)
	{
		psClass->psDestructor = psFunction;
	}
	return psFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// A new function.  Its overload number counts the functions with its name before it.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Function* CCPP65Generator::AddFunction(SCPP65Class* psClass, int64 lliName, ECPP65FunctionKind eKind, SCPP65Type* psReturn)
{
	SCPP65Function*			psFunction;
	SCPP65FunctionsOfName*	psOfName;
	SCPP65FunctionsOfName	sOfName;

	psFunction = NewMalloc<SCPP65Function>();
	psFunction->lliName = lliName;
	psFunction->psReturn = psReturn;
	psFunction->mapsParameters.Init();
	psFunction->bDefined = false;
	psFunction->psClass = psClass;
	psFunction->eKind = eKind;
	psFunction->bStatic = false;
	psFunction->bVirtual = false;
	psFunction->bPure = false;
	psFunction->iSlot = -1;
	psFunction->bCLinkage = false;
	psFunction->bExplicit = false;
	psFunction->bConstThis = false;
	psFunction->bImplicit = false;
	psFunction->psInherited = NULL;
	psFunction->mapcDefaults.Init();
	psFunction->bVariadic = false;
	psFunction->pcDefinition = NULL;
	psFunction->pcDeclarator = NULL;
	psFunction->psBindings = psClass ? psClass->psBindings : NULL;
	psFunction->bInline = false;
	psFunction->bQueued = false;
	psFunction->bGenerated = false;
	psFunction->bCalled = false;
	psFunction->psTemplate = NULL;
	psFunction->mszArguments.Init();
	psFunction->psNextOfName = NULL;
	mapsFunctions.Add(psFunction);

	psOfName = mcFunctionsOfNames.Get(lliName);
	if (psOfName)
	{
		psOfName->psLast->psNextOfName = psFunction;
		psOfName->psLast = psFunction;
	}
	else
	{
		sOfName.psFirst = psFunction;
		sOfName.psLast = psFunction;
		mcFunctionsOfNames.Put(lliName, &sOfName);
	}
	return psFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// The first function added with the name, or NULL.  The others follow it (psNextOfName) in the order they
// were added, as in mapsFunctions.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Function* CCPP65Generator::FirstFunctionOfName(int64 lliName)
{
	SCPP65FunctionsOfName*	psOfName;

	psOfName = mcFunctionsOfNames.Get(lliName);
	return psOfName ? psOfName->psFirst : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A function is called: it is generated later if it is defined but not generated
// yet.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::RequireFunction(SCPP65Function* psFunction)
{
	psFunction->bCalled = true;
	if (!psFunction->bGenerated && !psFunction->bQueued && (psFunction->pcDefinition || psFunction->bImplicit))
	{
		psFunction->bQueued = true;
		mapsQueue.Add(psFunction);
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
bool CCPP65Generator::SameParameters(SCPP65Function* psFunction, SCPP65Function* psOther)
{
	size	i;

	if ((psFunction->mapsParameters.NumElements() != psOther->mapsParameters.NumElements()) || (psFunction->bVariadic != psOther->bVariadic))
	{
		return false;
	}
	for (i = 0; i < psFunction->mapsParameters.NumElements(); i++)
	{
		if (!psFunction->mapsParameters.GetPtr(i)->Same(psOther->mapsParameters.GetPtr(i)))
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
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;
	SCPP65Variable*				psVariable;
	SCPP65Function*				psFunction;
	SCPP65Type*					psType;
	SCPP65Type*					psElement;
	CChars						szLabel;
	CCPPSyntaxName*				pcName;
	SCPP65Class*				psClass;
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
			psFunction = DeclareFunction(pcSpecifiers, pcInit->mpcDeclarator, false, NULL, false);
			EndQuiet();
			if (!psFunction)
			{
				mapcFailed.Add(pcInit->mpcDeclarator);
			}
			continue;
		}

		lliName = GetName(pcInit->mpcDeclarator);
		pcName = pcInit->mpcDeclarator->GetName();
		if ((lliName == -1) && pcName && (pcName->macpComponents.NumElements() == 2))
		{
			//int CClass::siStatic = 0; defines a static data member, so the class is laid out.
			psClass = GetComponentClass((CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(0), NULL, pcInit);
			if (psClass)
			{
				CompleteClass(psClass, pcInit);
			}
		}
		psVariable = pcInit->mpcSymbol ? GetVariable(pcInit->mpcSymbol) : NULL;
		if (!pcInit->mpcSymbol || ((lliName == -1) && !(psVariable && psVariable->psClass)))
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
			if (psType && !psVariable)
			{
				psVariable = AddVariable(&mapsGlobals, pcInit->mpcSymbol, psType, CPP65S_Global, 0);
			}
			if (psVariable)
			{
				psVariable->bRestrict |= IsRestrictPointer(pcInit->mpcDeclarator);
			}
			continue;
		}

		psType = GetType(pcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
		if (!psType)
		{
			bResult = false;
			continue;
		}
		psElement = psType->IsArray() ? psType->psBase : psType;
		if (psElement->IsClass() && !CompleteClass(psElement->psClass, pcInit))
		{
			bResult = false;
			continue;
		}

		if (!psVariable)
		{
			psVariable = AddVariable(&mapsGlobals, pcInit->mpcSymbol, psType, CPP65S_Global, 0);
		}
		psVariable->bInternal |= !psVariable->psClass && ((pcSpecifiers->muiFlags & CPPDS_STATIC) != 0);
		psVariable->bRestrict |= IsRestrictPointer(pcInit->mpcDeclarator);
		if (psVariable->bDefined)
		{
			bResult &= Error(pcInit, "The variable is defined twice");
			continue;
		}
		psVariable->bDefined = true;
		psVariable->psType = psType;

		szLabel.Init();
		AppendVariableLabel(&szLabel, psVariable);
		bBss = IsBss(pcInit->mpcInitializer, psType);
		BeginData(&szLabel, DataAlignment(psType, psElement->IsClass()), !psVariable->bInternal, HasUsedAttribute(pcInit->mpcDeclarator) ? ", keep" : NULL, bBss);

		//GlobalInitialiser writes the constant data.  What isn't constant is worked out by the global's own R_Initialise,
		//and an object is destroyed by its R_Finalise: the IR front end writes them (GlobalStartUpRoutines).
		mbThis = false;
		if (bBss)
		{
			AppendSpace(psType->iBytes);
			bResult &= GlobalStartUpRoutines(psType, pcInit->mpcInitializer, &szLabel, psElement->IsClass(), psElement->IsClass() && (psElement->psClass->psDestructor != NULL));
		}
		else if (!psElement->IsClass() && !psType->IsReference())
		{
			bInitialise = false;
			bResult &= GlobalInitialiser(psType, pcInit->mpcInitializer, pcInit, &bInitialise);
			bResult &= GlobalStartUpRoutines(psType, pcInit->mpcInitializer, &szLabel, bInitialise, false);
		}
		else
		{
			AppendZeros(psType->iBytes);
			bResult &= GlobalStartUpRoutines(psType, pcInit->mpcInitializer, &szLabel, true, psElement->IsClass() && (psElement->psClass->psDestructor != NULL));
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
		else if ((pcInitialiser->GetKind() == CPPS_StringExpression) && (psType->psBase->iBytes == 1))
		{
			auiBytes.Init();
			bResult = DecodeString(pcInitialiser, &auiBytes);
			for (i = 0; i < auiBytes.NumElements(); i++)
			{
				AppendData(psType->psBase, auiBytes.GetValue(i));
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
	if (pcInitialiser && psType->IsPointer() && (psType->psBase->iBytes == 1) && (pcInitialiser->GetKind() == CPPS_StringExpression))
	{
		auiBytes.Init();
		bResult = DecodeString(pcInitialiser, &auiBytes);
		if (bResult)
		{
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
void CCPP65Generator::AppendGlobalLabel(CChars* psz, SCPP65Class* psClass, int64 lliName)
{
	CChars	szName;
	char*	szIdentifier;
	int		iLength;

	if (!psClass)
	{
		AppendName(psz, lliName);
		return;
	}
	szIdentifier = mpcTokeniser->GetIdentifier(lliName, &iLength);
	szName.Init(szIdentifier, 0, iLength);
	AppendClassLabel(psz, psClass, szName.Text());
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65Variable* CCPP65Generator::AddVariable(CArrayCPP65Variables* papsVariables, CCPPSymbol* pcSymbol, SCPP65Type* psType, ECPP65Storage eStorage, int iOffset)
{
	SCPP65Variable*	psVariable;

	psVariable = NewMalloc<SCPP65Variable>();
	psVariable->pcSymbol = pcSymbol;
	psVariable->psType = psType;
	psVariable->eStorage = eStorage;
	psVariable->iOffset = iOffset;
	psVariable->psClass = NULL;
	psVariable->bDefined = false;
	psVariable->bUsed = false;
	psVariable->bInternal = false;
	psVariable->bRestrict = false;
	psVariable->psAnonymous = NULL;
	papsVariables->Add(psVariable);
	return psVariable;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::FreeVariables(CArrayCPP65Variables* papsVariables)
{
	size	i;

	for (i = 0; i < papsVariables->NumElements(); i++)
	{
		free(papsVariables->GetPtr(i));
	}
	papsVariables->ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// The local, parameter or global a symbol names.  Locals are searched first.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Variable* CCPP65Generator::GetVariable(CCPPSymbol* pcSymbol)
{
	size			i;
	SCPP65Variable*	psVariable;

	for (i = 0; i < mapsLocals.NumElements(); i++)
	{
		psVariable = mapsLocals.GetPtr(i);
		if (psVariable->pcSymbol == pcSymbol)
		{
			return psVariable;
		}
	}
	for (i = 0; i < mapsGlobals.NumElements(); i++)
	{
		psVariable = mapsGlobals.GetPtr(i);
		if (psVariable->pcSymbol == pcSymbol)
		{
			return psVariable;
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
// without a size (char* argv[]) is a pointer.  [dcl.fct]
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::GetDeclaratorType(SCPP65Type* psType, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntax* pcNode, CCPPSyntax* pcInitialiser)
{
	CCPPSyntaxPointerOperator*	pcPointer;
	CCPPSyntax*					pcSuffix;
	CCPPSyntaxArraySuffix*		pcArray;
	SCPP65Class*				psClass;
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
			psClass = pcPointer->mpcClass ? GetNameClass(pcPointer->mpcClass, pcNode) : NULL;
			if (!psClass)
			{
				Error(pcNode, "The class of a pointer to a member is not supported by the compiler yet");
				return NULL;
			}
			psType = mcTypes.MemberPointer(psClass, psType);
			break;
		default:
			Error(pcNode, "This declarator is not supported by the compiler yet");
			return NULL;
		}
	}

	for (i = pcDeclarator->macpSuffixes.NumElements(); i > 0; i--)
	{
		pcSuffix = pcDeclarator->macpSuffixes.GetPtr(i - 1);
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
		if (psType->IsClass() && !CompleteClass(psType->psClass, pcNode))
		{
			return NULL;
		}
		psType = mcTypes.Array(psType, (int)lliCount);
	}

	if (pcDeclarator->mpcNested)
	{
		return GetDeclaratorType(psType, pcDeclarator->mpcNested, pcNode, pcInitialiser);
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

	if (pcInitialiser && (pcInitialiser->GetKind() == CPPS_InitializerList) && psElement->IsInteger() && (psElement->iBytes == 1))
	{
		papcElements = &((CCPPSyntaxInitializerList*)pcInitialiser)->macpElements;
		if ((papcElements->NumElements() == 1) && (papcElements->GetPtr(0)->GetKind() == CPPS_StringExpression))
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
		iCount = DecodeString(pcInitialiser, &auiBytes) ? (int)auiBytes.NumElements() + 1 : 0;
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
	uint32								uiKeywords;
	bool								bSigned;
	CCPPSyntaxTypeSpecifier*			pcType;
	CCPPSyntaxName*						pcName;
	CCPPSyntaxClassSpecifier*			pcClass;
	CCPPSyntaxElaboratedTypeSpecifier*	pcElaborated;
	CCPPSymbol*							pcSymbol;
	SCPP65Class*						psClass;
	SCPP65Binding*						psBinding;

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
			pcClass = (CCPPSyntaxClassSpecifier*)pcType;
			psClass = pcClass->mpcSymbol ? GetClass(pcClass->mpcSymbol, pcNode) : GetUnnamedClass(pcClass);
			if (!psClass->pcSpecifier)
			{
				psClass->pcSpecifier = pcClass;
			}
			return psClass->psType;
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
			psClass = GetComponentClass(pcElaborated->mpcName->GetLast(), pcSymbol ? pcSymbol : pcElaborated->mpcName->mpcSymbol, pcNode);
			if (psClass)
			{
				return psClass->psType;
			}
		}
		else if (pcType->GetKind() == CPPS_NamedTypeSpecifier)
		{
			pcName = ((CCPPSyntaxNamedTypeSpecifier*)pcType)->mpcName;
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
					psClass = GetNameClass(pcName, pcNode);
					return psClass ? psClass->psType : NULL;
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
	Error(pcNode, "This declaration is not supported by the compiler yet");
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A function definition outside a class, a member of psClass if it is not NULL (or
// of the class its qualified name names).  Inline and constexpr functions are
// generated when they are called; others now.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::Function(CCPPSyntaxFunctionDefinition* pcFunction, SCPP65Class* psClass)
{
	CCPPSyntaxName*		pcName;
	SCPP65Function*		psFunction;
	size				uiErrors;
	size				i;

	pcName = pcFunction->mpcDeclarator->GetName();
	if (!psClass && pcName && (pcName->macpComponents.NumElements() >= 2))
	{
		psClass = GetComponentClass((CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(0), NULL, pcFunction);
		if (!psClass)
		{
			return Error(pcFunction, "The function's class has not been defined");
		}

		//CListBlock::CListBlock::Add names the class again (its injected name).
		for (i = 1; i + 1 < pcName->macpComponents.NumElements(); i++)
		{
			if (GetComponentName((CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(i)) != psClass->lliName)
			{
				return Error(pcFunction, "Namespaces and nested classes are not supported by the compiler yet");
			}
		}
		if (!CompleteClass(psClass, pcFunction))
		{
			return false;
		}
	}

	if (pcFunction->meBody == CPPFB_Skipped)
	{
		//A body skipped in a system header: only a declaration.
		BeginQuiet();
		DeclareFunction(pcFunction->mpcSpecifiers, pcFunction->mpcDeclarator, false, psClass, false);
		EndQuiet();
		return true;
	}
	psFunction = DeclareFunction(pcFunction->mpcSpecifiers, pcFunction->mpcDeclarator, true, psClass, false);
	if (!psFunction)
	{
		return false;
	}
	if (psFunction->pcDefinition || psFunction->bGenerated)
	{
		return Error(pcFunction, "The function is defined twice");
	}
	SetDefinition(psFunction, pcFunction);
	if (!psFunction->pcDefinition && !psFunction->bImplicit)
	{
		//A body skipped in a system header.
		return true;
	}
	psFunction->bInline = pcFunction->mpcSpecifiers && (pcFunction->mpcSpecifiers->muiFlags & (CPPDS_INLINE | CPPDS_CONSTEXPR));
	if (psFunction->bInline)
	{
		return true;
	}

	uiErrors = mcErrors.NumErrors();
	GenerateFunction(psFunction);
	return mcErrors.NumErrors() == uiErrors;
}


//////////////////////////////////////////////////////////////////////////
//
// Generates a function from its definition, or a constructor or destructor the
// generator writes, with the template arguments it was made with.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::GenerateFunction(SCPP65Function* psFunction)
{
	SCPP65Bindings*	psSaved;
	CChars			szLabel;
	bool			bShared;

	if (psFunction->bGenerated)
	{
		return;
	}
	psFunction->bGenerated = true;
	psFunction->bQueued = false;
	if (mpcSemanticPass)
	{
		//Only checked.
		psSaved = mpsBindings;
		mpsBindings = psFunction->psBindings;
		CheckFunction(psFunction);
		mpsBindings = psSaved;
		return;
	}

	//Each function is a section of its own.  Inline, template and generated functions are written by every unit
	//that needs them, each in a COMDAT section the linker keeps one of.
	szLabel.Init();
	AppendFunctionLabel(&szLabel, psFunction);
	bShared = !IsUnitFunction(psFunction) && (psFunction->bInline || psFunction->bImplicit || psFunction->psTemplate);
	if (bShared && !Emit(&szLabel))
	{
		szLabel.Kill();
		return;
	}
	AppendSection(&mszCode, "text", &szLabel, bShared ? ", comdat" : NULL);
	if (!IsUnitFunction(psFunction))
	{
		AppendGlobal(&mszCode, &szLabel);
	}
	szLabel.Kill();
	psSaved = mpsBindings;
	mpsBindings = psFunction->psBindings;

	if (psFunction->bImplicit)
	{
		//An implicit default constructor, an inherited one or a destructor.
		BeginFunction(psFunction);
		if (((psFunction->mapsParameters.NumElements() != 0) && !psFunction->psInherited) || !mpcIRFrontEnd->Body(psFunction))
		{
			szLabel.Init("An implicit copy constructor or assignment (or one the IR front end can't write) is not supported on this target yet: ");
			AppendFunctionLabel(&szLabel, psFunction);
			if (psFunction->mapsParameters.NumElements() == 0)
			{
				AppendUnsupported(&szLabel);
			}
			ErrorAt(-1, szLabel.Text());
			szLabel.Kill();
			EndFunction(psFunction);
		}
	}
	else
	{
		GenerateDefinition(psFunction);
	}
	mpsBindings = psSaved;
}


//////////////////////////////////////////////////////////////////////////
//
// The code for a function's definition: its parameters and locals get their places,
// a constructor makes its bases and fields first, a destructor sets its vtable
// pointer first (EndFunction destroys its fields and bases), then the body.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::GenerateDefinition(SCPP65Function* psFunction)
{
	CCPPSyntaxFunctionDefinition*	pcFunction;
	size							uiErrors;
	CChars							szMessage;

	pcFunction = psFunction->pcDefinition;
	if ((pcFunction->macpMemberInitializers.NumElements() != 0) && (psFunction->eKind != CPP65FK_Constructor))
	{
		return Error(pcFunction, "Only constructors have member initialisers");
	}

	BeginFunction(psFunction);
	AddParameters(psFunction);
	if (!AllocateLocals(pcFunction->mpcBody))
	{
		EndFunction(psFunction);
		return false;
	}

	//The IR front end writes the whole function.  An error it reported (ReportTypeOf: C++ that is wrong) is the reason
	//when it can't; else it is C++ the front end doesn't do yet.
	uiErrors = mcErrors.NumErrors();
	if (mpcIRFrontEnd->Body(psFunction))
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
		szMessage.Append(mpsTarget->szName);
		szMessage.Append(" compiler doesn't support yet");
		AppendUnsupported(&szMessage);
		Error(pcFunction, szMessage.Text());
		szMessage.Kill();
	}
	EndFunction(psFunction);
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
void CCPP65Generator::AddParameters(SCPP65Function* psFunction)
{
	CCPPSyntaxFunctionSuffix*		pcSuffix;
	CCPPSyntaxParameter*			pcParameter;
	SCPP65Type*						psType;
	SCPP65Variable*					psVariable;
	SCPP65Binding*					psPack;
	CCPPSymbol*						pcSymbol;
	size							i;
	size							j;
	size							k;
	size							uiPack;

	pcSuffix = psFunction->pcDefinition->mpcDeclarator->GetFunctionSuffix();
	j = 0;
	for (i = 0; i < pcSuffix->macpParameters.NumElements(); i++)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		psPack = PackBinding(pcParameter);
		uiPack = psPack ? psPack->papsPack->NumElements() : ((j < psFunction->mapsParameters.NumElements()) ? 1 : 0);
		if (psPack)
		{
			mpcPackParameter = pcParameter->mpcSymbol;
		}
		for (k = 0; k < uiPack; k++)
		{
			psType = psFunction->mapsParameters.GetPtr(j);
			pcSymbol = (psPack && pcParameter->mpcSymbol) ? NewPackElement(pcParameter->mpcSymbol) : pcParameter->mpcSymbol;
			if (pcSymbol)
			{
				psVariable = AddVariable(&mapsLocals, pcSymbol, ParameterVariableType(psType, pcParameter), CPP65S_Parameter, (int)j);
				psVariable->bRestrict = !psPack && pcParameter->mpcDeclarator && IsRestrictPointer(pcParameter->mpcDeclarator);
				if (psType->IsClass())
				{
					//Laid out now, as the IR front end needs it.
					CompleteClass(psType->psClass, pcParameter);
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
void CCPP65Generator::BeginFunction(SCPP65Function* psFunction)
{
	mpsFunction = psFunction;
	FreeVariables(&mapsLocals);

	mbThis = psFunction->psClass && !psFunction->bStatic;
	mpcPackParameter = NULL;
	mapcPackElements.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// Leaves the function BeginFunction started, as the IR front end leaves a
// function it writes.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::EndFunction(SCPP65Function* psFunction)
{
	//Only a function the IR front end couldn't write ends here (it ends the ones it writes itself): nothing is written.
	mpsFunction = NULL;
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
			if ((psType->IsArray() ? psType->psBase : psType)->IsClass() && !CompleteClass((psType->IsArray() ? psType->psBase : psType)->psClass, pcNode))
			{
				return false;
			}

			AddVariable(&mapsLocals, pcInit->mpcSymbol, psType, CPP65S_Local, 0);
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
	SCPP65Class*	psClass;
	SCPP65Field*	psField;
	SCPP65Variable*	psVariable;
	size			i;

	psClass = GetUnnamedClass(pcSpecifier);
	if (!psClass || !CompleteClass(psClass, pcSpecifier))
	{
		return Error(pcSpecifier, "This anonymous class is not supported by the compiler yet");
	}

	for (i = 0; i < psClass->macFields.NumElements(); i++)
	{
		psField = psClass->macFields.Get(i);
		if (!psField->pcSymbol)
		{
			return Error(pcSpecifier, "Anonymous classes in anonymous local classes are not supported by the compiler yet");
		}
		psVariable = AddVariable(&mapsLocals, psField->pcSymbol, psField->psType, CPP65S_Local, psField->iOffset);
		psVariable->psAnonymous = psClass;
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
	SCPP65Variable*		psVariable;
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

	psVariable = AddVariable(&mapsLocals, pcInit->mpcSymbol, psType, CPP65S_Static, miStatics);
	miStatics++;
	psVariable->bDefined = true;
	szLabel.Init();
	AppendVariableLabel(&szLabel, psVariable);
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
void CCPP65Generator::AppendVariableLabel(CChars* psz, SCPP65Variable* psVariable)
{
	char*	szName;
	int		iLength;

	if (psVariable->eStorage == CPP65S_Static)
	{
		szName = mpcTokeniser->GetIdentifier(psVariable->pcSymbol->mlliName, &iLength);
		psz->Append('S');
		psz->Append(psVariable->iOffset);
		psz->Append('_');
		psz->Append(szName, iLength);
		return;
	}
	psVariable->bUsed = true;
	AppendGlobalLabel(psz, psVariable->psClass, psVariable->pcSymbol->mlliName);
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
// Signed divide: A / B.  The quotient is in A and the remainder in B.  The quotient
// is negative when the signs differ, the remainder has the dividend's sign.  The
// 64 bit routine is called with A and B 64 bits wide and leaves them so; it widens
// X and Y while it works.  Changes A, B, X and Y.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendDivideSigned(const char* szName, int iBytes)
{
	if (iBytes == 8)
	{
		Line("wtth (X, Y), 64")->AppendNewLine();
	}
	Line("cpy X, A")->AppendNewLine();
	Line("xor X, B")->AppendNewLine();
	Line("psh X")->AppendNewLine();
	Line("psh A")->AppendNewLine();
	Line("abs Y, A")->AppendNewLine();
	Line("abs A, B")->AppendNewLine();
	Line("cpy B, 0")->AppendNewLine();
	Line("div")->AppendNewLine();
	Line("cpy W, 0")->AppendNewLine();
	Line("pul X")->AppendNewLine();
	Line("cmp X, 0")->AppendNewLine();
	Line("brac n, ")->Append(szName)->Append('1')->AppendNewLine();
	Line("neg B, B")->AppendNewLine();
	mszCode.Append(szName);
	mszCode.Append("1:");
	mszCode.AppendNewLine();
	Line("pul X")->AppendNewLine();
	Line("cmp X, 0")->AppendNewLine();
	Line("brac n, ")->Append(szName)->Append('2')->AppendNewLine();
	Line("neg Y, Y")->AppendNewLine();
	mszCode.Append(szName);
	mszCode.Append("2:");
	mszCode.AppendNewLine();
	Line("cpy A, Y")->AppendNewLine();
	if (iBytes == 8)
	{
		Line("wtth (X, Y), 32")->AppendNewLine();
	}
	Line("ret")->AppendNewLine();
	mszCode.AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// Runtime routines the generated code calls.  Only the ones used are added.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendRuntime(void)
{
	CChars	szLabel;
	bool	bPure;
	bool	bDivide;
	bool	bDivide64;

	if (mpsTarget->eTarget == CPPT_W65C816)
	{
		AppendRuntime816();
		return;
	}
	szLabel.Init("R_Pure");
	bPure = mbPureVirtual && Emit(&szLabel);
	szLabel.Kill();
	szLabel.Init("R_DivideSigned");
	bDivide = maszRuntime.Contains("R_DivideSigned") && Emit(&szLabel);
	szLabel.Kill();
	if (bPure)
	{
		//A pure virtual function was called.
		BeginRuntime("R_Pure");
		Line("stp")->AppendNewLine();
		mszCode.AppendNewLine();
	}
	if (bDivide)
	{
		BeginRuntime("R_DivideSigned");
		AppendDivideSigned("R_DivideSigned", 4);
	}
	szLabel.Init("R_DivideSigned64");
	bDivide64 = maszRuntime.Contains("R_DivideSigned64") && Emit(&szLabel);
	szLabel.Kill();
	if (bDivide64)
	{
		BeginRuntime("R_DivideSigned64");
		AppendDivideSigned("R_DivideSigned64", 8);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The W65C816's runtime: R_Pure, and the tables the W65C816 back end's code reads (R816_STable7 ...), each a
// COMDAT section so only one copy is linked.  The R816_ routines it calls are Libc65's (CCPP65Builder::AddRuntime816).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendRuntime816(void)
{
	CChars		szLabel;
	CArrayChars	aszRoutines;
	CCPPErrors	cErrors;
	size		i;

	szLabel.Init("R_Pure");
	if (mbPureVirtual && Emit(&szLabel))
	{
		BeginRuntime("R_Pure");
		Line("stp")->AppendNewLine();
		mszCode.AppendNewLine();
	}
	szLabel.Kill();

	aszRoutines.Init();
	for (i = 0; i < maszRuntime.NumElements(); i++)
	{
		if (strncmp(maszRuntime.GetText(i), "R816_", 5) == 0)
		{
			aszRoutines.Add(maszRuntime.GetText(i));
		}
	}
	if (aszRoutines.NumElements() != 0)
	{
		cErrors.Init();
		if (!CIR816Backend::GenerateRuntime(&aszRoutines, &mszCode, &cErrors))
		{
			ErrorAt(-1, cErrors.Text());
		}
		cErrors.Kill();
	}
	aszRoutines.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A runtime routine's COMDAT section and label (any unit may write it).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::BeginRuntime(const char* szName)
{
	CChars	szLabel;

	//Resident: it is called with A and B wider than a function's 32 bits, which the lazy loader can't keep.
	szLabel.Init(szName);
	AppendSection(&mszCode, "text", &szLabel, ", comdat, resident");
	AppendGlobal(&mszCode, &szLabel);
	szLabel.Kill();
	mszCode.Append(szName);
	mszCode.Append(':');
	mszCode.AppendNewLine();
}
