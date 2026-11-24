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
#include "BaseLib/StringHelper.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/ConstructorCall.h"
#include "IR65BackendInternal.h"
#include "CPP65DebugWriter.h"


//////////////////////////////////////////////////////////////////////////
//
// piLabels is the counter labels are numbered from (the generator's).  The
// soft float routines' labels the code calls are GetRuntime's.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Init(CIRModule* pcModule, int* piLabels, CScratchPadAllocator* pcArena)
{
	mcSelector.Init(this);
	mcCalls.Init(this);
	mcFusion.Init(this);
	mcFolds.Init(this);
	mcPlaces.Init(this);
	InitBackend(pcModule, piLabels, NULL);
	mcDominators.Init();
	mcLiveness.Init();
	masTemporaries.Init();
	mauiAddressed.Init();
	masArguments.Init();
	miRegisterArguments = 0;
	masHomes.Init();
	maiCalls.Init();
	maiChangesXY.Init();
	maiChangesY.Init();
	masCopies.Init();
	masCopySteps.Init();
	masStubs.Init();
	msFrame.Init();
	miReturnLabel = 0;
	msInA = IRNone();
	msAvailable = IRNone();
	masFused.Init();
	mszDeferred.Init();
	mszBodyLabel.Init();
	mpsFlags = NULL;
	mpcFlags = NULL;
	mbFlagsKnown = false;
	mbRightImmediate = false;
	mszAddress = "B";
	mbCopyAddress = false;
	mcFlags.Init();
	mcWidths.Init();
	mcPeephole.Init();
	mcBackendPasses.Init();
	mcMost.Init();
	maiChangesB.Init();
	maiLosesB.Init();
	masChunks.Init();
	mbHomeB = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Kill(void)
{
	size	ui;

	mszBodyLabel.Kill();
	mcSelector.Kill();
	mcCalls.Kill();
	mcFusion.Kill();
	mcFolds.Kill();
	mcPlaces.Kill();
	masChunks.Kill();
	maiLosesB.Kill();
	maiChangesB.Kill();
	mcMost.Kill();
	mcPeephole.Kill();
	mcBackendPasses.Kill();
	mcWidths.Kill();
	mcFlags.Kill();
	mszDeferred.Kill();
	masFused.Kill();
	masStubs.Kill();
	masCopySteps.Kill();
	masCopies.Kill();
	maiChangesY.Kill();
	maiChangesXY.Kill();
	maiCalls.Kill();
	for (ui = 0; ui < masHomes.NumElements(); ui++)
	{
		masHomes.Get(ui)->asRanges.Kill();
	}
	masHomes.Kill();
	masArguments.Kill();
	mauiAddressed.Kill();
	masTemporaries.Kill();
	mcLiveness.Kill();
	mcDominators.Kill();
	CIRBackend::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The function's label, frame, blocks and return, appended to pszDest
// (see Prologue and Epilogue).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::GenerateFunction(CIRFunction* pcFunction, const char* szLabel, CChars* pszDest, CCPPErrors* pcErrors)
{
	size		uiStart;
	int			iTry;
	bool		bHomeB;
	size		uiErrors;
	size		ui;
	SIR65Frame	sLaidOut;
	SIR65Frame	sWritten;

	mpcFunction = pcFunction;
	mpszOut = pszDest;
	mpcErrors = pcErrors;
	mbResult = true;
	uiStart = pszDest->Length();
	muiSource = 0;

	ExpandMultiplies();
	mcBackendPasses.IsolateThrowns(pcFunction);
	pcFunction->Renumber();
	pcFunction->ComputePredecessors();
	masTemporaries.FakeSetUsedElements(0);
	CountUses();
	AdjustComparisons();
	PreferSingleBranches();
	mcBackendPasses.OrderBlocks(pcFunction, false);
	mcBackendPasses.MoveLoopTests(pcFunction);
	mcBackendPasses.SinkLoads(pcFunction);
	mcBackendPasses.SinkSteps(pcFunction);
	CountUses();
	CopyAddresses();
	CountUses();
	mcPlaces.FindNarrowOnly();
	mpcModule->Count(CPPCC_Dominators);
	mcDominators.Compute(pcFunction);
	mcLiveness.Compute(pcFunction, &mcDominators);
	mcPlaces.FunctionArguments();
	mcPlaces.Layout();
	mcMost.Compute(pcFunction);
	sLaidOut = msFrame;
	for (ui = 0; ui < masTemporaries.NumElements(); ui++)
	{
		masTemporaries.Get(ui)->sLaidOut = masTemporaries.Get(ui)->sPlace;
	}
	maiChangesB.FakeSetUsedElements(0);
	maiLosesB.FakeSetUsedElements(0);
	mbHomeB = false;
	mcPlaces.Allocate();
	WriteBody(szLabel, uiStart);

	//B as a home: the code just written says where B is worked in, which a class kept in B must not be live across.
	//The code is written again with B given to such classes and checked (CheckHomeB); where it lost a value kept in
	//B the classes live there are kept elsewhere and it is written again, and after IR65_HOME_B_TRIES without B.
	if (mpcOptions->mbHomeB && mbResult)
	{
		uiErrors = mpcErrors ? mpcErrors->NumErrors() : 0;
		sWritten = msFrame;
		mcPlaces.CheckHomeB(true);
		bHomeB = false;
		for (iTry = 0; iTry < IR65_HOME_B_TRIES; iTry++)
		{
			Reallocate(&sLaidOut, true);
			if (masHomes.Get(IR65R_B)->asRanges.NumElements() == 0)
			{
				if (iTry == 0)
				{
					//The same homes as the code written.
					msFrame = sWritten;
					bHomeB = true;
				}
				break;
			}
			pszDest->SetLength(uiStart);
			if (!WriteBody(szLabel, uiStart))
			{
				//Kept from the code without B, which has none.
				if (mpcErrors)
				{
					mpcErrors->Truncate(uiErrors);
				}
				mbResult = true;
				break;
			}
			if (mcPlaces.CheckHomeB(false))
			{
				bHomeB = true;
				break;
			}
		}
		if (!bHomeB)
		{
			Reallocate(&sLaidOut, false);
			pszDest->SetLength(uiStart);
			WriteBody(szLabel, uiStart);
		}
	}
	if (mbSourceMarkers)
	{
		mcSourceMarkers.Take(pszDest, uiStart);
	}
	//The passes over the body's code: a register function's stub before it (RegisterStub) is as it must be, and
	//the body's label is theirs to start from (the stub's jsr of it is no branch they know).
	mpcModule->Begin(CPPCP_CodePasses);
	Peephole(muiBodyStart);
	mcFlags.SetExceptions(mpcOptions->mbExceptions);
	mcWidths.SetExceptions(mpcOptions->mbExceptions);
	mcPeephole.SetExceptions(mpcOptions->mbExceptions);
	mcFlags.Optimise(pszDest, muiBodyStart);
	CountLoaded(mcFlags.IsLoaded());
	mcWidths.Optimise(pszDest, muiBodyStart);
	CountLoaded(mcWidths.IsLoaded());
	mcPeephole.Optimise(pszDest, muiBodyStart, msFrame.bHomeV);
	CountLoaded(mcPeephole.IsLoaded());
	IR65PseudoWithoutW(pszDest, muiBodyStart);
	mpcModule->End(CPPCP_CodePasses);
	if (mbSourceMarkers)
	{
		mcSourceMarkers.Put(pszDest, uiStart);
	}
	return mbResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Homes given again, from the frame and places Layout left (psLaidOut,
// each temporary's sLaidOut), with B one of them or not.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Reallocate(SIR65Frame* psLaidOut, bool bHomeB)
{
	size	ui;

	msFrame = *psLaidOut;
	for (ui = 0; ui < masTemporaries.NumElements(); ui++)
	{
		masTemporaries.Get(ui)->sPlace = masTemporaries.Get(ui)->sLaidOut;
	}
	mbHomeB = bHomeB;
	mcPlaces.Allocate();
}


//////////////////////////////////////////////////////////////////////////
//
// A record for each of the function's temporaries, and one more (as
// CIRFunction::CountUses counts).  The records made are nothing known; those
// there already are kept, as the function may have more temporaries than
// when the last pass ran.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::GrowTemporaries(void)
{
	SIR65Temporary	sNew;

	memset(&sNew, 0, sizeof(SIR65Temporary));
	sNew.iNextInClass = -1;
	sNew.iLeader = -1;
	while (masTemporaries.NumElements() < mpcFunction->NumTemporaries() + 1)
	{
		masTemporaries.Add(&sNew);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// How many times each temporary is used, by instructions, phis and jumps.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::CountUses(void)
{
	CArrayTemplate<uint32>	auiUses;
	size					ui;

	auiUses.Init();
	mpcFunction->CountUses(&auiUses);
	GrowTemporaries();
	for (ui = 0; ui < auiUses.NumElements(); ui++)
	{
		masTemporaries.Get(ui)->uiUses = *auiUses.Get(ui);
	}
	auiUses.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIR65Temporary* CIR65Backend::GetTemporary(size uiTemporary)
{
	return masTemporaries.Get(uiTemporary);
}


//////////////////////////////////////////////////////////////////////////
//
// What the epilogue does after its label: the kept pseudo registers
// pulled, the frame removed and V pulled, then ret.
//
// With exceptions (ExceptionsPlan.txt) a function returns with the carry
// clear unless it threw, whether or not it can throw (it may be called
// through a pointer, and that call is tested).  Removing the frame (add S,
// n) leaves it clear; with no frame to remove, add S, 0 does.  bThrow: the
// exit of a block that ends in throw, which sets it (cmp S, S: no borrow).
// pul and ret leave the flags alone.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::ReturnCode(bool bThrow)
{
	bool	bClear;

	int32	iUnit;

	for (iUnit = IR65_PSEUDO_UNITS - 1; iUnit >= 0; iUnit--)
	{
		if (msFrame.uiSaved & (1u << iUnit))
		{
			Line("pul [")->Append(IR65_SAVED_UNIT(iUnit))->Append(" + W]")->AppendNewLine();
		}
	}
	bClear = false;
	if (!msFrame.bFrameless)
	{
		if (msFrame.iFrameBytes > 0)
		{
			Line("add S, ")->Append(msFrame.iFrameBytes)->AppendNewLine();
			bClear = true;
		}
		Line("pul V")->AppendNewLine();
	}
	if (msFrame.bHomeV)
	{
		Line("pul V")->AppendNewLine();
	}
	if (bThrow)
	{
		Line("cmp S, S")->AppendNewLine();
	}
	else if (mpcOptions->mbExceptions && !bClear)
	{
		Line("add S, 0")->AppendNewLine();
	}
	Line("ret")->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// How many instructions ReturnCode writes.
//
//////////////////////////////////////////////////////////////////////////
int CIR65Backend::ReturnInstructions(void)
{
	int	iInstructions;

	iInstructions = msFrame.NumSaved() + 1;
	if (!msFrame.bFrameless)
	{
		iInstructions += (msFrame.iFrameBytes > 0) ? 2 : 1;
	}
	if (mpcOptions->mbExceptions && (msFrame.bFrameless || (msFrame.iFrameBytes == 0)))
	{
		//The add S, 0 that clears the carry.
		iInstructions++;
	}
	if (msFrame.bHomeV)
	{
		iInstructions++;
	}
	return iInstructions;
}


//////////////////////////////////////////////////////////////////////////
//
// The function's code from its homes (Allocate): the prologue, the blocks
// and the epilogue, appended to the text from uiStart on, each block's
// instructions and jump noted in masChunks.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::WriteBody(const char* szLabel, size uiStart)
{
	size		ui;
	CIRBlock*	pcBlock;
	int32		iLabel;

	//A function with nothing in the frame doesn't set V up when its arguments can be found from S: a leaf (S doesn't
	//move) or one that reads them before pushing anything.  Otherwise V points at the frame.
	msFrame.SetFrameless(msFrame.bHomeV || ((msFrame.iFrameBytes == 0) && (msFrame.iThisOffset < 0) && (mcPlaces.IsLeaf() || mcPlaces.ArgumentsReadFirst())));

	maiBlockLabels.FakeSetUsedElements(0);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		iLabel = NewLabel();
		maiBlockLabels.Add(&iLabel);
	}
	miReturnLabel = NewLabel();
	masChunks.FakeSetUsedElements(0);
	muiSource = 0;

	Prologue(szLabel);
	mpsFlags = NULL;
	mpcFlags = NULL;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		if (!Block(pcBlock, (ui + 1 < mpcFunction->NumBlocks()) ? mpcFunction->GetBlock(ui + 1) : NULL))
		{
			mbResult = false;
		}
	}
	Epilogue();
	return mbResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A pass over the function's code read it (bLoaded) or left it as it was.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::CountLoaded(bool bLoaded)
{
	mpcModule->Count(bLoaded ? CPPCC_Code65Loaded : CPPCC_Code65Refused);
}


//////////////////////////////////////////////////////////////////////////
//
//	label:						; with arguments in registers the stub for callers that push them all
//			...					; (RegisterStub), then label.aby:
//			psh V
//			sub S, frame
//			cpy V, S			; only psh V where V is a home (SIR65Frame::bHomeV)
//			psh [n + W]			; each [$80 + W] to [$FF + W] unit used
//			...					; parameters copied to their homes
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Prologue(const char* szLabel)
{
	size			ui;
	SIRReference	sParameter;
	SIRType			sType;
	SIR65Place*		psPlace;
	SIR65Place		sArrival;
	CChars			szBody;

	muiBodyStart = mpszOut->Length();
	mpszOut->Append(szLabel);
	mpszOut->Append(':');
	mpszOut->AppendNewLine();
	mszBodyLabel.Clear();
	if (miRegisterArguments > 0)
	{
		//A register function: the plain label is the stub for callers that push everything, then the body's own label
		//(visible to other objects when the plain one is).
		szBody.Init(szLabel);
		AppendRegisterSuffix(&szBody, &masArguments, IR65_REGISTER_LETTERS);
		mcCalls.RegisterStub(szBody.Text());
		if (!(mpcFunction->muiAttributes & IRA_Local) && !mpcModule->IsLocal(mpcFunction->muiSymbol))
		{
			mpszOut->Append((mpcFunction->muiAttributes & IRA_Weak) ? "\t\t.weak " : "\t\t.global ");
			mpszOut->Append(szBody);
			mpszOut->AppendNewLine();
		}
		muiBodyStart = mpszOut->Length();
		mpszOut->Append(szBody);
		mpszOut->Append(':');
		mpszOut->AppendNewLine();
		mszBodyLabel.Set(szBody);
		szBody.Kill();
	}
	AppendFramePrologue(mpszOut, &msFrame);

	//The parameters that came in registers first (the pushed ones are read through A).
	StoreRegisterParameters();
	for (ui = 0; ui < mpcFunction->masParameters.NumElements(); ui++)
	{
		sParameter = *mpcFunction->masParameters.Get(ui);
		sType = mpcFunction->GetTemporaryType(sParameter);
		psPlace = mcPlaces.PlaceOf(sParameter);
		if (((ui == 0) && (mpcFunction->muiAttributes & IRA_ThisCall)) || (mcPlaces.ParameterRegister(ui) >= 0))
		{
			continue;
		}
		if ((psPlace->uiKind == IR65P_Register) || (psPlace->uiKind == IR65P_Pseudo) || (psPlace->uiKind == IR65P_Frame))
		{
			mcPlaces.ArrivalPlace(ui, &sArrival);
			if (psPlace->uiKind == IR65P_Register)
			{
				mcSelector.LoadPlace(&sArrival, sType, mcPlaces.RegisterName(psPlace), IR65E_Zero);
			}
			else
			{
				mcSelector.LoadPlace(&sArrival, sType, "A", IR65E_Zero);
				mcSelector.StorePlace(psPlace, sType, "A");
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A parameter that arrived in szRegister goes to its home psPlace: to a
// pseudo register or the frame now; to X or Y later (StoreRegisterParameters
// orders those), noted in aszSources and apsPlaces.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::StoreArrivedRegister(const char* szRegister, SIR65Place* psPlace, SIRType sType, const char** aszSources, SIR65Place** apsPlaces, int* piMoves)
{
	if ((psPlace->uiKind == IR65P_Pseudo) || (psPlace->uiKind == IR65P_Frame))
	{
		mcSelector.StorePlace(psPlace, sType, szRegister);
	}
	else if ((psPlace->uiKind == IR65P_Register) && (StringCompare(mcPlaces.RegisterName(psPlace), szRegister) != 0))
	{
		aszSources[*piMoves] = szRegister;
		apsPlaces[*piMoves] = psPlace;
		(*piMoves)++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The parameters that arrived in registers (a thiscall's this in X, the
// others in A, B and Y: FunctionArguments) are copied to their homes, and
// to the frame when an argaddr wants their address: those kept in memory
// first, then those kept in X and Y, a register read before it is written
// (this from X to Y before a parameter into X), and two that swap X and Y
// through A, which by then has nothing left to give.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::StoreRegisterParameters(void)
{
	static const char*	aszRegisters[IR65_ARGUMENT_REGISTERS] = { "A", "B", "Y" };
	const char*			aszSources[IR65_ARGUMENT_REGISTERS + 1];
	SIR65Place*			apsPlaces[IR65_ARGUMENT_REGISTERS + 1];
	const char*			szRegister;
	SIRReference		sParameter;
	size				ui;
	int					iMoves;
	int					iRegister;
	int					i;
	int					j;
	bool				bRead;

	iMoves = 0;
	for (ui = 0; ui < mpcFunction->masParameters.NumElements(); ui++)
	{
		sParameter = *mpcFunction->masParameters.Get(ui);
		if ((ui == 0) && (mpcFunction->muiAttributes & IRA_ThisCall))
		{
			szRegister = "X";
			if (msFrame.iThisOffset >= 0)
			{
				Line("cpy [")->Append(1 + msFrame.iThisOffset)->Append(" + V], X")->AppendNewLine();
			}
		}
		else
		{
			iRegister = mcPlaces.ParameterRegister(ui);
			if (iRegister < 0)
			{
				continue;
			}
			szRegister = aszRegisters[iRegister];
			if (msFrame.aiRegisterOffset[iRegister] >= 0)
			{
				Line("cpy [")->Append(1 + msFrame.aiRegisterOffset[iRegister])->Append(" + V], ")->Append(szRegister)->AppendNewLine();
			}
		}
		StoreArrivedRegister(szRegister, mcPlaces.PlaceOf(sParameter), mpcFunction->GetTemporaryType(sParameter), aszSources, apsPlaces, &iMoves);
	}

	//Into X and Y: one nothing still to be copied reads goes first.
	while (iMoves > 0)
	{
		for (i = 0; i < iMoves; i++)
		{
			bRead = false;
			for (j = 0; j < iMoves; j++)
			{
				bRead |= (j != i) && (StringCompare(aszSources[j], mcPlaces.RegisterName(apsPlaces[i])) == 0);
			}
			if (!bRead)
			{
				break;
			}
		}
		if (i == iMoves)
		{
			//X and Y take each other's.
			Line("cpy A, X")->AppendNewLine();
			Line("cpy X, Y")->AppendNewLine();
			Line("cpy Y, A")->AppendNewLine();
			return;
		}
		Line("cpy ")->Append(mcPlaces.RegisterName(apsPlaces[i]))->Append(", ")->Append(aszSources[i])->AppendNewLine();
		iMoves--;
		aszSources[i] = aszSources[iMoves];
		apsPlaces[i] = apsPlaces[iMoves];
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The prologue's instructions that make psFrame: V saved (a home), or V
// saved and the frame made from S, then the [$80 + W] units it keeps.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::AppendFramePrologue(CChars* pszDest, SIR65Frame* psFrame)
{
	int32	iUnit;

	if (psFrame->bHomeV)
	{
		pszDest->Append("\t\tpsh V")->AppendNewLine();
	}
	if (!psFrame->bFrameless)
	{
		pszDest->Append("\t\tpsh V")->AppendNewLine();
		if (psFrame->iFrameBytes > 0)
		{
			pszDest->Append("\t\tsub S, ")->Append(psFrame->iFrameBytes)->AppendNewLine();
		}
		pszDest->Append("\t\tcpy V, S")->AppendNewLine();
	}
	for (iUnit = 0; iUnit < IR65_PSEUDO_UNITS; iUnit++)
	{
		if (psFrame->uiSaved & (1u << iUnit))
		{
			pszDest->Append("\t\tpsh [")->Append(IR65_SAVED_UNIT(iUnit))->Append(" + W]")->AppendNewLine();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The function's frame for a debugger, its variables' slots' places, and a
// register function's body as an entry of its own (its stub may call it
// after pushing what it was given).
//
//////////////////////////////////////////////////////////////////////////
CIRWrittenFunction* CIR65Backend::TakeWritten(void)
{
	CIR65Written*	pcWritten;

	pcWritten = NewMalloc<CIR65Written>();
	pcWritten->Init();
	pcWritten->msFrame = msFrame;
	AddVariableSlots(pcWritten);
	if (!mszBodyLabel.Empty())
	{
		pcWritten->AddEntry(mszBodyLabel.Text());
	}
	return pcWritten;
}


//////////////////////////////////////////////////////////////////////////
//
// A variable's slot is at [1 + its offset + V].  A parameter read where it
// was passed (its argaddr) is in the frame too when it came in a register,
// else in the bytes its caller pushed, just above the frame address: from V
// where the arguments start (SIR65Frame::iArguments) in a function with a
// frame of its own, from the frame address in one without.
//
//////////////////////////////////////////////////////////////////////////
int32 CIR65Backend::VariableSlotPlace(SIRInstruction* psSlot)
{
	SIR65Place*	psPlace;

	psPlace = &masTemporaries.Get(psSlot->sResult.Index())->sPlace;
	if (psPlace->uiKind == IR65P_ArgumentAddress)
	{
		return psPlace->iOffset + (msFrame.bFrameless ? 1 : msFrame.iArguments);
	}
	return 1 + psPlace->iOffset;
}


//////////////////////////////////////////////////////////////////////////
//
// For a debugger (the frame model, Debugger65C64Plan.txt), after the
// prologue (AppendFramePrologue, at its body's label): the frame address
// (S before the jsr pushed its 4 byte return address) is V plus what is
// below it (a frame of its own: .dframe V, its variables from V too), or
// found by following its code (frameless, V a home or untouched); what it
// pushed is saved there (.dsaved): the caller's V, and each [$80 + W]
// unit it keeps.  A prologue not as it was written is described by
// following it.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::DescribeFrame(CChars* pszCode, const char* szLabel, CIRWrittenFunction* pcWritten)
{
	SIR65Frame*	psFrame;
	CChars		szPrologue;
	CChars		szDirectives;
	size		uiAt;
	int32		iBelow;
	int32		iUnit;

	if (!pcWritten)
	{
		return;
	}
	psFrame = &((CIR65Written*)pcWritten)->msFrame;
	uiAt = CCPP65DebugWriter::FindAfterLabel(pszCode->Text(), (pcWritten->NumEntries() > 0) ? pcWritten->GetEntry(0) : szLabel);
	if (uiAt == MAX_SIZE)
	{
		return;
	}
	szPrologue.Init();
	AppendFramePrologue(&szPrologue, psFrame);
	szDirectives.Init();
	if (strncmp(pszCode->Text() + uiAt, szPrologue.Text(), szPrologue.Length()) != 0)
	{
		szDirectives.Append("\t\t.dframe none")->AppendNewLine();
	}
	else
	{
		uiAt += szPrologue.Length();
		iBelow = 4;
		if (psFrame->bHomeV || !psFrame->bFrameless)
		{
			iBelow += 4;
			szDirectives.Append("\t\t.dsaved V, ")->Append(1 - iBelow)->AppendNewLine();
		}
		if (psFrame->bFrameless)
		{
			szDirectives.Append("\t\t.dframe none")->AppendNewLine();
		}
		else
		{
			iBelow += psFrame->iFrameBytes;
			szDirectives.Append("\t\t.dframe V, ")->Append(iBelow)->AppendNewLine();
			szDirectives.Append("\t\t.dbase V")->AppendNewLine();
		}
		for (iUnit = 0; iUnit < IR65_PSEUDO_UNITS; iUnit++)
		{
			if (psFrame->uiSaved & (1u << iUnit))
			{
				iBelow += 4;
				szDirectives.Append("\t\t.dsaved [")->Append(IR65_SAVED_UNIT(iUnit))->Append("], 4, ")->Append(1 - iBelow)->AppendNewLine();
			}
		}
	}
	pszCode->Insert(uiAt, &szDirectives);
	szDirectives.Kill();
	szPrologue.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//	Lreturn:
//			pul [n + W]			; in reverse
//			add S, frame
//			pul V
//			ret
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Epilogue(void)
{
	size		ui;
	size		uj;
	size		uiText;
	SIR65Stub*	psStub;
	SIR65Stub*	psSame;

	Label(miReturnLabel);
	ReturnCode();

	//Edges whose copies were moved out of the way.  Tested calls' that copy nothing share their code when they go to the
	//same block with the same bytes to remove (the tested calls of one landing): their labels are written together.
	for (ui = 0; ui < masStubs.NumElements(); ui++)
	{
		psStub = masStubs.Get(ui);
		if (psStub->bWritten)
		{
			continue;
		}
		uiText = mpszOut->Length();
		Label(psStub->iLabel);
		if (psStub->bTested && !EdgeCopies(psStub->pcFrom, psStub->pcTo))
		{
			for (uj = ui + 1; uj < masStubs.NumElements(); uj++)
			{
				psSame = masStubs.Get(uj);
				if (psSame->bTested && !psSame->bWritten && (psSame->pcTo == psStub->pcTo) && (psSame->iRemove == psStub->iRemove) && !EdgeCopies(psSame->pcFrom, psSame->pcTo))
				{
					Label(psSame->iLabel);
					psSame->bWritten = true;
				}
			}
		}
		if (psStub->iRemove > 0)
		{
			Line("add S, ")->Append(psStub->iRemove)->AppendNewLine();
		}
		Edge(psStub->pcFrom, psStub->pcTo);
		Branch("bra ", *maiBlockLabels.Get(psStub->pcTo->muiIndex));
		mcPlaces.AddChunk(mcLiveness.Jump(psStub->pcFrom), mcLiveness.Jump(psStub->pcFrom), uiText);
	}
	masStubs.FakeSetUsedElements(0);
	mpszOut->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// Starts an instruction line.  The caller appends the rest and the new line.
//
//////////////////////////////////////////////////////////////////////////
CChars* CIR65Backend::Line(const char* szText)
{
	mpszOut->Append("\t\t");
	return mpszOut->Append(szText);
}


//////////////////////////////////////////////////////////////////////////
//
// szBranch is the instruction and its flag ("bras z, "), or "bra ".
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Branch(const char* szBranch, int32 iLabel)
{
	Line(szBranch)->Append('L')->Append(iLabel)->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// Bytes a value of sType takes as an argument or in memory.
//
//////////////////////////////////////////////////////////////////////////
int CIR65Backend::ValueBytes(SIRType sType)
{
	return mpcModule->GetSize(sType);
}


//////////////////////////////////////////////////////////////////////////
//
// 64 bit values are worked on with the registers 64 bits wide.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::IsWide(SIRType sType)
{
	return sType.Bits() == 64;
}


//////////////////////////////////////////////////////////////////////////
//
// i8 and i16 (and i1), held zero extended.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::IsNarrow(SIRType sType)
{
	int	iBits;

	iBits = sType.IntegerBits();
	return (iBits > 0) && (iBits < 32);
}


//////////////////////////////////////////////////////////////////////////
//
// True if going from pcFrom to pcTo copies values to pcTo's phis (a phi
// that shares its home with the value it takes needs no copy).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::HasMoves(CIRBlock* pcFrom, CIRBlock* pcTo)
{
	size			ui;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;

	for (ui = 0; ui < pcTo->masPhis.NumElements(); ui++)
	{
		psPhi = pcTo->masPhis.Get(ui);
		psArgument = pcTo->FindPhiArgument(psPhi, pcFrom);
		if (psArgument && (mcPlaces.PlaceOf(psPhi->sResult)->uiKind != IR65P_None) && !mcPlaces.SamePlace(psArgument->sValue, psPhi->sResult))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// True if an instruction writes nothing: a folded ptradd, slots, argaddrs,
// args (the call pushes them) and instructions without effects whose
// results aren't used.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Skipped(SIRInstruction* psInstruction)
{
	switch (psInstruction->uiOpcode)
	{
	case IRO_Slot:
	case IRO_ArgAddress:
	case IRO_Arg:
	case IRO_Thrown:
		return true;
	case IRO_Call:
	case IRO_Store:
		return false;
	case IRO_Load:
		if (psInstruction->uiFlags & IRF_Volatile)
		{
			return false;
		}
		break;
	}
	if (!psInstruction->sResult.IsTemporary())
	{
		return false;
	}
	return (mcPlaces.PlaceOf(psInstruction->sResult)->uiKind == IR65P_Folded) || (masTemporaries.Get(psInstruction->sResult.Index())->uiUses == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// The first instruction after uiAfter (before uiEnd) that writes something
// (see Skipped); uiEnd if none does.
//
//////////////////////////////////////////////////////////////////////////
size CIR65Backend::NextWritten(CIRBlock* pcBlock, size uiAfter, size uiEnd)
{
	size	uiNext;

	for (uiNext = uiAfter + 1; (uiNext < uiEnd) && Skipped(pcBlock->masInstructions.Get(uiNext)); uiNext++)
	{
	}
	return uiNext;
}


//////////////////////////////////////////////////////////////////////////
//
// A block: its instructions and its jump.  An instruction CanFuse says
// isn't written in its turn is written by the next one written, its
// consumer (WriteInstruction), as the W65C816 back end's fused instructions
// are; the block's jump writes those left.  A comparison used only by the
// block's brif branches directly.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Block(CIRBlock* pcBlock, CIRBlock* pcNext)
{
	size			ui;
	size			uiFirst;
	size			uiText;
	size			uiNumInstructions;
	SIRInstruction*	psFused;
	bool			bResult;

	msAvailable = IRNone();
	msInA = IRNone();
	masFused.FakeSetUsedElements(0);
	Label(*maiBlockLabels.Get(pcBlock->muiIndex));

	psFused = mcPlaces.FusedComparison(pcBlock);
	uiNumInstructions = pcBlock->masInstructions.NumElements();
	if (psFused)
	{
		uiNumInstructions--;
	}
	mbFlagsKnown = (uiNumInstructions == 0) && FlagsKnown(pcBlock, psFused);

	bResult = true;
	uiFirst = 0;
	for (ui = 0; ui < uiNumInstructions; ui++)
	{
		if (Skipped(pcBlock->masInstructions.Get(ui)) || mcFusion.CanFuse(pcBlock, ui))
		{
			//Writes nothing, or its consumer writes it.
			continue;
		}
		uiText = mpszOut->Length();
		bResult &= WriteInstruction(pcBlock, ui);
		mcPlaces.AddChunk(mcLiveness.Position(pcBlock, uiFirst), mcLiveness.Position(pcBlock, ui), uiText);
		uiFirst = ui + 1;
	}
	uiText = mpszOut->Length();
	bResult &= WriteFused(pcBlock, masFused.NumElements());
	NextInstruction();
	Source(psFused ? psFused->uiSource : pcBlock->msJump.uiSource);
	bResult &= Jump(pcBlock, pcNext, psFused);
	mcPlaces.AddChunk(mcLiveness.Position(pcBlock, uiFirst), mcLiveness.Jump(pcBlock), uiText);
	masFused.FakeSetUsedElements(0);
	mbFlagsKnown = false;
	if (psFused && !psFused->sType.IsFloat())
	{
		//Kept until another block compares: a block FlagsKnown allows has only this one before it.
		mpsFlags = psFused;
		mpcFlags = pcBlock;
	}
	msAvailable = IRNone();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Instruction uiIndex, the consumer of the instructions CanFuse left for it
// (masFused): the ones it writes as part of itself first (a sext its load,
// a div's partner the div, a store the operation it stores back: add
// [4 + X], 1), else each written in order and then itself.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::WriteInstruction(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psInstruction;
	SIR65Fused*		psLast;
	size			uiBefore;
	bool			bResult;

	psInstruction = pcBlock->masInstructions.Get(uiIndex);
	psLast = (masFused.NumElements() > 0) ? masFused.Get(masFused.NumElements() - 1) : NULL;
	bResult = true;
	if (psLast)
	{
		uiBefore = masFused.NumElements() - 1;
		if (psLast->uiKind == IR65FU_Extended)
		{
			bResult &= WriteFused(pcBlock, uiBefore);
			Source(psLast->psInstruction->uiSource);
			NextInstruction();
			bResult &= mcSelector.Load(pcBlock, psLast->uiIndex, psInstruction);
			masFused.FakeSetUsedElements(0);
			return bResult;
		}
		if (psLast->uiKind == IR65FU_Paired)
		{
			bResult &= WriteFused(pcBlock, uiBefore);
			Source(psLast->psInstruction->uiSource);
			NextInstruction();
			bResult &= mcSelector.Divide(psLast->psInstruction, psInstruction);
			masFused.FakeSetUsedElements(0);
			return bResult;
		}
		if ((psLast->uiKind == IR65FU_InA) && (psInstruction->uiOpcode == IRO_Store))
		{
			bResult &= WriteFused(pcBlock, uiBefore);
			Source(psLast->psInstruction->uiSource);
			NextInstruction();
			if (mcSelector.ReadModifyWrite(psLast->psInstruction, psInstruction))
			{
				masFused.FakeSetUsedElements(0);
				return bResult;
			}
			bResult &= mcSelector.Instruction(pcBlock, psLast->uiIndex);
			masFused.FakeSetUsedElements(0);
		}
		//What is left (none after an operation the store couldn't write with itself) is written, then the consumer
		//(which may read a deferred load among them).
	}
	bResult &= WriteFused(pcBlock, masFused.NumElements());
	Source(psInstruction->uiSource);
	NextInstruction();
	bResult &= mcSelector.Instruction(pcBlock, uiIndex);
	masFused.FakeSetUsedElements(0);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The first uiCount instructions CanFuse left for the consumer, in order:
// those it takes from A written, a deferred load (read later as an operand)
// nothing.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::WriteFused(CIRBlock* pcBlock, size uiCount)
{
	size		ui;
	SIR65Fused*	psFused;
	bool		bResult;

	bResult = true;
	for (ui = 0; ui < uiCount; ui++)
	{
		psFused = masFused.Get(ui);
		Source(psFused->psInstruction->uiSource);
		if (psFused->uiKind == IR65FU_InA)
		{
			NextInstruction();
			bResult &= mcSelector.Instruction(pcBlock, psFused->uiIndex);
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Before an instruction is written: what the one before left in A is what
// A holds as it starts.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::NextInstruction(void)
{
	msAvailable = msInA;
	msInA = IRNone();
}


//////////////////////////////////////////////////////////////////////////
//
// Branches to iLabel when a brif's condition is bJumpIf: psFused's compare
// if it has one, else the condition tested against 0.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::BranchOn(SIRInstruction* psFused, SIRReference sCondition, bool bJumpIf, int32 iLabel)
{
	if (psFused)
	{
		return mcSelector.Compare(psFused, bJumpIf, iLabel);
	}
	TestZero(sCondition);
	Branch(bJumpIf ? "brac z, " : "bras z, ", iLabel);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if pcBlock (without phis) is only entered by pcFrom's jump, with no
// copies on the way, so it starts with the flags pcFrom left.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::EnteredOnlyFrom(CIRBlock* pcBlock, CIRBlock* pcFrom)
{
	return (pcBlock->masPhis.NumElements() == 0) && (pcBlock->mapcPredecessors.NumElements() == 1) && (pcBlock->mapcPredecessors.GetPtr(0) == pcFrom) && !HasMoves(pcFrom, pcBlock);
}


//////////////////////////////////////////////////////////////////////////
//
// True if sValue is the constant 0.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::IsZero(SIRReference sValue)
{
	return sValue.IsConstant() && (mpcFunction->GetConstant(sValue)->ulliBits == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// True if the flags pcBlock is entered with already give psFused, an eq or
// ne whose block has nothing else in it: its only predecessor's jump
// compared the same operands (cmp leaves z set when they are equal) and
// nothing was copied on the way.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::FlagsKnown(CIRBlock* pcBlock, SIRInstruction* psFused)
{
	if ((psFused == NULL) || (mpsFlags == NULL) || ((psFused->uiOpcode != IRO_Eq) && (psFused->uiOpcode != IRO_Ne)) || !psFused->sType.Equals(mpsFlags->sType) || !EnteredOnlyFrom(pcBlock, mpcFlags))
	{
		return false;
	}
	return SameOperands(psFused, mpsFlags);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the edge from pcFrom to pcTo copies anything (Edge): a phi of
// pcTo's whose value from pcFrom isn't in its place already.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::EdgeCopies(CIRBlock* pcFrom, CIRBlock* pcTo)
{
	size			ui;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;

	for (ui = 0; ui < pcTo->masPhis.NumElements(); ui++)
	{
		psPhi = pcTo->masPhis.Get(ui);
		psArgument = pcTo->FindPhiArgument(psPhi, pcFrom);
		if (!psArgument || ((mcPlaces.PlaceOf(psPhi->sResult)->uiKind != IR65P_None) && !mcPlaces.SamePlace(psArgument->sValue, psPhi->sResult)))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The copies pcTo's phis need from pcFrom, as one parallel copy
// (ParallelCopy): a cycle is broken through B.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Edge(CIRBlock* pcFrom, CIRBlock* pcTo)
{
	size			ui;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	SIRCopy*		psCopy;
	SIRCopyStep*	psStep;

	masCopies.FakeSetUsedElements(0);
	for (ui = 0; ui < pcTo->masPhis.NumElements(); ui++)
	{
		psPhi = pcTo->masPhis.Get(ui);
		psArgument = pcTo->FindPhiArgument(psPhi, pcFrom);
		if (psArgument == NULL)
		{
			Error("Phi has no argument for a predecessor.");
			continue;
		}
		if ((mcPlaces.PlaceOf(psPhi->sResult)->uiKind == IR65P_None) || mcPlaces.SamePlace(psArgument->sValue, psPhi->sResult))
		{
			continue;
		}
		psCopy = masCopies.Add();
		psCopy->sDestination = psPhi->sResult;
		psCopy->sSource = psArgument->sValue;
		psCopy->sType = psPhi->sType;
		psCopy->bSaved = false;
	}

	ParallelCopy(&masCopies, &masCopySteps);
	for (ui = 0; ui < masCopySteps.NumElements(); ui++)
	{
		psStep = masCopySteps.Get(ui);
		if (psStep->bSave)
		{
			mcSelector.LoadValue(psStep->sSource, "B", IR65E_Zero);
		}
		else
		{
			Move(psStep->sDestination, psStep->sSource, psStep->sType, psStep->bSaved);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if writing sDestination's place changes sSource's (ParallelCopy).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::CopyOverlaps(SIRReference sSource, SIRReference sDestination)
{
	return sSource.IsTemporary() && mcPlaces.Overlaps(mcPlaces.PlaceOf(sSource), mcPlaces.PlaceOf(sDestination));
}


//////////////////////////////////////////////////////////////////////////
//
// True if the two values are in one home (ParallelCopy).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::CopySamePlace(SIRReference sValue1, SIRReference sValue2)
{
	return mcPlaces.SamePlace(sValue1, sValue2);
}


//////////////////////////////////////////////////////////////////////////
//
// One phi copy: into a register, from a register or a small constant
// straight to memory, otherwise through A (or from B, for a cycle).
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Move(SIRReference sDestination, SIRReference sSource, SIRType sType, bool bFromB)
{
	SIR65Place*	psDestination;
	CChars*		psz;

	psDestination = mcPlaces.PlaceOf(sDestination);
	if (bFromB)
	{
		mcSelector.StorePlace(psDestination, sType, "B");
	}
	else if (psDestination->uiKind == IR65P_Register)
	{
		mcSelector.LoadValue(sSource, mcPlaces.RegisterName(psDestination), IR65E_Zero);
	}
	else if (mcPlaces.InRegister(sSource) && !IsWide(sType))
	{
		mcSelector.StorePlace(psDestination, sType, mcPlaces.RegisterName(mcPlaces.PlaceOf(sSource)));
	}
	else if (mcSelector.SmallImmediate(sSource, -8, 8))
	{
		psz = Line("cpy ");
		mcSelector.AppendPlace(psz, psDestination);
		psz->Append(", ");
		mcSelector.AppendImmediate(psz, sSource, IR65E_Zero);
		psz->AppendNewLine();
	}
	else
	{
		mcSelector.LoadValue(sSource, "A", IR65E_Zero);
		mcSelector.StorePlace(psDestination, sType, "A");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Sets z from a value: cmp with 0 where it is kept, or in A.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::TestZero(SIRReference sValue)
{
	CChars*		psz;
	SIR65Place*	psPlace;

	if (sValue.IsTemporary() && !mcPlaces.IsInA(sValue))
	{
		psPlace = mcPlaces.PlaceOf(sValue);
		if ((psPlace->uiKind == IR65P_Register) || (psPlace->uiKind == IR65P_Pseudo))
		{
			psz = Line("cmp ");
			mcSelector.AppendPlace(psz, psPlace);
			psz->Append(", 0")->AppendNewLine();
			return;
		}
	}
	mcSelector.LoadValue(sValue, "A", IR65E_Zero);
	Line("cmp A, 0")->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// The block's jump.  A branch tests its condition (or does psFused's
// comparison) and goes to the else block, a target's phis getting their
// values on the way; the then block is fallen through to when it is next.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Jump(CIRBlock* pcBlock, CIRBlock* pcNext, SIRInstruction* psFused)
{
	SIRJump*	psJump;
	int32		iElse;
	bool		bResult;
	bool		bThenMoves;
	bool		bElseMoves;
	SIR65Stub*	psStub;

	psJump = &pcBlock->msJump;
	bResult = true;
	switch (psJump->uiKind)
	{
	case IRJ_Jump:
		Edge(pcBlock, psJump->pcThen);
		if (psJump->pcThen != pcNext)
		{
			Branch("bra ", *maiBlockLabels.Get(psJump->pcThen->muiIndex));
		}
		break;

	case IRJ_Branch:
		if ((pcBlock->masInstructions.NumElements() > 0) && (pcBlock->masInstructions.Tail()->uiOpcode == IRO_Thrown) && pcBlock->masInstructions.Tail()->sResult.Equals(psJump->sArgument))
		{
			//A tested call (IsolateThrowns: a thrown is always this): the call branched on the carry to the code for the
			//way that threw, which is after the function's ret (CIR65Calls::Call).  This is the way that didn't.
			Edge(pcBlock, psJump->pcElse);
			if (psJump->pcElse != pcNext)
			{
				Branch("bra ", *maiBlockLabels.Get(psJump->pcElse->muiIndex));
			}
			break;
		}
		bThenMoves = HasMoves(pcBlock, psJump->pcThen);
		bElseMoves = HasMoves(pcBlock, psJump->pcElse);
		if ((psJump->pcElse == pcNext) && (psJump->pcThen != pcNext) && bThenMoves && !bElseMoves)
		{
			//The then edge's copies go after the function's ret (a loop that goes round when the test is true).
			psStub = masStubs.Add();
			psStub->iLabel = NewLabel();
			psStub->pcFrom = pcBlock;
			psStub->pcTo = psJump->pcThen;
			psStub->iRemove = 0;
			psStub->bTested = false;
			psStub->bWritten = false;
			bResult = BranchOn(psFused, psJump->sArgument, true, psStub->iLabel);
			break;
		}
		if ((psJump->pcElse == pcNext) && (psJump->pcThen != pcNext) && !bThenMoves)
		{
			//Branch to the then block, fall through to the else block.
			bResult = BranchOn(psFused, psJump->sArgument, true, *maiBlockLabels.Get(psJump->pcThen->muiIndex));
			Edge(pcBlock, psJump->pcElse);
			break;
		}
		iElse = bElseMoves ? NewLabel() : *maiBlockLabels.Get(psJump->pcElse->muiIndex);
		bResult = BranchOn(psFused, psJump->sArgument, false, iElse);
		Edge(pcBlock, psJump->pcThen);
		if ((psJump->pcThen != pcNext) || bElseMoves)
		{
			Branch("bra ", *maiBlockLabels.Get(psJump->pcThen->muiIndex));
		}
		if (bElseMoves)
		{
			Label(iElse);
			Edge(pcBlock, psJump->pcElse);
			Branch("bra ", *maiBlockLabels.Get(psJump->pcElse->muiIndex));
		}
		break;

	case IRJ_Return:
		if (!psJump->sType.IsNone())
		{
			mcSelector.LoadValue(psJump->sArgument, "A", IR65E_Zero);
		}
		if (pcNext != NULL)
		{
			if (!mpcOptions->PrefersSize() && (ReturnInstructions() <= IR65_INLINE_RETURN))
			{
				//A short epilogue is written here rather than jumped to (for speed: it costs code).
				ReturnCode();
			}
			else
			{
				Branch("bra ", miReturnLabel);
			}
		}
		break;

	case IRJ_Unreachable:
		Line("stp")->AppendNewLine();
		break;

	case IRJ_Throw:
		//The function's exit with the carry set: the caller's thrown.
		ReturnCode(true);
		break;

	default:
		return Error("Block has no jump.");
	}
	return bResult;
}
