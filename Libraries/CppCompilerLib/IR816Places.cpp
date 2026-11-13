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
#include "IR816BackendInternal.h"
#include "IRLiveSets.h"


//The W65C816 back end's places: the frame's layout (Layout), pointers folded into their loads and stores
//(FoldPointers), the temporaries' places in their regions (AddRegions, PackTemporaries), shared where they are
//never live at once, and the temporaries kept in X (AllocateX).


#define IR816_ARGUMENT_STANDIN	0x10000		//The pushed parameters' stand-in places while temporaries are placed (AddRegions), past any frame.
#define IR816_PAIR_WEIGHT(iDepth, bCopy, bIn)	(4 * (iDepth) + ((bCopy) ? 2 : 0) + ((bIn) ? 1 : 0))	//A pair's weight: the loops it is in, then a phi's copy (a whole copy saved), then a copy into a loop's head (made each time the loop is entered; each of its ways out only some of the times).


//////////////////////////////////////////////////////////////////////////
//
// Whether a frame offset is a direct page operand, [D + n], so needn't be
// reached through X.
//
//////////////////////////////////////////////////////////////////////////
bool IR816OnDirectPage(int32 iOffset)
{
	return iOffset < IR816_DIRECT_PAGE;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a pointer at a frame offset can be read where it is, as [[D + n]]
// or [[D + n], y]: all of its 3 bytes are on the direct page.
//
//////////////////////////////////////////////////////////////////////////
bool IR816PointerOnDirectPage(int32 iOffset)
{
	return iOffset + IR816_LONG_POINTER_BYTES <= IR816_DIRECT_PAGE;
}


//////////////////////////////////////////////////////////////////////////
//
// The frame (msFrame), its parts added in order: the temporaries after the scratch bytes, shared by those that are
// never live at once; then each slot its own place.  The arguments are above the frame and the link:
// the hidden result pointer first, then the parameters in order.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::Layout(void)
{
	size				ui;
	size				uiBlock;
	size				uiInstruction;
	int32				iNone;
	int32				iArgument;
	int32				iPlace;
	int32				iRest;
	size				uiArgument;
	CIRBlock*			pcBlock;
	SIRInstruction*		psInstruction;
	SIRReference		sParameter;

	iNone = -1;
	maiOffsets.FakeSetUsedElements(0);
	maiSlots.FakeSetUsedElements(0);
	maiParameters.FakeSetUsedElements(0);
	for (ui = 0; ui < mpcFunction->NumTemporaries(); ui++)
	{
		maiOffsets.Add(&iNone);
		maiSlots.Add(&iNone);
	}

	//The temporaries share places when they aren't live at once (PackTemporaries); then the slots, each a place of its
	//own.
	FoldPointers();
	msFrame.iFrameBytes = PackTemporaries() - 1;

	//A frame (with its arguments) past the page keeps S, as it was when aligned (AlignFrame), in two bytes in the page.
	iRest = IR816_LINK_BYTES + (IsHiddenResult(mpcFunction->msReturnType) ? 4 : 0);
	for (ui = 0; ui < mpcFunction->masParameters.NumElements(); ui++)
	{
		iRest += ValueBytes(mpcFunction->GetTemporaryType(*mpcFunction->masParameters.Get(ui)));
	}
	for (uiBlock = 0; uiBlock < mpcFunction->NumBlocks(); uiBlock++)
	{
		pcBlock = mpcFunction->GetBlock(uiBlock);
		for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
		{
			iRest += (ValueBytes(pcBlock->masPhis.Get(ui)->sType) + 1) & ~1;
		}
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if (psInstruction->uiOpcode == IRO_Slot)
			{
				iRest += ((int32)mpcFunction->GetConstant(psInstruction->asArguments[0])->ulliBits + 1) & ~1;
			}
		}
	}
	msFrame.iSavedS = ((msFrame.Link() + iRest > IR816_DIRECT_PAGE) && (msFrame.Link() + 2 <= IR816_DIRECT_PAGE)) ? msFrame.Add(2) : -1;
	for (uiBlock = 0; uiBlock < mpcFunction->NumBlocks(); uiBlock++)
	{
		pcBlock = mpcFunction->GetBlock(uiBlock);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if (psInstruction->uiOpcode == IRO_Slot)
			{
				*maiSlots.Get(psInstruction->sResult.Index()) = msFrame.Add((int32)mpcFunction->GetConstant(psInstruction->asArguments[0])->ulliBits);
			}
		}
	}
	//The register arguments' words are the frame's top, the first lowest: the prologue pushes them there (Prologue).
	msFrame.iRegisterBase = (msFrame.iRegisterWords > 0) ? msFrame.Add(2 * msFrame.iRegisterWords) : -1;

	//The pushed arguments are above the frame and the link, in order.
	iArgument = msFrame.Arguments();
	uiArgument = 0;
	if (IsHiddenResult(mpcFunction->msReturnType))
	{
		msFrame.iHiddenResult = ArgumentPlace(masArguments.Get(uiArgument), &iArgument);
		uiArgument++;
	}
	for (ui = 0; ui < mpcFunction->masParameters.NumElements(); ui++)
	{
		sParameter = *mpcFunction->masParameters.Get(ui);
		iPlace = ArgumentPlace(masArguments.Get(uiArgument), &iArgument);
		uiArgument++;
		maiParameters.Add(&iPlace);
		*maiOffsets.Get(sParameter.Index()) = iPlace;
	}
	msFrame.iArgumentBytes = iArgument - msFrame.Arguments();

	//The temporaries at a pushed parameter's stand-in place (AddRegions) get the parameter's place: no frame reaches
	//IR816_ARGUMENT_STANDIN.
	for (ui = 0; ui < mpcFunction->NumTemporaries(); ui++)
	{
		if (*maiOffsets.Get(ui) >= IR816_ARGUMENT_STANDIN)
		{
			*maiOffsets.Get(ui) += msFrame.Arguments() - IR816_ARGUMENT_STANDIN;
		}
	}

	//Frame reaches past the direct page through X, so nothing is kept in X then.
	if (!IR816OnDirectPage(iArgument))
	{
		for (ui = 0; ui < mabInX.NumElements(); ui++)
		{
			*mabInX.Get(ui) = false;
		}
	}
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
void CIRW65C816Backend::FoldPointers(void)
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

	uiTemporaries = mpcFunction->NumTemporaries();
	sNone = IRNone();
	iZero = 0;
	masFoldBase.FakeSetUsedElements(0);
	maiFoldOffset.FakeSetUsedElements(0);
	masFoldIndex.FakeSetUsedElements(0);
	aiAddressUses.Init(CIRW65C816Backend::GetScratch());
	aiUses.Init(CIRW65C816Backend::GetScratch());
	auiCounted.Init(CIRW65C816Backend::GetScratch());
	mpcFunction->CountUses(&auiCounted);
	maiCountUses.FakeSetUsedElements(0);
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		maiCountUses.Add(&iZero);
		masFoldBase.Add(&sNone);
		maiFoldOffset.Add(&iZero);
		masFoldIndex.Add(&sNone);
		aiAddressUses.Add(&iZero);
		iUses = (int32)*auiCounted.Get(ui);
		aiUses.Add(&iUses);
	}
	auiCounted.Kill();

	//How many uses (CountUses) of each temporary are loads' or stores' addresses, and shifts' counts.
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
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
						(*maiCountUses.Get(psInstruction->asArguments[uk].Index()))++;
					}
				}
			}
		}
	}

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uj = 0; uj < pcBlock->masInstructions.NumElements(); uj++)
		{
			psInstruction = pcBlock->masInstructions.Get(uj);
			if ((psInstruction->uiOpcode == IRO_PtrAdd) && psInstruction->sResult.IsTemporary() && psInstruction->asArguments[1].IsConstant() &&
				(psInstruction->asArguments[0].IsTemporary() || psInstruction->asArguments[0].IsSymbol()) &&
				(*aiUses.Get(psInstruction->sResult.Index()) > 0) && (*aiUses.Get(psInstruction->sResult.Index()) == *aiAddressUses.Get(psInstruction->sResult.Index())))
			{
				lliOffset = (int64)mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits;
				if ((lliOffset >= 0) && (lliOffset <= 0xFF00))
				{
					*masFoldBase.Get(psInstruction->sResult.Index()) = psInstruction->asArguments[0];
					*maiFoldOffset.Get(psInstruction->sResult.Index()) = (int32)lliOffset;
				}
			}
		}
	}

	//A folded pointer's base that is itself folded is followed to the end.
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		while (!masFoldBase.Get(ui)->IsNone() && masFoldBase.Get(ui)->IsTemporary() && !masFoldBase.Get(masFoldBase.Get(ui)->Index())->IsNone())
		{
			uk = masFoldBase.Get(ui)->Index();
			*maiFoldOffset.Get(ui) += *maiFoldOffset.Get(uk);
			*masFoldBase.Get(ui) = *masFoldBase.Get(uk);
		}
		if (*maiFoldOffset.Get(ui) > 0xFF00)
		{
			*masFoldBase.Get(ui) = sNone;
			*maiFoldOffset.Get(ui) = 0;
		}
	}
	FoldIndices(&aiUses);
	maiUses.FakeSetUsedElements(0);
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		maiUses.Add(aiUses.Get(ui));
	}
	aiUses.Kill();
	aiAddressUses.Kill();
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
void CIRW65C816Backend::FoldIndices(CArrayTemplate<int32>* paiUses)
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

	uiTemporaries = mpcFunction->NumTemporaries();
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
		if (!masFoldBase.Get(ui)->IsNone() && masFoldBase.Get(ui)->IsTemporary())
		{
			*abBase.Get(masFoldBase.Get(ui)->Index()) = true;
		}
	}
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
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
			if (sPointer.IsTemporary() && (Words(psInstruction->sType) == 1) && !(psInstruction->uiFlags & IRF_Volatile))
			{
				(*aiWordUses.Get(sPointer.Index()))++;
			}
			else if (sPointer.IsTemporary() && (Words(psInstruction->sType) == 2) && !(psInstruction->uiFlags & IRF_Volatile))
			{
				(*aiPairUses.Get(sPointer.Index()))++;
			}
		}
	}

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uj = 0; uj < pcBlock->masInstructions.NumElements(); uj++)
		{
			psInstruction = pcBlock->masInstructions.Get(uj);
			if ((psInstruction->uiOpcode != IRO_PtrAdd) || !psInstruction->sResult.IsTemporary() || !psInstruction->asArguments[0].IsTemporary() ||
				!psInstruction->asArguments[1].IsTemporary() || !masFoldBase.Get(psInstruction->sResult.Index())->IsNone() ||
				!masFoldBase.Get(psInstruction->asArguments[0].Index())->IsNone() || *abBase.Get(psInstruction->sResult.Index()) ||
				(*paiUses->Get(psInstruction->sResult.Index()) == 0) ||
				(*paiUses->Get(psInstruction->sResult.Index()) != *aiWordUses.Get(psInstruction->sResult.Index()) + ((psInstruction->uiFlags & IRF_InArray) ? *aiPairUses.Get(psInstruction->sResult.Index()) : 0)))
			{
				continue;
			}
			psExtend = *apsDefinitions.Get(psInstruction->asArguments[1].Index());
			if ((psExtend == NULL) || (psExtend->uiOpcode != IRO_ZExt) || !psExtend->asArguments[0].IsTemporary() ||
				!mpcFunction->GetTemporaryType(psExtend->asArguments[0]).Is(IRTK_I16))
			{
				continue;
			}
			*masFoldBase.Get(psInstruction->sResult.Index()) = psInstruction->asArguments[0];
			*maiFoldOffset.Get(psInstruction->sResult.Index()) = 0;
			*masFoldIndex.Get(psInstruction->sResult.Index()) = psExtend->asArguments[0];
			(*aiFoldedUses.Get(psInstruction->asArguments[1].Index()))++;
		}
	}

	//A zext only the folded ptradds use isn't worked out.
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		psExtend = *apsDefinitions.Get(ui);
		if (psExtend && (psExtend->uiOpcode == IRO_ZExt) && (*aiFoldedUses.Get(ui) > 0) && (*aiFoldedUses.Get(ui) == *paiUses->Get(ui)))
		{
			*masFoldBase.Get(ui) = psExtend->asArguments[0];
			*maiFoldOffset.Get(ui) = 0;
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
SIRReference CIRW65C816Backend::FoldedIndex(SIRReference sValue)
{
	if (sValue.IsTemporary() && (sValue.Index() < masFoldIndex.NumElements()))
	{
		return *masFoldIndex.Get(sValue.Index());
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
bool CIRW65C816Backend::IsFolded(SIRInstruction* psInstruction)
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
bool CIRW65C816Backend::LoadY(SIRReference sPointer, int32 iOffset)
{
	SIRReference	sIndex;

	sIndex = FoldedIndex(sPointer);
	if (sIndex.IsNone())
	{
		if (iOffset == 0)
		{
			return false;
		}
		Line("ldy ", iOffset);
		return true;
	}
	Operand("ldy", sIndex, 0);
	for (; iOffset > 0; iOffset--)
	{
		Line("iny");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A value, or the base pointer a folded pointer adds to.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRW65C816Backend::FoldedBase(SIRReference sValue)
{
	if (sValue.IsTemporary() && (sValue.Index() < masFoldBase.NumElements()) && !masFoldBase.Get(sValue.Index())->IsNone())
	{
		return *masFoldBase.Get(sValue.Index());
	}
	return sValue;
}


#define IR816_EVENT_USE		0


#define IR816_EVENT_DEFINE	1


#define IR816_EVENT_WRITE	2


struct SIR816Event
{
	int32	iTemporary;
	int32	iPosition;
	int32	iKind;		//IR816_EVENT_USE, _DEFINE or _WRITE.
};


//A load that may have its pointer's place (PackTemporaries).
struct SIR816LoadPair
{
	SIRReference	sResult;
	SIRReference	sPointer;
	int32			iPosition;
	int32			iDepth;
	bool			bWide;		//Two or more words: Load keeps one on the stack.
};


//What PackTemporaries finds in the blocks, in order.
struct SIR816PackEvents
{
	CArrayTemplate<SIR816Event>		asEvents;
	CArrayTemplate<SIR816LoadPair>	asLoadPairs;
	CArrayTemplate<int32>			aiBlockEvents;	//Per block: its first event, and one past the last block's last.
	CArrayTemplate<int32>			aiBlockStart;	//Per block: its first position...
	CArrayTemplate<int32>			aiBlockEnd;		//...and its last.

	void	Init(void);
	void	Kill(void);
};


//A run of places temporaries may have (CIRW65C816Backend::AddRegions): the pushed arguments' (at their stand-in places
//until Layout knows the frame's size) or the frame's own.  Its words follow the regions' before it in the
//occupants' lists (maiWordFirst), so places far apart cost nothing.
struct SIR816Region
{
	int32	iStart;
	int32	iEnd;		//One past its last byte.
	int32	iFirstWord;
};


//////////////////////////////////////////////////////////////////////////
//
// The positions each temporary is live over, as ranges in position order
// (with holes where it isn't live), and the pairs of temporaries that may
// meet at one position when they have the same place: an instruction's
// result and an argument it reads a word at a time ahead of writing that
// word, and a phi and its value on a jump that copies the value to it.
// And the regions a temporary's place must lie wholly inside (IsFree).
//
//////////////////////////////////////////////////////////////////////////
class CIRW65C816LiveRanges
{
public:
	CArrayTemplate<int32>	maiFirst;		//Per temporary: its first range (or -1), a list through maiNext.
	CArrayTemplate<int32>	maiLast;		//Per temporary: its last range.
	CArrayTemplate<int32>	maiStart;		//Per range.
	CArrayTemplate<int32>	maiEnd;
	CArrayTemplate<int32>	maiNext;
	CArrayTemplate<int32>	maiPairFirst;	//Per temporary: its first pair (or -1), a list through maiPairNext.
	CArrayTemplate<int32>	maiPairNext;
	CArrayTemplate<int32>	maiPairOther;	//Per pair: the other temporary...
	CArrayTemplate<int32>	maiPairPosition;	//...where they meet...
	CArrayTemplate<int32>	maiPairWeight;		//...and how much sharing a place saves (IR816_PAIR_WEIGHT).
	CArrayTemplate<int32>	maiWordFirst;		//Per word of the regions: its first occupant (or -1), a list through maiOccupantNext.
	CArrayTemplate<int32>	maiOccupantNext;
	CArrayTemplate<int32>	maiOccupant;		//Per occupant: the temporary placed on the word.
	CArrayTemplate<SIR816Region>	masRegions;

	void	Init(size uiTemporaries);
	void	Kill(void);
	void	Add(int32 iTemporary, int32 iStart, int32 iEnd);
	void	AddPair(SIRReference sOne, SIRReference sOther, int32 iPosition, int32 iWeight);
	bool	IsPair(int32 iOne, int32 iOther, int32 iPosition);
	bool	IsPaired(int32 iOne, int32 iOther);
	bool	Interferes(int32 iOne, int32 iOther, bool bSamePlace);
	int32	Start(int32 iTemporary);
	int32	End(int32 iTemporary);
	void	AddRegion(int32 iStart, int32 iEnd);
	bool	Fits(int32 iOffset, int32 iBytes);
	int32	WordOf(int32 iOffset);
	bool	IsFree(int32 iTemporary, int32 iOffset, int32 iBytes, CArrayTemplate<int32>* paiOffsets);
	bool	LiveAcross(int32 iTemporary, int32 iPosition);
	void	Take(int32 iTemporary, int32 iOffset, int32 iBytes);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816LiveRanges::Init(size uiTemporaries)
{
	size	ui;
	int32	iNone;

	iNone = -1;
	maiFirst.Init(CIRW65C816Backend::GetScratch());
	maiLast.Init(CIRW65C816Backend::GetScratch());
	maiPairFirst.Init(CIRW65C816Backend::GetScratch());
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		maiFirst.Add(&iNone);
		maiLast.Add(&iNone);
		maiPairFirst.Add(&iNone);
	}
	maiStart.Init(CIRW65C816Backend::GetScratch());
	maiEnd.Init(CIRW65C816Backend::GetScratch());
	maiNext.Init(CIRW65C816Backend::GetScratch());
	maiPairNext.Init(CIRW65C816Backend::GetScratch());
	maiPairOther.Init(CIRW65C816Backend::GetScratch());
	maiPairPosition.Init(CIRW65C816Backend::GetScratch());
	maiPairWeight.Init(CIRW65C816Backend::GetScratch());
	maiWordFirst.Init(CIRW65C816Backend::GetScratch());
	maiOccupantNext.Init(CIRW65C816Backend::GetScratch());
	maiOccupant.Init(CIRW65C816Backend::GetScratch());
	masRegions.Init(CIRW65C816Backend::GetScratch());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816LiveRanges::Kill(void)
{
	masRegions.Kill();
	maiOccupant.Kill();
	maiOccupantNext.Kill();
	maiWordFirst.Kill();
	maiPairWeight.Kill();
	maiPairPosition.Kill();
	maiPairOther.Kill();
	maiPairNext.Kill();
	maiNext.Kill();
	maiEnd.Kill();
	maiStart.Kill();
	maiPairFirst.Kill();
	maiLast.Kill();
	maiFirst.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Ranges are added in position order; one that overlaps the last is merged into it.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816LiveRanges::Add(int32 iTemporary, int32 iStart, int32 iEnd)
{
	int32	iLast;
	int32	iRange;
	int32	iNone;

	iLast = *maiLast.Get(iTemporary);
	if ((iLast != -1) && (*maiEnd.Get(iLast) >= iStart))
	{
		if (iEnd > *maiEnd.Get(iLast))
		{
			*maiEnd.Get(iLast) = iEnd;
		}
		return;
	}
	iNone = -1;
	iRange = (int32)maiStart.NumElements();
	maiStart.Add(&iStart);
	maiEnd.Add(&iEnd);
	maiNext.Add(&iNone);
	if (iLast == -1)
	{
		*maiFirst.Get(iTemporary) = iRange;
	}
	else
	{
		*maiNext.Get(iLast) = iRange;
	}
	*maiLast.Get(iTemporary) = iRange;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816LiveRanges::AddPair(SIRReference sOne, SIRReference sOther, int32 iPosition, int32 iWeight)
{
	int32	iPair;
	int32	iOne;
	int32	iOther;

	if (!sOne.IsTemporary() || !sOther.IsTemporary() || sOne.Equals(sOther))
	{
		return;
	}
	iOne = (int32)sOne.Index();
	iOther = (int32)sOther.Index();

	iPair = (int32)maiPairOther.NumElements();
	maiPairOther.Add(&iOther);
	maiPairPosition.Add(&iPosition);
	maiPairWeight.Add(&iWeight);
	maiPairNext.Add(maiPairFirst.Get(iOne));
	*maiPairFirst.Get(iOne) = iPair;

	iPair = (int32)maiPairOther.NumElements();
	maiPairOther.Add(&iOne);
	maiPairPosition.Add(&iPosition);
	maiPairWeight.Add(&iWeight);
	maiPairNext.Add(maiPairFirst.Get(iOther));
	*maiPairFirst.Get(iOther) = iPair;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816LiveRanges::IsPair(int32 iOne, int32 iOther, int32 iPosition)
{
	int32	iPair;

	for (iPair = *maiPairFirst.Get(iOne); iPair != -1; iPair = *maiPairNext.Get(iPair))
	{
		if ((*maiPairOther.Get(iPair) == iOther) && (*maiPairPosition.Get(iPair) == iPosition))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether two temporaries are a pair anywhere.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816LiveRanges::IsPaired(int32 iOne, int32 iOther)
{
	int32	iPair;

	for (iPair = *maiPairFirst.Get(iOne); iPair != -1; iPair = *maiPairNext.Get(iPair))
	{
		if (*maiPairOther.Get(iPair) == iOther)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether two temporaries are live at once, but where a pair of them in the
// same place (bSamePlace) meets at a single position: one's range ends there
// and the other's starts.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816LiveRanges::Interferes(int32 iOne, int32 iOther, bool bSamePlace)
{
	int32	iA;
	int32	iB;
	int32	iLow;
	int32	iHigh;

	iA = *maiFirst.Get(iOne);
	iB = *maiFirst.Get(iOther);
	while ((iA != -1) && (iB != -1))
	{
		iLow = (*maiStart.Get(iA) > *maiStart.Get(iB)) ? *maiStart.Get(iA) : *maiStart.Get(iB);
		iHigh = (*maiEnd.Get(iA) < *maiEnd.Get(iB)) ? *maiEnd.Get(iA) : *maiEnd.Get(iB);
		if (iLow <= iHigh)
		{
			//Only one ending where the other starts (a result that is never used, at the place of an argument still
			//live after it, is live with it).
			if (!bSamePlace || (iLow != iHigh) || !IsPair(iOne, iOther, iLow) ||
				!(((*maiEnd.Get(iA) == iLow) && (*maiStart.Get(iB) == iLow)) || ((*maiEnd.Get(iB) == iLow) && (*maiStart.Get(iA) == iLow))))
			{
				return true;
			}
		}
		if (*maiEnd.Get(iA) < *maiEnd.Get(iB))
		{
			iA = *maiNext.Get(iA);
		}
		else
		{
			iB = *maiNext.Get(iB);
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The first position a temporary is live at, or -1 when it never is.
//
//////////////////////////////////////////////////////////////////////////
int32 CIRW65C816LiveRanges::Start(int32 iTemporary)
{
	int32	iRange;

	iRange = *maiFirst.Get(iTemporary);
	return (iRange == -1) ? -1 : *maiStart.Get(iRange);
}


//////////////////////////////////////////////////////////////////////////
//
// The last position a temporary is live at, or -1 when it never is.
//
//////////////////////////////////////////////////////////////////////////
int32 CIRW65C816LiveRanges::End(int32 iTemporary)
{
	int32	iRange;

	iRange = *maiLast.Get(iTemporary);
	return (iRange == -1) ? -1 : *maiEnd.Get(iRange);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a temporary is live before and after iPosition (not only read or
// written there).
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816LiveRanges::LiveAcross(int32 iTemporary, int32 iPosition)
{
	int32	iRange;

	for (iRange = *maiFirst.Get(iTemporary); iRange != -1; iRange = *maiNext.Get(iRange))
	{
		if ((*maiStart.Get(iRange) < iPosition) && (*maiEnd.Get(iRange) > iPosition))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Temporaries may have places from iStart up to iEnd.  Its words come
// after those of the regions added before it, so only the last may be
// filled far (the frame's).
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816LiveRanges::AddRegion(int32 iStart, int32 iEnd)
{
	SIR816Region*	psRegion;
	SIR816Region*	psLast;
	int32			iFirstWord;

	//The first word comes from the last region before Add, which may move the array.
	psLast = (masRegions.NumElements() > 0) ? masRegions.Tail() : NULL;
	iFirstWord = psLast ? psLast->iFirstWord + (psLast->iEnd - psLast->iStart + 1) / 2 : 0;
	psRegion = masRegions.Add();
	psRegion->iStart = iStart;
	psRegion->iEnd = iEnd;
	psRegion->iFirstWord = iFirstWord;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether iBytes from iOffset lie wholly inside one region.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816LiveRanges::Fits(int32 iOffset, int32 iBytes)
{
	size			ui;
	SIR816Region*	psRegion;

	for (ui = 0; ui < masRegions.NumElements(); ui++)
	{
		psRegion = masRegions.Get(ui);
		if ((iOffset >= psRegion->iStart) && (iOffset + iBytes <= psRegion->iEnd))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The word (in the occupants' lists) of the place at iOffset in its
// region, or -1 when it is in none.
//
//////////////////////////////////////////////////////////////////////////
int32 CIRW65C816LiveRanges::WordOf(int32 iOffset)
{
	size			ui;
	SIR816Region*	psRegion;

	for (ui = 0; ui < masRegions.NumElements(); ui++)
	{
		psRegion = masRegions.Get(ui);
		if ((iOffset >= psRegion->iStart) && (iOffset < psRegion->iEnd))
		{
			return psRegion->iFirstWord + (iOffset - psRegion->iStart) / 2;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether iBytes from iOffset are free for a temporary: inside one region
// (Fits) and no temporary placed on them is live with it (but a pair
// meeting in the same place).
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816LiveRanges::IsFree(int32 iTemporary, int32 iOffset, int32 iBytes, CArrayTemplate<int32>* paiOffsets)
{
	int32	iFirst;
	int32	iWord;
	int32	iOccupant;
	int32	iOther;

	if (!Fits(iOffset, iBytes))
	{
		return false;
	}
	iFirst = WordOf(iOffset);
	for (iWord = iFirst; iWord < iFirst + iBytes / 2; iWord++)
	{
		if (iWord >= (int32)maiWordFirst.NumElements())
		{
			break;
		}
		for (iOccupant = *maiWordFirst.Get(iWord); iOccupant != -1; iOccupant = *maiOccupantNext.Get(iOccupant))
		{
			iOther = *maiOccupant.Get(iOccupant);
			if (Interferes(iTemporary, iOther, *paiOffsets->Get(iOther) == iOffset))
			{
				return false;
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Places a temporary on iBytes from iOffset (in a region, as IsFree found
// them).
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816LiveRanges::Take(int32 iTemporary, int32 iOffset, int32 iBytes)
{
	int32	iFirst;
	int32	iWord;
	int32	iOccupant;
	int32	iNone;

	iNone = -1;
	iFirst = WordOf(iOffset);
	for (iWord = iFirst; (iFirst >= 0) && (iWord < iFirst + iBytes / 2); iWord++)
	{
		while (iWord >= (int32)maiWordFirst.NumElements())
		{
			maiWordFirst.Add(&iNone);
		}
		iOccupant = (int32)maiOccupant.NumElements();
		maiOccupant.Add(&iTemporary);
		maiOccupantNext.Add(maiWordFirst.Get(iWord));
		*maiWordFirst.Get(iWord) = iOccupant;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A temporary's use, definition or write (a phi's value put in the phi's
// place on the way out of a block) at a position.
//
//////////////////////////////////////////////////////////////////////////
static void IR816AddEvent(CArrayTemplate<SIR816Event>* pasEvents, SIRReference sValue, int32 iPosition, int32 iKind)
{
	SIR816Event*	psEvent;

	if (!sValue.IsTemporary())
	{
		return;
	}
	psEvent = pasEvents->Add();
	psEvent->iTemporary = (int32)sValue.Index();
	psEvent->iPosition = iPosition;
	psEvent->iKind = iKind;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the instruction's result can have the place of its argument
// iArgument when that argument dies there: true when each word of the
// argument is read before that word of the result is written and never
// after.  A shift copies its value to the result before reading the count; a
// load reads through the pointer for each word (PackTemporaries pairs some
// with their pointers, and Load reads the words that overlap it last).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816CanShareArgument(uint16 uiOpcode, int iArgument)
{
	switch (uiOpcode)
	{
	case IRO_Add:
	case IRO_PtrAdd:
	case IRO_Sub:
	case IRO_And:
	case IRO_Or:
	case IRO_Xor:
	case IRO_Not:
	case IRO_Neg:
	case IRO_Copy:
	case IRO_Mul:
	case IRO_UDiv:
	case IRO_SDiv:
	case IRO_URem:
	case IRO_SRem:
	case IRO_Eq:
	case IRO_Ne:
	case IRO_SLt:
	case IRO_SLe:
	case IRO_SGt:
	case IRO_SGe:
	case IRO_ULt:
	case IRO_ULe:
	case IRO_UGt:
	case IRO_UGe:
	case IRO_SExt:
	case IRO_ZExt:
	case IRO_Trunc:
	case IRO_PtrToI:
	case IRO_IToPtr:
	case IRO_SpaceCast:
	case IRO_FAdd:
	case IRO_FSub:
	case IRO_FMul:
	case IRO_FDiv:
	case IRO_FNeg:
	case IRO_FEq:
	case IRO_FNe:
	case IRO_FLt:
	case IRO_FLe:
	case IRO_FGt:
	case IRO_FGe:
	case IRO_FOrdered:
	case IRO_FUnordered:
	case IRO_SIToF:
	case IRO_UIToF:
	case IRO_FToSI:
	case IRO_FToUI:
	case IRO_FExt:
	case IRO_FTrunc:
	case IRO_Call:
		return true;
	case IRO_Shl:
	case IRO_LShr:
	case IRO_AShr:
		return iArgument == 0;
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The regions PackTemporaries places temporaries in.  First the pushed
// arguments': each pushed parameter has a stand-in place from
// IR816_ARGUMENT_STANDIN, as ArgumentPlace orders them for Layout (which
// moves the stand-ins to the arguments' offsets once the frame's size is
// known), taken while it is live.  Not when a parameter's address is
// taken or the function is variadic: a parameter's place may then be read
// after it is dead.  Then the frame's own, last as it is the one that
// fills far.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::AddRegions(CIRW65C816LiveRanges* pcRanges)
{
	CIRBlock*		pcBlock;
	SIRW65C816Argument*	psArgument;
	SIRReference	sParameter;
	size			ui;
	size			uj;
	size			uiArgument;
	int32			iPushed;
	int32			iPlace;
	bool			bShare;

	bShare = !(mpcFunction->muiAttributes & IRA_Variadic);
	for (ui = 0; bShare && (ui < mpcFunction->NumBlocks()); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uj = 0; bShare && (uj < pcBlock->masInstructions.NumElements()); uj++)
		{
			bShare = pcBlock->masInstructions.Get(uj)->uiOpcode != IRO_ArgAddress;
		}
	}

	if (bShare)
	{
		iPushed = IR816_ARGUMENT_STANDIN;
		uiArgument = 0;
		if (IsHiddenResult(mpcFunction->msReturnType))
		{
			ArgumentPlace(masArguments.Get(uiArgument), &iPushed);
			uiArgument++;
		}
		for (ui = 0; ui < mpcFunction->masParameters.NumElements(); ui++, uiArgument++)
		{
			psArgument = masArguments.Get(uiArgument);
			iPlace = ArgumentPlace(psArgument, &iPushed);
			if (psArgument->iRegister < 0)
			{
				*maiOffsets.Get(mpcFunction->masParameters.Get(ui)->Index()) = iPlace;
			}
		}
		pcRanges->AddRegion(IR816_ARGUMENT_STANDIN, iPushed);
		for (ui = 0, uiArgument = IsHiddenResult(mpcFunction->msReturnType) ? 1 : 0; ui < mpcFunction->masParameters.NumElements(); ui++, uiArgument++)
		{
			sParameter = *mpcFunction->masParameters.Get(ui);
			psArgument = masArguments.Get(uiArgument);
			if ((psArgument->iRegister < 0) && (pcRanges->Start((int32)sParameter.Index()) >= 0))
			{
				pcRanges->Take((int32)sParameter.Index(), *maiOffsets.Get(sParameter.Index()), 2 * psArgument->iWords);
			}
		}
	}
	pcRanges->AddRegion(IR816_FIRST_TEMPORARY, IR816_ARGUMENT_STANDIN);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether iOne is placed after iOther (PackTemporaries): it starts later,
// or with it but with a lighter pair there.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816PlacedAfter(CIRW65C816LiveRanges* pcRanges, CArrayTemplate<int32>* paiStartWeights, int32 iOne, int32 iOther)
{
	if (pcRanges->Start(iOne) != pcRanges->Start(iOther))
	{
		return pcRanges->Start(iOne) > pcRanges->Start(iOther);
	}
	return *paiStartWeights->Get(iOne) < *paiStartWeights->Get(iOther);
}


//////////////////////////////////////////////////////////////////////////
//
// The depth of the innermost loop both blocks are in, 0 when they share
// none: how often a jump from one to the other is made.
//
//////////////////////////////////////////////////////////////////////////
int32 CIRW65C816Backend::SharedLoopDepth(CIRBlock* pcFrom, CIRBlock* pcTo)
{
	int32	iLoop;

	for (iLoop = mcLoops.Innermost(pcFrom); iLoop >= 0; iLoop = mcLoops.GetLoop(iLoop)->iParent)
	{
		if (mcLoops.Contains(iLoop, pcTo))
		{
			return mcLoops.GetLoop(iLoop)->iDepth;
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SIR816PackEvents::Init(void)
{
	asEvents.Init(CIRW65C816Backend::GetScratch());
	asLoadPairs.Init(CIRW65C816Backend::GetScratch());
	aiBlockEvents.Init(CIRW65C816Backend::GetScratch());
	aiBlockStart.Init(CIRW65C816Backend::GetScratch());
	aiBlockEnd.Init(CIRW65C816Backend::GetScratch());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SIR816PackEvents::Kill(void)
{
	aiBlockEnd.Kill();
	aiBlockStart.Kill();
	aiBlockEvents.Kill();
	asLoadPairs.Kill();
	asEvents.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each block's uses and definitions, for working out what is live.
//
//////////////////////////////////////////////////////////////////////////
static void IR816AddLiveEvents(SIR816PackEvents* psEvents, size uiBlocks, CIRLiveSets* pcSets)
{
	SIR816Event*	psEvent;
	size			ui;
	int32			iEvent;

	for (ui = 0; ui < uiBlocks; ui++)
	{
		for (iEvent = *psEvents->aiBlockEvents.Get(ui); iEvent < *psEvents->aiBlockEvents.Get(ui + 1); iEvent++)
		{
			psEvent = psEvents->asEvents.Get(iEvent);
			if (psEvent->iKind == IR816_EVENT_USE)
			{
				pcSets->Use(ui, IRReference(IRRK_Temporary, (uint32)psEvent->iTemporary));
			}
			else if (psEvent->iKind == IR816_EVENT_DEFINE)
			{
				pcSets->Define(ui, IRReference(IRRK_Temporary, (uint32)psEvent->iTemporary));
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The ranges: in each block from the start (live in) or the definition, to
// the end (live out) or the last use.
//
//////////////////////////////////////////////////////////////////////////
static void IR816AddRanges(SIR816PackEvents* psEvents, size uiBlocks, size uiTemporaries, CIRLiveSets* pcSets, CIRW65C816LiveRanges* pcRanges)
{
	CArrayTemplate<int32>	aiOpen;			//Per temporary: where its range in the block being scanned starts, or -1.
	CArrayTemplate<int32>	aiLastUse;		//Per temporary: its last use or definition in that block.
	CArrayTemplate<int32>	aiOpened;		//The temporaries with a range open in that block.
	SIR816Event*			psEvent;
	size					ui;
	size					uj;
	int32					iNone;
	int32					iTemporary;
	int32					iEvent;
	int32					iEnd;

	iNone = -1;
	aiOpen.Init(CIRW65C816Backend::GetScratch());
	aiLastUse.Init(CIRW65C816Backend::GetScratch());
	aiOpened.Init(CIRW65C816Backend::GetScratch());
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		aiOpen.Add(&iNone);
		aiLastUse.Add(&iNone);
	}
	for (ui = 0; ui < uiBlocks; ui++)
	{
		for (uj = 0; uj < uiTemporaries; uj++)
		{
			if (pcSets->IsLiveIn(ui, uj))
			{
				iTemporary = (int32)uj;
				*aiOpen.Get(uj) = *psEvents->aiBlockStart.Get(ui);
				*aiLastUse.Get(uj) = *psEvents->aiBlockStart.Get(ui);
				aiOpened.Add(&iTemporary);
			}
		}
		for (iEvent = *psEvents->aiBlockEvents.Get(ui); iEvent < *psEvents->aiBlockEvents.Get(ui + 1); iEvent++)
		{
			psEvent = psEvents->asEvents.Get(iEvent);
			if (psEvent->iKind != IR816_EVENT_WRITE)
			{
				if (*aiOpen.Get(psEvent->iTemporary) == -1)
				{
					*aiOpen.Get(psEvent->iTemporary) = psEvent->iPosition;
					aiOpened.Add(&psEvent->iTemporary);
				}
				if (psEvent->iPosition > *aiLastUse.Get(psEvent->iTemporary))
				{
					*aiLastUse.Get(psEvent->iTemporary) = psEvent->iPosition;
				}
			}
		}
		for (uj = 0; uj < aiOpened.NumElements(); uj++)
		{
			iTemporary = *aiOpened.Get(uj);
			iEnd = pcSets->IsLiveOut(ui, (size)iTemporary) ? *psEvents->aiBlockEnd.Get(ui) : *aiLastUse.Get(iTemporary);
			pcRanges->Add(iTemporary, *aiOpen.Get(iTemporary), iEnd);
			*aiOpen.Get(iTemporary) = -1;
			*aiLastUse.Get(iTemporary) = -1;
		}
		aiOpened.FakeSetUsedElements(0);
		for (iEvent = *psEvents->aiBlockEvents.Get(ui); iEvent < *psEvents->aiBlockEvents.Get(ui + 1); iEvent++)
		{
			psEvent = psEvents->asEvents.Get(iEvent);
			if (psEvent->iKind == IR816_EVENT_WRITE)
			{
				pcRanges->Add(psEvent->iTemporary, psEvent->iPosition, psEvent->iPosition);
			}
		}
	}
	aiOpened.Kill();
	aiLastUse.Kill();
	aiOpen.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Gives the temporaries (but the parameters, which are the caller's
// arguments) their places from IR816_FIRST_TEMPORARY on, sharing places
// between temporaries that are never live at once.  Positions number the
// phis, instructions, jump and end of each block in the order the blocks
// are written.  A temporary is live from its definition, or the start of a
// block it is live into, to its last use, or the end of a block it is live
// out of (CIRW65C816LiveRanges).  An argument is used where its call pushes it; a
// phi's arguments at the end of their blocks (after the jump's argument: a
// brif tests it before the copies) and its result there too, as the jumps
// put the values straight in its place (EdgeCopies, one parallel copy).  Uses of an instruction's
// operands and its result's definition are at the same position, so they
// only share a place when they are a pair (IR816CanShareArgument, a call's
// argument, or a load's pointer: a one word load's, or one a phi joins them
// by, p = p->psNext) at exactly the same place.  Each temporary, in the
// order they start, is placed at a pair's place when it can be, else at the
// first place free, in the regions AddRegions gives: a pushed parameter's
// place may be a pair's too, as the callee owns its pushed arguments, so
// once a parameter is dead a temporary paired with it (a field masked out
// of it, a copy) may take its place.  Returns the first byte after the
// temporaries in the frame.
//
//////////////////////////////////////////////////////////////////////////
int32 CIRW65C816Backend::PackTemporaries(void)
{
	CIRW65C816LiveRanges		cRanges;
	CIRLiveSets				cSets;
	SIR816PackEvents		sEvents;
	CArrayTemplate<int32>	aiOrder;
	CArrayTemplate<bool>	abParameter;
	size					uiTemporaries;
	size					ui;
	int32					iNext;

	uiTemporaries = mpcFunction->NumTemporaries();
	cRanges.Init(uiTemporaries);
	sEvents.Init();
	AddPackEvents(&cRanges, &sEvents);

	//What is live into and out of each block, then the ranges.
	cSets.Init(CIRW65C816Backend::GetScratch());
	cSets.Start(mpcFunction);
	IR816AddLiveEvents(&sEvents, mpcFunction->NumBlocks(), &cSets);
	cSets.Solve();
	IR816AddRanges(&sEvents, mpcFunction->NumBlocks(), uiTemporaries, &cSets, &cRanges);

	abParameter.Init(CIRW65C816Backend::GetScratch());
	abParameter.GrowToAtLeastNumElements(uiTemporaries + 1, true, 0);
	for (ui = 0; ui < mpcFunction->masParameters.NumElements(); ui++)
	{
		*abParameter.Get(mpcFunction->masParameters.Get(ui)->Index()) = true;
	}
	aiOrder.Init(CIRW65C816Backend::GetScratch());
	OrderTemporaries(&cRanges, &abParameter, &aiOrder);

	AddRegions(&cRanges);
	iNext = PlaceTemporaries(&cRanges, &aiOrder, &abParameter);

	//A temporary with no definition or use still has a place (it is never live).
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		if ((cRanges.Start((int32)ui) == -1) && (ValueBytes(mpcFunction->GetTemporaryType(IRReference(IRRK_Temporary, (uint32)ui))) != 0))
		{
			*maiOffsets.Get(ui) = IR816_FIRST_TEMPORARY;
		}
	}

	AllocateX(&cRanges, &abParameter);
	abParameter.Kill();
	aiOrder.Kill();
	cSets.Kill();
	sEvents.Kill();
	cRanges.Kill();
	return iNext;
}


//////////////////////////////////////////////////////////////////////////
//
// The uses, definitions and writes of the function's blocks, at their
// positions (PackTemporaries), and the pairs.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::AddPackEvents(CIRW65C816LiveRanges* pcRanges, SIR816PackEvents* psEvents)
{
	CIRBlock*		pcBlock;
	SIR816LoadPair*	psLoadPair;
	size			ui;
	size			uj;
	int32			iPosition;
	int32			iEvent;
	int32			iWeight;

	iPosition = 0;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		iEvent = (int32)psEvents->asEvents.NumElements();
		psEvents->aiBlockEvents.Add(&iEvent);
		psEvents->aiBlockStart.Add(&iPosition);
		for (uj = 0; uj < pcBlock->masPhis.NumElements(); uj++)
		{
			IR816AddEvent(&psEvents->asEvents, pcBlock->masPhis.Get(uj)->sResult, iPosition, IR816_EVENT_DEFINE);
		}
		iPosition++;
		iWeight = IR816_PAIR_WEIGHT(mcLoops.Depth(pcBlock), false, false);
		for (uj = 0; uj < pcBlock->masInstructions.NumElements(); uj++)
		{
			AddInstructionPackEvents(pcRanges, psEvents, pcBlock, uj, iPosition, iWeight);
			iPosition++;
		}

		//The jump's argument, then the phi arguments of the successors (copied on the way out, after a brif has tested
		//its condition: Jump).
		IR816AddEvent(&psEvents->asEvents, pcBlock->msJump.sArgument, iPosition, IR816_EVENT_USE);
		iPosition++;
		AddSuccessorPackEvents(pcRanges, psEvents, pcBlock, iPosition);
		psEvents->aiBlockEnd.Add(&iPosition);
		iPosition++;
	}
	iEvent = (int32)psEvents->asEvents.NumElements();
	psEvents->aiBlockEvents.Add(&iEvent);
	for (ui = 0; ui < psEvents->asLoadPairs.NumElements(); ui++)
	{
		psLoadPair = psEvents->asLoadPairs.Get(ui);
		if (!psLoadPair->bWide || pcRanges->IsPaired((int32)psLoadPair->sResult.Index(), (int32)psLoadPair->sPointer.Index()))
		{
			pcRanges->AddPair(psLoadPair->sResult, psLoadPair->sPointer, psLoadPair->iPosition, psLoadPair->iDepth);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The events and pairs of pcBlock's instruction uiInstruction at iPosition.
// iWeight is a pair's in the block.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::AddInstructionPackEvents(CIRW65C816LiveRanges* pcRanges, SIR816PackEvents* psEvents, CIRBlock* pcBlock, size uiInstruction, int32 iPosition, int32 iWeight)
{
	SIRInstruction*	psInstruction;
	SIR816LoadPair*	psLoadPair;
	size			uk;
	int32			iCallPosition;
	int32			iArgument;

	psInstruction = pcBlock->masInstructions.Get(uiInstruction);
	if (psInstruction->uiOpcode == IRO_Arg)
	{
		//Used where the call pushes it.
		iCallPosition = iPosition;
		for (uk = uiInstruction + 1; uk < pcBlock->masInstructions.NumElements(); uk++)
		{
			if (pcBlock->masInstructions.Get(uk)->uiOpcode == IRO_Call)
			{
				iCallPosition = iPosition + (int32)(uk - uiInstruction);
				pcRanges->AddPair(pcBlock->masInstructions.Get(uk)->sResult, FoldedBase(psInstruction->asArguments[0]), iCallPosition, iWeight);
				break;
			}
		}
		IR816AddEvent(&psEvents->asEvents, FoldedBase(psInstruction->asArguments[0]), iCallPosition, IR816_EVENT_USE);
	}
	else
	{
		IR816AddEvent(&psEvents->asEvents, FoldedBase(psInstruction->asArguments[0]), iPosition, IR816_EVENT_USE);
		IR816AddEvent(&psEvents->asEvents, FoldedBase(psInstruction->asArguments[1]), iPosition, IR816_EVENT_USE);
		IR816AddEvent(&psEvents->asEvents, FoldedIndex(psInstruction->asArguments[0]), iPosition, IR816_EVENT_USE);
		IR816AddEvent(&psEvents->asEvents, FoldedIndex(psInstruction->asArguments[1]), iPosition, IR816_EVENT_USE);
	}
	if (!psInstruction->sResult.IsTemporary() || !FoldedBase(psInstruction->sResult).Equals(psInstruction->sResult))
	{
		return;
	}
	IR816AddEvent(&psEvents->asEvents, psInstruction->sResult, iPosition, IR816_EVENT_DEFINE);
	for (iArgument = 0; iArgument < IR_INSTRUCTION_ARGUMENTS; iArgument++)
	{
		if (IR816CanShareArgument(psInstruction->uiOpcode, iArgument) && FoldedBase(psInstruction->asArguments[iArgument]).Equals(psInstruction->asArguments[iArgument]))
		{
			pcRanges->AddPair(psInstruction->sResult, psInstruction->asArguments[iArgument], iPosition, iWeight);
		}
	}
	if ((psInstruction->uiOpcode == IRO_Load) && !(psInstruction->uiFlags & IRF_Volatile) && FoldedBase(psInstruction->asArguments[0]).IsTemporary())
	{
		//Its pointer's place: Load reads the words that overlap the pointer last (only worth keeping a word on
		//the stack for when a phi joins them, p = p->psNext, so they're paired after the phis).
		psLoadPair = psEvents->asLoadPairs.Add();
		psLoadPair->sResult = psInstruction->sResult;
		psLoadPair->sPointer = FoldedBase(psInstruction->asArguments[0]);
		psLoadPair->iPosition = iPosition;
		psLoadPair->iDepth = iWeight;
		psLoadPair->bWide = Words(psInstruction->sType) > 1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The phis of pcBlock's successors at the end of pcBlock (iPosition): their
// arguments used and their results written, on the way out.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::AddSuccessorPackEvents(CIRW65C816LiveRanges* pcRanges, SIR816PackEvents* psEvents, CIRBlock* pcBlock, int32 iPosition)
{
	CIRBlock*		pcSuccessor;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	size			ui;
	size			uj;

	for (ui = 0; ui < pcBlock->NumSuccessors(); ui++)
	{
		pcSuccessor = pcBlock->GetSuccessor(ui);
		for (uj = 0; uj < pcSuccessor->masPhis.NumElements(); uj++)
		{
			psPhi = pcSuccessor->masPhis.Get(uj);
			psArgument = pcSuccessor->FindPhiArgument(psPhi, pcBlock);
			if (psArgument)
			{
				IR816AddEvent(&psEvents->asEvents, psArgument->sValue, iPosition, IR816_EVENT_USE);
			}

			//Written here, on the way out.
			IR816AddEvent(&psEvents->asEvents, psPhi->sResult, iPosition, IR816_EVENT_WRITE);
			if (psArgument)
			{
				//The copy is made going that way: as often as the innermost loop both blocks are in goes round (once
				//per loop for a way out of it).
				pcRanges->AddPair(psPhi->sResult, psArgument->sValue, iPosition, IR816_PAIR_WEIGHT(SharedLoopDepth(pcBlock, pcSuccessor), true, mcLoops.IsHead(pcSuccessor)));
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The temporaries (but the parameters) in the order they are placed: as they
// start; of those starting together, first the one with the heaviest pair
// there (two phis given the same value on the way out of a block: the one
// round a loop takes its place).
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::OrderTemporaries(CIRW65C816LiveRanges* pcRanges, CArrayTemplate<bool>* pabParameter, CArrayTemplate<int32>* paiOrder)
{
	CArrayTemplate<int32>	aiStartWeights;	//Per temporary: the weight of its heaviest pair where it starts, or -1.
	size					uiTemporaries;
	size					ui;
	size					uj;
	int32					iPair;
	int32					iWeight;
	int32					iTemporary;

	uiTemporaries = mpcFunction->NumTemporaries();
	aiStartWeights.Init(CIRW65C816Backend::GetScratch());
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		iWeight = -1;
		for (iPair = *pcRanges->maiPairFirst.Get(ui); iPair != -1; iPair = *pcRanges->maiPairNext.Get(iPair))
		{
			if ((*pcRanges->maiPairPosition.Get(iPair) == pcRanges->Start((int32)ui)) && (*pcRanges->maiPairWeight.Get(iPair) > iWeight))
			{
				iWeight = *pcRanges->maiPairWeight.Get(iPair);
			}
		}
		aiStartWeights.Add(&iWeight);
	}
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		iTemporary = (int32)ui;
		if ((pcRanges->Start(iTemporary) >= 0) && (ValueBytes(mpcFunction->GetTemporaryType(IRReference(IRRK_Temporary, (uint32)ui))) != 0) && !*pabParameter->Get(ui))
		{
			paiOrder->Add(&iTemporary);
		}
	}
	for (ui = 1; ui < paiOrder->NumElements(); ui++)
	{
		iTemporary = *paiOrder->Get(ui);
		for (uj = ui; (uj > 0) && IR816PlacedAfter(pcRanges, &aiStartWeights, *paiOrder->Get(uj - 1), iTemporary); uj--)
		{
			*paiOrder->Get(uj) = *paiOrder->Get(uj - 1);
		}
		*paiOrder->Get(uj) = iTemporary;
	}
	aiStartWeights.Kill();
}


//A pair GroupTemporaries may merge: its weight, the lower temporary and the pair in its list.
struct SIR816MergePair
{
	int32	iWeight;
	int32	iOne;
	int32	iPair;
};


//////////////////////////////////////////////////////////////////////////
//
// Heaviest first, then in order (the same each time).
//
//////////////////////////////////////////////////////////////////////////
static int CompareIR816MergePair(const void* pvLeft, const void* pvRight)
{
	SIR816MergePair*	psLeft;
	SIR816MergePair*	psRight;

	psLeft = (SIR816MergePair*)pvLeft;
	psRight = (SIR816MergePair*)pvRight;
	if (psLeft->iWeight != psRight->iWeight)
	{
		return (psLeft->iWeight > psRight->iWeight) ? -1 : 1;
	}
	if (psLeft->iOne != psRight->iOne)
	{
		return (psLeft->iOne < psRight->iOne) ? -1 : 1;
	}
	if (psLeft->iPair != psRight->iPair)
	{
		return (psLeft->iPair < psRight->iPair) ? -1 : 1;
	}
	return 0;
}


//The temporaries PlaceTemporaries puts at one place, merged before any is placed (GroupTemporaries).
class CIRW65C816Groups
{
public:
	CArrayTemplate<int32>	maiParent;		//Per temporary: its group (union-find), or -1 when it isn't grouped.
	CArrayTemplate<int32>	maiNext;		//Per temporary: the next one in its group, or -1.
	CArrayTemplate<int32>	maiLast;		//Per group: its last temporary.
	CArrayTemplate<int32>	maiStart;		//Per group: the first and last positions any of it is live at.
	CArrayTemplate<int32>	maiEnd;
	CArrayTemplate<int32>	maiFixed;		//Per group: the place of the pushed parameter in it, or -1.

	void	Init(size uiTemporaries);
	void	Kill(void);
	int32	Find(int32 iTemporary);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Groups::Init(size uiTemporaries)
{
	size	ui;
	int32	iNone;

	iNone = -1;
	maiParent.Init(CIRW65C816Backend::GetScratch());
	maiNext.Init(CIRW65C816Backend::GetScratch());
	maiLast.Init(CIRW65C816Backend::GetScratch());
	maiStart.Init(CIRW65C816Backend::GetScratch());
	maiEnd.Init(CIRW65C816Backend::GetScratch());
	maiFixed.Init(CIRW65C816Backend::GetScratch());
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		maiParent.Add(&iNone);
		maiNext.Add(&iNone);
		maiLast.Add(&iNone);
		maiStart.Add(&iNone);
		maiEnd.Add(&iNone);
		maiFixed.Add(&iNone);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Groups::Kill(void)
{
	maiFixed.Kill();
	maiEnd.Kill();
	maiStart.Kill();
	maiLast.Kill();
	maiNext.Kill();
	maiParent.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The group a grouped temporary is in (its first temporary).
//
//////////////////////////////////////////////////////////////////////////
int32 CIRW65C816Groups::Find(int32 iTemporary)
{
	int32	iParent;

	while (*maiParent.Get(iTemporary) != iTemporary)
	{
		iParent = *maiParent.Get(*maiParent.Get(iTemporary));
		*maiParent.Get(iTemporary) = iParent;
		iTemporary = iParent;
	}
	return iTemporary;
}


//////////////////////////////////////////////////////////////////////////
//
// Merges the temporaries that are pairs into groups that will share a
// place, the heaviest pairs first (IR816_PAIR_WEIGHT): two groups merge when
// none of one is live with any of the other at one place (Interferes: only
// a pair meeting where one ends and the other starts), at most one pushed
// parameter (whose place is fixed) between them, and everything fits at its
// place.  The temporaries placed (paiOrder) and the pushed parameters with
// stand-in places (AddRegions) are grouped, each alone to start with.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::GroupTemporaries(CIRW65C816LiveRanges* pcRanges, CIRW65C816Groups* pcGroups, CArrayTemplate<int32>* paiOrder, CArrayTemplate<bool>* pabParameter)
{
	CArrayTemplate<SIR816MergePair>	asPairs;
	SIR816MergePair*		psMerge;
	size					ui;
	int32					iTemporary;
	int32					iOther;
	int32					iPair;
	int32					iOne;
	int32					iTwo;
	int32					iA;
	int32					iB;
	bool					bMerge;

	for (ui = 0; ui < paiOrder->NumElements(); ui++)
	{
		iTemporary = *paiOrder->Get(ui);
		*pcGroups->maiParent.Get(iTemporary) = iTemporary;
	}
	for (ui = 0; ui < mpcFunction->masParameters.NumElements(); ui++)
	{
		iTemporary = (int32)mpcFunction->masParameters.Get(ui)->Index();
		if ((*maiOffsets.Get(iTemporary) >= 0) && (pcRanges->Start(iTemporary) >= 0))
		{
			*pcGroups->maiParent.Get(iTemporary) = iTemporary;
			*pcGroups->maiFixed.Get(iTemporary) = *maiOffsets.Get(iTemporary);
		}
	}
	for (ui = 0; ui < pcGroups->maiParent.NumElements(); ui++)
	{
		iTemporary = (int32)ui;
		if (*pcGroups->maiParent.Get(ui) == iTemporary)
		{
			*pcGroups->maiLast.Get(ui) = iTemporary;
			*pcGroups->maiStart.Get(ui) = pcRanges->Start(iTemporary);
			*pcGroups->maiEnd.Get(ui) = pcRanges->End(iTemporary);
		}
	}

	asPairs.Init(CIRW65C816Backend::GetScratch());
	for (ui = 0; ui < pcGroups->maiParent.NumElements(); ui++)
	{
		iTemporary = (int32)ui;
		if (*pcGroups->maiParent.Get(ui) == -1)
		{
			continue;
		}
		for (iPair = *pcRanges->maiPairFirst.Get(ui); iPair != -1; iPair = *pcRanges->maiPairNext.Get(iPair))
		{
			iOther = *pcRanges->maiPairOther.Get(iPair);
			if ((iOther > iTemporary) && (*pcGroups->maiParent.Get(iOther) != -1))
			{
				psMerge = asPairs.Add();
				psMerge->iWeight = *pcRanges->maiPairWeight.Get(iPair);
				psMerge->iOne = iTemporary;
				psMerge->iPair = iPair;
			}
		}
	}

	asPairs.QuickSort(CompareIR816MergePair);
	for (ui = 0; ui < asPairs.NumElements(); ui++)
	{
		psMerge = asPairs.Get(ui);
		iPair = psMerge->iPair;
		iOne = pcGroups->Find(psMerge->iOne);
		iTwo = pcGroups->Find(*pcRanges->maiPairOther.Get(iPair));
		if ((iOne == iTwo) || ((*pcGroups->maiFixed.Get(iOne) >= 0) && (*pcGroups->maiFixed.Get(iTwo) >= 0)))
		{
			continue;
		}

		//Nothing of one live with anything of the other (only when their spans meet).
		bMerge = true;
		if ((*pcGroups->maiStart.Get(iOne) <= *pcGroups->maiEnd.Get(iTwo)) && (*pcGroups->maiStart.Get(iTwo) <= *pcGroups->maiEnd.Get(iOne)))
		{
			for (iA = iOne; bMerge && (iA != -1); iA = *pcGroups->maiNext.Get(iA))
			{
				for (iB = iTwo; bMerge && (iB != -1); iB = *pcGroups->maiNext.Get(iB))
				{
					bMerge = !pcRanges->Interferes(iA, iB, true);
				}
			}
		}

		//A pushed parameter's place has room for all of them.
		if (bMerge && ((*pcGroups->maiFixed.Get(iOne) >= 0) || (*pcGroups->maiFixed.Get(iTwo) >= 0)))
		{
			iOther = (*pcGroups->maiFixed.Get(iOne) >= 0) ? *pcGroups->maiFixed.Get(iOne) : *pcGroups->maiFixed.Get(iTwo);
			for (iA = iOne; bMerge && (iA != -1); iA = *pcGroups->maiNext.Get(iA))
			{
				bMerge = *pabParameter->Get(iA) || pcRanges->Fits(iOther, PlaceBytes(iA));
			}
			for (iB = iTwo; bMerge && (iB != -1); iB = *pcGroups->maiNext.Get(iB))
			{
				bMerge = *pabParameter->Get(iB) || pcRanges->Fits(iOther, PlaceBytes(iB));
			}
		}
		if (!bMerge)
		{
			continue;
		}

		*pcGroups->maiNext.Get(*pcGroups->maiLast.Get(iOne)) = iTwo;
		*pcGroups->maiLast.Get(iOne) = *pcGroups->maiLast.Get(iTwo);
		*pcGroups->maiParent.Get(iTwo) = iOne;
		if (*pcGroups->maiStart.Get(iTwo) < *pcGroups->maiStart.Get(iOne))
		{
			*pcGroups->maiStart.Get(iOne) = *pcGroups->maiStart.Get(iTwo);
		}
		if (*pcGroups->maiEnd.Get(iTwo) > *pcGroups->maiEnd.Get(iOne))
		{
			*pcGroups->maiEnd.Get(iOne) = *pcGroups->maiEnd.Get(iTwo);
		}
		if (*pcGroups->maiFixed.Get(iTwo) >= 0)
		{
			*pcGroups->maiFixed.Get(iOne) = *pcGroups->maiFixed.Get(iTwo);
		}
	}
	asPairs.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The bytes a temporary's place takes: its value's, in whole words.
//
//////////////////////////////////////////////////////////////////////////
int32 CIRW65C816Backend::PlaceBytes(int32 iTemporary)
{
	return (ValueBytes(mpcFunction->GetTemporaryType(IRReference(IRRK_Temporary, (uint32)iTemporary))) + 1) & ~1;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether every temporary of a group (but its parameter, placed already)
// may be at iOffset.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Backend::IsGroupFree(CIRW65C816LiveRanges* pcRanges, CIRW65C816Groups* pcGroups, int32 iGroup, int32 iOffset, CArrayTemplate<bool>* pabParameter)
{
	int32	iTemporary;

	for (iTemporary = iGroup; iTemporary != -1; iTemporary = *pcGroups->maiNext.Get(iTemporary))
	{
		if (!*pabParameter->Get(iTemporary) && !pcRanges->IsFree(iTemporary, iOffset, PlaceBytes(iTemporary), &maiOffsets))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Places a temporary on its own: at a pair's place (the one where they
// meet in the deepest loop: the copy saved is done most), else at the
// first place free.  IsFree keeps it inside its region: past the pushed
// arguments is the caller's.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::PlaceTemporary(CIRW65C816LiveRanges* pcRanges, int32 iTemporary)
{
	int32	iBytes;
	int32	iPair;
	int32	iOffset;
	int32	iDepth;
	int32	iBestDepth;
	int32	iBestOffset;

	iBytes = PlaceBytes(iTemporary);
	iBestDepth = -1;
	iBestOffset = -1;
	for (iPair = *pcRanges->maiPairFirst.Get(iTemporary); iPair != -1; iPair = *pcRanges->maiPairNext.Get(iPair))
	{
		iOffset = *maiOffsets.Get(*pcRanges->maiPairOther.Get(iPair));
		if (pcRanges->IsFree(iTemporary, iOffset, iBytes, &maiOffsets))
		{
			iDepth = *pcRanges->maiPairWeight.Get(iPair);
			if (iDepth > iBestDepth)
			{
				iBestDepth = iDepth;
				iBestOffset = iOffset;
			}
		}
	}
	iOffset = iBestOffset;
	if (iOffset < 0)
	{
		iOffset = IR816_FIRST_TEMPORARY;
		while (!pcRanges->IsFree(iTemporary, iOffset, iBytes, &maiOffsets))
		{
			iOffset += 2;
		}
	}
	pcRanges->Take(iTemporary, iOffset, iBytes);
	*maiOffsets.Get(iTemporary) = iOffset;
}


//////////////////////////////////////////////////////////////////////////
//
// Places the temporaries (paiOrder, in the order they start) a group at a
// time (GroupTemporaries): at its pushed parameter's place, else at the
// place of a group it is paired with (where they meet in the deepest loop),
// else at the first place free for all of it.  A group that can't be at its
// parameter's place is placed a temporary at a time.  Returns the first byte
// after the temporaries in the frame.
//
//////////////////////////////////////////////////////////////////////////
int32 CIRW65C816Backend::PlaceTemporaries(CIRW65C816LiveRanges* pcRanges, CArrayTemplate<int32>* paiOrder, CArrayTemplate<bool>* pabParameter)
{
	CIRW65C816Groups			cGroups;
	CArrayTemplate<bool>	abPlaced;		//Per group.
	size					ui;
	int32					iTemporary;
	int32					iGroup;
	int32					iMember;
	int32					iPair;
	int32					iOther;
	int32					iOffset;
	int32					iDepth;
	int32					iBestDepth;
	int32					iBestOffset;
	int32					iNext;

	cGroups.Init(mpcFunction->NumTemporaries());
	GroupTemporaries(pcRanges, &cGroups, paiOrder, pabParameter);
	abPlaced.Init(CIRW65C816Backend::GetScratch());
	abPlaced.GrowToAtLeastNumElements(mpcFunction->NumTemporaries() + 1, true, 0);

	for (ui = 0; ui < paiOrder->NumElements(); ui++)
	{
		iGroup = cGroups.Find(*paiOrder->Get(ui));
		if (*abPlaced.Get(iGroup))
		{
			continue;
		}
		*abPlaced.Get(iGroup) = true;

		iBestOffset = -1;
		if (*cGroups.maiFixed.Get(iGroup) >= 0)
		{
			if (IsGroupFree(pcRanges, &cGroups, iGroup, *cGroups.maiFixed.Get(iGroup), pabParameter))
			{
				iBestOffset = *cGroups.maiFixed.Get(iGroup);
			}
			else
			{
				for (iMember = iGroup; iMember != -1; iMember = *cGroups.maiNext.Get(iMember))
				{
					if (!*pabParameter->Get(iMember))
					{
						PlaceTemporary(pcRanges, iMember);
					}
				}
				continue;
			}
		}
		else
		{
			iBestDepth = -1;
			for (iMember = iGroup; iMember != -1; iMember = *cGroups.maiNext.Get(iMember))
			{
				for (iPair = *pcRanges->maiPairFirst.Get(iMember); iPair != -1; iPair = *pcRanges->maiPairNext.Get(iPair))
				{
					iOther = *pcRanges->maiPairOther.Get(iPair);
					iOffset = *maiOffsets.Get(iOther);
					iDepth = *pcRanges->maiPairWeight.Get(iPair);
					if ((iOffset >= 0) && (iDepth > iBestDepth) && IsGroupFree(pcRanges, &cGroups, iGroup, iOffset, pabParameter))
					{
						iBestDepth = iDepth;
						iBestOffset = iOffset;
					}
				}
			}
			if (iBestOffset < 0)
			{
				iBestOffset = IR816_FIRST_TEMPORARY;
				while (!IsGroupFree(pcRanges, &cGroups, iGroup, iBestOffset, pabParameter))
				{
					iBestOffset += 2;
				}
			}
		}
		for (iMember = iGroup; iMember != -1; iMember = *cGroups.maiNext.Get(iMember))
		{
			if (!*pabParameter->Get(iMember))
			{
				pcRanges->Take(iMember, iBestOffset, PlaceBytes(iMember));
				*maiOffsets.Get(iMember) = iBestOffset;
			}
		}
	}

	iNext = IR816_FIRST_TEMPORARY;
	for (ui = 0; ui < paiOrder->NumElements(); ui++)
	{
		iTemporary = *paiOrder->Get(ui);
		iOffset = *maiOffsets.Get(iTemporary);
		if ((iOffset < IR816_ARGUMENT_STANDIN) && (iOffset + PlaceBytes(iTemporary) > iNext))
		{
			iNext = iOffset + PlaceBytes(iTemporary);
		}
	}
	abPlaced.Kill();
	cGroups.Kill();
	return iNext;
}


#define IR816_X_BAD			-100000		//A use X can't serve: the temporary isn't kept in X (AllocateX's abBad, not a score).
#define IR816_X_MAX_DEPTH	4			//The loops a use's weight counts (CIRLoops::Weight).

//What keeping a temporary in X saves, in about cycles, at each place it is used or defined.
#define IR816_X_READ		3			//Read: txa for an lda, phx for a push, a commutative instruction's operands swapped.
#define IR816_X_SCRATCH		-5			//Read through a scratch word: it costs.
#define IR816_X_NOTHING		0			//Read as a conversion: neither saved nor spent.
#define IR816_X_DEFINE		3			//Defined: tax for an sta.
#define IR816_X_STEP		6			//inx or dex for an add or a sub, when both are in X (to each of them).
#define IR816_X_PHI			5			//A phi's result.
#define IR816_X_PHI_VALUE	3			//A phi's argument.
#define IR816_X_JUMP		3			//A jump's condition.


//An answer that may depend on the instruction.
enum EIR816XAnswer
{
	IR816XA_No,
	IR816XA_Yes,
	IR816XA_Shift,		//A shift's: see IR816XResult and IR816ClobbersX.
};


//How X serves an opcode's instructions.
struct SIR816XOpcode
{
	int32	aiUse[2];		//What keeping each argument in X saves where it is read (IR816_X_).
	uint8	uiResult;		//EIR816XAnswer: whether the result can be kept in X.
	uint8	uiClobbers;		//EIR816XAnswer: whether writing it changes X.
};


#define IR816_X_NO_USE		{ IR816_X_BAD, IR816_X_BAD }
#define IR816_X_READS		{ IR816_X_READ, IR816_X_READ }
#define IR816_X_LEFT		{ IR816_X_READ, IR816_X_SCRATCH }		//A second operand is read through a scratch word.
#define IR816_X_CONVERTS	{ IR816_X_NOTHING, IR816_X_NOTHING }


//In the order of EIROpcode.  A result goes to its place through Store, which can put it in X, or is worked out in A and put
//in X.  What changes X is a call (and the runtime and soft float routines) and a shift by a count only known at run time.
static SIR816XOpcode gasIR816XOpcodes[IRO_NumGeneric] =
{
	{ IR816_X_NO_USE, IR816XA_No, IR816XA_No },								//none

	{ IR816_X_READS, IR816XA_Yes, IR816XA_No },								//add
	{ IR816_X_LEFT, IR816XA_Yes, IR816XA_No },								//sub
	{ IR816_X_READS, IR816XA_Yes, IR816XA_Yes },							//mul
	{ IR816_X_READS, IR816XA_Yes, IR816XA_Yes },							//sdiv
	{ IR816_X_READS, IR816XA_Yes, IR816XA_Yes },							//udiv
	{ IR816_X_READS, IR816XA_Yes, IR816XA_Yes },							//srem
	{ IR816_X_READS, IR816XA_Yes, IR816XA_Yes },							//urem
	{ IR816_X_READS, IR816XA_Yes, IR816XA_No },								//and
	{ IR816_X_READS, IR816XA_Yes, IR816XA_No },								//or
	{ IR816_X_READS, IR816XA_Yes, IR816XA_No },								//xor
	{ IR816_X_READS, IR816XA_Shift, IR816XA_Shift },						//shl
	{ IR816_X_READS, IR816XA_Shift, IR816XA_Shift },						//lshr
	{ IR816_X_READS, IR816XA_Shift, IR816XA_Shift },						//ashr
	{ { IR816_X_SCRATCH, IR816_X_SCRATCH }, IR816XA_Yes, IR816XA_No },		//neg
	{ IR816_X_READS, IR816XA_Yes, IR816XA_No },								//not
	{ IR816_X_READS, IR816XA_Yes, IR816XA_No },								//copy
	{ IR816_X_CONVERTS, IR816XA_Yes, IR816XA_No },							//sext
	{ IR816_X_CONVERTS, IR816XA_Yes, IR816XA_No },							//zext
	{ IR816_X_CONVERTS, IR816XA_Yes, IR816XA_No },							//trunc

	{ IR816_X_LEFT, IR816XA_No, IR816XA_No },								//eq
	{ IR816_X_LEFT, IR816XA_No, IR816XA_No },								//ne
	{ IR816_X_READS, IR816XA_No, IR816XA_No },								//slt
	{ IR816_X_READS, IR816XA_No, IR816XA_No },								//sle
	{ IR816_X_READS, IR816XA_No, IR816XA_No },								//sgt
	{ IR816_X_READS, IR816XA_No, IR816XA_No },								//sge
	{ IR816_X_LEFT, IR816XA_No, IR816XA_No },								//ult
	{ IR816_X_LEFT, IR816XA_No, IR816XA_No },								//ule
	{ IR816_X_LEFT, IR816XA_No, IR816XA_No },								//ugt
	{ IR816_X_LEFT, IR816XA_No, IR816XA_No },								//uge

	{ IR816_X_NO_USE, IR816XA_No, IR816XA_Yes },							//fadd
	{ IR816_X_NO_USE, IR816XA_No, IR816XA_Yes },							//fsub
	{ IR816_X_NO_USE, IR816XA_No, IR816XA_Yes },							//fmul
	{ IR816_X_NO_USE, IR816XA_No, IR816XA_Yes },							//fdiv
	{ IR816_X_NO_USE, IR816XA_No, IR816XA_No },								//fneg
	{ IR816_X_NO_USE, IR816XA_No, IR816XA_Yes },							//feq
	{ IR816_X_NO_USE, IR816XA_No, IR816XA_Yes },							//fne
	{ IR816_X_NO_USE, IR816XA_No, IR816XA_Yes },							//flt
	{ IR816_X_NO_USE, IR816XA_No, IR816XA_Yes },							//fle
	{ IR816_X_NO_USE, IR816XA_No, IR816XA_Yes },							//fgt
	{ IR816_X_NO_USE, IR816XA_No, IR816XA_Yes },							//fge
	{ IR816_X_NO_USE, IR816XA_No, IR816XA_Yes },							//fordered
	{ IR816_X_NO_USE, IR816XA_No, IR816XA_Yes },							//funordered
	{ IR816_X_NO_USE, IR816XA_No, IR816XA_Yes },							//sitof
	{ IR816_X_NO_USE, IR816XA_No, IR816XA_Yes },							//uitof
	{ IR816_X_NO_USE, IR816XA_No, IR816XA_Yes },							//ftosi
	{ IR816_X_NO_USE, IR816XA_No, IR816XA_Yes },							//ftoui
	{ IR816_X_NO_USE, IR816XA_No, IR816XA_Yes },							//fext
	{ IR816_X_NO_USE, IR816XA_No, IR816XA_Yes },							//ftrunc

	{ IR816_X_NO_USE, IR816XA_No, IR816XA_No },								//slot
	{ IR816_X_NO_USE, IR816XA_No, IR816XA_No },								//argaddr
	{ IR816_X_NO_USE, IR816XA_No, IR816XA_No },								//ptradd
	{ IR816_X_CONVERTS, IR816XA_Yes, IR816XA_No },							//ptrtoi
	{ IR816_X_CONVERTS, IR816XA_No, IR816XA_No },							//itoptr
	{ IR816_X_NO_USE, IR816XA_No, IR816XA_No },								//spacecast
	{ IR816_X_NO_USE, IR816XA_Yes, IR816XA_No },							//load
	{ { IR816_X_READ, IR816_X_BAD }, IR816XA_No, IR816XA_No },				//store
	{ IR816_X_READS, IR816XA_No, IR816XA_No },								//arg
	{ IR816_X_NO_USE, IR816XA_Yes, IR816XA_Yes },							//call
};


//////////////////////////////////////////////////////////////////////////
//
// An opcode's row; a target's opcodes have no use for X, as IRO_None.
//
//////////////////////////////////////////////////////////////////////////
static SIR816XOpcode* IR816XOpcode(uint16 uiOpcode)
{
	return &gasIR816XOpcodes[(uiOpcode < IRO_NumGeneric) ? uiOpcode : IRO_None];
}


//////////////////////////////////////////////////////////////////////////
//
// What keeping argument iArgument of an instruction in X saves (in about
// cycles) where it is read.  IR816_X_BAD for a use that can't read X.
//
//////////////////////////////////////////////////////////////////////////
int32 IR816XUse(uint16 uiOpcode, int iArgument)
{
	return IR816XOpcode(uiOpcode)->aiUse[iArgument];
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an instruction's result can be kept in X.  A shift's only if it is
// 16 bits, by a constant.
//
//////////////////////////////////////////////////////////////////////////
bool IR816XResult(SIRInstruction* psInstruction)
{
	uint8	uiResult;

	uiResult = IR816XOpcode(psInstruction->uiOpcode)->uiResult;
	if (uiResult == IR816XA_Shift)
	{
		return psInstruction->asArguments[1].IsConstant() && psInstruction->sType.Is(IRTK_I16);
	}
	return uiResult == IR816XA_Yes;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether writing an instruction changes X.  A shift's does unless it is by
// a constant.
//
//////////////////////////////////////////////////////////////////////////
bool IR816ClobbersX(SIRInstruction* psInstruction)
{
	uint8	uiClobbers;

	uiClobbers = IR816XOpcode(psInstruction->uiOpcode)->uiClobbers;
	if (uiClobbers == IR816XA_Shift)
	{
		return !psInstruction->asArguments[1].IsConstant();
	}
	return uiClobbers == IR816XA_Yes;
}


//////////////////////////////////////////////////////////////////////////
//
// Register allocation of X: one word temporaries (16 bits or fewer) whose
// results Store writes, or phis, are kept in X instead of their places when
// nothing that changes X is done while they are live (a call, a shift by a
// count in X) and they don't interfere with the others in X (pairs in one
// place may meet, as PackTemporaries' ranges allow).  Each is scored by what
// X saves where it is used and defined (tax for sta, txa for lda, phx, inx,
// cpx, phi copies), 8 times more for each loop it is in, and the best are
// taken first.  Layout takes them all out again for a frame past the direct
// page (Frame reaches it through X).
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::AllocateX(CIRW65C816LiveRanges* pcRanges, CArrayTemplate<bool>* pabParameter)
{
	CArrayTemplate<int32>	aiScore;		//Per temporary.
	CArrayTemplate<uint8>	abCandidate;	//Per temporary.
	CArrayTemplate<uint8>	abBad;			//Per temporary: a use X can't serve (a veto, whatever the score).
	CArrayTemplate<int32>	aiClobbers;		//Positions that change X.
	CArrayTemplate<int32>	aiOrder;
	CArrayTemplate<int32>	aiInX;
	CIRBlock*				pcBlock;
	CIRBlock*				pcSuccessor;
	SIRInstruction*			psInstruction;
	SIRPhi*					psPhi;
	SIRPhiArgument*			psArgument;
	SIRReference			sValue;
	size					uiTemporaries;
	size					ui;
	size					uj;
	size					uk;
	int32					iPosition;
	int32					iWeight;
	int32					iDepth;
	int32					iTemporary;
	int32					iScore;
	int						iArgument;
	bool					bFree;

	uiTemporaries = mpcFunction->NumTemporaries();
	mabInX.FakeSetUsedElements(0);
	mabInX.GrowToAtLeastNumElements(uiTemporaries + 1, true, 0);
	aiScore.Init(CIRW65C816Backend::GetScratch());
	aiScore.GrowToAtLeastNumElements(uiTemporaries + 1, true, 0);
	abCandidate.Init(CIRW65C816Backend::GetScratch());
	abCandidate.GrowToAtLeastNumElements(uiTemporaries + 1, true, 0);
	abBad.Init(CIRW65C816Backend::GetScratch());
	abBad.GrowToAtLeastNumElements(uiTemporaries + 1, true, 0);
	aiClobbers.Init(CIRW65C816Backend::GetScratch());

	//The scores and the positions that change X, numbered as PackTemporaries numbers them.
	iPosition = 0;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		iDepth = mcLoops.Depth(pcBlock);
		iWeight = (int32)CIRLoops::Weight(iDepth, IR816_X_MAX_DEPTH);
		for (uj = 0; uj < pcBlock->masPhis.NumElements(); uj++)
		{
			*abCandidate.Get(pcBlock->masPhis.Get(uj)->sResult.Index()) = true;
		}
		iPosition++;
		for (uj = 0; uj < pcBlock->masInstructions.NumElements(); uj++)
		{
			psInstruction = pcBlock->masInstructions.Get(uj);
			if (IR816ClobbersX(psInstruction))
			{
				aiClobbers.Add(&iPosition);
			}
			if (((psInstruction->uiOpcode == IRO_Load) || (psInstruction->uiOpcode == IRO_Store)) && (Words(psInstruction->sType) > 1))
			{
				//A 2 word access's folded index is read again for its second word (LoadY), after X may hold the first
				//(a 32 bit result in A and X).
				sValue = FoldedIndex(psInstruction->asArguments[(psInstruction->uiOpcode == IRO_Load) ? 0 : 1]);
				if (sValue.IsTemporary())
				{
					*abBad.Get(sValue.Index()) = true;
				}
			}
			for (iArgument = 0; iArgument < 2; iArgument++)
			{
				sValue = psInstruction->asArguments[iArgument];
				if (sValue.IsTemporary())
				{
					iScore = IR816XUse(psInstruction->uiOpcode, iArgument);
					if (!FoldedBase(sValue).Equals(sValue))
					{
						iScore = IR816_X_BAD;
					}
					if (iScore == IR816_X_BAD)
					{
						*abBad.Get(sValue.Index()) = true;
					}
					else
					{
						*aiScore.Get(sValue.Index()) += iScore * iWeight;
					}
				}
			}
			sValue = psInstruction->sResult;
			if (sValue.IsTemporary() && FoldedBase(sValue).Equals(sValue) && IR816XResult(psInstruction))
			{
				*abCandidate.Get(sValue.Index()) = true;
				*aiScore.Get(sValue.Index()) += IR816_X_DEFINE * iWeight;
				if (((psInstruction->uiOpcode == IRO_Add) || (psInstruction->uiOpcode == IRO_Sub)) && psInstruction->asArguments[0].IsTemporary() &&
					psInstruction->asArguments[1].IsConstant() && (mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits <= 2))
				{
					*aiScore.Get(sValue.Index()) += IR816_X_STEP * iWeight;
					*aiScore.Get(psInstruction->asArguments[0].Index()) += IR816_X_STEP * iWeight;
				}
			}
			iPosition++;
		}
		if (pcBlock->msJump.sArgument.IsTemporary())
		{
			*aiScore.Get(pcBlock->msJump.sArgument.Index()) += IR816_X_JUMP * iWeight;
		}
		iPosition++;
		for (uj = 0; uj < pcBlock->NumSuccessors(); uj++)
		{
			pcSuccessor = pcBlock->GetSuccessor(uj);
			for (uk = 0; uk < pcSuccessor->masPhis.NumElements(); uk++)
			{
				psPhi = pcSuccessor->masPhis.Get(uk);
				psArgument = pcSuccessor->FindPhiArgument(psPhi, pcBlock);
				*aiScore.Get(psPhi->sResult.Index()) += IR816_X_PHI * iWeight;
				if (psArgument && psArgument->sValue.IsTemporary())
				{
					*aiScore.Get(psArgument->sValue.Index()) += IR816_X_PHI_VALUE * iWeight;
				}
			}
		}
		iPosition++;
	}

	//The candidates, best first.
	aiOrder.Init(CIRW65C816Backend::GetScratch());
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		iTemporary = (int32)ui;
		if (*abCandidate.Get(ui) && !*abBad.Get(ui) && !*pabParameter->Get(ui) && (*aiScore.Get(ui) > 0) && (pcRanges->Start(iTemporary) >= 0) &&
			(Words(mpcFunction->GetTemporaryType(IRReference(IRRK_Temporary, (uint32)ui))) == 1))
		{
			aiOrder.Add(&iTemporary);
		}
	}
	for (ui = 1; ui < aiOrder.NumElements(); ui++)
	{
		iTemporary = *aiOrder.Get(ui);
		for (uj = ui; (uj > 0) && (*aiScore.Get(*aiOrder.Get(uj - 1)) < *aiScore.Get(iTemporary)); uj--)
		{
			*aiOrder.Get(uj) = *aiOrder.Get(uj - 1);
		}
		*aiOrder.Get(uj) = iTemporary;
	}

	aiInX.Init(CIRW65C816Backend::GetScratch());
	for (ui = 0; ui < aiOrder.NumElements(); ui++)
	{
		iTemporary = *aiOrder.Get(ui);
		bFree = true;
		for (uj = 0; bFree && (uj < aiClobbers.NumElements()); uj++)
		{
			bFree = !pcRanges->LiveAcross(iTemporary, *aiClobbers.Get(uj));
		}
		for (uj = 0; bFree && (uj < aiInX.NumElements()); uj++)
		{
			bFree = !pcRanges->Interferes(iTemporary, *aiInX.Get(uj), true);
		}
		if (bFree)
		{
			aiInX.Add(&iTemporary);
			*mabInX.Get(iTemporary) = true;
		}
	}

	aiInX.Kill();
	aiOrder.Kill();
	aiClobbers.Kill();
	abBad.Kill();
	abCandidate.Kill();
	aiScore.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Backend::InX(SIRReference sValue)
{
	return sValue.IsTemporary() && (sValue.Index() < mabInX.NumElements()) && *mabInX.Get(sValue.Index());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CIRW65C816Backend::OffsetOf(SIRReference sTemporary)
{
	return *maiOffsets.Get(sTemporary.Index());
}

