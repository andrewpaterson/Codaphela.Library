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
#include "IRW65C816BackendInternal.h"


//The W65C816 back end's fused instructions: which instructions are left to be worked out by the one that reads them
//(CanFuse, CanFuseReturn), and whether their inputs are still there when it does.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Fusion::Init(CIRW65C816Backend* pcBackend)
{
	mpcBackend = pcBackend;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Fusion::Kill(void)
{
	mpcBackend = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether sResult's place overlaps sInput's (a temporary's) so that writing
// its first word changes a later word of sInput's: any overlap when
// bAnyOverlap (a pointer read for each word), else one not at the same place.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Fusion::FusedOverlaps(SIRReference sResult, SIRReference sInput, bool bAnyOverlap)
{
	int32	iResult;
	int32	iInput;

	if (!sInput.IsTemporary() || sResult.IsNone())
	{
		return false;
	}
	iResult = mpcBackend->mcPlaces.OffsetOf(sResult);
	iInput = mpcBackend->mcPlaces.OffsetOf(sInput);
	if ((iResult >= iInput + mpcBackend->ValueBytes(mpcBackend->mpcFunction->GetTemporaryType(sInput))) || (iInput >= iResult + mpcBackend->ValueBytes(mpcBackend->mpcFunction->GetTemporaryType(sResult))))
	{
		return false;
	}
	return bAnyOverlap || (iResult != iInput);
}


//////////////////////////////////////////////////////////////////////////
//
// The instruction waiting to be read into A (CanFuse) that works out
// sValue, else NULL.
//
//////////////////////////////////////////////////////////////////////////
SIRInstruction* CIRW65C816Fusion::Fused(SIRReference sValue)
{
	int		i;

	for (i = 0; i < mpcBackend->miFused; i++)
	{
		if (mpcBackend->mapsFused[i]->sResult.Equals(sValue))
		{
			return mpcBackend->mapsFused[i];
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether what psInstruction (to be fused) reads is still there for each
// word after sResult's earlier words are written: a load's pointer anywhere
// else (it is read for each word) and in the direct page, other values not
// partly overlapped.  The fused values it reads aren't written.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Fusion::FusedInputsKept(SIRInstruction* psInstruction, SIRReference sResult)
{
	SIRReference	sPointer;
	SIRReference	sIndex;
	int32			iOffset;
	int				i;

	if (psInstruction->uiOpcode == IRO_Load)
	{
		//A direct slot's bytes are no temporary's: nothing is written over them.  Past the direct page it is read
		//through X ([D + X]), which what it is fused into may be keeping a word in (a return's high word).
		if (mpcBackend->mcFolds.DirectSlot(psInstruction->asArguments[0], &iOffset))
		{
			return IR816OnDirectPage(iOffset + 2 * mpcBackend->Words(psInstruction->sType));
		}
		sPointer = mpcBackend->mcFolds.FoldedBase(psInstruction->asArguments[0]);
		sIndex = mpcBackend->mcFolds.FoldedIndex(psInstruction->asArguments[0]);
		if (sIndex.IsTemporary() && !mpcBackend->mcPlaces.InX(sIndex) && FusedOverlaps(sResult, sIndex, true))
		{
			return false;
		}
		if (sPointer.IsTemporary())
		{
			return IR816PointerOnDirectPage(mpcBackend->mcPlaces.OffsetOf(sPointer)) && !FusedOverlaps(sResult, sPointer, true);
		}
		return sPointer.IsSymbol();
	}
	for (i = 0; i < 2; i++)
	{
		if (!Fused(psInstruction->asArguments[i]) && FusedOverlaps(sResult, psInstruction->asArguments[i], false))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether psInstruction is an and with a constant.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816AndsConstant(SIRInstruction* psInstruction)
{
	return (psInstruction->uiOpcode == IRO_And) && (psInstruction->asArguments[0].IsConstant() || psInstruction->asArguments[1].IsConstant());
}


//////////////////////////////////////////////////////////////////////////
//
// Whether instruction uiIndex of pcBlock, a load, and, or, xor or not of
// two or more words, or an add or sub, needn't be written: its result's one
// use is the next instruction, an and, or, xor, not, add or sub, or the
// value of a store, which then reads each word into A as it gets to it
// (WordInA): no store and load of each.  The instructions before it may
// be fused too (a chain read into A a word at a time), with one add or sub
// at most in the chain and the instruction that reads it (the carry), and a
// load in it only when the store at the end is to the same place (else the
// store could change a word still to be loaded).
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Fusion::CanFuse(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psInstruction;
	SIRInstruction*	psNext;
	SIRInstruction*	psChained;
	SIRReference	sResult;
	SIRReference	sNextResult;
	int				iCarries;		//The next instruction's.
	int				iChainCarries;	//The chain's, this one's too.
	int				i;
	bool			bStore;
	bool			bWordCopy;
	size			uiNext;

	if (mpcBackend->miFused >= IR816_MAX_FUSED)
	{
		return false;
	}
	psInstruction = pcBlock->masInstructions.Get(uiIndex);

	//The next instruction that writes code (IsFolded ones don't).
	for (uiNext = uiIndex + 1; (uiNext < pcBlock->masInstructions.NumElements()) && mpcBackend->mcFolds.IsFolded(pcBlock->masInstructions.Get(uiNext)); uiNext++)
	{
	}
	if (uiNext >= pcBlock->masInstructions.NumElements())
	{
		return CanFuseReturn(pcBlock, psInstruction);
	}
	psNext = pcBlock->masInstructions.Get(uiNext);
	sResult = psInstruction->sResult;

	//A 1 word load only stored by the next instruction is a copy through A (a byte's under the store's sep m), from
	//any address: there is no second word to read after the first is written.
	bWordCopy = (mpcBackend->miFused == 0) && (psInstruction->uiOpcode == IRO_Load) && (mpcBackend->Words(psInstruction->sType) == 1) && (psNext->uiOpcode == IRO_Store) &&
		psNext->sType.Equals(psInstruction->sType) && psNext->asArguments[0].Equals(sResult) && !psNext->asArguments[1].Equals(sResult);
	if (!sResult.IsTemporary() || ((mpcBackend->Words(psInstruction->sType) < 2) && !bWordCopy) || psInstruction->sType.IsFloat() ||
		(mpcBackend->masTemporaries.Get(sResult.Index())->iUseBlock != mpcBackend->miBlock))
	{
		return false;
	}

	iCarries = 0;
	iChainCarries = 0;
	switch (psInstruction->uiOpcode)
	{
	case IRO_Load:
		if (psInstruction->uiFlags & IRF_Volatile)
		{
			return false;
		}
		break;
	case IRO_Add:
	case IRO_PtrAdd:
		if (!mpcBackend->mcFolds.FoldedBase(sResult).Equals(sResult))
		{
			return false;
		}
		break;
	case IRO_Sub:
		break;
	case IRO_And:
	case IRO_Or:
	case IRO_Xor:
	case IRO_Not:
		break;
	default:
		return false;
	}

	bStore = false;
	sNextResult = IRNone();
	switch (psNext->uiOpcode)
	{
	case IRO_Store:
		if (!psNext->asArguments[0].Equals(sResult) || psNext->asArguments[1].Equals(sResult))
		{
			return false;
		}
		bStore = true;
		break;
	case IRO_Add:
	case IRO_PtrAdd:
	case IRO_Sub:
		if ((psNext->uiOpcode != IRO_Sub) && !mpcBackend->mcFolds.FoldedBase(psNext->sResult).Equals(psNext->sResult))
		{
			return false;
		}
		iCarries++;
		//Fall through.
	case IRO_And:
	case IRO_Or:
	case IRO_Xor:
	case IRO_Not:
		if ((mpcBackend->Words(psNext->sType) != mpcBackend->Words(psInstruction->sType)) || psNext->asArguments[0].Equals(psNext->asArguments[1]))
		{
			return false;
		}
		if (!psNext->asArguments[0].Equals(sResult) && !(psNext->asArguments[1].Equals(sResult) && (psNext->uiOpcode != IRO_Sub) && (psNext->uiOpcode != IRO_Not)))
		{
			return false;
		}
		sNextResult = psNext->sResult;
		break;
	case IRO_Mul:
	case IRO_UDiv:
	case IRO_SDiv:
	case IRO_URem:
	case IRO_SRem:
		//Read into the scratch routine's operand a word at a time (RuntimeCall), all before its result is written.
		if (!mpcBackend->mcSelector.IsScratchCall(psNext) || (mpcBackend->Words(psNext->sType) != mpcBackend->Words(psInstruction->sType)) || psNext->asArguments[0].Equals(psNext->asArguments[1]) ||
			(!psNext->asArguments[0].Equals(sResult) && !psNext->asArguments[1].Equals(sResult)))
		{
			return false;
		}
		break;
	default:
		return false;
	}
	if (sNextResult.IsTemporary() && (mpcBackend->mcPlaces.InX(sNextResult) || !IR816OnDirectPage(mpcBackend->mcPlaces.OffsetOf(sNextResult))))
	{
		return false;
	}

	//The whole chain: the carry, loads before a store, and what each reads.
	for (i = 0; i <= mpcBackend->miFused; i++)
	{
		psChained = (i < mpcBackend->miFused) ? mpcBackend->mapsFused[i] : psInstruction;
		if ((psChained->uiOpcode == IRO_Add) || (psChained->uiOpcode == IRO_PtrAdd) || (psChained->uiOpcode == IRO_Sub))
		{
			iChainCarries++;
		}
		if ((psChained->uiOpcode == IRO_Load) && bStore && !bWordCopy && !psChained->asArguments[0].Equals(psNext->asArguments[1]))
		{
			return false;
		}
		if (!FusedInputsKept(psChained, sNextResult))
		{
			return false;
		}
	}
	if (iChainCarries && (IR816AndsConstant(psInstruction) || IR816AndsConstant(psNext)))
	{
		//An and of a constant word 0 doesn't read the word: an add's carry wouldn't get to the next.
		return false;
	}
	return iChainCarries + iCarries <= 1;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether psInstruction, pcBlock's last, is fused into the block's ret of
// its two words (Return reads the high word into A then X first, so the
// chain can't have an add or sub's carry).
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Fusion::CanFuseReturn(CIRBlock* pcBlock, SIRInstruction* psInstruction)
{
	SIRInstruction*	psChained;
	int				i;

	if ((pcBlock->msJump.uiKind != IRJ_Return) || !pcBlock->msJump.sArgument.Equals(psInstruction->sResult) || (mpcBackend->msFrame.iHiddenResult >= 0) ||
		!psInstruction->sResult.IsTemporary() || (mpcBackend->Words(psInstruction->sType) != 2) || psInstruction->sType.IsFloat() ||
		(mpcBackend->masTemporaries.Get(psInstruction->sResult.Index())->iUseBlock != mpcBackend->miBlock))
	{
		return false;
	}
	for (i = 0; i <= mpcBackend->miFused; i++)
	{
		psChained = (i < mpcBackend->miFused) ? mpcBackend->mapsFused[i] : psInstruction;
		switch (psChained->uiOpcode)
		{
		case IRO_Load:
			if (psChained->uiFlags & IRF_Volatile)
			{
				return false;
			}
		case IRO_And:
		case IRO_Or:
		case IRO_Xor:
		case IRO_Not:
			break;
		default:
			return false;
		}
		if (!FusedInputsKept(psChained, IRNone()))
		{
			return false;
		}
	}
	return true;
}
