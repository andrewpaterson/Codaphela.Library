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


//The 65C64 back end's own rewrites of a function's IR before code is written for it (GenerateFunction); the ones any
//back end may make are CIRBackendPasses'.


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
