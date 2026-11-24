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


//The 65C64 back end's frame layout and register allocation: where each temporary is kept (Layout, Allocate).


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int CompareIR65Weights(const void* pv1, const void* pv2)
{
	SIR65Weight*	ps1;
	SIR65Weight*	ps2;

	ps1 = (SIR65Weight*)pv1;
	ps2 = (SIR65Weight*)pv2;
	if (ps1->uiWeight != ps2->uiWeight)
	{
		return (ps1->uiWeight > ps2->uiWeight) ? -1 : 1;
	}
	return ps1->iLeader - ps2->iLeader;
}


//////////////////////////////////////////////////////////////////////////
//
// An empty frame, before Layout and Allocate fill it.
//
//////////////////////////////////////////////////////////////////////////
void SIR65Frame::Init(void)
{
	int		i;

	iFrameBytes = 0;
	iThisOffset = -1;
	uiSaved = 0;
	bHomeV = false;
	bFrameless = false;
	cRegister = 'V';
	iArguments = 0;
	for (i = 0; i < IR65_ARGUMENT_REGISTERS; i++)
	{
		aiRegisterOffset[i] = -1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// iBytes more of the frame, at least 4 and a multiple of 4.
//
//////////////////////////////////////////////////////////////////////////
int32 SIR65Frame::Add(int32 iBytes)
{
	int32	iOffset;

	iOffset = iFrameBytes;
	iFrameBytes += (iBytes < 4) ? 4 : ((iBytes + 3) & ~3);
	return iOffset;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether V is set up (GenerateFunction decides), and so the frame register
// and where the arguments start: past the frame and the link, or from S past
// the return address, the saved pseudo registers and a saved V.
//
//////////////////////////////////////////////////////////////////////////
void SIR65Frame::SetFrameless(bool bFramelessIn)
{
	bFrameless = bFramelessIn;
	cRegister = bFrameless ? 'S' : 'V';
	iArguments = bFrameless ? (1 + 4 + 4 * NumSaved() + (bHomeV ? 4 : 0)) : (1 + iFrameBytes + 8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Places::Init(CIR65Backend* pcBackend)
{
	mpcBackend = pcBackend;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Places::Kill(void)
{
	mpcBackend = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The comparison a block's brif alone uses, which branches without making
// its i1; NULL for none.
//
//////////////////////////////////////////////////////////////////////////
SIRInstruction* CIR65Places::FusedComparison(CIRBlock* pcBlock)
{
	SIRInstruction*	psLast;

	psLast = mpcBackend->BranchComparison(pcBlock);
	if (psLast && (mpcBackend->masTemporaries.Get(psLast->sResult.Index())->uiUses == 1))
	{
		return psLast;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A result is left in A, and not kept, when the next instruction (or the
// block's jump) is its only use and takes it from A.  A comparison its
// block's brif fuses has no result either.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Places::MarkInA(void)
{
	size			ui;
	size			uiInstruction;
	size			uiNext;
	size			uiNumInstructions;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRInstruction*	psNext;
	SIRInstruction*	psFused;
	SIRReference	sResult;
	bool			bInA;

	for (ui = 0; ui < mpcBackend->mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcBackend->mpcFunction->GetBlock(ui);
		psFused = FusedComparison(pcBlock);
		uiNumInstructions = pcBlock->masInstructions.NumElements();
		if (psFused)
		{
			mpcBackend->masTemporaries.Get(psFused->sResult.Index())->sPlace.bInA = true;
			uiNumInstructions--;
		}
		for (uiInstruction = 0; uiInstruction < uiNumInstructions; uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			sResult = psInstruction->sResult;
			if (!sResult.IsTemporary() || (mpcBackend->masTemporaries.Get(sResult.Index())->uiUses != 1) || mpcBackend->Skipped(psInstruction))
			{
				continue;
			}

			//The next instruction that writes something.
			uiNext = mpcBackend->NextWritten(pcBlock, uiInstruction, uiNumInstructions);
			if (uiNext < uiNumInstructions)
			{
				psNext = pcBlock->masInstructions.Get(uiNext);
				bInA = mpcBackend->mcSelector.TakesInA(psNext, sResult) || mpcBackend->mcSelector.TakesInB(psNext, sResult) || ((psNext->uiOpcode == IRO_Load) && psNext->asArguments[0].Equals(sResult)) || mpcBackend->mcFolds.FoldedBase(psNext, sResult);
			}
			else
			{
				bInA = mpcBackend->mcSelector.JumpTakesInA(pcBlock, psFused, sResult) || (psFused && mpcBackend->mcSelector.TakesInB(psFused, sResult)) || (psFused && mpcBackend->mcFolds.FoldedBase(psFused, sResult));
			}
			mpcBackend->masTemporaries.Get(sResult.Index())->sPlace.bInA = bInA;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Where a parameter the caller pushed starts, from the first argument.
//
//////////////////////////////////////////////////////////////////////////
int32 CIR65Places::ParameterOffset(size uiParameter)
{
	size	ui;
	int32	iOffset;

	iOffset = 0;
	for (ui = 0; ui < uiParameter; ui++)
	{
		if (((ui == 0) && (mpcBackend->mpcFunction->muiAttributes & IRA_ThisCall)) || (ParameterRegister(ui) >= 0))
		{
			continue;
		}
		iOffset += mpcBackend->ValueBytes(mpcBackend->mpcFunction->GetTemporaryType(*mpcBackend->mpcFunction->masParameters.Get(ui)));
	}
	return iOffset;
}


//////////////////////////////////////////////////////////////////////////
//
// The register units an argument of sType takes: one for 32 bits or less,
// two for 64 (which are pushed: only one unit goes in a register).
//
//////////////////////////////////////////////////////////////////////////
int CIR65Places::ArgumentUnits(SIRType sType)
{
	return (mpcBackend->ValueBytes(sType) + 3) / 4;
}


//////////////////////////////////////////////////////////////////////////
//
// The function's parameters as its callers pass them (masArguments, less a
// thiscall's this, which is in X): the first three of 32 bits or less in A,
// B and Y when its symbol is marked for register arguments and it isn't
// variadic (CIRBackend::AssignRegisters), the rest pushed.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Places::FunctionArguments(void)
{
	SIRRegisterArgument*	psArgument;
	size					ui;

	mpcBackend->masArguments.FakeSetUsedElements(0);
	for (ui = (mpcBackend->mpcFunction->muiAttributes & IRA_ThisCall) ? 1 : 0; ui < mpcBackend->mpcFunction->masParameters.NumElements(); ui++)
	{
		psArgument = mpcBackend->masArguments.Add();
		psArgument->iUnits = ArgumentUnits(mpcBackend->mpcFunction->GetTemporaryType(*mpcBackend->mpcFunction->masParameters.Get(ui)));
		psArgument->iRegister = -1;
	}
	mpcBackend->miRegisterArguments = 0;
	if (mpcBackend->IsRegisterCall(IRReference(IRRK_Symbol, mpcBackend->mpcFunction->muiSymbol)) && !(mpcBackend->mpcFunction->muiAttributes & IRA_Variadic))
	{
		mpcBackend->miRegisterArguments = mpcBackend->AssignRegisters(&mpcBackend->masArguments, IR65_ARGUMENT_REGISTERS, 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The register a parameter arrives in (0 A, 1 B, 2 Y), or -1 when it was
// pushed (or is a thiscall's this, in X).
//
//////////////////////////////////////////////////////////////////////////
int CIR65Places::ParameterRegister(size uiParameter)
{
	size	uiFirst;

	uiFirst = (mpcBackend->mpcFunction->muiAttributes & IRA_ThisCall) ? 1 : 0;
	if ((uiParameter < uiFirst) || (uiParameter - uiFirst >= mpcBackend->masArguments.NumElements()))
	{
		return -1;
	}
	return mpcBackend->masArguments.Get(uiParameter - uiFirst)->iRegister;
}


//////////////////////////////////////////////////////////////////////////
//
// Where the caller pushed a parameter.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Places::ArrivalPlace(size uiParameter, SIR65Place* psPlace)
{
	psPlace->uiKind = IR65P_Argument;
	psPlace->iOffset = ParameterOffset(uiParameter);
	psPlace->iBytes = mpcBackend->ValueBytes(mpcBackend->mpcFunction->GetTemporaryType(*mpcBackend->mpcFunction->masParameters.Get(uiParameter)));
	psPlace->bInA = false;
}


//////////////////////////////////////////////////////////////////////////
//
// Slots get frame space (their temporary is its address) and an argaddr is
// where the caller pushed its parameter (a thiscall's this, and a parameter
// that came in a register, get a frame place, stored from the register, for
// it).  Results left in A are marked.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Places::Layout(void)
{
	size			ui;
	size			uiInstruction;
	int32			iParameter;
	int				iRegister;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIR65Place*		psPlace;

	mpcBackend->GrowTemporaries();
	for (ui = 0; ui < mpcBackend->masTemporaries.NumElements(); ui++)
	{
		memset(&mpcBackend->masTemporaries.Get(ui)->sPlace, 0, sizeof(SIR65Place));
	}
	mpcBackend->mauiAddressed.FakeSetUsedElements(0);
	mpcBackend->mauiAddressed.GrowToAtLeastNumElements(mpcBackend->mpcFunction->masParameters.NumElements(), true, 0);
	mpcBackend->msFrame.Init();
	mpcBackend->mcFolds.MarkFolded();
	MarkInA();

	for (ui = 0; ui < mpcBackend->mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcBackend->mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if (!psInstruction->sResult.IsTemporary())
			{
				continue;
			}
			psPlace = &mpcBackend->masTemporaries.Get(psInstruction->sResult.Index())->sPlace;
			if (psInstruction->uiOpcode == IRO_Slot)
			{
				psPlace->uiKind = IR65P_FrameAddress;
				psPlace->iBytes = (int32)mpcBackend->mpcFunction->GetConstant(psInstruction->asArguments[0])->ulliBits;
				psPlace->iOffset = mpcBackend->msFrame.Add(psPlace->iBytes);
			}
			else if (psInstruction->uiOpcode == IRO_ArgAddress)
			{
				iParameter = (int32)mpcBackend->mpcFunction->GetConstant(psInstruction->asArguments[0])->ulliBits;
				if ((size)iParameter >= mpcBackend->mpcFunction->masParameters.NumElements())
				{
					mpcBackend->Error("argaddr of a parameter the function doesn't have.");
				}
				else if ((iParameter == 0) && (mpcBackend->mpcFunction->muiAttributes & IRA_ThisCall))
				{
					if (mpcBackend->msFrame.iThisOffset < 0)
					{
						mpcBackend->msFrame.iThisOffset = mpcBackend->msFrame.Add(4);
					}
					psPlace->uiKind = IR65P_FrameAddress;
					psPlace->iOffset = mpcBackend->msFrame.iThisOffset;
				}
				else if (ParameterRegister((size)iParameter) >= 0)
				{
					//A parameter that came in a register is kept in the frame for its address (the prologue stores it).
					iRegister = ParameterRegister((size)iParameter);
					if (mpcBackend->msFrame.aiRegisterOffset[iRegister] < 0)
					{
						mpcBackend->msFrame.aiRegisterOffset[iRegister] = mpcBackend->msFrame.Add(4);
					}
					psPlace->uiKind = IR65P_FrameAddress;
					psPlace->iOffset = mpcBackend->msFrame.aiRegisterOffset[iRegister];
				}
				else
				{
					psPlace->uiKind = IR65P_ArgumentAddress;
					psPlace->iOffset = ParameterOffset((size)iParameter);
					*mpcBackend->mauiAddressed.Get((size)iParameter) = 1;
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if a temporary needs a home: it is used, isn't an address and isn't
// left in A.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::NeedsHome(int32 iTemporary)
{
	SIR65Place*		psPlace;
	SIRReference	sTemporary;

	psPlace = &mpcBackend->masTemporaries.Get(iTemporary)->sPlace;
	sTemporary = IRReference(IRRK_Temporary, (uint32)iTemporary);
	return (psPlace->uiKind == IR65P_None) && !psPlace->bInA && (mpcBackend->masTemporaries.Get(iTemporary)->uiUses > 0) && (mpcBackend->mcLiveness.NumRanges(sTemporary) > 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Gives every temporary that needs one a home.  Phis and the values they
// take share a class (and a home) where they don't interfere; classes are
// given homes most weighted first (a use in a loop counts 8 times more): X
// or Y if nothing that changes them is done while the class is live, then
// a low pseudo register if no call is made.  A parameter used at most once,
// or that gets none of those, stays where it was pushed.  Then a kept
// pseudo register if the class weighs 8 or more and there is no frame yet
// (it costs a push and a pull, the frame's place nothing more), then the
// frame.  A function that needs V for nothing (CanUseV) first tries with
// V as a third register, for pointers and i32s (Fits32BitRegister): calls
// keep it (every function keeps V), so it is where one live across calls
// goes; that stands if V was taken and nothing had to go in the frame.  W
// is a fourth for those between calls, multiplies and divides, which
// change it as they do Y.  B is a fifth (mbHomeB, after W) for one live
// across no code that works in B (maiChangesB, from the code written
// without it) nor where B was lost the last time (maiLosesB; CheckHomeB).
//
//////////////////////////////////////////////////////////////////////////
void CIR65Places::Allocate(void)
{
	CArrayIR65Weights			asWeights;
	SIR65Weight*				psWeight;
	size						ui;
	int32						iTemporary;
	int32						iMember;
	int32						iNumTemporaries;

	ClearHomes();
	FindChanges();

	iNumTemporaries = (int32)mpcBackend->mpcFunction->NumTemporaries();
	mpcBackend->GrowTemporaries();
	for (iTemporary = 0; iTemporary < iNumTemporaries; iTemporary++)
	{
		mpcBackend->masTemporaries.Get(iTemporary)->iNextInClass = -1;
		mpcBackend->masTemporaries.Get(iTemporary)->iLeader = iTemporary;
	}
	Coalesce();

	asWeights.Init();
	for (iTemporary = 0; iTemporary < iNumTemporaries; iTemporary++)
	{
		if (!NeedsHome(iTemporary) || (Find(iTemporary) != iTemporary))
		{
			continue;
		}
		psWeight = asWeights.Add();
		psWeight->iLeader = iTemporary;
		psWeight->uiWeight = 0;
		for (iMember = iTemporary; iMember >= 0; iMember = mpcBackend->masTemporaries.Get(iMember)->iNextInClass)
		{
			psWeight->uiWeight += mpcBackend->mcLiveness.GetWeight(IRReference(IRRK_Temporary, (uint32)iMember));
		}
	}
	DiscountAddresses(&asWeights);
	asWeights.QuickSort(CompareIR65Weights);
	mpcBackend->msFrame.bHomeV = CanUseV();
	mpcBackend->muiWeightV = 0;
	mpcBackend->mbFrameOnly = false;
	if (!AllocateClasses(&asWeights) || (mpcBackend->msFrame.bHomeV && !PaysV()))
	{
		//With V the frame's register after all.  If it kept anything in a kept pseudo register there will be a frame:
		//that is where those go.
		mpcBackend->mbFrameOnly = mpcBackend->msFrame.bHomeV && (mpcBackend->msFrame.uiSaved != 0);
		ClearHomes();
		for (ui = 0; ui < asWeights.NumElements(); ui++)
		{
			PlaceClass(asWeights.Get(ui)->iLeader, IR65P_None, 0, 0);
		}
		mpcBackend->msFrame.uiSaved = 0;
		mpcBackend->msFrame.bHomeV = false;
		AllocateClasses(&asWeights);
	}
	asWeights.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether V as a home was worth it, once every class has one: something
// took it, and what it saves there (IR65_REGISTER_GAIN a use: muiWeightV)
// is more than keeping what a frame would hold in kept pseudo registers
// costs (IR65_KEPT_COST each, against the frame's 9 cycles less V's 6).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::PaysV(void)
{
	int		iSaved;

	if (mpcBackend->masHomes.Get(IR65R_V)->asRanges.NumElements() == 0)
	{
		return false;
	}
	iSaved = mpcBackend->msFrame.NumSaved();
	return (iSaved == 0) || (IR65_REGISTER_GAIN * (int64)mpcBackend->muiWeightV > (int64)IR65_KEPT_COST * iSaved - 3);
}


//////////////////////////////////////////////////////////////////////////
//
// Gives each class a home, most weighted first.  False if one had to go in
// the frame while V is a home (it can't be both).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::AllocateClasses(CArrayIR65Weights* pasWeights)
{
	SIR65Weight*	psWeight;
	size			ui;

	for (ui = 0; ui < pasWeights->NumElements(); ui++)
	{
		psWeight = pasWeights->Get(ui);
		if (!AllocateClass(psWeight->iLeader, psWeight->uiWeight))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// No home is taken: the registers and the pseudo registers, and no frame
// homes.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Places::ClearHomes(void)
{
	SIR65Home*	psHome;
	size		ui;

	for (ui = 0; ui < mpcBackend->masHomes.NumElements(); ui++)
	{
		mpcBackend->masHomes.Get(ui)->asRanges.Kill();
	}
	mpcBackend->masHomes.FakeSetUsedElements(0);
	for (ui = 0; ui < IR65_HOME_FRAME; ui++)
	{
		psHome = mpcBackend->masHomes.Add();
		psHome->asRanges.Init();
		psHome->iOffset = -1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the function needs V for nothing, so V can be a home
// (CCPPCompileOptions::mbHomeV): Layout put nothing in the frame (no
// slot, no parameter whose address is taken) and it isn't variadic.  Its
// pushed parameters are then all read by the prologue, from S, into homes
// (AllocateClass leaves none where it was pushed).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::CanUseV(void)
{
	size	ui;

	if (!mpcBackend->mpcOptions->mbHomeV || (mpcBackend->msFrame.iFrameBytes != 0) || (mpcBackend->msFrame.iThisOffset >= 0) || (mpcBackend->mpcFunction->muiAttributes & IRA_Variadic))
	{
		return false;
	}
	for (ui = 0; ui < mpcBackend->mpcFunction->masParameters.NumElements(); ui++)
	{
		if (*mpcBackend->mauiAddressed.Get(ui))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if every temporary of the class is a pointer or an i32: V and W
// hold those.  They are always 32 bits wide (wth, wthx and cpyx don't take
// them: WidthRegister), so not a narrow value, which is compared at its
// own width in its register.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::Fits32BitRegister(int32 iLeader)
{
	int32	iMember;
	SIRType	sType;

	for (iMember = iLeader; iMember >= 0; iMember = mpcBackend->masTemporaries.Get(iMember)->iNextInClass)
	{
		sType = mpcBackend->mpcFunction->GetTemporaryType(IRReference(IRRK_Temporary, (uint32)iMember));
		if (!sType.IsPointer() && !sType.Is(IRTK_I32))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer in memory costs nothing more than one in a register where it is
// a load's or store's address ([[n + W]]) or stepped by a small constant in
// its own class (add [n + W], 1), so those uses (and the step's result)
// don't count towards its class's weight.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Places::DiscountAddresses(CArrayIR65Weights* pasWeights)
{
	CArrayTemplate<uint32>	auiDiscounts;	//Per class leader.
	size					ui;
	size					uiInstruction;
	CIRBlock*				pcBlock;
	SIRInstruction*			psInstruction;
	SIRReference			sAddress;
	SIR65Weight*			psWeight;
	uint32					uiUse;
	int32					iLeader;
	uint32*					puiDiscount;

	auiDiscounts.Init();
	auiDiscounts.GrowToAtLeastNumElements(mpcBackend->mpcFunction->NumTemporaries(), true, 0);
	for (ui = 0; ui < mpcBackend->mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcBackend->mpcFunction->GetBlock(ui);
		uiUse = CIRLoops::Weight(mpcBackend->mcLiveness.LoopDepth(pcBlock), IR_LOOP_WEIGHT_MAX_DEPTH);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			sAddress = IRAddressOf(psInstruction);
			if (sAddress.IsTemporary() && (PlaceOf(sAddress)->uiKind != IR65P_Folded) && !psInstruction->asArguments[0].Equals(psInstruction->asArguments[1]))
			{
				*auiDiscounts.Get((size)Find((int32)sAddress.Index())) += uiUse;
			}
			else if ((psInstruction->uiOpcode == IRO_PtrAdd) && psInstruction->asArguments[0].IsTemporary() && psInstruction->sResult.IsTemporary() &&
					 mpcBackend->mcSelector.SmallImmediate(psInstruction->asArguments[1], -8, 8) && (Find((int32)psInstruction->asArguments[0].Index()) == Find((int32)psInstruction->sResult.Index())))
			{
				*auiDiscounts.Get((size)Find((int32)psInstruction->sResult.Index())) += 2 * uiUse;
			}
		}
	}
	for (ui = 0; ui < pasWeights->NumElements(); ui++)
	{
		psWeight = pasWeights->Get(ui);
		iLeader = psWeight->iLeader;
		puiDiscount = auiDiscounts.Get((size)iLeader);
		psWeight->uiWeight = (psWeight->uiWeight > *puiDiscount) ? (psWeight->uiWeight - *puiDiscount) : 1;
	}
	auiDiscounts.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The positions of instructions that change X and Y (and the low pseudo
// registers) behind the lowering's back: calls (and the runtime routines
// that are called), mul and div.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Places::FindChanges(void)
{
	size			ui;
	size			uiInstruction;
	CIRBlock*		pcBlock;
	int32			iPosition;
	uint8			uiFlags;

	mpcBackend->maiCalls.FakeSetUsedElements(0);
	mpcBackend->maiChangesXY.FakeSetUsedElements(0);
	mpcBackend->maiChangesY.FakeSetUsedElements(0);
	for (ui = 0; ui < mpcBackend->mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcBackend->mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			iPosition = mpcBackend->mcLiveness.Position(pcBlock, uiInstruction);
			uiFlags = IR65OpcodeFlags(pcBlock->masInstructions.Get(uiInstruction)->uiOpcode);
			if (uiFlags & IR65F_CALLS)
			{
				mpcBackend->maiCalls.Add(&iPosition);
			}
			else if (uiFlags & IR65F_CHANGES_XY)
			{
				mpcBackend->maiChangesXY.Add(&iPosition);
			}
			else if (uiFlags & IR65F_CHANGES_Y)
			{
				mpcBackend->maiChangesY.Add(&iPosition);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Puts each phi in the class of each value it takes that needs a home, if
// they don't interfere.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Places::Coalesce(void)
{
	size			ui;
	size			uiPhi;
	size			uiArgument;
	CIRBlock*		pcBlock;
	SIRPhi*			psPhi;
	SIRReference	sValue;
	int32			iPhi;
	int32			iValue;

	for (ui = 0; ui < mpcBackend->mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcBackend->mpcFunction->GetBlock(ui);
		for (uiPhi = 0; uiPhi < pcBlock->masPhis.NumElements(); uiPhi++)
		{
			psPhi = pcBlock->masPhis.Get(uiPhi);
			if (!NeedsHome((int32)psPhi->sResult.Index()))
			{
				continue;
			}
			for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
			{
				sValue = psPhi->asArguments.Get(uiArgument)->sValue;
				if (!sValue.IsTemporary() || !NeedsHome((int32)sValue.Index()))
				{
					continue;
				}
				iPhi = Find((int32)psPhi->sResult.Index());
				iValue = Find((int32)sValue.Index());
				if ((iPhi != iValue) && !Interferes(iPhi, iValue))
				{
					Join(iPhi, iValue);
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The first temporary of iTemporary's class.
//
//////////////////////////////////////////////////////////////////////////
int32 CIR65Places::Find(int32 iTemporary)
{
	return mpcBackend->masTemporaries.Get(iTemporary)->iLeader;
}


//////////////////////////////////////////////////////////////////////////
//
// True if any temporary of one class is live where one of the other is.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::Interferes(int32 iLeader1, int32 iLeader2)
{
	int32			iMember1;
	int32			iMember2;
	SIRReference	sMember1;
	SIRReference	sMember2;

	for (iMember1 = iLeader1; iMember1 >= 0; iMember1 = mpcBackend->masTemporaries.Get(iMember1)->iNextInClass)
	{
		sMember1 = IRReference(IRRK_Temporary, (uint32)iMember1);
		for (iMember2 = iLeader2; iMember2 >= 0; iMember2 = mpcBackend->masTemporaries.Get(iMember2)->iNextInClass)
		{
			sMember2 = IRReference(IRRK_Temporary, (uint32)iMember2);
			if (CIRLiveness::Intersect(mpcBackend->mcLiveness.GetRanges(sMember1), mpcBackend->mcLiveness.NumRanges(sMember1), mpcBackend->mcLiveness.GetRanges(sMember2), mpcBackend->mcLiveness.NumRanges(sMember2)))
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// iLeader2's class joins the end of iLeader1's.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Places::Join(int32 iLeader1, int32 iLeader2)
{
	int32	iMember;

	for (iMember = iLeader1; mpcBackend->masTemporaries.Get(iMember)->iNextInClass >= 0; iMember = mpcBackend->masTemporaries.Get(iMember)->iNextInClass)
	{
	}
	mpcBackend->masTemporaries.Get(iMember)->iNextInClass = iLeader2;
	for (iMember = iLeader2; iMember >= 0; iMember = mpcBackend->masTemporaries.Get(iMember)->iNextInClass)
	{
		mpcBackend->masTemporaries.Get(iMember)->iLeader = iLeader1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if the class is live across an instruction at one of paiPositions
// (sorted): live both where it reads and where it writes.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::Crosses(int32 iLeader, CArrayTemplate<int32>* paiPositions)
{
	int32			iMember;
	SIRReference	sMember;
	SIRRange*		psRanges;
	size			uiNumRanges;
	size			ui;
	size			uiLow;
	size			uiHigh;
	size			uiMiddle;

	if (paiPositions->NumElements() == 0)
	{
		return false;
	}
	for (iMember = iLeader; iMember >= 0; iMember = mpcBackend->masTemporaries.Get(iMember)->iNextInClass)
	{
		sMember = IRReference(IRRK_Temporary, (uint32)iMember);
		psRanges = mpcBackend->mcLiveness.GetRanges(sMember);
		uiNumRanges = mpcBackend->mcLiveness.NumRanges(sMember);
		for (ui = 0; ui < uiNumRanges; ui++)
		{
			uiLow = 0;
			uiHigh = paiPositions->NumElements();
			while (uiLow < uiHigh)
			{
				uiMiddle = (uiLow + uiHigh) / 2;
				if (*paiPositions->Get(uiMiddle) < psRanges[ui].iFrom)
				{
					uiLow = uiMiddle + 1;
				}
				else
				{
					uiHigh = uiMiddle;
				}
			}
			if ((uiLow < paiPositions->NumElements()) && (*paiPositions->Get(uiLow) + 1 < psRanges[ui].iTo))
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// True if a temporary of the class is live at one of the positions (sorted):
// read there, or live across it.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::LiveAt(int32 iLeader, CArrayTemplate<int32>* paiPositions)
{
	int32			iMember;
	SIRRange*		psRanges;
	size			uiNumRanges;
	size			ui;
	size			uiLow;
	size			uiHigh;
	size			uiMiddle;

	if (paiPositions->NumElements() == 0)
	{
		return false;
	}
	for (iMember = iLeader; iMember >= 0; iMember = mpcBackend->masTemporaries.Get(iMember)->iNextInClass)
	{
		psRanges = mpcBackend->mcLiveness.GetRanges(IRReference(IRRK_Temporary, (uint32)iMember));
		uiNumRanges = mpcBackend->mcLiveness.NumRanges(IRReference(IRRK_Temporary, (uint32)iMember));
		for (ui = 0; ui < uiNumRanges; ui++)
		{
			uiLow = 0;
			uiHigh = paiPositions->NumElements();
			while (uiLow < uiHigh)
			{
				uiMiddle = (uiLow + uiHigh) / 2;
				if (*paiPositions->Get(uiMiddle) < psRanges[ui].iFrom)
				{
					uiLow = uiMiddle + 1;
				}
				else
				{
					uiHigh = uiMiddle;
				}
			}
			if ((uiLow < paiPositions->NumElements()) && (*paiPositions->Get(uiLow) < psRanges[ui].iTo))
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// True if a temporary of the class is one of the function's parameters.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::HasParameter(int32 iLeader)
{
	int32	iMember;
	size	ui;

	for (iMember = iLeader; iMember >= 0; iMember = mpcBackend->masTemporaries.Get(iMember)->iNextInClass)
	{
		for (ui = 0; ui < mpcBackend->mpcFunction->masParameters.NumElements(); ui++)
		{
			if (mpcBackend->mpcFunction->masParameters.Get(ui)->Index() == (uint32)iMember)
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Notes the code written from uiStart to now as that of the instructions
// at positions iFrom to iTo (a jump's copies at the jump's position).
//
//////////////////////////////////////////////////////////////////////////
void CIR65Places::AddChunk(int32 iFrom, int32 iTo, size uiStart)
{
	SIR65Chunk*	psChunk;

	psChunk = mpcBackend->masChunks.Add();
	psChunk->iFrom = iFrom;
	psChunk->iTo = iTo;
	psChunk->uiStart = uiStart;
	psChunk->uiEnd = mpcBackend->mpszOut->Length();
}


//////////////////////////////////////////////////////////////////////////
//
// True if the identifier character c can't be part of a name, so a B
// beside it is the register.
//
//////////////////////////////////////////////////////////////////////////
static bool IR65NotNameCharacter(char c)
{
	return !(((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) || ((c >= '0') && (c <= '9')) || (c == '_') || (c == '.') || (c == '$') || (c == '@'));
}


//////////////////////////////////////////////////////////////////////////
//
// True if the register B is named in the uiLength characters at sz.
//
//////////////////////////////////////////////////////////////////////////
static bool IR65NamesB(const char* sz, size uiLength)
{
	size	ui;

	for (ui = 0; ui < uiLength; ui++)
	{
		if ((sz[ui] == 'B') && ((ui == 0) || IR65NotNameCharacter(sz[ui - 1])) && ((ui + 1 == uiLength) || IR65NotNameCharacter(sz[ui + 1])))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// What a line of code does with B: reads it (*pbReads), writes it
// (*pbWrites), and whether the write is all it does to it (*pbOverwrites: a
// cpy, cpyx or pul into B that doesn't read it).  A call reads and writes it;
// so does any width change of it.  Labels and directives do nothing.
//
//////////////////////////////////////////////////////////////////////////
static void IR65LineUsesB(const char* szLine, size uiLength, bool* pbReads, bool* pbWrites, bool* pbOverwrites)
{
	const char*	szEnd;
	const char*	szMnemonic;
	size		uiMnemonic;
	const char*	szFirst;
	size		uiFirst;
	const char*	szSecond;
	size		uiSecond;
	const char*	sz;
	int			iDepth;
	bool		bFirstB;

	*pbReads = false;
	*pbWrites = false;
	*pbOverwrites = false;
	szEnd = szLine + uiLength;
	while ((szLine < szEnd) && ((*szLine == '\t') || (*szLine == ' ')))
	{
		szLine++;
	}
	if ((szLine == szEnd) || (*szLine == '.') || (*szLine == ';') || (szEnd[-1] == ':'))
	{
		return;
	}
	for (sz = szLine; sz < szEnd; sz++)
	{
		if (*sz == ';')
		{
			szEnd = sz;
			break;
		}
	}
	szMnemonic = szLine;
	for (sz = szLine; (sz < szEnd) && (*sz != ' ') && (*sz != '\t'); sz++)
	{
	}
	uiMnemonic = sz - szMnemonic;
	while ((sz < szEnd) && ((*sz == ' ') || (*sz == '\t')))
	{
		sz++;
	}

	//The operands: split at the first comma outside brackets.
	szFirst = sz;
	szSecond = szEnd;
	iDepth = 0;
	for (; sz < szEnd; sz++)
	{
		if ((*sz == '[') || (*sz == '('))
		{
			iDepth++;
		}
		else if ((*sz == ']') || (*sz == ')'))
		{
			iDepth--;
		}
		else if ((*sz == ',') && (iDepth == 0))
		{
			szSecond = sz + 1;
			break;
		}
	}
	uiFirst = sz - szFirst;
	while ((szSecond < szEnd) && (*szSecond == ' '))
	{
		szSecond++;
	}
	uiSecond = szEnd - szSecond;

	if (((uiMnemonic == 3) && (memcmp(szMnemonic, "jsr", 3) == 0)) || ((uiMnemonic == 3) && (memcmp(szMnemonic, "div", 3) == 0)))
	{
		*pbReads = true;
		*pbWrites = true;
		return;
	}
	if ((uiMnemonic == 3) && (memcmp(szMnemonic, "mul", 3) == 0))
	{
		*pbReads = true;
		return;
	}
	if ((uiMnemonic == 3) && ((memcmp(szMnemonic, "ret", 3) == 0) || (memcmp(szMnemonic, "stp", 3) == 0)))
	{
		return;
	}
	if ((uiMnemonic >= 3) && (memcmp(szMnemonic, "bra", 3) == 0))
	{
		*pbReads = IR65NamesB(szFirst, szEnd - szFirst);
		return;
	}
	if ((uiMnemonic == 3) && (memcmp(szMnemonic, "psh", 3) == 0))
	{
		*pbReads = IR65NamesB(szFirst, szEnd - szFirst);
		return;
	}
	if ((uiMnemonic == 3) && (memcmp(szMnemonic, "pul", 3) == 0))
	{
		*pbWrites = IR65NamesB(szFirst, szEnd - szFirst);
		*pbOverwrites = *pbWrites;
		return;
	}
	if ((uiMnemonic >= 3) && (memcmp(szMnemonic, "wth", 3) == 0))
	{
		*pbReads = IR65NamesB(szFirst, szEnd - szFirst);
		*pbWrites = *pbReads;
		return;
	}

	//An operation: first, second.
	bFirstB = (uiFirst == 1) && (szFirst[0] == 'B');
	*pbReads = IR65NamesB(szSecond, uiSecond) || (!bFirstB && IR65NamesB(szFirst, uiFirst));
	if (bFirstB)
	{
		if ((uiMnemonic == 3) && (memcmp(szMnemonic, "cmp", 3) == 0))
		{
			*pbReads = true;
		}
		else if (((uiMnemonic == 3) && (memcmp(szMnemonic, "cpy", 3) == 0)) || ((uiMnemonic == 4) && (memcmp(szMnemonic, "cpyx", 4) == 0)))
		{
			*pbWrites = true;
			*pbOverwrites = !*pbReads;
		}
		else
		{
			*pbReads = true;
			*pbWrites = true;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Adds iPosition to the sorted positions, once.
//
//////////////////////////////////////////////////////////////////////////
static void IR65AddPosition(CArrayTemplate<int32>* paiPositions, int32 iPosition)
{
	size	uiLow;
	size	uiHigh;
	size	uiMiddle;

	uiLow = 0;
	uiHigh = paiPositions->NumElements();
	while (uiLow < uiHigh)
	{
		uiMiddle = (uiLow + uiHigh) / 2;
		if (*paiPositions->Get(uiMiddle) < iPosition)
		{
			uiLow = uiMiddle + 1;
		}
		else
		{
			uiHigh = uiMiddle;
		}
	}
	if ((uiLow < paiPositions->NumElements()) && (*paiPositions->Get(uiLow) == iPosition))
	{
		return;
	}
	paiPositions->InsertAt(&iPosition, uiLow);
}


//////////////////////////////////////////////////////////////////////////
//
// With bChanges, notes the positions of each chunk of code just written
// that writes B (maiChangesB): a class kept in B must not be live across
// them.  Otherwise checks the code just written with B as a home: in each
// chunk, a value kept in B that is live across it must not be written
// there; one read there for the last time must not be read after B is
// written (unless the chunk's own result, kept in B, is worked out in it in
// place: add B, 1); and one defined part way through the chunk isn't
// allowed.  A chunk that loses a value so has its positions noted
// (maiLosesB) for the next Allocate, and the check fails.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::CheckHomeB(bool bChanges)
{
	size				uiChunk;
	SIR65Chunk*			psChunk;
	CArrayIRRanges*		pasRanges;
	SIRRange*			psRange;
	size				ui;
	const char*			szText;
	size				uiLine;
	size				uiEnd;
	bool				bReads;
	bool				bWrites;
	bool				bOverwrites;
	bool				bThrough;
	bool				bDies;
	bool				bEarly;
	bool				bResult;
	bool				bLost;
	bool				bWritten;
	bool				bInPlace;
	bool				bReadSince;
	bool				bAnyWrite;
	bool				bAllRight;
	int32				iPosition;

	bAllRight = true;
	pasRanges = &mpcBackend->masHomes.Get(IR65R_B)->asRanges;
	szText = mpcBackend->mpszOut->Text();
	for (uiChunk = 0; uiChunk < mpcBackend->masChunks.NumElements(); uiChunk++)
	{
		psChunk = mpcBackend->masChunks.Get(uiChunk);
		bThrough = false;
		bDies = false;
		bEarly = false;
		bResult = false;
		if (!bChanges)
		{
			for (ui = 0; ui < pasRanges->NumElements(); ui++)
			{
				psRange = pasRanges->Get(ui);
				if ((psRange->iFrom <= psChunk->iFrom) && (psRange->iTo > psChunk->iTo + 1))
				{
					bThrough = true;
				}
				else if ((psRange->iFrom <= psChunk->iFrom) && (psRange->iTo > psChunk->iFrom))
				{
					bDies = true;
				}
				else if ((psRange->iFrom > psChunk->iFrom) && (psRange->iFrom < psChunk->iTo + 1))
				{
					bEarly = true;
				}
				else if (psRange->iFrom == psChunk->iTo + 1)
				{
					bResult = true;
				}
			}
			if (!bThrough && !bDies && !bEarly && !bResult)
			{
				continue;
			}
		}

		bLost = false;
		bWritten = false;
		bInPlace = false;
		bReadSince = true;
		bAnyWrite = false;
		uiLine = psChunk->uiStart;
		while (uiLine < psChunk->uiEnd)
		{
			for (uiEnd = uiLine; (uiEnd < psChunk->uiEnd) && (szText[uiEnd] != '\n') && (szText[uiEnd] != '\r'); uiEnd++)
			{
			}
			IR65LineUsesB(&szText[uiLine], uiEnd - uiLine, &bReads, &bWrites, &bOverwrites);
			if (bReads && bWritten && bDies && !bInPlace)
			{
				//The value read for the last time here was written over first.
				bLost = true;
			}
			if (bWrites)
			{
				if (bOverwrites && bWritten && !bReadSince && bResult)
				{
					//Written twice with no read between: the result worked out in B over its own operand.
					bLost = true;
				}
				if (!bWritten && bReads && bResult)
				{
					bInPlace = true;
				}
				else if (bOverwrites && bInPlace)
				{
					bInPlace = false;
				}
				bWritten = true;
				bAnyWrite = true;
				bReadSince = false;
			}
			if (bReads)
			{
				bReadSince = true;
			}
			for (uiLine = uiEnd; (uiLine < psChunk->uiEnd) && ((szText[uiLine] == '\n') || (szText[uiLine] == '\r')); uiLine++)
			{
			}
		}

		if (bChanges)
		{
			if (bAnyWrite)
			{
				for (iPosition = psChunk->iFrom; iPosition <= psChunk->iTo; iPosition += 2)
				{
					IR65AddPosition(&mpcBackend->maiChangesB, iPosition);
				}
			}
			continue;
		}
		if ((bThrough || bEarly) && bAnyWrite)
		{
			bLost = true;
		}
		if (bLost)
		{
			for (iPosition = psChunk->iFrom; iPosition <= psChunk->iTo; iPosition += 2)
			{
				IR65AddPosition(&mpcBackend->maiLosesB, iPosition);
			}
			bAllRight = false;
		}
	}
	return bAllRight;
}


//////////////////////////////////////////////////////////////////////////
//
// True if none of the class's ranges overlap those given the home already.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::HomeFree(int32 iHome, int32 iLeader)
{
	CArrayIRRanges*	pasRanges;
	int32			iMember;
	SIRReference	sMember;
	SIRRange*		psRanges;
	size			uiNumRanges;
	size			ui;

	pasRanges = &mpcBackend->masHomes.Get(iHome)->asRanges;
	for (iMember = iLeader; iMember >= 0; iMember = mpcBackend->masTemporaries.Get(iMember)->iNextInClass)
	{
		sMember = IRReference(IRRK_Temporary, (uint32)iMember);
		psRanges = mpcBackend->mcLiveness.GetRanges(sMember);
		uiNumRanges = mpcBackend->mcLiveness.NumRanges(sMember);
		for (ui = 0; ui < uiNumRanges; ui++)
		{
			if (FindRange(pasRanges, &psRanges[ui]) < 0)
			{
				return false;
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Where psRange goes in the sorted pasRanges, or -1 if it overlaps one.
//
//////////////////////////////////////////////////////////////////////////
int32 CIR65Places::FindRange(CArrayIRRanges* pasRanges, SIRRange* psRange)
{
	size	uiLow;
	size	uiHigh;
	size	uiMiddle;

	uiLow = 0;
	uiHigh = pasRanges->NumElements();
	while (uiLow < uiHigh)
	{
		uiMiddle = (uiLow + uiHigh) / 2;
		if (pasRanges->Get(uiMiddle)->iTo <= psRange->iFrom)
		{
			uiLow = uiMiddle + 1;
		}
		else
		{
			uiHigh = uiMiddle;
		}
	}
	if ((uiLow < pasRanges->NumElements()) && (pasRanges->Get(uiLow)->iFrom < psRange->iTo))
	{
		return -1;
	}
	return (int32)uiLow;
}


//////////////////////////////////////////////////////////////////////////
//
// Gives a class's ranges to a home (HomeFree must be true).
//
//////////////////////////////////////////////////////////////////////////
void CIR65Places::TakeHome(int32 iHome, int32 iLeader)
{
	CArrayIRRanges*	pasRanges;
	int32			iMember;
	SIRReference	sMember;
	SIRRange*		psRanges;
	size			uiNumRanges;
	size			ui;
	int32			iIndex;

	pasRanges = &mpcBackend->masHomes.Get(iHome)->asRanges;
	for (iMember = iLeader; iMember >= 0; iMember = mpcBackend->masTemporaries.Get(iMember)->iNextInClass)
	{
		sMember = IRReference(IRRK_Temporary, (uint32)iMember);
		psRanges = mpcBackend->mcLiveness.GetRanges(sMember);
		uiNumRanges = mpcBackend->mcLiveness.NumRanges(sMember);
		for (ui = 0; ui < uiNumRanges; ui++)
		{
			iIndex = FindRange(pasRanges, &psRanges[ui]);
			if (iIndex < 0)
			{
				mpcBackend->Error("Home given twice.");
				return;
			}
			pasRanges->InsertAt(&psRanges[ui], (size)iIndex);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Sets the home of each temporary of a class.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Places::PlaceClass(int32 iLeader, uint8 uiKind, int32 iOffset, int32 iBytes)
{
	int32		iMember;
	SIR65Place*	psPlace;

	for (iMember = iLeader; iMember >= 0; iMember = mpcBackend->masTemporaries.Get(iMember)->iNextInClass)
	{
		psPlace = &mpcBackend->masTemporaries.Get(iMember)->sPlace;
		psPlace->uiKind = uiKind;
		psPlace->iOffset = iOffset;
		psPlace->iBytes = iBytes;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if iUnits homes from iHome are free for the class; takes them.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::TakeHomes(int32 iHome, int32 iUnits, int32 iLeader)
{
	int32	i;

	for (i = 0; i < iUnits; i++)
	{
		if (!HomeFree(iHome + i, iLeader))
		{
			return false;
		}
	}
	for (i = 0; i < iUnits; i++)
	{
		TakeHome(iHome + i, iLeader);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// See Allocate.  False if the class needs the frame while V is a home.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::AllocateClass(int32 iLeader, uint32 uiWeight)
{
	SIRType		sType;
	bool		bCall;
	bool		bChangesXY;
	bool		bChangesY;
	bool		bV;
	int32		iParameter;
	int32		iUnits;
	int32		iUnit;
	int32		iHome;
	int32		iOffset;
	size		ui;
	SIR65Home*	psHome;
	SIR65Place	sArrival;

	sType = mpcBackend->mpcFunction->GetTemporaryType(IRReference(IRRK_Temporary, (uint32)iLeader));
	bCall = Crosses(iLeader, &mpcBackend->maiCalls);
	bChangesXY = bCall || Crosses(iLeader, &mpcBackend->maiChangesXY);
	bChangesY = bChangesXY || Crosses(iLeader, &mpcBackend->maiChangesY);
	iUnits = mpcBackend->IsWide(sType) ? 2 : 1;

	//V, where it is a home: a value live across a call (X and Y are changed by it, and a kept pseudo register costs
	//more to save than V does), or one worth V's push and pull (or they are paid for already).
	bV = mpcBackend->msFrame.bHomeV && (iUnits == 1) && Fits32BitRegister(iLeader) && (bCall || (uiWeight >= IR65_V_WEIGHT) || (mpcBackend->masHomes.Get(IR65R_V)->asRanges.NumElements() > 0));

	//A pushed parameter alone in its class (not a thiscall's this, which comes in X, nor one that comes in A, B or Y),
	//whose arrival place nothing stores to through an argaddr.
	iParameter = -1;
	if (mpcBackend->masTemporaries.Get(iLeader)->iNextInClass < 0)
	{
		for (ui = 0; ui < mpcBackend->mpcFunction->masParameters.NumElements(); ui++)
		{
			if ((mpcBackend->mpcFunction->masParameters.Get(ui)->Index() == (uint32)iLeader) && !((ui == 0) && (mpcBackend->mpcFunction->muiAttributes & IRA_ThisCall)) && (ParameterRegister(ui) < 0) &&
				!*mpcBackend->mauiAddressed.Get(ui))
			{
				iParameter = (int32)ui;
				break;
			}
		}
	}

	//A parameter used at most once isn't worth copying (but is, where V is a home: the prologue reads it from S).
	if ((iParameter < 0) || (uiWeight > 2) || mpcBackend->msFrame.bHomeV)
	{
		if ((iUnits == 1) && !bChangesXY && TakeHomes(IR65R_X, 1, iLeader))
		{
			PlaceClass(iLeader, IR65P_Register, IR65R_X, 4);
			return true;
		}
		if ((iUnits == 1) && !bChangesY && TakeHomes(IR65R_Y, 1, iLeader))
		{
			PlaceClass(iLeader, IR65P_Register, IR65R_Y, 4);
			return true;
		}
		//W, where nothing that changes it is done while the class is live (calls, mul and div: what changes Y).  Before
		//V, which costs more to have and is the only register for a class live across calls.
		if (mpcBackend->mpcOptions->mbHomeW && (iUnits == 1) && !bChangesY && Fits32BitRegister(iLeader) && TakeHomes(IR65R_W, 1, iLeader))
		{
			PlaceClass(iLeader, IR65P_Register, IR65R_W, 4);
			return true;
		}
		//B, where the code doesn't work in it while the class is live (CheckHomeB): not across what changes Y either.
		if (mpcBackend->mbHomeB && (iUnits == 1) && !bChangesY && !HasParameter(iLeader) && !Crosses(iLeader, &mpcBackend->maiChangesB) && !LiveAt(iLeader, &mpcBackend->maiLosesB) && TakeHomes(IR65R_B, 1, iLeader))
		{
			PlaceClass(iLeader, IR65P_Register, IR65R_B, 4);
			return true;
		}
		if (bV && TakeHomes(IR65R_V, 1, iLeader))
		{
			mpcBackend->muiWeightV += uiWeight;
			PlaceClass(iLeader, IR65P_Register, IR65R_V, 4);
			return true;
		}
		iUnit = bCall ? -1 : FindUnits(IR65_HOME_PSEUDO, iUnits, iLeader);
		if (iUnit >= 0)
		{
			PlaceClass(iLeader, IR65P_Pseudo, IR65_PSEUDO_UNIT_BYTES * iUnit, IR65_PSEUDO_UNIT_BYTES * iUnits);
			return true;
		}
	}
	if ((iParameter >= 0) && !mpcBackend->msFrame.bHomeV)
	{
		ArrivalPlace((size)iParameter, &sArrival);
		PlaceClass(iLeader, sArrival.uiKind, sArrival.iOffset, sArrival.iBytes);
		return true;
	}
	//A kept pseudo register costs a push and a pull (10 cycles) and the frame's place costs the same to use: so only
	//where there is no frame yet.
	iUnit = (((uiWeight >= 8) && (mpcBackend->msFrame.iFrameBytes == 0) && !mpcBackend->mbFrameOnly) || mpcBackend->msFrame.bHomeV) ? FindUnits(IR65_HOME_SAVED, iUnits, iLeader) : -1;
	if (iUnit >= 0)
	{
		mpcBackend->msFrame.uiSaved |= ((1u << iUnits) - 1) << iUnit;
		PlaceClass(iLeader, IR65P_Pseudo, IR65_SAVED_UNIT(iUnit), IR65_PSEUDO_UNIT_BYTES * iUnits);
		return true;
	}
	if (mpcBackend->msFrame.bHomeV)
	{
		return false;
	}

	for (iHome = IR65_HOME_FRAME; iHome + iUnits <= (int32)mpcBackend->masHomes.NumElements(); iHome++)
	{
		if ((iUnits == 2) && (mpcBackend->masHomes.Get(iHome + 1)->iOffset != mpcBackend->masHomes.Get(iHome)->iOffset + IR65_PSEUDO_UNIT_BYTES))
		{
			continue;
		}
		if (TakeHomes(iHome, iUnits, iLeader))
		{
			PlaceClass(iLeader, IR65P_Frame, mpcBackend->masHomes.Get(iHome)->iOffset, IR65_PSEUDO_UNIT_BYTES * iUnits);
			return true;
		}
	}
	iOffset = mpcBackend->msFrame.Add(IR65_PSEUDO_UNIT_BYTES * iUnits);
	iHome = (int32)mpcBackend->masHomes.NumElements();
	for (iUnit = 0; iUnit < iUnits; iUnit++)
	{
		psHome = mpcBackend->masHomes.Add();
		psHome->asRanges.Init();
		psHome->iOffset = iOffset + IR65_PSEUDO_UNIT_BYTES * iUnit;
	}
	TakeHomes(iHome, iUnits, iLeader);
	PlaceClass(iLeader, IR65P_Frame, iOffset, IR65_PSEUDO_UNIT_BYTES * iUnits);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The first of iUnits pseudo register units from iFirstHome (the low or the
// kept ones) free for the class, taken; -1 if none are.
//
//////////////////////////////////////////////////////////////////////////
int32 CIR65Places::FindUnits(int32 iFirstHome, int32 iUnits, int32 iLeader)
{
	int32	iUnit;

	for (iUnit = 0; iUnit + iUnits <= IR65_PSEUDO_UNITS; iUnit++)
	{
		if (TakeHomes(iFirstHome + iUnit, iUnits, iLeader))
		{
			return iUnit;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIR65Place* CIR65Places::PlaceOf(SIRReference sValue)
{
	return &mpcBackend->masTemporaries.Get(sValue.Index())->sPlace;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the place holds a value: a register, a pseudo register, the
// frame or a parameter where it was pushed.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::IsHome(SIR65Place* psPlace)
{
	return (psPlace->uiKind == IR65P_Register) || (psPlace->uiKind == IR65P_Pseudo) || (psPlace->uiKind == IR65P_Frame) || (psPlace->uiKind == IR65P_Argument);
}


//////////////////////////////////////////////////////////////////////////
//
// True if a home is 4 bytes of memory: a pseudo register, the frame or a
// 4 byte parameter where it was pushed.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::IsMemoryHome(SIR65Place* psPlace)
{
	return (psPlace->uiKind == IR65P_Pseudo) || (psPlace->uiKind == IR65P_Frame) || ((psPlace->uiKind == IR65P_Argument) && (psPlace->iBytes == 4));
}


//////////////////////////////////////////////////////////////////////////
//
// True if two temporaries have the same home.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::SamePlace(SIRReference sValue1, SIRReference sValue2)
{
	SIR65Place*	psPlace1;
	SIR65Place*	psPlace2;

	if (!sValue1.IsTemporary() || !sValue2.IsTemporary())
	{
		return false;
	}
	psPlace1 = PlaceOf(sValue1);
	psPlace2 = PlaceOf(sValue2);
	return IsHome(psPlace1) && (psPlace1->uiKind == psPlace2->uiKind) && (psPlace1->iOffset == psPlace2->iOffset) && (psPlace1->iBytes == psPlace2->iBytes);
}


//////////////////////////////////////////////////////////////////////////
//
// True if writing one home changes the other.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::Overlaps(SIR65Place* psPlace1, SIR65Place* psPlace2)
{
	if (!IsHome(psPlace1) || (psPlace1->uiKind != psPlace2->uiKind))
	{
		return false;
	}
	if (psPlace1->uiKind == IR65P_Register)
	{
		return psPlace1->iOffset == psPlace2->iOffset;
	}
	return (psPlace1->iOffset < psPlace2->iOffset + psPlace2->iBytes) && (psPlace2->iOffset < psPlace1->iOffset + psPlace1->iBytes);
}


//////////////////////////////////////////////////////////////////////////
//
// True if sValue is a load deferred for the instruction being written
// (IR65FU_Deferred, CanDefer).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::IsDeferred(SIRReference sValue)
{
	size	ui;

	if (!sValue.IsTemporary())
	{
		return false;
	}
	for (ui = 0; ui < mpcBackend->masFused.NumElements(); ui++)
	{
		if ((mpcBackend->masFused.Get(ui)->uiKind == IR65FU_Deferred) && mpcBackend->masFused.Get(ui)->psInstruction->sResult.Equals(sValue))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the instruction before left sValue in A.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::IsInA(SIRReference sValue)
{
	return sValue.IsTemporary() && mpcBackend->msAvailable.Equals(sValue);
}


//////////////////////////////////////////////////////////////////////////
//
// True if sValue's home is a register.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::InRegister(SIRReference sValue)
{
	return sValue.IsTemporary() && (PlaceOf(sValue)->uiKind == IR65P_Register);
}


//////////////////////////////////////////////////////////////////////////
//
// True if sValue is a constant, or an i8 or i16 load only ever compared
// (as the type it is), so the bits it is extended to are never read.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::NarrowOnly(SIRReference sValue)
{
	return sValue.IsConstant() || (sValue.IsTemporary() && mpcBackend->masTemporaries.Get(sValue.Index())->bNarrowOnly);
}


//////////////////////////////////////////////////////////////////////////
//
// Marks the narrow loads whose only uses are integer comparisons (NarrowOnly).
//
//////////////////////////////////////////////////////////////////////////
void CIR65Places::FindNarrowOnly(void)
{
	size			ui;
	size			uiInstruction;
	size			uiArgument;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	SIRReference	sValue;

	mpcBackend->GrowTemporaries();
	for (ui = 0; ui < mpcBackend->masTemporaries.NumElements(); ui++)
	{
		mpcBackend->masTemporaries.Get(ui)->bNarrowOnly = false;
	}
	for (ui = 0; ui < mpcBackend->mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcBackend->mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if ((psInstruction->uiOpcode == IRO_Load) && mpcBackend->IsNarrow(psInstruction->sType) && !psInstruction->sType.Is(IRTK_I1))
			{
				mpcBackend->masTemporaries.Get(psInstruction->sResult.Index())->bNarrowOnly = true;
			}
		}
	}
	for (ui = 0; ui < mpcBackend->mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcBackend->mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masPhis.NumElements(); uiInstruction++)
		{
			psPhi = pcBlock->masPhis.Get(uiInstruction);
			for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
			{
				sValue = psPhi->asArguments.Get(uiArgument)->sValue;
				if (sValue.IsTemporary())
				{
					mpcBackend->masTemporaries.Get(sValue.Index())->bNarrowOnly = false;
				}
			}
		}
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if (IsIRIntegerComparison(psInstruction->uiOpcode))
			{
				continue;
			}
			for (uiArgument = 0; uiArgument < 2; uiArgument++)
			{
				sValue = psInstruction->asArguments[uiArgument];
				if (sValue.IsTemporary())
				{
					mpcBackend->masTemporaries.Get(sValue.Index())->bNarrowOnly = false;
				}
			}
		}
		sValue = pcBlock->msJump.sArgument;
		if (sValue.IsTemporary())
		{
			mpcBackend->masTemporaries.Get(sValue.Index())->bNarrowOnly = false;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CIR65Places::RegisterName(SIR65Place* psPlace)
{
	static const char*	aszNames[IR65R_Count] = { "X", "Y", "V", "W", "B" };

	return aszNames[psPlace->iOffset];
}


//////////////////////////////////////////////////////////////////////////
//
// True if a register home can be written as an address ([X], [4 + V]) by
// any instruction: not W, which add, sub, cmp, shl, shr and cpyx can't
// address with, and whose [n + W] is how the pseudo registers are written
// (a cpy's [W] is MemoryOperand's own case); nor B, which cpyx can't, and
// which MemoryOperand uses as [B] when an address is nowhere else.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::IsAddressRegister(SIR65Place* psPlace)
{
	return (psPlace->uiKind == IR65P_Register) && (psPlace->iOffset != IR65R_W) && (psPlace->iOffset != IR65R_B);
}


//////////////////////////////////////////////////////////////////////////
//
// True if a register home can be given another width (wth, wthx, cpyx): X
// and Y; V and W are always 32 bits wide.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::HasWidth(SIR65Place* psPlace)
{
	return (psPlace->uiKind == IR65P_Register) && ((psPlace->iOffset == IR65R_X) || (psPlace->iOffset == IR65R_Y));
}


//////////////////////////////////////////////////////////////////////////
//
// True if a memory operand addresses the frame (or the arguments) with S, which
// add, sub, cmp, shl and shr can't do (they address with V, X or Y: the user,
// 2026-10-28).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::IsFrameOperand(CChars* pszOperand)
{
	return pszOperand->Find("S]") != (size)-1;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the function calls nothing (not even the runtime), so nothing is
// pushed while it runs.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::IsLeaf(void) const
{
	size	ui;
	size	uiInstruction;

	for (ui = 0; ui < mpcBackend->mpcFunction->NumBlocks(); ui++)
	{
		for (uiInstruction = 0; uiInstruction < mpcBackend->mpcFunction->GetBlock(ui)->masInstructions.NumElements(); uiInstruction++)
		{
			if (IR65OpcodeFlags(mpcBackend->mpcFunction->GetBlock(ui)->masInstructions.Get(uiInstruction)->uiOpcode) & (IR65F_CALLS | IR65F_CHANGES_XY))
			{
				return false;
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the parameters left where they arrived are only read in the
// entry block before anything is pushed (for a call's arguments or the
// runtime), and no argaddr is taken: then S hasn't moved since the
// prologue, so they can be read from [n + S] without a frame.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::ArgumentsReadFirst(void)
{
	size			ui;
	size			uiInstruction;
	size			uiFirstPush;
	size			uiArgument;
	size			uiPhi;
	CIRBlock*		pcBlock;
	CIRBlock*		pcEntry;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;

	pcEntry = mpcBackend->mpcFunction->GetEntry();
	for (uiFirstPush = 0; uiFirstPush < pcEntry->masInstructions.NumElements(); uiFirstPush++)
	{
		psInstruction = pcEntry->masInstructions.Get(uiFirstPush);
		if ((psInstruction->uiOpcode == IRO_Arg) || (IR65OpcodeFlags(psInstruction->uiOpcode) & (IR65F_CALLS | IR65F_CHANGES_XY)))
		{
			break;
		}
	}

	for (ui = 0; ui < mpcBackend->mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcBackend->mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if (psInstruction->uiOpcode == IRO_ArgAddress)
			{
				return false;
			}
			if ((IsArrivalPlace(psInstruction->asArguments[0]) || IsArrivalPlace(psInstruction->asArguments[1])) && ((pcBlock != pcEntry) || (uiInstruction >= uiFirstPush)))
			{
				return false;
			}
		}
		if (IsArrivalPlace(pcBlock->msJump.sArgument) && ((pcBlock != pcEntry) || (uiFirstPush < pcBlock->masInstructions.NumElements())))
		{
			return false;
		}
		for (uiPhi = 0; uiPhi < pcBlock->masPhis.NumElements(); uiPhi++)
		{
			psPhi = pcBlock->masPhis.Get(uiPhi);
			for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
			{
				if (IsArrivalPlace(psPhi->asArguments.Get(uiArgument)->sValue))
				{
					return false;
				}
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if sValue is a parameter read from where it arrived.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Places::IsArrivalPlace(SIRReference sValue)
{
	return sValue.IsTemporary() && (PlaceOf(sValue)->uiKind == IR65P_Argument);
}


//////////////////////////////////////////////////////////////////////////
//
// The number of callee saved pseudo registers the prologue pushes.
//
//////////////////////////////////////////////////////////////////////////
int SIR65Frame::NumSaved(void) const
{
	int		iUnit;
	int		iCount;

	iCount = 0;
	for (iUnit = 0; iUnit < IR65_PSEUDO_UNITS; iUnit++)
	{
		if (uiSaved & (1u << iUnit))
		{
			iCount++;
		}
	}
	return iCount;
}
