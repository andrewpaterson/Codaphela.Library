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
#include <stdio.h>
#include "IRParser.h"
#include "SourceFile.h"
#include "CPP65Generator.h"
#include "IRInliner.h"
#include "IRPrinter.h"
#include "IROptimiser.h"
#include "IR816Backend.h"
#include "CPP65IRFrontEnd.h"


//The IR front end's work over a whole unit: inlining (InlineUnit), recursion, what functions don't write,
//specialising callees for constant and strong arguments, dead arguments, and the inlining profile.


//////////////////////////////////////////////////////////////////////////
//
// Whether pcFunction may call itself: through the calls of the unit's
// functions (calls of functions in other units are taken not to).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::IsRecursive(CIRFunction* pcFunction)
{
	CArrayTemplate<CIRFunction*>	apcToDo;
	CArrayTemplate<CIRFunction*>	apcSeen;
	CIRFunction*					pcCaller;
	CIRFunction*					pcCallee;
	CIRBlock*						pcBlock;
	SIRInstruction*					psInstruction;
	size							ui;
	size							uiBlock;
	size							uiInstruction;
	bool							bSeen;
	bool							bRecursive;

	apcToDo.Init();
	apcSeen.Init();
	apcToDo.Add(&pcFunction);
	bRecursive = false;
	while ((apcToDo.NumElements() > 0) && !bRecursive)
	{
		pcCaller = *apcToDo.Tail();
		apcToDo.RemoveTail();
		for (uiBlock = 0; (uiBlock < pcCaller->NumBlocks()) && !bRecursive; uiBlock++)
		{
			pcBlock = pcCaller->GetBlock(uiBlock);
			for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
			{
				psInstruction = pcBlock->masInstructions.Get(uiInstruction);
				if ((psInstruction->uiOpcode != IRO_Call) || !psInstruction->asArguments[0].IsSymbol())
				{
					continue;
				}
				pcCallee = mcModule.FindFunction(psInstruction->asArguments[0].Index());
				if (pcCallee == pcFunction)
				{
					bRecursive = true;
					break;
				}
				bSeen = (pcCallee == NULL);
				for (ui = 0; (ui < apcSeen.NumElements()) && !bSeen; ui++)
				{
					bSeen = *apcSeen.Get(ui) == pcCallee;
				}
				if (!bSeen)
				{
					apcSeen.Add(&pcCallee);
					apcToDo.Add(&pcCallee);
				}
			}
		}
	}
	apcSeen.Kill();
	apcToDo.Kill();
	return bRecursive;
}


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
	CIROptimiser		cOptimiser;
	CChars				szCode;
	SCPP65IRWritten*	psWritten;
	size				ui;
	int					iRound;
	bool				bResult;
	CArrayTemplate<bool>	abChanged;
	CArrayTemplate<bool>	abCallsNoWrites;
	bool				bFalse;

	bFalse = false;

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
	abChanged.Init();
	for (ui = 0; ui < masWritten.NumElements(); ui++)
	{
		abChanged.Add(&bFalse);
	}
	abCallsNoWrites.Init();
	if (sbInline && sbOptimise)
	{
		SpecialiseConstantArguments(&abChanged);
		RemoveDeadArguments(&abChanged);
		MarkNoWrites(&abCallsNoWrites);
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
				InitOptimiser(&cOptimiser);
				cOptimiser.Optimise(psWritten->pcFunction);
				cOptimiser.Kill();
				if ((iRound + 1 == IR65_INLINE_ROUNDS) || !cInliner.InlineCalls(psWritten->pcFunction))
				{
					break;
				}
			}
		}
		else if ((ui - 1 < abCallsNoWrites.NumElements()) && *abCallsNoWrites.Get(ui - 1))
		{
			//What is in memory is still known after its calls to functions that write nothing (MarkNoWrites).
			InitOptimiser(&cOptimiser);
			cOptimiser.Optimise(psWritten->pcFunction);
			cOptimiser.Kill();
		}
		else if (!*abChanged.Get(ui - 1))
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
	cInliner.Kill();
	abCallsNoWrites.Kill();
	abChanged.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Mod/ref summaries (InlineUnit, before inlining): which of the unit's
// functions write no memory their callers can see (CIROptimiser::
// WritesMemory), marked on their symbols (CIRModule::SetNoWrites) so the
// optimiser keeps what it knows is in memory across calls to them.  Each
// is taken to write nothing until it is seen to (a store outside its
// frame, a call to one that writes), again until none changes, so
// functions that call each other and store nothing are marked too.
// pabCalls (per written function) gets those that call one of them, to be
// optimised again.  Functions of other units are taken to write.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::MarkNoWrites(CArrayTemplate<bool>* pabCalls)
{
	CIROptimiser		cOptimiser;
	SCPP65IRWritten*	psWritten;
	CIRFunction*		pcFunction;
	CIRBlock*			pcBlock;
	SIRInstruction*		psInstruction;
	size				ui;
	size				uiBlock;
	size				uiInstruction;
	bool				bChanged;
	bool				bCalls;

	for (ui = 0; ui < masWritten.NumElements(); ui++)
	{
		psWritten = masWritten.Get(ui);
		if (psWritten->pcFunction)
		{
			mcModule.SetNoWrites(psWritten->pcFunction->muiSymbol, true);
		}
	}

	InitOptimiser(&cOptimiser);
	do
	{
		bChanged = false;
		for (ui = 0; ui < masWritten.NumElements(); ui++)
		{
			pcFunction = masWritten.Get(ui)->pcFunction;
			if (pcFunction && mcModule.IsNoWrites(pcFunction->muiSymbol) && cOptimiser.WritesMemory(pcFunction))
			{
				mcModule.SetNoWrites(pcFunction->muiSymbol, false);
				bChanged = true;
			}
		}
	}
	while (bChanged);
	cOptimiser.Kill();

	for (ui = 0; ui < masWritten.NumElements(); ui++)
	{
		pcFunction = masWritten.Get(ui)->pcFunction;
		bCalls = false;
		for (uiBlock = 0; pcFunction && (uiBlock < pcFunction->NumBlocks()) && !bCalls; uiBlock++)
		{
			pcBlock = pcFunction->GetBlock(uiBlock);
			for (uiInstruction = 0; (uiInstruction < pcBlock->masInstructions.NumElements()) && !bCalls; uiInstruction++)
			{
				psInstruction = pcBlock->masInstructions.Get(uiInstruction);
				bCalls = (psInstruction->uiOpcode == IRO_Call) && psInstruction->asArguments[0].IsSymbol() && mcModule.IsNoWrites(psInstruction->asArguments[0].Index());
			}
		}
		pabCalls->Add(&bCalls);
	}
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
			CIR816Backend::AlignFrame(&szCode, psWritten->szLabel.Text(), &psWritten->sFrame, !psWritten->pcFunction || IsRecursive(psWritten->pcFunction), mpcGenerator->mpcFrameProfile);
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
// Interprocedural constant propagation (FinishUnit, before inlining).  A
// function of the unit that only the unit calls (local, its address never
// taken) whose parameter is a constant at every call is specialised for
// it: with one value the parameter's uses are the constant; with up to
// IR65_SPECIALISE_VALUES values each gets a copy of the function (named
// with the parameter and the value) its calls go to, and the function goes
// when nothing else names it.  Only parameters worth it are: those that
// are (or something worked out from them is) a shift's count, a multiply's
// or a divide's operand, or an argument of a specialisable parameter of
// another (StrongParameters).  The copies' own calls then have constants
// (softfloat.cpp's Add(..., iFraction) calls RoundPack(..., iFraction)), so
// it goes round again, IR65_SPECIALISE_ROUNDS times at most.  pabChanged
// (per written function) gets the functions whose code must be written
// again.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::SpecialiseConstantArguments(CArrayTemplate<bool>* pabChanged)
{
	CArrayTemplate<uint32>	auiStrong;
	size					ui;
	int						iRound;
	bool					bChanged;
	bool					bAny;
	uint32					uiStrong;
	uint32					uiZero;

	uiZero = 0;
	auiStrong.Init();
	for (iRound = 0; iRound < IR65_SPECIALISE_ROUNDS; iRound++)
	{
		//Which parameters are worth it, to a fixed point (a parameter passed on is as good as the one it is passed to).
		auiStrong.FakeSetUsedElements(0);
		for (ui = 0; ui < masWritten.NumElements(); ui++)
		{
			auiStrong.Add(&uiZero);
		}
		do
		{
			bChanged = false;
			for (ui = 0; ui < masWritten.NumElements(); ui++)
			{
				if (masWritten.Get(ui)->pcFunction)
				{
					uiStrong = StrongParameters(masWritten.Get(ui)->pcFunction, &auiStrong);
					if (uiStrong != *auiStrong.Get(ui))
					{
						*auiStrong.Get(ui) = uiStrong;
						bChanged = true;
					}
				}
			}
		}
		while (bChanged);

		bAny = false;
		for (ui = 0; ui < auiStrong.NumElements(); ui++)
		{
			if (*auiStrong.Get(ui) != 0)
			{
				bAny |= SpecialiseCallee(ui, *auiStrong.Get(ui), pabChanged);
			}
		}
		if (!bAny)
		{
			break;
		}
	}
	auiStrong.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The written function whose symbol is uiSymbol, as its index in
// masWritten, or -1.
//
//////////////////////////////////////////////////////////////////////////
int CCPP65IRFrontEnd::WrittenIndex(uint32 uiSymbol)
{
	size	ui;

	for (ui = 0; ui < masWritten.NumElements(); ui++)
	{
		if (masWritten.Get(ui)->pcFunction && (masWritten.Get(ui)->pcFunction->muiSymbol == uiSymbol))
		{
			return (int)ui;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an instruction's result is worked out from its operands so
// simply that a constant operand makes it a constant the back end does
// better with (a count worked out from a parameter, say 61 - iFraction).
//
//////////////////////////////////////////////////////////////////////////
static bool IRSpecialiseFollows(uint16 uiOpcode)
{
	switch (uiOpcode)
	{
	case IRO_Add:
	case IRO_Sub:
	case IRO_Mul:
	case IRO_And:
	case IRO_Or:
	case IRO_Xor:
	case IRO_Not:
	case IRO_Neg:
	case IRO_Shl:
	case IRO_LShr:
	case IRO_AShr:
	case IRO_ZExt:
	case IRO_SExt:
	case IRO_Trunc:
	case IRO_Copy:
		return true;
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The parameters of pcFunction (a bit each, the first 32) worth
// specialising for a constant (SpecialiseConstantArguments): a value
// worked out from one (IRSpecialiseFollows) is a shift's count, a
// multiply's or divide's operand, or an argument for a parameter of a
// written function that is worth it (pauiStrong, per written function).
//
//////////////////////////////////////////////////////////////////////////
uint32 CCPP65IRFrontEnd::StrongParameters(CIRFunction* pcFunction, CArrayTemplate<uint32>* pauiStrong)
{
	CArrayTemplate<uint8>	abDerived;
	size					uiParameter;
	size					uiBlock;
	size					ui;
	size					uj;
	size					uiFirst;
	CIRBlock*				pcBlock;
	SIRInstruction*			psInstruction;
	SIRPhi*					psPhi;
	SIRReference			sValue;
	uint32					uiStrong;
	uint8					uiZero;
	bool					bChanged;
	bool					bStrong;
	int						iCallee;

	uiStrong = 0;
	uiZero = 0;
	abDerived.Init();
	for (uiParameter = 0; (uiParameter < pcFunction->masParameters.NumElements()) && (uiParameter < 32); uiParameter++)
	{
		abDerived.FakeSetUsedElements(0);
		for (ui = 0; ui < pcFunction->NumTemporaries(); ui++)
		{
			abDerived.Add(&uiZero);
		}
		sValue = *pcFunction->masParameters.Get(uiParameter);
		if (!sValue.IsTemporary())
		{
			continue;
		}
		*abDerived.Get(sValue.Index()) = 1;

		//What is worked out from it.
		do
		{
			bChanged = false;
			for (uiBlock = 0; uiBlock < pcFunction->NumBlocks(); uiBlock++)
			{
				pcBlock = pcFunction->GetBlock(uiBlock);
				for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
				{
					psPhi = pcBlock->masPhis.Get(ui);
					for (uj = 0; uj < psPhi->asArguments.NumElements(); uj++)
					{
						sValue = psPhi->asArguments.Get(uj)->sValue;
						if (sValue.IsTemporary() && *abDerived.Get(sValue.Index()) && psPhi->sResult.IsTemporary() && !*abDerived.Get(psPhi->sResult.Index()))
						{
							*abDerived.Get(psPhi->sResult.Index()) = 1;
							bChanged = true;
						}
					}
				}
				for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
				{
					psInstruction = pcBlock->masInstructions.Get(ui);
					if (!IRSpecialiseFollows(psInstruction->uiOpcode) || !psInstruction->sResult.IsTemporary() || *abDerived.Get(psInstruction->sResult.Index()))
					{
						continue;
					}
					for (uj = 0; uj < 2; uj++)
					{
						sValue = psInstruction->asArguments[uj];
						if (sValue.IsTemporary() && *abDerived.Get(sValue.Index()))
						{
							*abDerived.Get(psInstruction->sResult.Index()) = 1;
							bChanged = true;
							break;
						}
					}
				}
			}
		}
		while (bChanged);

		//Where it counts.
		bStrong = false;
		for (uiBlock = 0; (uiBlock < pcFunction->NumBlocks()) && !bStrong; uiBlock++)
		{
			pcBlock = pcFunction->GetBlock(uiBlock);
			for (ui = 0; (ui < pcBlock->masInstructions.NumElements()) && !bStrong; ui++)
			{
				psInstruction = pcBlock->masInstructions.Get(ui);
				switch (psInstruction->uiOpcode)
				{
				case IRO_Shl:
				case IRO_LShr:
				case IRO_AShr:
					sValue = psInstruction->asArguments[1];
					bStrong = sValue.IsTemporary() && *abDerived.Get(sValue.Index());
					break;
				case IRO_Mul:
				case IRO_UDiv:
				case IRO_SDiv:
				case IRO_URem:
				case IRO_SRem:
					for (uj = 0; uj < 2; uj++)
					{
						sValue = psInstruction->asArguments[uj];
						bStrong |= sValue.IsTemporary() && *abDerived.Get(sValue.Index());
					}
					break;
				case IRO_Call:
					if (!psInstruction->asArguments[0].IsSymbol())
					{
						break;
					}
					iCallee = WrittenIndex(psInstruction->asArguments[0].Index());
					if (iCallee < 0)
					{
						break;
					}
					for (uiFirst = ui; (uiFirst > 0) && (pcBlock->masInstructions.Get(uiFirst - 1)->uiOpcode == IRO_Arg); uiFirst--)
					{
					}
					for (uj = uiFirst; (uj < ui) && (uj - uiFirst < 32); uj++)
					{
						sValue = pcBlock->masInstructions.Get(uj)->asArguments[0];
						if (sValue.IsTemporary() && *abDerived.Get(sValue.Index()) && (*pauiStrong->Get((size)iCallee) & ((uint32)1 << (uj - uiFirst))))
						{
							bStrong = true;
						}
					}
					break;
				}
			}
		}
		if (bStrong)
		{
			uiStrong |= (uint32)1 << uiParameter;
		}
	}
	abDerived.Kill();
	return uiStrong;
}


//////////////////////////////////////////////////////////////////////////
//
// Every use of sOld (an operand, a phi's argument, a branch's or return's
// value) in pcFunction is sNew.
//
//////////////////////////////////////////////////////////////////////////
static void IRReplaceUses(CIRFunction* pcFunction, SIRReference sOld, SIRReference sNew)
{
	size			uiBlock;
	size			ui;
	size			uj;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;

	for (uiBlock = 0; uiBlock < pcFunction->NumBlocks(); uiBlock++)
	{
		pcBlock = pcFunction->GetBlock(uiBlock);
		for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
		{
			psPhi = pcBlock->masPhis.Get(ui);
			for (uj = 0; uj < psPhi->asArguments.NumElements(); uj++)
			{
				if (psPhi->asArguments.Get(uj)->sValue.Equals(sOld))
				{
					psPhi->asArguments.Get(uj)->sValue = sNew;
				}
			}
		}
		for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
		{
			psInstruction = pcBlock->masInstructions.Get(ui);
			for (uj = 0; uj < 2; uj++)
			{
				if (psInstruction->asArguments[uj].Equals(sOld))
				{
					psInstruction->asArguments[uj] = sNew;
				}
			}
		}
		if (pcBlock->msJump.sArgument.Equals(sOld))
		{
			pcBlock->msJump.sArgument = sNew;
		}
	}
}


//A call of a function being specialised: the written function it is in, its block and its index there.
struct SCPP65CallSite
{
	size		uiWritten;
	CIRBlock*	pcBlock;
	size		uiCall;
};


//////////////////////////////////////////////////////////////////////////
//
// The argument for the callee's parameter uiParameter of the call at
// psSite (the arg instructions straight before it), or none.
//
//////////////////////////////////////////////////////////////////////////
static SIRReference IRCallArgument(SCPP65CallSite* psSite, size uiParameter)
{
	size	uiFirst;

	for (uiFirst = psSite->uiCall; (uiFirst > 0) && (psSite->pcBlock->masInstructions.Get(uiFirst - 1)->uiOpcode == IRO_Arg); uiFirst--)
	{
	}
	if (uiFirst + uiParameter >= psSite->uiCall)
	{
		return IRNone();
	}
	return psSite->pcBlock->masInstructions.Get(uiFirst + uiParameter)->asArguments[0];
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
		if (!(((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) || ((c >= '0') && (c <= '9')) || (c == '_') || (c == '.')))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Specialises the written function uiCallee for the first of its strong
// parameters (uiStrong) that has a constant at every call
// (SpecialiseConstantArguments).  True if it did.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::SpecialiseCallee(size uiCallee, uint32 uiStrong, CArrayTemplate<bool>* pabChanged)
{
	CArrayTemplate<SCPP65CallSite>	asSites;
	CArrayTemplate<uint64>			aulliValues;
	SCPP65CallSite*					psSite;
	SCPP65CallSite					sSite;
	SCPP65IRWritten*				psWritten;
	CIRFunction*					pcCallee;
	CIRFunction*					pcFunction;
	CIRFunction*					pcCopy;
	CIRBlock*						pcBlock;
	SIRInstruction*					psInstruction;
	SIRReference					sArgument;
	SIRType							sType;
	CChars							szName;
	CChars							szMarker;
	CChars							szSection;
	uint64							ulliValue;
	uint32							uiSymbol;
	size							uiParameter;
	size							uiWritten;
	size							uiBlock;
	size							ui;
	size							uj;
	size							uk;
	size							uiMarker;
	size							uiMarkerLength;
	size							uiSectionLength;
	bool							bTaken;
	bool							bAllConstant;
	bool							bDone;
	bool							bTrue;

	bTrue = true;
	psWritten = masWritten.Get(uiCallee);
	pcCallee = psWritten->pcFunction;
	if (!pcCallee || (pcCallee->NumBlocks() == 0) || (pcCallee->muiAttributes & IRA_Variadic) ||
		!((pcCallee->muiAttributes & IRA_Local) || mcModule.IsLocal(pcCallee->muiSymbol)) ||
		(CIRInliner::NumInstructions(pcCallee) > IR65_SPECIALISE_INSTRUCTIONS))
	{
		return false;
	}

	//Its calls; it mustn't be named otherwise (its address taken).
	uiSymbol = pcCallee->muiSymbol;
	asSites.Init();
	bTaken = false;
	for (uiWritten = 0; (uiWritten < masWritten.NumElements()) && !bTaken; uiWritten++)
	{
		pcFunction = masWritten.Get(uiWritten)->pcFunction;
		for (uiBlock = 0; pcFunction && (uiBlock < pcFunction->NumBlocks()) && !bTaken; uiBlock++)
		{
			pcBlock = pcFunction->GetBlock(uiBlock);
			for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
			{
				psInstruction = pcBlock->masInstructions.Get(ui);
				for (uj = 0; uj < 2; uj++)
				{
					if (psInstruction->asArguments[uj].IsSymbol() && (psInstruction->asArguments[uj].Index() == uiSymbol))
					{
						if ((psInstruction->uiOpcode == IRO_Call) && (uj == 0))
						{
							sSite.uiWritten = uiWritten;
							sSite.pcBlock = pcBlock;
							sSite.uiCall = ui;
							asSites.Add(&sSite);
						}
						else
						{
							bTaken = true;
						}
					}
				}
			}
			for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
			{
				for (uj = 0; uj < pcBlock->masPhis.Get(ui)->asArguments.NumElements(); uj++)
				{
					sArgument = pcBlock->masPhis.Get(ui)->asArguments.Get(uj)->sValue;
					bTaken |= sArgument.IsSymbol() && (sArgument.Index() == uiSymbol);
				}
			}
			bTaken |= pcBlock->msJump.sArgument.IsSymbol() && (pcBlock->msJump.sArgument.Index() == uiSymbol);
		}
	}
	if (bTaken || (asSites.NumElements() == 0))
	{
		asSites.Kill();
		return false;
	}

	//The first strong parameter that is a constant at every call, with few values.
	aulliValues.Init();
	bDone = false;
	//bDone first: the specialised callee may have been removed (freed).
	for (uiParameter = 0; !bDone && (uiParameter < pcCallee->masParameters.NumElements()) && (uiParameter < 32); uiParameter++)
	{
		if (!(uiStrong & ((uint32)1 << uiParameter)))
		{
			continue;
		}
		sType = pcCallee->GetTemporaryType(*pcCallee->masParameters.Get(uiParameter));
		aulliValues.FakeSetUsedElements(0);
		bAllConstant = true;
		for (ui = 0; (ui < asSites.NumElements()) && bAllConstant; ui++)
		{
			psSite = asSites.Get(ui);
			sArgument = IRCallArgument(psSite, uiParameter);
			if (!sArgument.IsConstant())
			{
				bAllConstant = false;
				break;
			}
			ulliValue = masWritten.Get(psSite->uiWritten)->pcFunction->GetConstant(sArgument)->ulliBits;
			for (uj = 0; (uj < aulliValues.NumElements()) && (*aulliValues.Get(uj) != ulliValue); uj++)
			{
			}
			if (uj == aulliValues.NumElements())
			{
				aulliValues.Add(&ulliValue);
			}
		}
		if (!bAllConstant || (aulliValues.NumElements() > IR65_SPECIALISE_VALUES))
		{
			continue;
		}
		bDone = true;

		if (aulliValues.NumElements() == 1)
		{
			//One value: the parameter is that constant (the calls still pass it).
			IRReplaceUses(pcCallee, *pcCallee->masParameters.Get(uiParameter), pcCallee->AddConstant(sType, *aulliValues.Get(0)));
			OptimiseSpecialised(pcCallee);
			*pabChanged->Get(uiCallee) = true;
			break;
		}

		//A copy for each value, and its calls go to it.
		for (uk = 0; uk < aulliValues.NumElements(); uk++)
		{
			ulliValue = *aulliValues.Get(uk);
			szName.Init(mcModule.GetSymbolName(uiSymbol));
			szName.Append("__K");
			szName.Append((int)uiParameter);
			szName.Append('_');
			if ((int64)ulliValue < 0)
			{
				szName.Append('m');
				szName.Append((int64)(0 - ulliValue));
			}
			else
			{
				szName.Append((int64)ulliValue);
			}
			pcCopy = CopyFunction(pcCallee, szName.Text());
			if (pcCopy)
			{
				IRReplaceUses(pcCopy, *pcCopy->masParameters.Get(uiParameter), pcCopy->AddConstant(sType, ulliValue));
				OptimiseSpecialised(pcCopy);

				//Written after the function (its marker straight after the function's), in a section of its own.
				psWritten = masWritten.Add();
				psWritten->pcFunction = pcCopy;
				psWritten->szLabel.Init(szName.Text());
				psWritten->uiPieces = 0;
				memset(&psWritten->sFrame, 0, sizeof(psWritten->sFrame));
				psWritten->uiFingerprint = 0;
				pabChanged->Add(&bTrue);
				szMarker.Init(IR65_FRONT_END_MARKER);
				szMarker.Append((int)uiCallee);
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

				for (ui = 0; ui < asSites.NumElements(); ui++)
				{
					psSite = asSites.Get(ui);
					sArgument = IRCallArgument(psSite, uiParameter);
					if (masWritten.Get(psSite->uiWritten)->pcFunction->GetConstant(sArgument)->ulliBits == ulliValue)
					{
						psSite->pcBlock->masInstructions.Get(psSite->uiCall)->asArguments[0] = IRReference(IRRK_Symbol, pcCopy->muiSymbol);
						*pabChanged->Get(psSite->uiWritten) = true;
					}
				}
			}
			szName.Kill();
		}

		//Nothing calls the function now: it goes, with its section line, unless the generator's code names it
		//otherwise (a table of functions).
		psWritten = masWritten.Get(uiCallee);
		szSection.Init();
		mpcGenerator->AppendSection(&szSection, "text", &psWritten->szLabel, NULL);
		uiSectionLength = szSection.Length();
		szSection.Append(IR65_FRONT_END_MARKER);
		szSection.Append((int)uiCallee);
		szSection.Append(IR65_FRONT_END_MARKER);
		uiMarker = mpcGenerator->mszCode.Find(0, szSection.Text());
		if (uiMarker != ARRAY_ELEMENT_NOT_FOUND)
		{
			mpcGenerator->mszCode.Remove(uiMarker, uiMarker + uiSectionLength);
		}
		if (IRLabelNamed(mpcGenerator->mszCode.Text(), psWritten->szLabel.Text()))
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
			CCPP65Assembly::PutCode(mpcGenerator->mpcSpillStore, mpcGenerator->muiSpillUnit, uiCallee, "", 0, &psWritten->uiPieces);
			*pabChanged->Get(uiCallee) = false;
		}
		szSection.Kill();
	}
	aulliValues.Kill();
	asSites.Kill();
	return bDone;
}


//////////////////////////////////////////////////////////////////////////
//
// A copy of pcFunction (through its text) named szName, in the unit, local
// and taking register arguments as it does, or NULL if it couldn't be
// made.  An existing function of that name is it.
//
//////////////////////////////////////////////////////////////////////////
CIRFunction* CCPP65IRFrontEnd::CopyFunction(CIRFunction* pcFunction, const char* szName)
{
	CChars			szText;
	CChars			szHeader;
	CIRPrinter		cPrinter;
	CIRParser		cParser;
	CCPPErrors		cErrors;
	int				iSymbol;
	size			uiAt;
	bool			bResult;
	CIRFunction*	pcCopy;

	iSymbol = mcModule.FindSymbol(szName);
	if (iSymbol >= 0)
	{
		pcCopy = mcModule.FindFunction((uint32)iSymbol);
		if (pcCopy)
		{
			return pcCopy;
		}
	}
	szText.Init();
	cPrinter.Init(&mcModule, &szText);
	cPrinter.PrintFunction(pcFunction);
	cPrinter.Kill();

	//The function's own name (in its first line) is the copy's.
	szHeader.Init("@");
	szHeader.Append(mcModule.GetSymbolName(pcFunction->muiSymbol));
	szHeader.Append('(');
	uiAt = szText.Find(0, szHeader.Text());
	if (uiAt == ARRAY_ELEMENT_NOT_FOUND)
	{
		szHeader.Kill();
		szText.Kill();
		return NULL;
	}
	szText.Remove(uiAt + 1, uiAt + szHeader.Length() - 1);
	szText.Insert(uiAt + 1, szName);
	szHeader.Kill();

	cErrors.Init();
	cParser.Init(&mcModule, &cErrors, "specialised");
	bResult = cParser.Parse(szText.Text());
	cParser.Kill();
	cErrors.Kill();
	szText.Kill();
	iSymbol = mcModule.FindSymbol(szName);
	if (!bResult || (iSymbol < 0))
	{
		return NULL;
	}
	pcCopy = mcModule.FindFunction((uint32)iSymbol);
	if (pcCopy)
	{
		mcModule.SetLocal((uint32)iSymbol);
		if (mcModule.IsRegisterCall(pcFunction->muiSymbol))
		{
			mcModule.SetRegisterCall((uint32)iSymbol);
		}
	}
	return pcCopy;
}


//////////////////////////////////////////////////////////////////////////
//
// A specialised function optimised again (its constant folded through).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::OptimiseSpecialised(CIRFunction* pcFunction)
{
	CIROptimiser	cOptimiser;

	if (sbOptimise)
	{
		InitOptimiser(&cOptimiser);
		cOptimiser.Optimise(pcFunction);
		cOptimiser.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The parameters (the first 32) pcFunction never reads, as bits: no use of
// its temporary.  None if anything takes a parameter's address (argaddr):
// an object passed by value in pieces is reached through its first's.  A
// member function's this stays (IRA_ThisCall: the 65C64 passes it in X).
//
//////////////////////////////////////////////////////////////////////////
uint32 CCPP65IRFrontEnd::DeadParameters(CIRFunction* pcFunction)
{
	CArrayTemplate<uint8>	abUsed;
	CIRBlock*				pcBlock;
	SIRInstruction*			psInstruction;
	SIRPhi*					psPhi;
	SIRReference			sValue;
	size					uiBlock;
	size					ui;
	size					uj;
	size					uiParameters;
	uint32					uiDead;
	uint8					uiZero;

	uiParameters = pcFunction->masParameters.NumElements();
	if (uiParameters > 32)
	{
		uiParameters = 32;
	}

	//Which temporaries are read.
	uiZero = 0;
	abUsed.Init();
	abUsed.GrowToAtLeastNumElements(pcFunction->NumTemporaries(), true, 0);
	for (uiBlock = 0; uiBlock < pcFunction->NumBlocks(); uiBlock++)
	{
		pcBlock = pcFunction->GetBlock(uiBlock);
		for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
		{
			psInstruction = pcBlock->masInstructions.Get(ui);
			if (psInstruction->uiOpcode == IRO_ArgAddress)
			{
				abUsed.Kill();
				return 0;
			}
			for (uj = 0; uj < 2; uj++)
			{
				sValue = psInstruction->asArguments[uj];
				if (sValue.IsTemporary() && (sValue.Index() < abUsed.NumElements()))
				{
					*abUsed.Get(sValue.Index()) = 1;
				}
			}
		}
		for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
		{
			psPhi = pcBlock->masPhis.Get(ui);
			for (uj = 0; uj < psPhi->asArguments.NumElements(); uj++)
			{
				sValue = psPhi->asArguments.Get(uj)->sValue;
				if (sValue.IsTemporary() && (sValue.Index() < abUsed.NumElements()))
				{
					*abUsed.Get(sValue.Index()) = 1;
				}
			}
		}
		sValue = pcBlock->msJump.sArgument;
		if (sValue.IsTemporary() && (sValue.Index() < abUsed.NumElements()))
		{
			*abUsed.Get(sValue.Index()) = 1;
		}
	}

	uiDead = 0;
	for (ui = (pcFunction->muiAttributes & IRA_ThisCall) ? 1 : 0; ui < uiParameters; ui++)
	{
		sValue = *pcFunction->masParameters.Get(ui);
		if (sValue.IsTemporary() && (sValue.Index() < abUsed.NumElements()) && !*abUsed.Get(sValue.Index()))
		{
			uiDead |= (uint32)1 << ui;
		}
	}
	abUsed.Kill();
	return uiDead;
}


//////////////////////////////////////////////////////////////////////////
//
// Dead argument removal (InlineUnit, after SpecialiseConstantArguments): a
// function of the unit that only the unit calls (local, its address never
// taken, not variadic) loses the parameters it never reads
// (DeadParameters), and its calls their arguments, so they aren't worked
// out, loaded or pushed (a parameter specialised for its one value is one
// such).  The callee and its callers are optimised again (what only made
// an argument goes) and get pabChanged (per written function) so their
// code is written again.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::RemoveDeadArguments(CArrayTemplate<bool>* pabChanged)
{
	CArrayTemplate<SCPP65CallSite>	asSites;
	CArrayTemplate<bool>			abOptimise;
	SCPP65CallSite*					psSite;
	SCPP65CallSite					sSite;
	CIRFunction*					pcCallee;
	CIRFunction*					pcFunction;
	CIRBlock*						pcBlock;
	SIRInstruction*					psInstruction;
	SIRReference					sArgument;
	uint32							uiSymbol;
	uint32							uiDead;
	size							uiCallee;
	size							uiWritten;
	size							uiBlock;
	size							uiParameters;
	size							uiFirst;
	size							ui;
	size							uj;
	size							uk;
	bool							bTaken;
	bool							bFalse;

	if (!sbOptimise)
	{
		return;
	}
	bFalse = false;
	abOptimise.Init();
	for (ui = 0; ui < masWritten.NumElements(); ui++)
	{
		abOptimise.Add(&bFalse);
	}
	asSites.Init();
	for (uiCallee = 0; uiCallee < masWritten.NumElements(); uiCallee++)
	{
		pcCallee = masWritten.Get(uiCallee)->pcFunction;
		if (!pcCallee || (pcCallee->NumBlocks() == 0) || (pcCallee->muiAttributes & IRA_Variadic) ||
			!((pcCallee->muiAttributes & IRA_Local) || mcModule.IsLocal(pcCallee->muiSymbol)))
		{
			continue;
		}
		uiDead = DeadParameters(pcCallee);
		if (uiDead == 0)
		{
			continue;
		}

		//Its calls, each with an argument per parameter; it mustn't be named otherwise (its address taken).
		uiSymbol = pcCallee->muiSymbol;
		uiParameters = pcCallee->masParameters.NumElements();
		asSites.FakeSetUsedElements(0);
		bTaken = false;
		for (uiWritten = 0; (uiWritten < masWritten.NumElements()) && !bTaken; uiWritten++)
		{
			pcFunction = masWritten.Get(uiWritten)->pcFunction;
			for (uiBlock = 0; pcFunction && (uiBlock < pcFunction->NumBlocks()) && !bTaken; uiBlock++)
			{
				pcBlock = pcFunction->GetBlock(uiBlock);
				for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
				{
					psInstruction = pcBlock->masInstructions.Get(ui);
					for (uj = 0; uj < 2; uj++)
					{
						if (psInstruction->asArguments[uj].IsSymbol() && (psInstruction->asArguments[uj].Index() == uiSymbol))
						{
							if ((psInstruction->uiOpcode == IRO_Call) && (uj == 0) && !(psInstruction->uiFlags & IRF_Variadic))
							{
								for (uiFirst = ui; (uiFirst > 0) && (pcBlock->masInstructions.Get(uiFirst - 1)->uiOpcode == IRO_Arg); uiFirst--)
								{
								}
								bTaken |= (ui - uiFirst) != uiParameters;
								sSite.uiWritten = uiWritten;
								sSite.pcBlock = pcBlock;
								sSite.uiCall = ui;
								asSites.Add(&sSite);
							}
							else
							{
								bTaken = true;
							}
						}
					}
				}
				for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
				{
					for (uj = 0; uj < pcBlock->masPhis.Get(ui)->asArguments.NumElements(); uj++)
					{
						sArgument = pcBlock->masPhis.Get(ui)->asArguments.Get(uj)->sValue;
						bTaken |= sArgument.IsSymbol() && (sArgument.Index() == uiSymbol);
					}
				}
				bTaken |= pcBlock->msJump.sArgument.IsSymbol() && (pcBlock->msJump.sArgument.Index() == uiSymbol);
			}
		}
		if (bTaken || (asSites.NumElements() == 0))
		{
			continue;
		}

		//The calls' arguments for them, last call first (the sites before a removal keep their places), last
		//parameter first.
		for (ui = asSites.NumElements(); ui > 0; ui--)
		{
			psSite = asSites.Get(ui - 1);
			uiFirst = psSite->uiCall - uiParameters;
			for (uk = uiParameters; uk > 0; uk--)
			{
				if ((uk <= 32) && (uiDead & ((uint32)1 << (uk - 1))))
				{
					psSite->pcBlock->masInstructions.RemoveAt(uiFirst + uk - 1, true);
				}
			}
			*abOptimise.Get(psSite->uiWritten) = true;
		}

		//The parameters, last first.
		for (uk = uiParameters; uk > 0; uk--)
		{
			if ((uk <= 32) && (uiDead & ((uint32)1 << (uk - 1))))
			{
				pcCallee->masParameters.RemoveAt(uk - 1, true);
			}
		}
		*abOptimise.Get(uiCallee) = true;
	}
	asSites.Kill();

	for (ui = 0; ui < masWritten.NumElements(); ui++)
	{
		if (*abOptimise.Get(ui))
		{
			OptimiseSpecialised(masWritten.Get(ui)->pcFunction);
			*pabChanged->Get(ui) = true;
		}
	}
	abOptimise.Kill();
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
	const char*		sz;
	uint32			uiHash;

	szText.Init();
	PrintFunction(pcFunction, &szText, false);
	uiHash = 2166136261u;
	for (sz = szText.Text(); *sz != '\0'; sz++)
	{
		uiHash = (uiHash ^ (uint8)*sz) * 16777619u;
	}
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
