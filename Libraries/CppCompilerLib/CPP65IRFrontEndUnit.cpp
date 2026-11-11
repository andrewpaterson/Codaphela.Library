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
#include <stdio.h>
#include "SourceFile.h"
#include "CPP65Generator.h"
#include "IRInliner.h"
#include "IRInterprocedural.h"
#include "IROptimiser.h"
#include "IR816Backend.h"
#include "CPP65IRFrontEnd.h"
#include "CPPNameCharacters.h"
#include "FNVHash.h"


//The IR front end's work over a whole unit: inlining and the passes before it (InlineUnit, CIRInterprocedural), writing
//each function's code in the generator's (AddSpecialised, RemoveSpecialised, FinishUnit), and the inlining profile.


//////////////////////////////////////////////////////////////////////////
//
// After the generator has written every function of the unit: small
// functions are inlined into the functions written as IR (callees, which
// are written after their callers, first), which are optimised and
// generated again, and each function's code replaces its marker in the
// generator's code.  A function that then fails keeps the code Body
// generated (and isn't inlined into others).  Functions of other units are
// inlined too when the generator has imports (CCPP65Generator::SetImports),
// and the unit's small functions are added to them at the end.
//
// The functions are inlined (InlineUnit) before the unit's turn with the
// imports, with those exported so far.  In its turn (FinishUnit) it checks
// none it didn't find has been exported since: if one has, it returns
// false, keeping its turn, and the unit must be generated again
// (CCPP65Generator::MissedImports).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::InlineUnit(void)
{
	CIRInliner			cInliner;
	CIRInterprocedural	cInterprocedural;
	CChars				szCode;
	SCPP65IRWritten*	psWritten;
	size				ui;
	int					iRound;
	bool				bResult;

	//The inliner's and optimiser's instructions have no source of their own.
	mcModule.muiSource = 0;
	if (mpcGenerator->mpsTarget->eTarget == CPPT_W65C816)
	{
		cInliner.Init(&mcModule, IR816_INLINE_INSTRUCTIONS, IR816_INLINE_LOOP_INSTRUCTIONS, IR816_INLINE_CALLS);
		cInliner.SetForwardersJump(true);
	}
	else
	{
		cInliner.Init(&mcModule, IR65_INLINE_INSTRUCTIONS, IR65_INLINE_LOOP_INSTRUCTIONS, IR65_INLINE_CALLS);
	}
	//Other units' functions are found before the unit's turn, and checked in it (CIRImports::Missed).
	cInliner.SetImports(mpcGenerator->mpcImports, &maszMissing, &maszNotImported);
	mpcProfiledCaller = NULL;
	if (InliningProfile())
	{
		cInliner.SetProfile(this, (mpcGenerator->mpsTarget->eTarget == CPPT_W65C816) ? IR816_INLINE_HOT_INSTRUCTIONS : IR65_INLINE_HOT_INSTRUCTIONS);
	}
	cInterprocedural.Init(&mcModule, this);
	if (sbInline && sbOptimise)
	{
		cInterprocedural.SpecialiseConstantArguments();
		cInterprocedural.RemoveDeadArguments();
		cInterprocedural.MarkNoWrites();
	}
	for (ui = masWritten.NumElements(); sbInline && (ui > 0); ui--)
	{
		psWritten = masWritten.Get(ui - 1);
		if (!psWritten->pcFunction)
		{
			continue;
		}
		if (cInliner.InlineCalls(psWritten->pcFunction))
		{
			//Optimising can make an indirect call direct (a function pointer argument, a vtable entry), which is then
			//inlined too.
			for (iRound = 0; iRound < IR65_INLINE_ROUNDS; iRound++)
			{
				cInterprocedural.Optimise(psWritten->pcFunction);
				if ((iRound + 1 == IR65_INLINE_ROUNDS) || !cInliner.InlineCalls(psWritten->pcFunction))
				{
					break;
				}
			}
		}
		else if (cInterprocedural.CallsNoWrites(ui - 1))
		{
			//What is in memory is still known after its calls to functions that write nothing (MarkNoWrites).
			cInterprocedural.Optimise(psWritten->pcFunction);
		}
		else if (!cInterprocedural.IsChanged(ui - 1))
		{
			continue;
		}
		if (sbPrint)
		{
			fprintf(stderr, "Inlined\n");
			PrintIR(psWritten->pcFunction);
		}
		//IR that stops verifying once inlined keeps the code written before (a pass's mistake, recorded for finding it).
		bResult = VerifyIR(psWritten->pcFunction, __FILE__, __LINE__);
		szCode.Init();
		if (bResult)
		{
			bResult = GenerateCode(psWritten->pcFunction, psWritten->szLabel.Text(), &szCode, &psWritten->sFrame);
		}
		if (bResult)
		{
			CCPP65Assembly::PutCode(mpcGenerator->mpcSpillStore, mpcGenerator->muiSpillUnit, ui - 1, szCode.Text(), szCode.Length(), &psWritten->uiPieces);
		}
		else
		{
			mcModule.RemoveFunction(psWritten->pcFunction);
			psWritten->pcFunction = NULL;
		}
		szCode.Kill();
	}
	cInterprocedural.Kill();
	cInliner.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// After InlineUnit, in the unit's turn (it waits for it, if the builder
// hasn't, CIRImports::Arrive): the unit's small functions are exported,
// and each function's code is finished.  False, with the turn kept, if the
// unit missed a function exported since it inlined.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::FinishUnit(void)
{
	CChars				szCode;
	SCPP65IRWritten*	psWritten;
	size				ui;
	CIRSourceMarkers	cMarkers;

	//In its turn the unit has found what it would have, as if the units were compiled one after another, unless a
	//function it didn't find was exported since.
	if (mpcGenerator->mpcImports)
	{
		mpcGenerator->mpcImports->BeginTurn(mpcGenerator->muiImportsTurn);
		if (mpcGenerator->mpcImports->Missed(&maszNotImported))
		{
			return false;
		}
	}

	//The unit's small functions, as finished, for the units compiled after it.
	for (ui = 0; mpcGenerator->mpcImports && (ui < masWritten.NumElements()); ui++)
	{
		psWritten = masWritten.Get(ui);
		if (psWritten->pcFunction)
		{
			mpcGenerator->mpcImports->Export(&mcModule, psWritten->pcFunction);
		}
	}
	if (mpcGenerator->mpcImports)
	{
		mpcGenerator->mpcImports->EndTurn(mpcGenerator->muiImportsTurn);
	}

	//Each function's code is finished: on the W65C816 frames worth it are put on a page, now that the unit's calls
	//are known (CIR816Backend::AlignFrame; a listing's source markers are taken out while it looks at the code), then
	//the source markers become comments or .line directives.  The generator's code keeps the markers (the code stays
	//in the spill store until the unit is assembled, CCPP65Assembly).
	cMarkers.Init();
	mpcGenerator->mauiCodePieces.Kill();
	mpcGenerator->mauiCodePieces.Init();
	for (ui = 0; ui < masWritten.NumElements(); ui++)
	{
		psWritten = masWritten.Get(ui);
		szCode.Init();
		CCPP65Assembly::GetCode(mpcGenerator->mpcSpillStore, mpcGenerator->muiSpillUnit, ui, psWritten->uiPieces, &szCode);
		if (mpcGenerator->mpsTarget->eTarget == CPPT_W65C816)
		{
			cMarkers.Take(&szCode, 0);
			CIR816Backend::AlignFrame(&szCode, psWritten->szLabel.Text(), &psWritten->sFrame, !psWritten->pcFunction || CIRInterprocedural::IsRecursive(&mcModule, psWritten->pcFunction), mpcGenerator->mpcFrameProfile);
			cMarkers.Put(&szCode, 0);
		}
		SourceComments(&szCode, mpcGenerator->mbListing, mpcGenerator->mbLineTable);
		if (mpcGenerator->mbLineTable)
		{
			szCode.Append("\t\t.fingerprint \"");
			szCode.Append(psWritten->szLabel);
			szCode.Append("\", ");
			szCode.Append((int64)psWritten->uiFingerprint);
			szCode.Append(", ");
			szCode.Append(mpcGenerator->mlliCompileTime);
			szCode.AppendNewLine();
		}
		if (sbPrint)
		{
			fprintf(stderr, "%s\n", szCode.Text());
		}
		CCPP65Assembly::PutCode(mpcGenerator->mpcSpillStore, mpcGenerator->muiSpillUnit, ui, szCode.Text(), szCode.Length(), &psWritten->uiPieces);
		mpcGenerator->mauiCodePieces.Add(&psWritten->uiPieces);
		szCode.Kill();
	}
	cMarkers.Kill();
	mbCodeGiven = true;

	for (ui = 0; ui < masWritten.NumElements(); ui++)
	{
		masWritten.Get(ui)->szLabel.Kill();
	}
	masWritten.ReInit();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A function added by CIRInterprocedural (a copy of the function uiFunction
// for a constant): written after the function (its marker straight after the
// function's), in a section of its own.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::AddSpecialised(size uiFunction, CIRFunction* pcCopy)
{
	SCPP65IRWritten*	psWritten;
	CChars				szName;
	CChars				szMarker;
	size				uiMarker;
	size				uiMarkerLength;

	szName.Init(mcModule.GetSymbolName(pcCopy->muiSymbol));
	psWritten = masWritten.Add();
	psWritten->pcFunction = pcCopy;
	psWritten->szLabel.Init(szName.Text());
	psWritten->uiPieces = 0;
	memset(&psWritten->sFrame, 0, sizeof(psWritten->sFrame));
	psWritten->uiFingerprint = 0;

	szMarker.Init(IR65_FRONT_END_MARKER);
	szMarker.Append((int)uiFunction);
	szMarker.Append(IR65_FRONT_END_MARKER);
	uiMarker = mpcGenerator->mszCode.Find(0, szMarker.Text());
	uiMarkerLength = szMarker.Length();
	szMarker.Kill();
	szMarker.Init();
	mpcGenerator->AppendSection(&szMarker, "text", &szName, NULL);
	szMarker.Append(IR65_FRONT_END_MARKER);
	szMarker.Append((int)(masWritten.NumElements() - 1));
	szMarker.Append(IR65_FRONT_END_MARKER);
	if (uiMarker != ARRAY_ELEMENT_NOT_FOUND)
	{
		mpcGenerator->mszCode.Insert(uiMarker + uiMarkerLength, szMarker.Text());
	}
	else
	{
		mpcGenerator->mszCode.Append(szMarker);
	}
	szMarker.Kill();
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether szLabel is named in szText (not as the start of a longer name).
//
//////////////////////////////////////////////////////////////////////////
static bool IRLabelNamed(const char* szText, const char* szLabel)
{
	const char*	szAt;
	size		uiLength;
	char		c;

	uiLength = strlen(szLabel);
	for (szAt = strstr(szText, szLabel); szAt; szAt = strstr(szAt + 1, szLabel))
	{
		c = szAt[uiLength];
		if (!IsCPPNameCharacter(c) && (c != '.'))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Nothing in the IR calls the function now: it goes, with its section line,
// unless the generator's code names it otherwise (a table of functions).
// True if it went.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::RemoveSpecialised(size uiFunction)
{
	SCPP65IRWritten*	psWritten;
	CChars				szSection;
	size				uiMarker;
	size				uiSectionLength;
	bool				bNamed;

	psWritten = masWritten.Get(uiFunction);
	szSection.Init();
	mpcGenerator->AppendSection(&szSection, "text", &psWritten->szLabel, NULL);
	uiSectionLength = szSection.Length();
	szSection.Append(IR65_FRONT_END_MARKER);
	szSection.Append((int)uiFunction);
	szSection.Append(IR65_FRONT_END_MARKER);
	uiMarker = mpcGenerator->mszCode.Find(0, szSection.Text());
	if (uiMarker != ARRAY_ELEMENT_NOT_FOUND)
	{
		mpcGenerator->mszCode.Remove(uiMarker, uiMarker + uiSectionLength);
	}
	bNamed = IRLabelNamed(mpcGenerator->mszCode.Text(), psWritten->szLabel.Text());
	if (bNamed)
	{
		if (uiMarker != ARRAY_ELEMENT_NOT_FOUND)
		{
			szSection.SetLength(uiSectionLength);
			mpcGenerator->mszCode.Insert(uiMarker, szSection.Text());
		}
	}
	else
	{
		mcModule.RemoveFunction(psWritten->pcFunction);
		psWritten->pcFunction = NULL;
		CCPP65Assembly::PutCode(mpcGenerator->mpcSpillStore, mpcGenerator->muiSpillUnit, uiFunction, "", 0, &psWritten->uiPieces);
	}
	szSection.Kill();
	return !bNamed;
}


//////////////////////////////////////////////////////////////////////////
//
// The unit's functions, for CIRInterprocedural: those written, in order.
//
//////////////////////////////////////////////////////////////////////////
size CCPP65IRFrontEnd::NumUnitFunctions(void)
{
	return masWritten.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIRFunction* CCPP65IRFrontEnd::GetUnitFunction(size uiFunction)
{
	return masWritten.Get(uiFunction)->pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// A hash (32 bit FNV-1a) of the function's IR as printed: the same code
// gives the same hash in every build.
//
//////////////////////////////////////////////////////////////////////////
uint32 CCPP65IRFrontEnd::Fingerprint(CIRFunction* pcFunction)
{
	CChars			szText;
	uint32			uiHash;

	szText.Init();
	PrintFunction(pcFunction, &szText, false);
	uiHash = FNVHash32(szText.Text(), szText.Length());
	szText.Kill();
	return uiHash;
}


//////////////////////////////////////////////////////////////////////////
//
// The profile the inliner is guided by: the generator's, on the W65C816
// (CCPP65Builder::SetFrameProfile).  NULL for none.
//
//////////////////////////////////////////////////////////////////////////
CAsm65816Profile* CCPP65IRFrontEnd::InliningProfile(void)
{
	return (mpcGenerator->mpsTarget->eTarget == CPPT_W65C816) ? mpcGenerator->mpcFrameProfile : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The cycles of the profiled run (CIRInlineProfile).
//
//////////////////////////////////////////////////////////////////////////
uint64 CCPP65IRFrontEnd::ProfileCycles(void)
{
	return InliningProfile() ? InliningProfile()->NumCycles() : 0;
}


//////////////////////////////////////////////////////////////////////////
//
// How often the profiled run made the call (CIRInlineProfile): how often
// its source line's code was entered in the caller, when the profile has
// the caller as it is now (the same fingerprint), or else (the caller
// wasn't run, or the call is in code inlined from another function) in
// every function.  IR_INLINE_UNKNOWN when the caller's profile is of other
// code or the call has no source line.
//
//////////////////////////////////////////////////////////////////////////
uint64 CCPP65IRFrontEnd::CallCount(CIRFunction* pcCaller, SIRInstruction* psCall)
{
	CAsm65816Profile*			pcProfile;
	SAsm65816FunctionProfile*	psFunction;
	SAsm65816LineProfile*		psLine;
	SCPP65IRWritten*		psWritten;
	CSourceFile*			pcFile;
	const char*				szLabel;
	int						iLine;
	size					ui;
	uint64					ulliCount;
	uint64					ulliEntered;

	pcProfile = InliningProfile();
	if (!pcProfile || (psCall->uiSource == 0) || !SourceLine((int)psCall->uiSource - 1, &pcFile, &iLine))
	{
		return IR_INLINE_UNKNOWN;
	}
	szLabel = mcModule.GetSymbolName(pcCaller->muiSymbol);
	psFunction = pcProfile->Get(szLabel);
	if (pcCaller != mpcProfiledCaller)
	{
		//The caller's fingerprint (when it was written) against the profile's.
		mpcProfiledCaller = pcCaller;
		mbProfiledCaller = false;
		for (ui = 0; psFunction && (ui < masWritten.NumElements()); ui++)
		{
			psWritten = masWritten.Get(ui);
			if (psWritten->pcFunction == pcCaller)
			{
				mbProfiledCaller = (psFunction->uiHash == 0) || (psFunction->uiHash == psWritten->uiFingerprint);
				break;
			}
		}
	}
	if (psFunction && !mbProfiledCaller)
	{
		return IR_INLINE_UNKNOWN;
	}
	psLine = psFunction ? pcProfile->GetLine(szLabel, pcFile->FullName(), (uint32)iLine + 1) : NULL;
	if (psLine)
	{
		ulliCount = psLine->ulliEntries;
	}
	else
	{
		ulliCount = pcProfile->LineEntries(pcFile->FullName(), (uint32)iLine + 1);
	}
	if (psFunction && (ulliCount == 0))
	{
		//A call the profiled build inlined left its line no code: its callee's lines ran in the caller instead.  (A
		//call that stayed a call has its own count, even when another call of the callee there was inlined.)
		ulliEntered = InlinedEntries(pcProfile, szLabel, psCall);
		ulliCount = (ulliEntered > ulliCount) ? ulliEntered : ulliCount;
	}
	return ulliCount;
}


//////////////////////////////////////////////////////////////////////////
//
// How often the code of psCall's callee (a function of this unit) ran in
// the function szLabel, as the most entries of any of its entry block's
// source lines there: how often the profiled build entered a copy of it
// that it inlined there (the entry block's first line may be another
// function's, inlined into the callee).  0 if not known.
//
//////////////////////////////////////////////////////////////////////////
uint64 CCPP65IRFrontEnd::InlinedEntries(CAsm65816Profile* pcProfile, const char* szLabel, SIRInstruction* psCall)
{
	CIRFunction*			pcCallee;
	CIRBlock*				pcEntry;
	SAsm65816LineProfile*		psLine;
	CSourceFile*			pcFile;
	uint32					uiSource;
	uint64					ulliMost;
	int						iLine;
	size					ui;

	if ((psCall->uiOpcode != IRO_Call) || !psCall->asArguments[0].IsSymbol())
	{
		return 0;
	}
	pcCallee = mcModule.FindFunction(psCall->asArguments[0].Index());
	if (!pcCallee || (pcCallee->NumBlocks() == 0))
	{
		return 0;
	}
	pcEntry = pcCallee->GetBlock(0);
	ulliMost = 0;
	for (ui = 0; ui <= pcEntry->masInstructions.NumElements(); ui++)
	{
		uiSource = (ui < pcEntry->masInstructions.NumElements()) ? pcEntry->masInstructions.Get(ui)->uiSource : pcEntry->msJump.uiSource;
		if ((uiSource != 0) && SourceLine((int)uiSource - 1, &pcFile, &iLine))
		{
			psLine = pcProfile->GetLine(szLabel, pcFile->FullName(), (uint32)iLine + 1);
			if (psLine && (psLine->ulliEntries > ulliMost))
			{
				ulliMost = psLine->ulliEntries;
			}
		}
	}
	return ulliMost;
}
