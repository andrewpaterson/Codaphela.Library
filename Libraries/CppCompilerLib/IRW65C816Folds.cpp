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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Folds::Init(CIRW65C816Backend* pcBackend)
{
	mpcBackend = pcBackend;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Folds::Kill(void)
{
	mpcBackend = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer plus a constant that is only used as loads' and stores'
// addresses isn't worked out: they use the pointer it adds to, with the
// constant in Y ([[D + n]:24 + Y]) or added to a symbol.  Its base pointer
// (msFoldBase) and constant (miFoldOffset) are kept per temporary.  Then
// pointers plus zero extended 16 bit values are folded too (FoldIndices).
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Folds::FoldPointers(void)
{
	CArrayTemplate<int32>	aiAddressUses;
	CArrayTemplate<int32>	aiUses;
	CArrayTemplate<uint32>	auiCounted;
	size					uiTemporaries;
	size					ui;
	size					uj;
	size					uk;
	CIRBlock*				pcBlock;
	SIRInstruction*			psInstruction;
	SIRReference			sNone;
	int32					iZero;
	int32					iUses;
	int64					lliOffset;

	uiTemporaries = mpcBackend->mpcFunction->NumTemporaries();
	sNone = IRNone();
	iZero = 0;
	mpcBackend->GrowTemporaries();
	aiAddressUses.Init(CIRW65C816Backend::GetScratch());
	aiUses.Init(CIRW65C816Backend::GetScratch());
	auiCounted.Init(CIRW65C816Backend::GetScratch());
	mpcBackend->mpcFunction->CountUses(&auiCounted);
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		mpcBackend->masTemporaries.Get(ui)->iCountUses = iZero;
		mpcBackend->masTemporaries.Get(ui)->sFoldBase = sNone;
		mpcBackend->masTemporaries.Get(ui)->iFoldOffset = iZero;
		mpcBackend->masTemporaries.Get(ui)->sFoldIndex = sNone;
		aiAddressUses.Add(&iZero);
		iUses = (int32)*auiCounted.Get(ui);
		aiUses.Add(&iUses);
	}
	auiCounted.Kill();

	//How many uses (CountUses) of each temporary are loads' or stores' addresses, and shifts' counts.
	for (ui = 0; ui < mpcBackend->mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcBackend->mpcFunction->GetBlock(ui);
		for (uj = 0; uj < pcBlock->masInstructions.NumElements(); uj++)
		{
			psInstruction = pcBlock->masInstructions.Get(uj);
			for (uk = 0; uk < 2; uk++)
			{
				if (psInstruction->asArguments[uk].IsTemporary())
				{
					if (((psInstruction->uiOpcode == IRO_Load) && (uk == 0)) || ((psInstruction->uiOpcode == IRO_Store) && (uk == 1)))
					{
						(*aiAddressUses.Get(psInstruction->asArguments[uk].Index()))++;
					}
					if ((uk == 1) && ((psInstruction->uiOpcode == IRO_Shl) || (psInstruction->uiOpcode == IRO_LShr) || (psInstruction->uiOpcode == IRO_AShr)))
					{
						(mpcBackend->masTemporaries.Get(psInstruction->asArguments[uk].Index())->iCountUses)++;
					}
				}
			}
		}
	}

	for (ui = 0; ui < mpcBackend->mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcBackend->mpcFunction->GetBlock(ui);
		for (uj = 0; uj < pcBlock->masInstructions.NumElements(); uj++)
		{
			psInstruction = pcBlock->masInstructions.Get(uj);
			if ((psInstruction->uiOpcode == IRO_PtrAdd) && psInstruction->sResult.IsTemporary() && psInstruction->asArguments[1].IsConstant() &&
				(psInstruction->asArguments[0].IsTemporary() || psInstruction->asArguments[0].IsSymbol()) &&
				(*aiUses.Get(psInstruction->sResult.Index()) > 0) && (*aiUses.Get(psInstruction->sResult.Index()) == *aiAddressUses.Get(psInstruction->sResult.Index())))
			{
				lliOffset = (int64)mpcBackend->mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits;
				if ((lliOffset >= 0) && (lliOffset <= 0xFF00))
				{
					mpcBackend->masTemporaries.Get(psInstruction->sResult.Index())->sFoldBase = psInstruction->asArguments[0];
					mpcBackend->masTemporaries.Get(psInstruction->sResult.Index())->iFoldOffset = (int32)lliOffset;
				}
			}
		}
	}

	//A folded pointer's base that is itself folded is followed to the end.
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		while (!mpcBackend->masTemporaries.Get(ui)->sFoldBase.IsNone() && mpcBackend->masTemporaries.Get(ui)->sFoldBase.IsTemporary() && !mpcBackend->masTemporaries.Get(mpcBackend->masTemporaries.Get(ui)->sFoldBase.Index())->sFoldBase.IsNone())
		{
			uk = mpcBackend->masTemporaries.Get(ui)->sFoldBase.Index();
			mpcBackend->masTemporaries.Get(ui)->iFoldOffset += mpcBackend->masTemporaries.Get(uk)->iFoldOffset;
			mpcBackend->masTemporaries.Get(ui)->sFoldBase = mpcBackend->masTemporaries.Get(uk)->sFoldBase;
		}
		if (mpcBackend->masTemporaries.Get(ui)->iFoldOffset > 0xFF00)
		{
			mpcBackend->masTemporaries.Get(ui)->sFoldBase = sNone;
			mpcBackend->masTemporaries.Get(ui)->iFoldOffset = 0;
		}
	}
	FoldIndices(&aiUses);
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		mpcBackend->masTemporaries.Get(ui)->iUses = *aiUses.Get(ui);
	}
	aiUses.Kill();
	aiAddressUses.Kill();
	FindDirectSlots();
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer plus a zero extended i16 x (ptradd p, zext x) that is only used
// as the addresses of 1 word loads and stores isn't worked out: they put x
// in Y and use p ([[D + p]:24 + Y]; the W65C816 adds Y to all 24 bits).
// 2 word ones too when the ptradd is IRF_InArray: x + 2 then can't wrap
// (LoadY's iny).
// p must be a temporary that isn't folded itself, and nothing folded may
// add to the ptradd (Y would be x plus a constant, which can wrap).  The
// zext isn't worked out either when the folded ptradds are all its uses:
// it is folded to x (FoldedBase), so x lives to the loads and stores.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Folds::FoldIndices(CArrayTemplate<int32>* paiUses)
{
	CArrayTemplate<SIRInstruction*>	apsDefinitions;
	CArrayTemplate<int32>			aiWordUses;		//Per temporary: its uses as a 1 word load's or store's address.
	CArrayTemplate<int32>			aiPairUses;		//Per temporary: its uses as a 2 word load's or store's address.
	CArrayTemplate<int32>			aiFoldedUses;	//Per zext: its uses by ptradds folded here.
	CArrayTemplate<uint8>			abBase;			//Per temporary: something folded adds to it.
	SIRInstruction*					psInstruction;
	SIRInstruction*					psExtend;
	CIRBlock*						pcBlock;
	SIRInstruction*					psNull;
	SIRReference					sPointer;
	size							uiTemporaries;
	size							ui;
	size							uj;
	int32							iZero;
	uint8							uiFalse;

	uiTemporaries = mpcBackend->mpcFunction->NumTemporaries();
	psNull = NULL;
	iZero = 0;
	uiFalse = 0;
	apsDefinitions.Init(CIRW65C816Backend::GetScratch());
	aiWordUses.Init(CIRW65C816Backend::GetScratch());
	aiPairUses.Init(CIRW65C816Backend::GetScratch());
	aiFoldedUses.Init(CIRW65C816Backend::GetScratch());
	abBase.Init(CIRW65C816Backend::GetScratch());
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		apsDefinitions.Add(&psNull);
		aiWordUses.Add(&iZero);
		aiPairUses.Add(&iZero);
		aiFoldedUses.Add(&iZero);
		abBase.Add(&uiFalse);
	}
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		if (!mpcBackend->masTemporaries.Get(ui)->sFoldBase.IsNone() && mpcBackend->masTemporaries.Get(ui)->sFoldBase.IsTemporary())
		{
			*abBase.Get(mpcBackend->masTemporaries.Get(ui)->sFoldBase.Index()) = true;
		}
	}
	for (ui = 0; ui < mpcBackend->mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcBackend->mpcFunction->GetBlock(ui);
		for (uj = 0; uj < pcBlock->masInstructions.NumElements(); uj++)
		{
			psInstruction = pcBlock->masInstructions.Get(uj);
			if (psInstruction->sResult.IsTemporary())
			{
				*apsDefinitions.Get(psInstruction->sResult.Index()) = psInstruction;
			}
			if (psInstruction->uiOpcode == IRO_Load)
			{
				sPointer = psInstruction->asArguments[0];
			}
			else if (psInstruction->uiOpcode == IRO_Store)
			{
				sPointer = psInstruction->asArguments[1];
			}
			else
			{
				continue;
			}
			if (sPointer.IsTemporary() && (mpcBackend->Words(psInstruction->sType) == 1) && !(psInstruction->uiFlags & IRF_Volatile))
			{
				(*aiWordUses.Get(sPointer.Index()))++;
			}
			else if (sPointer.IsTemporary() && (mpcBackend->Words(psInstruction->sType) == 2) && !(psInstruction->uiFlags & IRF_Volatile))
			{
				(*aiPairUses.Get(sPointer.Index()))++;
			}
		}
	}

	for (ui = 0; ui < mpcBackend->mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcBackend->mpcFunction->GetBlock(ui);
		for (uj = 0; uj < pcBlock->masInstructions.NumElements(); uj++)
		{
			psInstruction = pcBlock->masInstructions.Get(uj);
			if ((psInstruction->uiOpcode != IRO_PtrAdd) || !psInstruction->sResult.IsTemporary() || !psInstruction->asArguments[0].IsTemporary() ||
				!psInstruction->asArguments[1].IsTemporary() || !mpcBackend->masTemporaries.Get(psInstruction->sResult.Index())->sFoldBase.IsNone() ||
				!mpcBackend->masTemporaries.Get(psInstruction->asArguments[0].Index())->sFoldBase.IsNone() || *abBase.Get(psInstruction->sResult.Index()) ||
				(*paiUses->Get(psInstruction->sResult.Index()) == 0) ||
				(*paiUses->Get(psInstruction->sResult.Index()) != *aiWordUses.Get(psInstruction->sResult.Index()) + ((psInstruction->uiFlags & IRF_InArray) ? *aiPairUses.Get(psInstruction->sResult.Index()) : 0)))
			{
				continue;
			}
			psExtend = *apsDefinitions.Get(psInstruction->asArguments[1].Index());
			if ((psExtend == NULL) || (psExtend->uiOpcode != IRO_ZExt) || !psExtend->asArguments[0].IsTemporary() ||
				!mpcBackend->mpcFunction->GetTemporaryType(psExtend->asArguments[0]).Is(IRTK_I16))
			{
				continue;
			}
			mpcBackend->masTemporaries.Get(psInstruction->sResult.Index())->sFoldBase = psInstruction->asArguments[0];
			mpcBackend->masTemporaries.Get(psInstruction->sResult.Index())->iFoldOffset = 0;
			mpcBackend->masTemporaries.Get(psInstruction->sResult.Index())->sFoldIndex = psExtend->asArguments[0];
			(*aiFoldedUses.Get(psInstruction->asArguments[1].Index()))++;
		}
	}

	//A zext only the folded ptradds use isn't worked out.
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		psExtend = *apsDefinitions.Get(ui);
		if (psExtend && (psExtend->uiOpcode == IRO_ZExt) && (*aiFoldedUses.Get(ui) > 0) && (*aiFoldedUses.Get(ui) == *paiUses->Get(ui)))
		{
			mpcBackend->masTemporaries.Get(ui)->sFoldBase = psExtend->asArguments[0];
			mpcBackend->masTemporaries.Get(ui)->iFoldOffset = 0;
		}
	}
	apsDefinitions.Kill();
	aiWordUses.Kill();
	aiPairUses.Kill();
	aiFoldedUses.Kill();
	abBase.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The i16 a folded pointer adds in Y (FoldIndices), else none.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRW65C816Folds::FoldedIndex(SIRReference sValue)
{
	if (sValue.IsTemporary() && (sValue.Index() < mpcBackend->masTemporaries.NumElements()))
	{
		return mpcBackend->masTemporaries.Get(sValue.Index())->sFoldIndex;
	}
	return IRNone();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an instruction isn't worked out: a ptradd or add folded into its
// loads and stores (FoldPointers), or a zext folded into its ptradds
// (FoldIndices).  It writes no code.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Folds::IsFolded(SIRInstruction* psInstruction)
{
	return ((psInstruction->uiOpcode == IRO_PtrAdd) || (psInstruction->uiOpcode == IRO_Add) || (psInstruction->uiOpcode == IRO_ZExt)) &&
		psInstruction->sResult.IsTemporary() && !FoldedBase(psInstruction->sResult).Equals(psInstruction->sResult);
}


//////////////////////////////////////////////////////////////////////////
//
// Y for word iOffset / 2 of a load or store through sPointer: the folded
// index (plus iOffset by iny: a 2 word access is IRF_InArray, FoldIndices,
// so it can't wrap), else the constant.  Word 0 with no index needs no Y
// ([[D + p]:24]): returns whether Y was loaded.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Folds::LoadY(SIRReference sPointer, int32 iOffset)
{
	SIRReference	sIndex;

	sIndex = FoldedIndex(sPointer);
	if (sIndex.IsNone())
	{
		if (iOffset == 0)
		{
			return false;
		}
		mpcBackend->Line("ldy ", iOffset);
		return true;
	}
	mpcBackend->mcSelector.Operand("ldy", sIndex, 0);
	for (; iOffset > 0; iOffset--)
	{
		mpcBackend->Line("iny");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A value, or the base pointer a folded pointer adds to.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRW65C816Folds::FoldedBase(SIRReference sValue)
{
	if (sValue.IsTemporary() && (sValue.Index() < mpcBackend->masTemporaries.NumElements()) && !mpcBackend->masTemporaries.Get(sValue.Index())->sFoldBase.IsNone())
	{
		return mpcBackend->masTemporaries.Get(sValue.Index())->sFoldBase;
	}
	return sValue;
}


//////////////////////////////////////////////////////////////////////////
//
// The kept slots (IRF_Volatile on the slot: a variable a debugger reads)
// whose every use is a load's or store's address, the slot's or a folded
// ptradd's of it with no index: each is addressed in place by its loads
// and stores (DirectSlot) and its address is never worked out, so it has
// no place of its own.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Folds::FindDirectSlots(void)
{
	size				uiBlock;
	size				uiInstruction;
	size				uiPhi;
	size				uiArgument;
	int					iArgument;
	CIRBlock*			pcBlock;
	SIRInstruction*		psInstruction;
	SIRReference		sUse;
	SIRReference		sBase;
	SIRW65C816Temporary*	psBase;
	bool				bAddress;

	for (uiBlock = 0; uiBlock < mpcBackend->mpcFunction->NumBlocks(); uiBlock++)
	{
		pcBlock = mpcBackend->mpcFunction->GetBlock(uiBlock);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if ((psInstruction->uiOpcode == IRO_Slot) && (psInstruction->uiFlags & IRF_Volatile) && psInstruction->sResult.IsTemporary())
			{
				mpcBackend->masTemporaries.Get(psInstruction->sResult.Index())->bDirectSlot = true;
			}
		}
	}

	//A use other than as a load's or store's address (with no index), or by a ptradd folded onto it, takes it away.
	for (uiBlock = 0; uiBlock < mpcBackend->mpcFunction->NumBlocks(); uiBlock++)
	{
		pcBlock = mpcBackend->mpcFunction->GetBlock(uiBlock);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			for (iArgument = 0; iArgument < IR_INSTRUCTION_ARGUMENTS; iArgument++)
			{
				sUse = psInstruction->asArguments[iArgument];
				sBase = FoldedBase(sUse);
				if (!sBase.IsTemporary() || !mpcBackend->masTemporaries.Get(sBase.Index())->bDirectSlot)
				{
					continue;
				}
				bAddress = ((psInstruction->uiOpcode == IRO_Load) && (iArgument == 0)) || ((psInstruction->uiOpcode == IRO_Store) && (iArgument == 1));
				bAddress = (bAddress && FoldedIndex(sUse).IsNone()) || ((iArgument == 0) && IsFolded(psInstruction));
				if (!bAddress)
				{
					mpcBackend->masTemporaries.Get(sBase.Index())->bDirectSlot = false;
				}
			}
		}
		for (uiPhi = 0; uiPhi < pcBlock->masPhis.NumElements(); uiPhi++)
		{
			for (uiArgument = 0; uiArgument < pcBlock->masPhis.Get(uiPhi)->asArguments.NumElements(); uiArgument++)
			{
				sBase = FoldedBase(pcBlock->masPhis.Get(uiPhi)->asArguments.Get(uiArgument)->sValue);
				if (sBase.IsTemporary())
				{
					mpcBackend->masTemporaries.Get(sBase.Index())->bDirectSlot = false;
				}
			}
		}
		sBase = FoldedBase(pcBlock->msJump.sArgument);
		if (sBase.IsTemporary())
		{
			psBase = mpcBackend->masTemporaries.Get(sBase.Index());
			psBase->bDirectSlot = false;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether sValue is a direct slot (FindDirectSlots).
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Folds::IsDirectSlot(SIRReference sValue)
{
	return sValue.IsTemporary() && mpcBackend->masTemporaries.Get(sValue.Index())->bDirectSlot;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a load or store through sPointer is in a direct slot: *piOffset
// is then where in the frame (D + offset), the slot's place plus what was
// folded onto it.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Folds::DirectSlot(SIRReference sPointer, int32* piOffset)
{
	SIRReference	sBase;

	sBase = FoldedBase(sPointer);
	if (!IsDirectSlot(sBase))
	{
		return false;
	}
	*piOffset = mpcBackend->masTemporaries.Get(sBase.Index())->iSlot;
	if (!sBase.Equals(sPointer))
	{
		*piOffset += mpcBackend->masTemporaries.Get(sPointer.Index())->iFoldOffset;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// What a use of sValue needs worked out: its folded base, unless that is a
// direct slot, which nothing is (IRNone).
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRW65C816Folds::Materialised(SIRReference sValue)
{
	sValue = FoldedBase(sValue);
	return IsDirectSlot(sValue) ? IRNone() : sValue;
}
