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
#include "IR65BackendInternal.h"


//////////////////////////////////////////////////////////////////////////
//
// piLabels is the counter labels are numbered from (the generator's).  The
// soft float routines' labels the code calls are GetRuntime's.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Init(CIRModule* pcModule, int* piLabels, CScratchPadAllocator* pcArena)
{
	InitBackend(pcModule, piLabels, NULL);
	mcDominators.Init();
	mcLiveness.Init();
	masPlaces.Init();
	mauiAddressed.Init();
	masArguments.Init();
	miRegisterArguments = 0;
	maiClasses.Init();
	maiLeaders.Init();
	masHomes.Init();
	maiCalls.Init();
	maiChangesXY.Init();
	maiChangesY.Init();
	mauiUses.Init();
	masCopies.Init();
	masCopySteps.Init();
	masStubs.Init();
	msFrame.Init();
	miReturnLabel = 0;
	msInA = IRNone();
	msAvailable = IRNone();
	masFused.Init();
	mszDeferred.Init();
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
	mauiNarrowOnly.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Kill(void)
{
	size	ui;

	mauiNarrowOnly.Kill();
	mcPeephole.Kill();
	mcBackendPasses.Kill();
	mcWidths.Kill();
	mcFlags.Kill();
	mszDeferred.Kill();
	masFused.Kill();
	masStubs.Kill();
	masCopySteps.Kill();
	masCopies.Kill();
	mauiUses.Kill();
	maiChangesY.Kill();
	maiChangesXY.Kill();
	maiCalls.Kill();
	for (ui = 0; ui < masHomes.NumElements(); ui++)
	{
		masHomes.Get(ui)->asRanges.Kill();
	}
	masHomes.Kill();
	maiLeaders.Kill();
	maiClasses.Kill();
	masArguments.Kill();
	mauiAddressed.Kill();
	masPlaces.Kill();
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
	size		ui;
	CIRBlock*	pcBlock;
	int32		iLabel;
	size		uiStart;

	mpcFunction = pcFunction;
	mpszOut = pszDest;
	mpcErrors = pcErrors;
	mbResult = true;
	uiStart = pszDest->Length();
	muiSource = 0;

	ExpandMultiplies();
	pcFunction->Renumber();
	pcFunction->ComputePredecessors();
	mpcFunction->CountUses(&mauiUses);
	AdjustComparisons();
	PreferSingleBranches();
	mcBackendPasses.OrderBlocks(pcFunction, false);
	mcBackendPasses.MoveLoopTests(pcFunction);
	mcBackendPasses.SinkLoads(pcFunction);
	mcBackendPasses.SinkSteps(pcFunction);
	mpcFunction->CountUses(&mauiUses);
	CopyAddresses();
	mpcFunction->CountUses(&mauiUses);
	FindNarrowOnly();
	mpcModule->Count(CPPCC_Dominators);
	mcDominators.Compute(pcFunction);
	mcLiveness.Compute(pcFunction, &mcDominators);
	FunctionArguments();
	Layout();
	Allocate();

	//A function with nothing in the frame doesn't set V up when its arguments can be found from S: a leaf (S doesn't
	//move) or one that reads them before pushing anything.  Otherwise V points at the frame.
	msFrame.SetFrameless(msFrame.bHomeV || ((msFrame.iFrameBytes == 0) && (msFrame.iThisOffset < 0) && (IsLeaf() || ArgumentsReadFirst())));

	maiBlockLabels.FakeSetUsedElements(0);
	for (ui = 0; ui < pcFunction->NumBlocks(); ui++)
	{
		iLabel = NewLabel();
		maiBlockLabels.Add(&iLabel);
	}
	miReturnLabel = NewLabel();

	Prologue(szLabel);
	mpsFlags = NULL;
	mpcFlags = NULL;
	for (ui = 0; ui < pcFunction->NumBlocks(); ui++)
	{
		pcBlock = pcFunction->GetBlock(ui);
		if (!Block(pcBlock, (ui + 1 < pcFunction->NumBlocks()) ? pcFunction->GetBlock(ui + 1) : NULL))
		{
			mbResult = false;
		}
	}
	Epilogue();
	if (mbSourceMarkers)
	{
		mcSourceMarkers.Take(pszDest, uiStart);
	}
	//The passes over the body's code: a register function's stub before it (RegisterStub) is as it must be, and
	//the body's label is theirs to start from (the stub's jsr of it is no branch they know).
	mpcModule->Begin(CPPCP_CodePasses);
	Peephole(muiBodyStart);
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
	int32			iUnit;
	SIRReference	sParameter;
	SIRType			sType;
	SIR65Place*		psPlace;
	SIR65Place		sArrival;
	CChars			szBody;

	muiBodyStart = mpszOut->Length();
	mpszOut->Append(szLabel);
	mpszOut->Append(':');
	mpszOut->AppendNewLine();
	if (miRegisterArguments > 0)
	{
		//A register function: the plain label is the stub for callers that push everything, then the body's own label
		//(visible to other objects when the plain one is).
		szBody.Init(szLabel);
		AppendRegisterSuffix(&szBody, &masArguments, IR65_REGISTER_LETTERS);
		RegisterStub(szBody.Text());
		if (!(mpcFunction->muiAttributes & IRA_Local) && !mpcModule->IsLocal(mpcFunction->muiSymbol))
		{
			mpszOut->Append("\t\t.global ");
			mpszOut->Append(szBody);
			mpszOut->AppendNewLine();
		}
		muiBodyStart = mpszOut->Length();
		mpszOut->Append(szBody);
		mpszOut->Append(':');
		mpszOut->AppendNewLine();
		szBody.Kill();
	}
	if (msFrame.bHomeV)
	{
		Line("psh V")->AppendNewLine();
	}
	if (!msFrame.bFrameless)
	{
		Line("psh V")->AppendNewLine();
		if (msFrame.iFrameBytes > 0)
		{
			Line("sub S, ")->Append(msFrame.iFrameBytes)->AppendNewLine();
		}
		Line("cpy V, S")->AppendNewLine();
	}
	for (iUnit = 0; iUnit < IR65_PSEUDO_UNITS; iUnit++)
	{
		if (msFrame.uiSaved & (1u << iUnit))
		{
			Line("psh [")->Append(IR65_SAVED_UNIT(iUnit))->Append(" + W]")->AppendNewLine();
		}
	}

	//The parameters that came in registers first (the pushed ones are read through A).
	StoreRegisterParameters();
	for (ui = 0; ui < mpcFunction->masParameters.NumElements(); ui++)
	{
		sParameter = *mpcFunction->masParameters.Get(ui);
		sType = mpcFunction->GetTemporaryType(sParameter);
		psPlace = PlaceOf(sParameter);
		if (((ui == 0) && (mpcFunction->muiAttributes & IRA_ThisCall)) || (ParameterRegister(ui) >= 0))
		{
			continue;
		}
		if ((psPlace->uiKind == IR65P_Register) || (psPlace->uiKind == IR65P_Pseudo) || (psPlace->uiKind == IR65P_Frame))
		{
			ArrivalPlace(ui, &sArrival);
			if (psPlace->uiKind == IR65P_Register)
			{
				LoadPlace(&sArrival, sType, RegisterName(psPlace), IR65E_Zero);
			}
			else
			{
				LoadPlace(&sArrival, sType, "A", IR65E_Zero);
				StorePlace(psPlace, sType, "A");
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
		StorePlace(psPlace, sType, szRegister);
	}
	else if ((psPlace->uiKind == IR65P_Register) && (StringCompare(RegisterName(psPlace), szRegister) != 0))
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
			iRegister = ParameterRegister(ui);
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
		StoreArrivedRegister(szRegister, PlaceOf(sParameter), mpcFunction->GetTemporaryType(sParameter), aszSources, apsPlaces, &iMoves);
	}

	//Into X and Y: one nothing still to be copied reads goes first.
	while (iMoves > 0)
	{
		for (i = 0; i < iMoves; i++)
		{
			bRead = false;
			for (j = 0; j < iMoves; j++)
			{
				bRead |= (j != i) && (StringCompare(aszSources[j], RegisterName(apsPlaces[i])) == 0);
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
		Line("cpy ")->Append(RegisterName(apsPlaces[i]))->Append(", ")->Append(aszSources[i])->AppendNewLine();
		iMoves--;
		aszSources[i] = aszSources[iMoves];
		apsPlaces[i] = apsPlaces[iMoves];
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A register function's plain label, for callers that push every argument
// (function pointers, vtables, thunks, assembly): on entry the arguments
// are at [5 + S] on, as they were pushed (a thiscall's this is in X either
// way).  Those the body wants in A, B and Y are loaded from there and it
// goes straight on to the body (szBody), which follows.  When it also takes
// arguments on the stack they must be just above its return address: they
// are pushed again (through A, before it is loaded), the body is called,
// they are removed and it returns.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::RegisterStub(const char* szBody)
{
	static const char*	aszRegisters[IR65_ARGUMENT_REGISTERS] = { "A", "B", "Y" };
	SIR65Frame			sFrame;
	SIR65Place			sArrival;
	SIRType				sType;
	size				ui;
	size				uiFirst;
	int32				iOffset;
	int32				iPushed;
	int					iBytes;
	int					i;

	//The arguments as they were all pushed, from S.
	sFrame = msFrame;
	msFrame.cRegister = 'S';
	msFrame.iArguments = 1 + 4;
	uiFirst = (mpcFunction->muiAttributes & IRA_ThisCall) ? 1 : 0;
	sArrival.uiKind = IR65P_Argument;
	sArrival.bInA = false;

	//The pushed ones again, last first.
	iPushed = 0;
	for (ui = mpcFunction->masParameters.NumElements(); ui > uiFirst; ui--)
	{
		if (ParameterRegister(ui - 1) >= 0)
		{
			continue;
		}
		sType = mpcFunction->GetTemporaryType(*mpcFunction->masParameters.Get(ui - 1));
		iBytes = ValueBytes(sType);
		iOffset = 0;
		for (i = (int)uiFirst; i < (int)(ui - 1); i++)
		{
			iOffset += ValueBytes(mpcFunction->GetTemporaryType(*mpcFunction->masParameters.Get(i)));
		}
		sArrival.iOffset = iOffset + iPushed;
		sArrival.iBytes = iBytes;
		LoadPlace(&sArrival, sType, "A", IR65E_Zero);
		if (iBytes != 4)
		{
			Line("wth A, ")->Append(8 * iBytes)->AppendNewLine();
		}
		Line("psh A")->AppendNewLine();
		if (iBytes != 4)
		{
			Line("wth A, 32")->AppendNewLine();
		}
		iPushed += iBytes;
	}

	//The registers, A last.
	for (i = IR65_ARGUMENT_REGISTERS - 1; i >= 0; i--)
	{
		iOffset = 0;
		for (ui = uiFirst; ui < mpcFunction->masParameters.NumElements(); ui++)
		{
			sType = mpcFunction->GetTemporaryType(*mpcFunction->masParameters.Get(ui));
			if (ParameterRegister(ui) == i)
			{
				sArrival.iOffset = iOffset + iPushed;
				sArrival.iBytes = ValueBytes(sType);
				LoadPlace(&sArrival, sType, aszRegisters[i], IR65E_Zero);
			}
			iOffset += ValueBytes(sType);
		}
	}
	msFrame = sFrame;

	if (iPushed > 0)
	{
		Line("jsr ")->Append(szBody)->AppendNewLine();
		Line("add S, ")->Append(iPushed)->AppendNewLine();
		Line("ret")->AppendNewLine();
	}
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
	int32		iUnit;
	size		ui;
	SIR65Stub*	psStub;

	Label(miReturnLabel);
	for (iUnit = IR65_PSEUDO_UNITS - 1; iUnit >= 0; iUnit--)
	{
		if (msFrame.uiSaved & (1u << iUnit))
		{
			Line("pul [")->Append(IR65_SAVED_UNIT(iUnit))->Append(" + W]")->AppendNewLine();
		}
	}
	if (!msFrame.bFrameless)
	{
		if (msFrame.iFrameBytes > 0)
		{
			Line("add S, ")->Append(msFrame.iFrameBytes)->AppendNewLine();
		}
		Line("pul V")->AppendNewLine();
	}
	if (msFrame.bHomeV)
	{
		Line("pul V")->AppendNewLine();
	}
	Line("ret")->AppendNewLine();

	//Edges whose copies were moved out of the way.
	for (ui = 0; ui < masStubs.NumElements(); ui++)
	{
		psStub = masStubs.Get(ui);
		Label(psStub->iLabel);
		Edge(psStub->pcFrom, psStub->pcTo);
		Branch("bra ", *maiBlockLabels.Get(psStub->pcTo->muiIndex));
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
// Calls a soft float routine (_ and its name) and removes iBytes of
// arguments.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::CallRuntime(const char* szName, int iBytes)
{
	CChars	szLabel;

	szLabel.Init("_");
	szLabel.Append(szName);
	UseRuntime(szLabel.Text());
	Line("jsr ")->Append(szLabel)->AppendNewLine();
	szLabel.Kill();
	RemoveArguments(iBytes);
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
		if (psArgument && (PlaceOf(psPhi->sResult)->uiKind != IR65P_None) && !SamePlace(psArgument->sValue, psPhi->sResult))
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
	return (PlaceOf(psInstruction->sResult)->uiKind == IR65P_Folded) || (*mauiUses.Get(psInstruction->sResult.Index()) == 0);
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
	size			uiNumInstructions;
	SIRInstruction*	psFused;
	bool			bResult;

	msAvailable = IRNone();
	msInA = IRNone();
	masFused.FakeSetUsedElements(0);
	Label(*maiBlockLabels.Get(pcBlock->muiIndex));

	psFused = FusedComparison(pcBlock);
	uiNumInstructions = pcBlock->masInstructions.NumElements();
	if (psFused)
	{
		uiNumInstructions--;
	}
	mbFlagsKnown = (uiNumInstructions == 0) && FlagsKnown(pcBlock, psFused);

	bResult = true;
	for (ui = 0; ui < uiNumInstructions; ui++)
	{
		if (Skipped(pcBlock->masInstructions.Get(ui)) || CanFuse(pcBlock, ui))
		{
			//Writes nothing, or its consumer writes it.
			continue;
		}
		bResult &= WriteInstruction(pcBlock, ui);
	}
	bResult &= WriteFused(pcBlock, masFused.NumElements());
	NextInstruction();
	Source(psFused ? psFused->uiSource : pcBlock->msJump.uiSource);
	bResult &= Jump(pcBlock, pcNext, psFused);
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
			bResult &= Load(pcBlock, psLast->uiIndex, psInstruction);
			masFused.FakeSetUsedElements(0);
			return bResult;
		}
		if (psLast->uiKind == IR65FU_Paired)
		{
			bResult &= WriteFused(pcBlock, uiBefore);
			Source(psLast->psInstruction->uiSource);
			NextInstruction();
			bResult &= Divide(psLast->psInstruction, psInstruction);
			masFused.FakeSetUsedElements(0);
			return bResult;
		}
		if ((psLast->uiKind == IR65FU_InA) && (psInstruction->uiOpcode == IRO_Store))
		{
			bResult &= WriteFused(pcBlock, uiBefore);
			Source(psLast->psInstruction->uiSource);
			NextInstruction();
			if (ReadModifyWrite(psLast->psInstruction, psInstruction))
			{
				masFused.FakeSetUsedElements(0);
				return bResult;
			}
			bResult &= Instruction(pcBlock, psLast->uiIndex);
			masFused.FakeSetUsedElements(0);
		}
		//What is left (none after an operation the store couldn't write with itself) is written, then the consumer
		//(which may read a deferred load among them).
	}
	bResult &= WriteFused(pcBlock, masFused.NumElements());
	Source(psInstruction->uiSource);
	NextInstruction();
	bResult &= Instruction(pcBlock, uiIndex);
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
			bResult &= Instruction(pcBlock, psFused->uiIndex);
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
		return Compare(psFused, bJumpIf, iLabel);
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
		if ((PlaceOf(psPhi->sResult)->uiKind == IR65P_None) || SamePlace(psArgument->sValue, psPhi->sResult))
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
			LoadValue(psStep->sSource, "B", IR65E_Zero);
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
	return sSource.IsTemporary() && Overlaps(PlaceOf(sSource), PlaceOf(sDestination));
}


//////////////////////////////////////////////////////////////////////////
//
// True if the two values are in one home (ParallelCopy).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::CopySamePlace(SIRReference sValue1, SIRReference sValue2)
{
	return SamePlace(sValue1, sValue2);
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

	psDestination = PlaceOf(sDestination);
	if (bFromB)
	{
		StorePlace(psDestination, sType, "B");
	}
	else if (psDestination->uiKind == IR65P_Register)
	{
		LoadValue(sSource, RegisterName(psDestination), IR65E_Zero);
	}
	else if (InRegister(sSource) && !IsWide(sType))
	{
		StorePlace(psDestination, sType, RegisterName(PlaceOf(sSource)));
	}
	else if (SmallImmediate(sSource, -8, 8))
	{
		psz = Line("cpy ");
		AppendPlace(psz, psDestination);
		psz->Append(", ");
		AppendImmediate(psz, sSource, IR65E_Zero);
		psz->AppendNewLine();
	}
	else
	{
		LoadValue(sSource, "A", IR65E_Zero);
		StorePlace(psDestination, sType, "A");
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

	if (sValue.IsTemporary() && !IsInA(sValue))
	{
		psPlace = PlaceOf(sValue);
		if ((psPlace->uiKind == IR65P_Register) || (psPlace->uiKind == IR65P_Pseudo))
		{
			psz = Line("cmp ");
			AppendPlace(psz, psPlace);
			psz->Append(", 0")->AppendNewLine();
			return;
		}
	}
	LoadValue(sValue, "A", IR65E_Zero);
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
		bThenMoves = HasMoves(pcBlock, psJump->pcThen);
		bElseMoves = HasMoves(pcBlock, psJump->pcElse);
		if ((psJump->pcElse == pcNext) && (psJump->pcThen != pcNext) && bThenMoves && !bElseMoves)
		{
			//The then edge's copies go after the function's ret (a loop that goes round when the test is true).
			psStub = masStubs.Add();
			psStub->iLabel = NewLabel();
			psStub->pcFrom = pcBlock;
			psStub->pcTo = psJump->pcThen;
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
			LoadValue(psJump->sArgument, "A", IR65E_Zero);
		}
		if (pcNext != NULL)
		{
			Branch("bra ", miReturnLabel);
		}
		break;

	case IRJ_Unreachable:
		Line("stp")->AppendNewLine();
		break;

	default:
		return Error("Block has no jump.");
	}
	return bResult;
}
