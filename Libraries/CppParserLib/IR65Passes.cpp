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
#include "BaseLib/StringHelper.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/IntegerHelper.h"
#include "IR65BackendInternal.h"


//The 65C64 back end's rewrites of a function's IR before code is written for it (GenerateFunction).


//////////////////////////////////////////////////////////////////////////
//
// Puts the blocks in reverse post order, a brif's else visited first so its
// then block comes straight after it (a loop's body after its test, which
// RotateLoops then moves to the bottom).  Blocks nothing reaches go last.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::OrderBlocks(void)
{
	CArrayIRBlocks	apcOrder;
	size			ui;

	mcDominators.Compute(mpcFunction, true);
	apcOrder.Init();
	for (ui = 0; ui < mcDominators.NumReachable(); ui++)
	{
		apcOrder.Add(mcDominators.GetReversePostOrder(ui));
	}
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		if (!mcDominators.IsReachable(mpcFunction->GetBlock(ui)))
		{
			apcOrder.Add(mpcFunction->GetBlock(ui));
		}
	}
	mpcFunction->SetBlockOrder(&apcOrder);
	mpcFunction->ComputePredecessors();
	apcOrder.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Moves a loop's test to after its body: a block that branches into the
// block after it or out of the loop, and that the last block of the loop
// jumps back to, goes after that block.  The loop is entered by a bra to
// the test, and each time round takes one branch.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::RotateLoops(void)
{
	CArrayIRBlocks	apcOrder;
	CArrayIRBlocks	apcHeads;
	CArrayIRBlocks	apcLatches;
	size			ui;
	size			uiPredecessor;
	size			uiHead;
	size			uiLatch;
	CIRBlock*		pcHead;
	CIRBlock*		pcLatch;
	CIRBlock*		pcPredecessor;
	CIRBlock*		pcOutside;
	SIRJump*		psJump;

	mcDominators.Compute(mpcFunction);
	apcHeads.Init();
	apcLatches.Init();
	for (ui = 1; ui + 1 < mpcFunction->NumBlocks(); ui++)
	{
		pcHead = mpcFunction->GetBlock(ui);
		psJump = &pcHead->msJump;
		if ((psJump->uiKind != IRJ_Branch) || !mcDominators.IsReachable(pcHead))
		{
			continue;
		}
		if (psJump->pcThen == mpcFunction->GetBlock(ui + 1))
		{
			pcOutside = psJump->pcElse;
		}
		else if (psJump->pcElse == mpcFunction->GetBlock(ui + 1))
		{
			pcOutside = psJump->pcThen;
		}
		else
		{
			continue;
		}

		pcLatch = NULL;
		for (uiPredecessor = 0; uiPredecessor < pcHead->mapcPredecessors.NumElements(); uiPredecessor++)
		{
			pcPredecessor = pcHead->mapcPredecessors.GetPtr(uiPredecessor);
			if ((pcPredecessor->muiIndex > ui) && mcDominators.IsBackEdge(pcPredecessor, pcHead) &&
				((pcLatch == NULL) || (pcPredecessor->muiIndex > pcLatch->muiIndex)))
			{
				pcLatch = pcPredecessor;
			}
		}
		if ((pcLatch == NULL) || (pcLatch->msJump.uiKind != IRJ_Jump) || ((pcOutside->muiIndex > ui) && (pcOutside->muiIndex <= pcLatch->muiIndex)))
		{
			continue;
		}
		apcHeads.Add(pcHead);
		apcLatches.Add(pcLatch);
	}

	if (apcHeads.NumElements() > 0)
	{
		apcOrder.Init();
		apcOrder.Copy(&mpcFunction->mapcBlocks);
		for (ui = 0; ui < apcHeads.NumElements(); ui++)
		{
			pcHead = apcHeads.GetPtr(ui);
			uiHead = apcOrder.Find(pcHead);
			apcOrder.RemoveAt(uiHead, true);
			uiLatch = apcOrder.Find(apcLatches.GetPtr(ui));
			apcOrder.InsertAt(&pcHead, uiLatch + 1);
		}
		mpcFunction->SetBlockOrder(&apcOrder);
		apcOrder.Kill();
	}
	apcLatches.Kill();
	apcHeads.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// x > c becomes x >= c + 1 and x <= c becomes x < c + 1 (signed and
// unsigned, up to 32 bits, unless c is the largest value), so every
// comparison with a constant is cmp x, c with the operands as they are.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::AdjustComparisons(void)
{
	size			ui;
	size			uiInstruction;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRConstant*	psConstant;
	int64			lliValue;
	int64			lliLimit;
	int				iBits;
	bool			bSigned;

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			iBits = psInstruction->sType.IntegerBits();
			if (!IsIRSwappedComparison(psInstruction->uiOpcode) || !psInstruction->asArguments[1].IsConstant() || (iBits <= 1) || (iBits > 32))
			{
				continue;
			}
			bSigned = IsIRSignedComparison(psInstruction->uiOpcode);
			psConstant = mpcFunction->GetConstant(psInstruction->asArguments[1]);
			if (bSigned)
			{
				lliValue = IRSignExtend(psInstruction->sType, psConstant->ulliBits);
				lliLimit = (((int64)1) << (iBits - 1)) - 1;
			}
			else
			{
				lliValue = (int64)IRTruncate(psInstruction->sType, psConstant->ulliBits);
				lliLimit = (int64)IRTruncate(psInstruction->sType, (uint64)-1);
			}
			if (lliValue < lliLimit)
			{
				switch (psInstruction->uiOpcode)
				{
				case IRO_SGt:	psInstruction->uiOpcode = IRO_SGe;	break;
				case IRO_SLe:	psInstruction->uiOpcode = IRO_SLt;	break;
				case IRO_UGt:	psInstruction->uiOpcode = IRO_UGe;	break;
				default:		psInstruction->uiOpcode = IRO_ULt;	break;
				}
				psInstruction->asArguments[1] = mpcFunction->AddInteger(psInstruction->sType, lliValue + 1);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A signed comparison branches in one instruction only when it goes on
// less (braso (n, v)); there is no branch on n = v.  Blocks are laid out
// with a brif's then block next where they can (OrderBlocks), so it
// branches to its else block when the comparison is false: a comparison
// that would then need two branches is turned round (slt is sge, sgt is
// sle) with its blocks swapped, so the else block comes next instead.  A
// loop's head is left as it is for RotateLoops (its test goes to the end,
// branching back when it is true).
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::PreferSingleBranches(void)
{
	size			ui;
	size			uiPredecessor;
	CIRBlock*		pcBlock;
	CIRBlock*		pcThen;
	SIRInstruction*	psFused;
	uint16			uiOpcode;
	bool			bHead;

	mcDominators.Compute(mpcFunction);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		psFused = FusedComparison(pcBlock);
		if ((psFused == NULL) || (pcBlock->msJump.pcThen == pcBlock->msJump.pcElse) || !mcDominators.IsReachable(pcBlock))
		{
			continue;
		}
		bHead = false;
		for (uiPredecessor = 0; uiPredecessor < pcBlock->mapcPredecessors.NumElements(); uiPredecessor++)
		{
			bHead |= mcDominators.IsBackEdge(pcBlock->mapcPredecessors.GetPtr(uiPredecessor), pcBlock);
		}
		if (bHead)
		{
			continue;
		}
		uiOpcode = psFused->uiOpcode;
		if (((uiOpcode != IRO_SLt) && (uiOpcode != IRO_SGt)) || ((uiOpcode == IRO_SLt) && IsZero(psFused->asArguments[1])))
		{
			//One branch already (x < 0 branches on n).
			continue;
		}
		psFused->uiOpcode = IRInvertedComparison(uiOpcode);
		pcThen = pcBlock->msJump.pcThen;
		pcBlock->msJump.pcThen = pcBlock->msJump.pcElse;
		pcBlock->msJump.pcElse = pcThen;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The unsigned 32 bit udiv or urem that is DividePartner of one earlier in
// its block moves to just after it, so one div gives both (Divide).
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::PairDivides(void)
{
	size			ui;
	size			uiFirst;
	size			uiPartner;
	CIRBlock*		pcBlock;
	SIRInstruction	sPartner;
	uint16			uiOpcode;

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiFirst = 0; uiFirst + 1 < pcBlock->masInstructions.NumElements(); uiFirst++)
		{
			uiOpcode = pcBlock->masInstructions.Get(uiFirst)->uiOpcode;
			if ((uiOpcode != IRO_UDiv) && (uiOpcode != IRO_URem))
			{
				continue;
			}
			for (uiPartner = uiFirst + 2; uiPartner < pcBlock->masInstructions.NumElements(); uiPartner++)
			{
				if (DividePartner(pcBlock->masInstructions.Get(uiFirst), pcBlock->masInstructions.Get(uiPartner)))
				{
					//Its operands are the first's, so it can be worked out as early.
					sPartner = *pcBlock->masInstructions.Get(uiPartner);
					pcBlock->masInstructions.RemoveAt(uiPartner, true);
					pcBlock->masInstructions.InsertAt(&sPartner, uiFirst + 1);
					break;
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if psPartner is the unsigned 32 bit udiv of psFirst's urem (or the
// urem of its udiv): the same operands, so the one div does both.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::DividePartner(SIRInstruction* psFirst, SIRInstruction* psPartner)
{
	if (((psFirst->uiOpcode != IRO_UDiv) && (psFirst->uiOpcode != IRO_URem)) || IsWide(psFirst->sType))
	{
		return false;
	}
	return (psPartner->uiOpcode == ((psFirst->uiOpcode == IRO_UDiv) ? IRO_URem : IRO_UDiv)) && psPartner->sType.Equals(psFirst->sType) &&
		psPartner->asArguments[0].Equals(psFirst->asArguments[0]) && psPartner->asArguments[1].Equals(psFirst->asArguments[1]) && psPartner->sResult.IsTemporary();
}


//////////////////////////////////////////////////////////////////////////
//
// Moves a load used once, later in its block, to just before its use when
// nothing between writes memory (a store, a call, a volatile load) or
// pushes arguments, so the use can take it from A or as a memory operand.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::SinkLoads(void)
{
	size			ui;
	size			uiLoad;
	size			uiUse;
	CIRBlock*		pcBlock;
	SIRInstruction*	psLoad;
	SIRInstruction*	psUse;
	SIRInstruction	sLoad;
	SIRReference	sResult;
	bool			bBlocked;

	mpcFunction->CountUses(&mauiUses);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiLoad = pcBlock->masInstructions.NumElements(); uiLoad > 0; uiLoad--)
		{
			psLoad = pcBlock->masInstructions.Get(uiLoad - 1);
			sResult = psLoad->sResult;
			if ((psLoad->uiOpcode != IRO_Load) || (psLoad->uiFlags & IRF_Volatile) || !sResult.IsTemporary() || (*mauiUses.Get(sResult.Index()) != 1))
			{
				continue;
			}
			bBlocked = false;
			for (uiUse = uiLoad; uiUse < pcBlock->masInstructions.NumElements(); uiUse++)
			{
				psUse = pcBlock->masInstructions.Get(uiUse);
				if (psUse->asArguments[0].Equals(sResult) || psUse->asArguments[1].Equals(sResult))
				{
					break;
				}
				if ((psUse->uiOpcode == IRO_Store) || (psUse->uiOpcode == IRO_Call) || (psUse->uiOpcode == IRO_Arg) || ((psUse->uiOpcode == IRO_Load) && (psUse->uiFlags & IRF_Volatile)))
				{
					bBlocked = true;
					break;
				}
			}
			if (bBlocked || (uiUse >= pcBlock->masInstructions.NumElements()) || (uiUse == uiLoad) ||
				(pcBlock->masInstructions.Get(uiUse)->uiOpcode == IRO_Arg) || (pcBlock->masInstructions.Get(uiUse)->uiOpcode == IRO_Call))
			{
				continue;
			}
			sLoad = *psLoad;
			pcBlock->masInstructions.RemoveAt(uiLoad - 1, true);
			pcBlock->masInstructions.InsertAt(&sLoad, uiUse - 1);
		}
		SinkToEnd(pcBlock);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Moves an operation whose result only a successor's phi takes (the next
// time round a loop, say: p + 1) to the end of its block, before the
// comparison the block's brif may take.  Its operands' other uses come
// first, so an operand that dies there can share its home (add Y, 1).
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::SinkToEnd(CIRBlock* pcBlock)
{
	size			uiInstruction;
	size			uiUse;
	size			uiEnd;
	SIRInstruction*	psInstruction;
	SIRInstruction*	psUse;
	SIRInstruction	sMoved;
	SIRReference	sResult;
	bool			bUsed;
	bool			bOperandLater;

	uiEnd = pcBlock->masInstructions.NumElements();
	psInstruction = pcBlock->masInstructions.SafeGet(uiEnd - 1);
	if (psInstruction && psInstruction->sResult.Equals(pcBlock->msJump.sArgument))
	{
		uiEnd--;
	}
	for (uiInstruction = uiEnd; uiInstruction > 0; uiInstruction--)
	{
		psInstruction = pcBlock->masInstructions.Get(uiInstruction - 1);
		sResult = psInstruction->sResult;
		switch (psInstruction->uiOpcode)
		{
		case IRO_Add:
		case IRO_Sub:
		case IRO_PtrAdd:
			break;
		default:
			continue;
		}
		if (!sResult.IsTemporary() || (*mauiUses.Get(sResult.Index()) != 1) || pcBlock->msJump.sArgument.Equals(sResult))
		{
			continue;
		}
		//Only worth it when an operand is used again later in the block.
		bUsed = false;
		bOperandLater = false;
		for (uiUse = uiInstruction; uiUse < pcBlock->masInstructions.NumElements(); uiUse++)
		{
			psUse = pcBlock->masInstructions.Get(uiUse);
			if (psUse->asArguments[0].Equals(sResult) || psUse->asArguments[1].Equals(sResult))
			{
				bUsed = true;
				break;
			}
			bOperandLater |= (psInstruction->asArguments[0].IsTemporary() && (psUse->asArguments[0].Equals(psInstruction->asArguments[0]) || psUse->asArguments[1].Equals(psInstruction->asArguments[0]))) ||
							 (psInstruction->asArguments[1].IsTemporary() && (psUse->asArguments[0].Equals(psInstruction->asArguments[1]) || psUse->asArguments[1].Equals(psInstruction->asArguments[1])));
		}
		if (bUsed || !bOperandLater || (uiInstruction == uiEnd))
		{
			continue;
		}
		sMoved = *psInstruction;
		pcBlock->masInstructions.RemoveAt(uiInstruction - 1, true);
		pcBlock->masInstructions.InsertAt(&sMoved, uiEnd - 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if psInstruction is a ptradd of a constant (which a load or store
// can fold into its address: [8 + X]).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::ConstantPtrAdd(SIRInstruction* psInstruction)
{
	return (psInstruction->uiOpcode == IRO_PtrAdd) && psInstruction->sResult.IsTemporary() && psInstruction->asArguments[1].IsConstant() && !IsWide(mpcFunction->TypeOf(psInstruction->asArguments[1]));
}


//////////////////////////////////////////////////////////////////////////
//
// Gives each load and store whose address is a ptradd of a constant a ptradd
// of its own just before it (unless the one it uses is there and used only
// by it), so MarkFolded can fold it into the address.  A ptradd left unused
// isn't written.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::CopyAddresses(void)
{
	CArrayIRInstructions	asNew;
	CArrayTemplate<int32>	aiDefinitions;	//Per temporary: its constant ptradd in asDefinitions, -1 for none.
	CArrayIRInstructions	asDefinitions;
	size					ui;
	size					uiInstruction;
	size					uiArgument;
	CIRBlock*				pcBlock;
	SIRInstruction*			psInstruction;
	SIRInstruction*			psLast;
	SIRInstruction			sCopy;
	SIRReference			sAddress;
	int32					iDefinition;
	bool					bChanged;

	//SinkLoads counted the uses (moving loads doesn't change them).
	aiDefinitions.Init();
	aiDefinitions.GrowToAtLeastNumElements(mpcFunction->NumTemporaries(), true, -1);
	asDefinitions.Init();
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if (ConstantPtrAdd(psInstruction))
			{
				*aiDefinitions.Get(psInstruction->sResult.Index()) = (int32)asDefinitions.NumElements();
				asDefinitions.Add(psInstruction);
			}
		}
	}

	if (asDefinitions.NumElements() > 0)
	{
		asNew.Init();
		for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
		{
			pcBlock = mpcFunction->GetBlock(ui);
			asNew.FakeSetUsedElements(0);
			bChanged = false;
			for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
			{
				sCopy = *pcBlock->masInstructions.Get(uiInstruction);
				uiArgument = (sCopy.uiOpcode == IRO_Load) ? 0 : 1;
				sAddress = IRAddressOf(&sCopy);
				if (sAddress.IsTemporary() && (sAddress.Index() < aiDefinitions.NumElements()))
				{
					iDefinition = *aiDefinitions.Get(sAddress.Index());
					psLast = asNew.SafeGet(asNew.NumElements() - 1);
					if ((iDefinition >= 0) && !(psLast && psLast->sResult.Equals(sAddress) && (*mauiUses.Get(sAddress.Index()) == 1)))
					{
						psInstruction = asNew.Add(asDefinitions.Get(iDefinition));
						psInstruction->sResult = mpcFunction->AddTemporary(psInstruction->sType);
						sCopy.asArguments[uiArgument] = psInstruction->sResult;
						bChanged = true;
					}
				}
				asNew.Add(&sCopy);
			}
			if (bChanged)
			{
				pcBlock->masInstructions.Copy(&asNew);
			}
		}
		asNew.Kill();
	}
	asDefinitions.Kill();
	aiDefinitions.Kill();
}
