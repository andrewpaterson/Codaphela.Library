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
// A ptradd of a constant just before the load or store that alone uses it
// becomes part of the load or store's address.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::MarkFolded(void)
{
	size			ui;
	size			uiInstruction;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRInstruction*	psPrevious;
	SIRReference	sAddress;
	SIR65Place*		psPlace;

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 1; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			sAddress = IRAddressOf(psInstruction);
			if (sAddress.IsNone())
			{
				continue;
			}
			psPrevious = pcBlock->masInstructions.Get(uiInstruction - 1);
			if (ConstantPtrAdd(psPrevious) && psPrevious->sResult.Equals(sAddress) && (*mauiUses.Get(sAddress.Index()) == 1))
			{
				psPlace = PlaceOf(sAddress);
				psPlace->uiKind = IR65P_Folded;
				psPlace->iOffset = (int32)mpcFunction->GetSigned(psPrevious->asArguments[1]);
				psPlace->sBase = psPrevious->asArguments[0];
				psPlace->bInA = false;
			}
		}
	}
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
// the return address and the saved pseudo registers.
//
//////////////////////////////////////////////////////////////////////////
void SIR65Frame::SetFrameless(bool bFramelessIn)
{
	bFrameless = bFramelessIn;
	cRegister = bFrameless ? 'S' : 'V';
	iArguments = bFrameless ? (1 + 4 + 4 * NumSaved()) : (1 + iFrameBytes + 8);
}


//////////////////////////////////////////////////////////////////////////
//
// The comparison a block's brif alone uses, which branches without making
// its i1; NULL for none.
//
//////////////////////////////////////////////////////////////////////////
SIRInstruction* CIR65Backend::FusedComparison(CIRBlock* pcBlock)
{
	SIRInstruction*	psLast;

	psLast = BranchComparison(pcBlock);
	if (psLast && (*mauiUses.Get(psLast->sResult.Index()) == 1))
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
void CIR65Backend::MarkInA(void)
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

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		psFused = FusedComparison(pcBlock);
		uiNumInstructions = pcBlock->masInstructions.NumElements();
		if (psFused)
		{
			masPlaces.Get(psFused->sResult.Index())->bInA = true;
			uiNumInstructions--;
		}
		for (uiInstruction = 0; uiInstruction < uiNumInstructions; uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			sResult = psInstruction->sResult;
			if (!sResult.IsTemporary() || (*mauiUses.Get(sResult.Index()) != 1) || Skipped(psInstruction))
			{
				continue;
			}

			//The next instruction that writes something.
			uiNext = NextWritten(pcBlock, uiInstruction, uiNumInstructions);
			if (uiNext < uiNumInstructions)
			{
				psNext = pcBlock->masInstructions.Get(uiNext);
				bInA = TakesInA(psNext, sResult) || TakesInB(psNext, sResult) || ((psNext->uiOpcode == IRO_Load) && psNext->asArguments[0].Equals(sResult)) || FoldedBase(psNext, sResult);
			}
			else
			{
				bInA = JumpTakesInA(pcBlock, psFused, sResult) || (psFused && TakesInB(psFused, sResult)) || (psFused && FoldedBase(psFused, sResult));
			}
			masPlaces.Get(sResult.Index())->bInA = bInA;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if psNext is a load or store whose address is a folded ptradd of
// sValue.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::FoldedBase(SIRInstruction* psNext, SIRReference sValue)
{
	SIRReference	sAddress;

	sAddress = IRAddressOf(psNext);
	return sAddress.IsTemporary() && (PlaceOf(sAddress)->uiKind == IR65P_Folded) && PlaceOf(sAddress)->sBase.Equals(sValue);
}


//////////////////////////////////////////////////////////////////////////
//
// Where a parameter the caller pushed starts, from the first argument.
//
//////////////////////////////////////////////////////////////////////////
int32 CIR65Backend::ParameterOffset(size uiParameter)
{
	size	ui;
	int32	iOffset;

	iOffset = 0;
	for (ui = 0; ui < uiParameter; ui++)
	{
		if (((ui == 0) && (mpcFunction->muiAttributes & IRA_ThisCall)) || (ParameterRegister(ui) >= 0))
		{
			continue;
		}
		iOffset += ValueBytes(mpcFunction->GetTemporaryType(*mpcFunction->masParameters.Get(ui)));
	}
	return iOffset;
}


//////////////////////////////////////////////////////////////////////////
//
// The register units an argument of sType takes: one for 32 bits or less,
// two for 64 (which are pushed: only one unit goes in a register).
//
//////////////////////////////////////////////////////////////////////////
int CIR65Backend::ArgumentUnits(SIRType sType)
{
	return (ValueBytes(sType) + 3) / 4;
}


//////////////////////////////////////////////////////////////////////////
//
// The function's parameters as its callers pass them (masArguments, less a
// thiscall's this, which is in X): the first three of 32 bits or less in A,
// B and Y when its symbol is marked for register arguments and it isn't
// variadic (CIRBackend::AssignRegisters), the rest pushed.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::FunctionArguments(void)
{
	SIRRegisterArgument*	psArgument;
	size					ui;

	masArguments.FakeSetUsedElements(0);
	for (ui = (mpcFunction->muiAttributes & IRA_ThisCall) ? 1 : 0; ui < mpcFunction->masParameters.NumElements(); ui++)
	{
		psArgument = masArguments.Add();
		psArgument->iUnits = ArgumentUnits(mpcFunction->GetTemporaryType(*mpcFunction->masParameters.Get(ui)));
		psArgument->iRegister = -1;
	}
	miRegisterArguments = 0;
	if (IsRegisterCall(IRReference(IRRK_Symbol, mpcFunction->muiSymbol)) && !(mpcFunction->muiAttributes & IRA_Variadic))
	{
		miRegisterArguments = AssignRegisters(&masArguments, IR65_ARGUMENT_REGISTERS, 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The register a parameter arrives in (0 A, 1 B, 2 Y), or -1 when it was
// pushed (or is a thiscall's this, in X).
//
//////////////////////////////////////////////////////////////////////////
int CIR65Backend::ParameterRegister(size uiParameter)
{
	size	uiFirst;

	uiFirst = (mpcFunction->muiAttributes & IRA_ThisCall) ? 1 : 0;
	if ((uiParameter < uiFirst) || (uiParameter - uiFirst >= masArguments.NumElements()))
	{
		return -1;
	}
	return masArguments.Get(uiParameter - uiFirst)->iRegister;
}


//////////////////////////////////////////////////////////////////////////
//
// Where the caller pushed a parameter.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::ArrivalPlace(size uiParameter, SIR65Place* psPlace)
{
	psPlace->uiKind = IR65P_Argument;
	psPlace->iOffset = ParameterOffset(uiParameter);
	psPlace->iBytes = ValueBytes(mpcFunction->GetTemporaryType(*mpcFunction->masParameters.Get(uiParameter)));
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
void CIR65Backend::Layout(void)
{
	size			ui;
	size			uiInstruction;
	int32			iParameter;
	int				iRegister;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIR65Place*		psPlace;

	masPlaces.FakeSetUsedElements(0);
	masPlaces.GrowToAtLeastNumElements(mpcFunction->NumTemporaries(), true, 0);
	mauiAddressed.FakeSetUsedElements(0);
	mauiAddressed.GrowToAtLeastNumElements(mpcFunction->masParameters.NumElements(), true, 0);
	msFrame.Init();
	MarkFolded();
	MarkInA();

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if (!psInstruction->sResult.IsTemporary())
			{
				continue;
			}
			psPlace = masPlaces.Get(psInstruction->sResult.Index());
			if (psInstruction->uiOpcode == IRO_Slot)
			{
				psPlace->uiKind = IR65P_FrameAddress;
				psPlace->iBytes = (int32)mpcFunction->GetConstant(psInstruction->asArguments[0])->ulliBits;
				psPlace->iOffset = msFrame.Add(psPlace->iBytes);
			}
			else if (psInstruction->uiOpcode == IRO_ArgAddress)
			{
				iParameter = (int32)mpcFunction->GetConstant(psInstruction->asArguments[0])->ulliBits;
				if ((size)iParameter >= mpcFunction->masParameters.NumElements())
				{
					Error("argaddr of a parameter the function doesn't have.");
				}
				else if ((iParameter == 0) && (mpcFunction->muiAttributes & IRA_ThisCall))
				{
					if (msFrame.iThisOffset < 0)
					{
						msFrame.iThisOffset = msFrame.Add(4);
					}
					psPlace->uiKind = IR65P_FrameAddress;
					psPlace->iOffset = msFrame.iThisOffset;
				}
				else if (ParameterRegister((size)iParameter) >= 0)
				{
					//A parameter that came in a register is kept in the frame for its address (the prologue stores it).
					iRegister = ParameterRegister((size)iParameter);
					if (msFrame.aiRegisterOffset[iRegister] < 0)
					{
						msFrame.aiRegisterOffset[iRegister] = msFrame.Add(4);
					}
					psPlace->uiKind = IR65P_FrameAddress;
					psPlace->iOffset = msFrame.aiRegisterOffset[iRegister];
				}
				else
				{
					psPlace->uiKind = IR65P_ArgumentAddress;
					psPlace->iOffset = ParameterOffset((size)iParameter);
					*mauiAddressed.Get((size)iParameter) = 1;
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
bool CIR65Backend::NeedsHome(int32 iTemporary)
{
	SIR65Place*		psPlace;
	SIRReference	sTemporary;

	psPlace = masPlaces.Get(iTemporary);
	sTemporary = IRReference(IRRK_Temporary, (uint32)iTemporary);
	return (psPlace->uiKind == IR65P_None) && !psPlace->bInA && (*mauiUses.Get(iTemporary) > 0) && (mcLiveness.NumRanges(sTemporary) > 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Gives every temporary that needs one a home.  Phis and the values they
// take share a class (and a home) where they don't interfere; classes are
// given homes most weighted first (a use in a loop counts 8 times more): X
// or Y if nothing that changes them is done while the class is live, then
// a low pseudo register if no call is made.  A parameter used at most once,
// or that gets none of those, stays where it was pushed.  Then a kept
// pseudo register if the class weighs 8 or more (it costs a push and a
// pull), then the frame.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Allocate(void)
{
	CArrayIR65Weights			asWeights;
	SIR65Weight*				psWeight;
	SIR65Home*					psHome;
	size						ui;
	int32						iTemporary;
	int32						iMember;
	int32						iNumTemporaries;

	for (ui = 0; ui < masHomes.NumElements(); ui++)
	{
		masHomes.Get(ui)->asRanges.Kill();
	}
	masHomes.FakeSetUsedElements(0);
	for (ui = 0; ui < IR65_HOME_FRAME; ui++)
	{
		psHome = masHomes.Add();
		psHome->asRanges.Init();
		psHome->iOffset = -1;
	}
	FindChanges();

	iNumTemporaries = (int32)mpcFunction->NumTemporaries();
	maiClasses.FakeSetUsedElements(0);
	maiLeaders.FakeSetUsedElements(0);
	for (iTemporary = 0; iTemporary < iNumTemporaries; iTemporary++)
	{
		iMember = -1;
		maiClasses.Add(&iMember);
		maiLeaders.Add(&iTemporary);
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
		for (iMember = iTemporary; iMember >= 0; iMember = *maiClasses.Get(iMember))
		{
			psWeight->uiWeight += mcLiveness.GetWeight(IRReference(IRRK_Temporary, (uint32)iMember));
		}
	}
	DiscountAddresses(&asWeights);
	asWeights.QuickSort(CompareIR65Weights);
	for (ui = 0; ui < asWeights.NumElements(); ui++)
	{
		psWeight = asWeights.Get(ui);
		AllocateClass(psWeight->iLeader, psWeight->uiWeight);
	}
	asWeights.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer in memory costs nothing more than one in a register where it is
// a load's or store's address ([[n + W]]) or stepped by a small constant in
// its own class (add [n + W], 1), so those uses (and the step's result)
// don't count towards its class's weight.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::DiscountAddresses(CArrayIR65Weights* pasWeights)
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
	auiDiscounts.GrowToAtLeastNumElements(mpcFunction->NumTemporaries(), true, 0);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		uiUse = CIRLoops::Weight(mcLiveness.LoopDepth(pcBlock), IR_LOOP_WEIGHT_MAX_DEPTH);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			sAddress = IRAddressOf(psInstruction);
			if (sAddress.IsTemporary() && (PlaceOf(sAddress)->uiKind != IR65P_Folded) && !psInstruction->asArguments[0].Equals(psInstruction->asArguments[1]))
			{
				*auiDiscounts.Get((size)Find((int32)sAddress.Index())) += uiUse;
			}
			else if ((psInstruction->uiOpcode == IRO_PtrAdd) && psInstruction->asArguments[0].IsTemporary() && psInstruction->sResult.IsTemporary() &&
					 SmallImmediate(psInstruction->asArguments[1], -8, 8) && (Find((int32)psInstruction->asArguments[0].Index()) == Find((int32)psInstruction->sResult.Index())))
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
void CIR65Backend::FindChanges(void)
{
	size			ui;
	size			uiInstruction;
	CIRBlock*		pcBlock;
	int32			iPosition;
	uint8			uiFlags;

	maiCalls.FakeSetUsedElements(0);
	maiChangesXY.FakeSetUsedElements(0);
	maiChangesY.FakeSetUsedElements(0);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			iPosition = mcLiveness.Position(pcBlock, uiInstruction);
			uiFlags = IR65OpcodeFlags(pcBlock->masInstructions.Get(uiInstruction)->uiOpcode);
			if (uiFlags & IR65F_CALLS)
			{
				maiCalls.Add(&iPosition);
			}
			else if (uiFlags & IR65F_CHANGES_XY)
			{
				maiChangesXY.Add(&iPosition);
			}
			else if (uiFlags & IR65F_CHANGES_Y)
			{
				maiChangesY.Add(&iPosition);
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
void CIR65Backend::Coalesce(void)
{
	size			ui;
	size			uiPhi;
	size			uiArgument;
	CIRBlock*		pcBlock;
	SIRPhi*			psPhi;
	SIRReference	sValue;
	int32			iPhi;
	int32			iValue;

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
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
int32 CIR65Backend::Find(int32 iTemporary)
{
	return *maiLeaders.Get(iTemporary);
}


//////////////////////////////////////////////////////////////////////////
//
// True if any temporary of one class is live where one of the other is.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Interferes(int32 iLeader1, int32 iLeader2)
{
	int32			iMember1;
	int32			iMember2;
	SIRReference	sMember1;
	SIRReference	sMember2;

	for (iMember1 = iLeader1; iMember1 >= 0; iMember1 = *maiClasses.Get(iMember1))
	{
		sMember1 = IRReference(IRRK_Temporary, (uint32)iMember1);
		for (iMember2 = iLeader2; iMember2 >= 0; iMember2 = *maiClasses.Get(iMember2))
		{
			sMember2 = IRReference(IRRK_Temporary, (uint32)iMember2);
			if (CIRLiveness::Intersect(mcLiveness.GetRanges(sMember1), mcLiveness.NumRanges(sMember1), mcLiveness.GetRanges(sMember2), mcLiveness.NumRanges(sMember2)))
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
void CIR65Backend::Join(int32 iLeader1, int32 iLeader2)
{
	int32	iMember;

	for (iMember = iLeader1; *maiClasses.Get(iMember) >= 0; iMember = *maiClasses.Get(iMember))
	{
	}
	*maiClasses.Get(iMember) = iLeader2;
	for (iMember = iLeader2; iMember >= 0; iMember = *maiClasses.Get(iMember))
	{
		*maiLeaders.Get(iMember) = iLeader1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if the class is live across an instruction at one of paiPositions
// (sorted): live both where it reads and where it writes.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Crosses(int32 iLeader, CArrayTemplate<int32>* paiPositions)
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
	for (iMember = iLeader; iMember >= 0; iMember = *maiClasses.Get(iMember))
	{
		sMember = IRReference(IRRK_Temporary, (uint32)iMember);
		psRanges = mcLiveness.GetRanges(sMember);
		uiNumRanges = mcLiveness.NumRanges(sMember);
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
// True if none of the class's ranges overlap those given the home already.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::HomeFree(int32 iHome, int32 iLeader)
{
	CArrayIRRanges*	pasRanges;
	int32			iMember;
	SIRReference	sMember;
	SIRRange*		psRanges;
	size			uiNumRanges;
	size			ui;

	pasRanges = &masHomes.Get(iHome)->asRanges;
	for (iMember = iLeader; iMember >= 0; iMember = *maiClasses.Get(iMember))
	{
		sMember = IRReference(IRRK_Temporary, (uint32)iMember);
		psRanges = mcLiveness.GetRanges(sMember);
		uiNumRanges = mcLiveness.NumRanges(sMember);
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
int32 CIR65Backend::FindRange(CArrayIRRanges* pasRanges, SIRRange* psRange)
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
void CIR65Backend::TakeHome(int32 iHome, int32 iLeader)
{
	CArrayIRRanges*	pasRanges;
	int32			iMember;
	SIRReference	sMember;
	SIRRange*		psRanges;
	size			uiNumRanges;
	size			ui;
	int32			iIndex;

	pasRanges = &masHomes.Get(iHome)->asRanges;
	for (iMember = iLeader; iMember >= 0; iMember = *maiClasses.Get(iMember))
	{
		sMember = IRReference(IRRK_Temporary, (uint32)iMember);
		psRanges = mcLiveness.GetRanges(sMember);
		uiNumRanges = mcLiveness.NumRanges(sMember);
		for (ui = 0; ui < uiNumRanges; ui++)
		{
			iIndex = FindRange(pasRanges, &psRanges[ui]);
			if (iIndex < 0)
			{
				Error("Home given twice.");
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
void CIR65Backend::PlaceClass(int32 iLeader, uint8 uiKind, int32 iOffset, int32 iBytes)
{
	int32		iMember;
	SIR65Place*	psPlace;

	for (iMember = iLeader; iMember >= 0; iMember = *maiClasses.Get(iMember))
	{
		psPlace = masPlaces.Get(iMember);
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
bool CIR65Backend::TakeHomes(int32 iHome, int32 iUnits, int32 iLeader)
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
// See Allocate.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::AllocateClass(int32 iLeader, uint32 uiWeight)
{
	SIRType		sType;
	bool		bCall;
	bool		bChangesXY;
	bool		bChangesY;
	int32		iParameter;
	int32		iUnits;
	int32		iUnit;
	int32		iHome;
	int32		iOffset;
	size		ui;
	SIR65Home*	psHome;
	SIR65Place	sArrival;

	sType = mpcFunction->GetTemporaryType(IRReference(IRRK_Temporary, (uint32)iLeader));
	bCall = Crosses(iLeader, &maiCalls);
	bChangesXY = bCall || Crosses(iLeader, &maiChangesXY);
	bChangesY = bChangesXY || Crosses(iLeader, &maiChangesY);
	iUnits = IsWide(sType) ? 2 : 1;

	//A pushed parameter alone in its class (not a thiscall's this, which comes in X, nor one that comes in A, B or Y),
	//whose arrival place nothing stores to through an argaddr.
	iParameter = -1;
	if (*maiClasses.Get(iLeader) < 0)
	{
		for (ui = 0; ui < mpcFunction->masParameters.NumElements(); ui++)
		{
			if ((mpcFunction->masParameters.Get(ui)->Index() == (uint32)iLeader) && !((ui == 0) && (mpcFunction->muiAttributes & IRA_ThisCall)) && (ParameterRegister(ui) < 0) &&
				!*mauiAddressed.Get(ui))
			{
				iParameter = (int32)ui;
				break;
			}
		}
	}

	//A parameter used at most once isn't worth copying.
	if ((iParameter < 0) || (uiWeight > 2))
	{
		if ((iUnits == 1) && !bChangesXY && TakeHomes(IR65R_X, 1, iLeader))
		{
			PlaceClass(iLeader, IR65P_Register, IR65R_X, 4);
			return;
		}
		if ((iUnits == 1) && !bChangesY && TakeHomes(IR65R_Y, 1, iLeader))
		{
			PlaceClass(iLeader, IR65P_Register, IR65R_Y, 4);
			return;
		}
		iUnit = bCall ? -1 : FindUnits(IR65_HOME_PSEUDO, iUnits, iLeader);
		if (iUnit >= 0)
		{
			PlaceClass(iLeader, IR65P_Pseudo, IR65_PSEUDO_UNIT_BYTES * iUnit, IR65_PSEUDO_UNIT_BYTES * iUnits);
			return;
		}
	}
	if (iParameter >= 0)
	{
		ArrivalPlace((size)iParameter, &sArrival);
		PlaceClass(iLeader, sArrival.uiKind, sArrival.iOffset, sArrival.iBytes);
		return;
	}
	iUnit = (uiWeight >= 8) ? FindUnits(IR65_HOME_SAVED, iUnits, iLeader) : -1;
	if (iUnit >= 0)
	{
		msFrame.uiSaved |= ((1u << iUnits) - 1) << iUnit;
		PlaceClass(iLeader, IR65P_Pseudo, IR65_SAVED_UNIT(iUnit), IR65_PSEUDO_UNIT_BYTES * iUnits);
		return;
	}

	for (iHome = IR65_HOME_FRAME; iHome + iUnits <= (int32)masHomes.NumElements(); iHome++)
	{
		if ((iUnits == 2) && (masHomes.Get(iHome + 1)->iOffset != masHomes.Get(iHome)->iOffset + IR65_PSEUDO_UNIT_BYTES))
		{
			continue;
		}
		if (TakeHomes(iHome, iUnits, iLeader))
		{
			PlaceClass(iLeader, IR65P_Frame, masHomes.Get(iHome)->iOffset, IR65_PSEUDO_UNIT_BYTES * iUnits);
			return;
		}
	}
	iOffset = msFrame.Add(IR65_PSEUDO_UNIT_BYTES * iUnits);
	iHome = (int32)masHomes.NumElements();
	for (iUnit = 0; iUnit < iUnits; iUnit++)
	{
		psHome = masHomes.Add();
		psHome->asRanges.Init();
		psHome->iOffset = iOffset + IR65_PSEUDO_UNIT_BYTES * iUnit;
	}
	TakeHomes(iHome, iUnits, iLeader);
	PlaceClass(iLeader, IR65P_Frame, iOffset, IR65_PSEUDO_UNIT_BYTES * iUnits);
}


//////////////////////////////////////////////////////////////////////////
//
// The first of iUnits pseudo register units from iFirstHome (the low or the
// kept ones) free for the class, taken; -1 if none are.
//
//////////////////////////////////////////////////////////////////////////
int32 CIR65Backend::FindUnits(int32 iFirstHome, int32 iUnits, int32 iLeader)
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
SIR65Place* CIR65Backend::PlaceOf(SIRReference sValue)
{
	return masPlaces.Get(sValue.Index());
}


//////////////////////////////////////////////////////////////////////////
//
// True if the place holds a value: a register, a pseudo register, the
// frame or a parameter where it was pushed.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::IsHome(SIR65Place* psPlace)
{
	return (psPlace->uiKind == IR65P_Register) || (psPlace->uiKind == IR65P_Pseudo) || (psPlace->uiKind == IR65P_Frame) || (psPlace->uiKind == IR65P_Argument);
}


//////////////////////////////////////////////////////////////////////////
//
// True if a home is 4 bytes of memory: a pseudo register, the frame or a
// 4 byte parameter where it was pushed.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::IsMemoryHome(SIR65Place* psPlace)
{
	return (psPlace->uiKind == IR65P_Pseudo) || (psPlace->uiKind == IR65P_Frame) || ((psPlace->uiKind == IR65P_Argument) && (psPlace->iBytes == 4));
}


//////////////////////////////////////////////////////////////////////////
//
// True if two temporaries have the same home.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::SamePlace(SIRReference sValue1, SIRReference sValue2)
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
bool CIR65Backend::Overlaps(SIR65Place* psPlace1, SIR65Place* psPlace2)
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
bool CIR65Backend::IsDeferred(SIRReference sValue)
{
	size	ui;

	if (!sValue.IsTemporary())
	{
		return false;
	}
	for (ui = 0; ui < masFused.NumElements(); ui++)
	{
		if ((masFused.Get(ui)->uiKind == IR65FU_Deferred) && masFused.Get(ui)->psInstruction->sResult.Equals(sValue))
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
bool CIR65Backend::IsInA(SIRReference sValue)
{
	return sValue.IsTemporary() && msAvailable.Equals(sValue);
}


//////////////////////////////////////////////////////////////////////////
//
// True if sValue's home is a register.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::InRegister(SIRReference sValue)
{
	return sValue.IsTemporary() && (PlaceOf(sValue)->uiKind == IR65P_Register);
}


//////////////////////////////////////////////////////////////////////////
//
// True if sValue is a constant, or an i8 or i16 load only ever compared
// (as the type it is), so the bits it is extended to are never read.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::NarrowOnly(SIRReference sValue)
{
	return sValue.IsConstant() || (sValue.IsTemporary() && *mauiNarrowOnly.Get(sValue.Index()));
}


//////////////////////////////////////////////////////////////////////////
//
// Marks the narrow loads whose only uses are integer comparisons (NarrowOnly).
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::FindNarrowOnly(void)
{
	size			ui;
	size			uiInstruction;
	size			uiArgument;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	SIRReference	sValue;

	mauiNarrowOnly.FakeSetUsedElements(0);
	mauiNarrowOnly.GrowToAtLeastNumElements(mpcFunction->NumTemporaries() + 1, true, 0);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if ((psInstruction->uiOpcode == IRO_Load) && IsNarrow(psInstruction->sType) && !psInstruction->sType.Is(IRTK_I1))
			{
				*mauiNarrowOnly.Get(psInstruction->sResult.Index()) = 1;
			}
		}
	}
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masPhis.NumElements(); uiInstruction++)
		{
			psPhi = pcBlock->masPhis.Get(uiInstruction);
			for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
			{
				sValue = psPhi->asArguments.Get(uiArgument)->sValue;
				if (sValue.IsTemporary())
				{
					*mauiNarrowOnly.Get(sValue.Index()) = 0;
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
					*mauiNarrowOnly.Get(sValue.Index()) = 0;
				}
			}
		}
		sValue = pcBlock->msJump.sArgument;
		if (sValue.IsTemporary())
		{
			*mauiNarrowOnly.Get(sValue.Index()) = 0;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CIR65Backend::RegisterName(SIR65Place* psPlace)
{
	return (psPlace->iOffset == IR65R_X) ? "X" : "Y";
}


//////////////////////////////////////////////////////////////////////////
//
// True if a memory operand addresses the frame (or the arguments) with S, which
// add, sub, cmp, shl and shr can't do (they address with V, X or Y: the user,
// 2026-10-28).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::IsFrameOperand(CChars* pszOperand)
{
	return pszOperand->Find("S]") != (size)-1;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the function calls nothing (not even the runtime), so nothing is
// pushed while it runs.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::IsLeaf(void)
{
	size	ui;
	size	uiInstruction;

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		for (uiInstruction = 0; uiInstruction < mpcFunction->GetBlock(ui)->masInstructions.NumElements(); uiInstruction++)
		{
			if (IR65OpcodeFlags(mpcFunction->GetBlock(ui)->masInstructions.Get(uiInstruction)->uiOpcode) & (IR65F_CALLS | IR65F_CHANGES_XY))
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
bool CIR65Backend::ArgumentsReadFirst(void)
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

	pcEntry = mpcFunction->GetEntry();
	for (uiFirstPush = 0; uiFirstPush < pcEntry->masInstructions.NumElements(); uiFirstPush++)
	{
		psInstruction = pcEntry->masInstructions.Get(uiFirstPush);
		if ((psInstruction->uiOpcode == IRO_Arg) || (IR65OpcodeFlags(psInstruction->uiOpcode) & (IR65F_CALLS | IR65F_CHANGES_XY)))
		{
			break;
		}
	}

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
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
bool CIR65Backend::IsArrivalPlace(SIRReference sValue)
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
