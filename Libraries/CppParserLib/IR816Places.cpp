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
#include "IR816BackendInternal.h"
#include "IRLiveSets.h"


//The W65C816 back end's places: the frame's layout (Layout), pointers folded into their loads and stores
//(FoldPointers), the temporaries' places in their regions (AddRegions, PackTemporaries), shared where they are
//never live at once, and the temporaries kept in X (AllocateX).


#define IR816_ARGUMENT_STANDIN	0x10000		//The pushed parameters' stand-in places while temporaries are placed (AddRegions), past any frame.


//////////////////////////////////////////////////////////////////////////
//
// The frame (msFrame), its parts added in order: the temporaries after the scratch bytes, shared by those that are
// never live at once; then each phi's way in and slot its own place.  The arguments are above the frame and the link:
// the hidden result pointer first, then the parameters in order.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Layout(void)
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
	SIRPhi*				psPhi;
	SIRInstruction*		psInstruction;
	SIRReference		sParameter;

	iNone = -1;
	maiOffsets.FakeSetUsedElements(0);
	maiIncoming.FakeSetUsedElements(0);
	maiSlots.FakeSetUsedElements(0);
	maiParameters.FakeSetUsedElements(0);
	for (ui = 0; ui < mpcFunction->NumTemporaries(); ui++)
	{
		maiOffsets.Add(&iNone);
		maiIncoming.Add(&iNone);
		maiSlots.Add(&iNone);
	}

	//The temporaries share places when they aren't live at once (PackTemporaries); then the phis' ways in and the
	//slots, each a place of its own.
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
	msFrame.iSavedS = ((msFrame.Link() + iRest > 256) && (msFrame.Link() + 2 <= 256)) ? msFrame.Add(2) : -1;
	for (uiBlock = 0; uiBlock < mpcFunction->NumBlocks(); uiBlock++)
	{
		pcBlock = mpcFunction->GetBlock(uiBlock);
		for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
		{
			psPhi = pcBlock->masPhis.Get(ui);
			if (IsCoalescedPhiBlock(pcBlock))
			{
				//The jumps in put the values straight in the phis' places.
				*maiIncoming.Get(psPhi->sResult.Index()) = OffsetOf(psPhi->sResult);
			}
			else
			{
				*maiIncoming.Get(psPhi->sResult.Index()) = msFrame.Add(ValueBytes(psPhi->sType));
			}
		}
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

	//The temporaries at a pushed parameter's stand-in place (AddRegions), and the coalesced phis' ways in that are their
	//phis' places, get the parameter's place: no frame reaches IR816_ARGUMENT_STANDIN.
	for (ui = 0; ui < mpcFunction->NumTemporaries(); ui++)
	{
		if (*maiOffsets.Get(ui) >= IR816_ARGUMENT_STANDIN)
		{
			*maiOffsets.Get(ui) += msFrame.Arguments() - IR816_ARGUMENT_STANDIN;
		}
		if (*maiIncoming.Get(ui) >= IR816_ARGUMENT_STANDIN)
		{
			*maiIncoming.Get(ui) += msFrame.Arguments() - IR816_ARGUMENT_STANDIN;
		}
	}

	//Frame reaches past the direct page through X, so nothing is kept in X then.
	if (iArgument > 255)
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
void CIR816Backend::FoldPointers(void)
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
	aiAddressUses.Init(CIR816Backend::GetScratch());
	aiUses.Init(CIR816Backend::GetScratch());
	auiCounted.Init(CIR816Backend::GetScratch());
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
void CIR816Backend::FoldIndices(CArrayTemplate<int32>* paiUses)
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
	apsDefinitions.Init(CIR816Backend::GetScratch());
	aiWordUses.Init(CIR816Backend::GetScratch());
	aiPairUses.Init(CIR816Backend::GetScratch());
	aiFoldedUses.Init(CIR816Backend::GetScratch());
	abBase.Init(CIR816Backend::GetScratch());
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
SIRReference CIR816Backend::FoldedIndex(SIRReference sValue)
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
bool CIR816Backend::IsFolded(SIRInstruction* psInstruction)
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
bool CIR816Backend::LoadY(SIRReference sPointer, int32 iOffset)
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
SIRReference CIR816Backend::FoldedBase(SIRReference sValue)
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


//A run of places temporaries may have (CIR816Backend::AddRegions): the pushed arguments' (at their stand-in places
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
class CIR816LiveRanges
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
	CArrayTemplate<int32>	maiPairWeight;		//...and how many loops the copy sharing a place saves is in.
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
void CIR816LiveRanges::Init(size uiTemporaries)
{
	size	ui;
	int32	iNone;

	iNone = -1;
	maiFirst.Init(CIR816Backend::GetScratch());
	maiLast.Init(CIR816Backend::GetScratch());
	maiPairFirst.Init(CIR816Backend::GetScratch());
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		maiFirst.Add(&iNone);
		maiLast.Add(&iNone);
		maiPairFirst.Add(&iNone);
	}
	maiStart.Init(CIR816Backend::GetScratch());
	maiEnd.Init(CIR816Backend::GetScratch());
	maiNext.Init(CIR816Backend::GetScratch());
	maiPairNext.Init(CIR816Backend::GetScratch());
	maiPairOther.Init(CIR816Backend::GetScratch());
	maiPairPosition.Init(CIR816Backend::GetScratch());
	maiPairWeight.Init(CIR816Backend::GetScratch());
	maiWordFirst.Init(CIR816Backend::GetScratch());
	maiOccupantNext.Init(CIR816Backend::GetScratch());
	maiOccupant.Init(CIR816Backend::GetScratch());
	masRegions.Init(CIR816Backend::GetScratch());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816LiveRanges::Kill(void)
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
void CIR816LiveRanges::Add(int32 iTemporary, int32 iStart, int32 iEnd)
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
void CIR816LiveRanges::AddPair(SIRReference sOne, SIRReference sOther, int32 iPosition, int32 iWeight)
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
bool CIR816LiveRanges::IsPair(int32 iOne, int32 iOther, int32 iPosition)
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
bool CIR816LiveRanges::IsPaired(int32 iOne, int32 iOther)
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
bool CIR816LiveRanges::Interferes(int32 iOne, int32 iOther, bool bSamePlace)
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
int32 CIR816LiveRanges::Start(int32 iTemporary)
{
	int32	iRange;

	iRange = *maiFirst.Get(iTemporary);
	return (iRange == -1) ? -1 : *maiStart.Get(iRange);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a temporary is live before and after iPosition (not only read or
// written there).
//
//////////////////////////////////////////////////////////////////////////
bool CIR816LiveRanges::LiveAcross(int32 iTemporary, int32 iPosition)
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
void CIR816LiveRanges::AddRegion(int32 iStart, int32 iEnd)
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
bool CIR816LiveRanges::Fits(int32 iOffset, int32 iBytes)
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
int32 CIR816LiveRanges::WordOf(int32 iOffset)
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
bool CIR816LiveRanges::IsFree(int32 iTemporary, int32 iOffset, int32 iBytes, CArrayTemplate<int32>* paiOffsets)
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
void CIR816LiveRanges::Take(int32 iTemporary, int32 iOffset, int32 iBytes)
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
void CIR816Backend::AddRegions(CIR816LiveRanges* pcRanges)
{
	CIRBlock*		pcBlock;
	SIR816Argument*	psArgument;
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
// Gives the temporaries (but the parameters, which are the caller's
// arguments) their places from IR816_FIRST_TEMPORARY on, sharing places
// between temporaries that are never live at once.  Positions number the
// phis, instructions and jump of each block in the order the blocks are
// written.  A temporary is live from its definition, or the start of a block
// it is live into, to its last use, or the end of a block it is live out of
// (CIR816LiveRanges).  An argument is used where its call pushes it; a
// phi's arguments at the end of their blocks and its result at the start of
// its own, or also at the ends of its predecessors when they put its values
// straight in its place (IsCoalescedPhiBlock).  Uses of an instruction's
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
int32 CIR816Backend::PackTemporaries(void)
{
	CIR816LiveRanges			cRanges;
	CIRLiveSets					cSets;
	CArrayTemplate<SIR816Event>	asEvents;
	CArrayTemplate<int32>		aiBlockEvents;	//Per block: its first event, and one past the last block's last.
	CArrayTemplate<int32>		aiBlockStart;
	CArrayTemplate<int32>		aiBlockEnd;
	CArrayTemplate<int32>		aiOpen;			//Per temporary: where its range in the block being scanned starts, or -1.
	CArrayTemplate<int32>		aiLastUse;		//Per temporary: its last use or definition in that block.
	CArrayTemplate<int32>		aiOpened;		//The temporaries with a range open in that block.
	CArrayTemplate<int32>		aiOrder;
	CArrayTemplate<bool>		abParameter;
	CIRBlock*					pcBlock;
	CIRBlock*					pcSuccessor;
	SIRInstruction*				psInstruction;
	SIRPhi*						psPhi;
	SIRPhiArgument*				psArgument;
	SIR816Event*				psEvent;
	size						uiTemporaries;
	size						uiBlocks;
	size						ui;
	size						uj;
	size						uk;
	int32						iPosition;
	int32						iCallPosition;
	int32						iNone;
	int32						iBytes;
	int32						iOffset;
	int32						iNext;
	int32						iTemporary;
	int32						iEvent;
	int32						iEnd;
	int32						iPair;
	int32						iArgument;
	bool						bFree;
	CArrayTemplate<int32>		aiDepths;		//Per block: how many loops it is in.
	int32						iDepth;
	int32						iBestDepth;
	int32						iBestOffset;
	CArrayTemplate<SIR816LoadPair>	asLoadPairs;
	SIR816LoadPair*				psLoadPair;

	uiTemporaries = mpcFunction->NumTemporaries();
	uiBlocks = mpcFunction->NumBlocks();
	iNone = -1;
	cRanges.Init(uiTemporaries);
	asEvents.Init(CIR816Backend::GetScratch());
	asLoadPairs.Init(CIR816Backend::GetScratch());
	aiBlockEvents.Init(CIR816Backend::GetScratch());
	aiBlockStart.Init(CIR816Backend::GetScratch());
	aiBlockEnd.Init(CIR816Backend::GetScratch());

	//Each block's loop depth, for choosing between pairs.
	aiDepths.Init(CIR816Backend::GetScratch());
	for (ui = 0; ui < uiBlocks; ui++)
	{
		iDepth = mcLoops.Depth(mpcFunction->GetBlock(ui));
		aiDepths.Add(&iDepth);
	}

	//The uses, definitions and writes, and the pairs.
	iPosition = 0;
	for (ui = 0; ui < uiBlocks; ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		iEvent = (int32)asEvents.NumElements();
		aiBlockEvents.Add(&iEvent);
		aiBlockStart.Add(&iPosition);
		for (uj = 0; uj < pcBlock->masPhis.NumElements(); uj++)
		{
			IR816AddEvent(&asEvents, pcBlock->masPhis.Get(uj)->sResult, iPosition, IR816_EVENT_DEFINE);
		}
		iPosition++;
		for (uj = 0; uj < pcBlock->masInstructions.NumElements(); uj++)
		{
			psInstruction = pcBlock->masInstructions.Get(uj);
			if (psInstruction->uiOpcode == IRO_Arg)
			{
				//Used where the call pushes it.
				iCallPosition = iPosition;
				for (uk = uj + 1; uk < pcBlock->masInstructions.NumElements(); uk++)
				{
					if (pcBlock->masInstructions.Get(uk)->uiOpcode == IRO_Call)
					{
						iCallPosition = iPosition + (int32)(uk - uj);
						cRanges.AddPair(pcBlock->masInstructions.Get(uk)->sResult, FoldedBase(psInstruction->asArguments[0]), iCallPosition, *aiDepths.Get(ui));
						break;
					}
				}
				IR816AddEvent(&asEvents, FoldedBase(psInstruction->asArguments[0]), iCallPosition, IR816_EVENT_USE);
			}
			else
			{
				IR816AddEvent(&asEvents, FoldedBase(psInstruction->asArguments[0]), iPosition, IR816_EVENT_USE);
				IR816AddEvent(&asEvents, FoldedBase(psInstruction->asArguments[1]), iPosition, IR816_EVENT_USE);
				IR816AddEvent(&asEvents, FoldedIndex(psInstruction->asArguments[0]), iPosition, IR816_EVENT_USE);
				IR816AddEvent(&asEvents, FoldedIndex(psInstruction->asArguments[1]), iPosition, IR816_EVENT_USE);
			}
			if (psInstruction->sResult.IsTemporary() && (FoldedBase(psInstruction->sResult).Equals(psInstruction->sResult)))
			{
				IR816AddEvent(&asEvents, psInstruction->sResult, iPosition, IR816_EVENT_DEFINE);
				for (iArgument = 0; iArgument < 2; iArgument++)
				{
					if (IR816CanShareArgument(psInstruction->uiOpcode, iArgument) && FoldedBase(psInstruction->asArguments[iArgument]).Equals(psInstruction->asArguments[iArgument]))
					{
						cRanges.AddPair(psInstruction->sResult, psInstruction->asArguments[iArgument], iPosition, *aiDepths.Get(ui));
					}
				}
				if ((psInstruction->uiOpcode == IRO_Load) && !(psInstruction->uiFlags & IRF_Volatile) && FoldedBase(psInstruction->asArguments[0]).IsTemporary())
				{
					//Its pointer's place: Load reads the words that overlap the pointer last (only worth keeping a word on
					//the stack for when a phi joins them, p = p->psNext, so they're paired after the phis).
					psLoadPair = asLoadPairs.Add();
					psLoadPair->sResult = psInstruction->sResult;
					psLoadPair->sPointer = FoldedBase(psInstruction->asArguments[0]);
					psLoadPair->iPosition = iPosition;
					psLoadPair->iDepth = *aiDepths.Get(ui);
					psLoadPair->bWide = Words(psInstruction->sType) > 1;
				}
			}
			iPosition++;
		}

		//The jump's argument and the phi arguments of the successors (copied on the way out).
		IR816AddEvent(&asEvents, pcBlock->msJump.sArgument, iPosition, IR816_EVENT_USE);
		for (uj = 0; uj < pcBlock->NumSuccessors(); uj++)
		{
			pcSuccessor = pcBlock->GetSuccessor(uj);
			for (uk = 0; uk < pcSuccessor->masPhis.NumElements(); uk++)
			{
				psPhi = pcSuccessor->masPhis.Get(uk);
				psArgument = pcSuccessor->FindPhiArgument(psPhi, pcBlock);
				if (psArgument)
				{
					IR816AddEvent(&asEvents, psArgument->sValue, iPosition, IR816_EVENT_USE);
				}
				if (IsCoalescedPhiBlock(pcSuccessor))
				{
					//Written here, on the way out.
					IR816AddEvent(&asEvents, psPhi->sResult, iPosition, IR816_EVENT_WRITE);
					if (psArgument)
					{
						//The copy is made going that way: as often as the shallower block of the two.
						iDepth = *aiDepths.Get(ui);
						if (*aiDepths.Get(pcSuccessor->muiIndex) < iDepth)
						{
							iDepth = *aiDepths.Get(pcSuccessor->muiIndex);
						}
						cRanges.AddPair(psPhi->sResult, psArgument->sValue, iPosition, iDepth);
					}
				}
			}
		}
		aiBlockEnd.Add(&iPosition);
		iPosition++;
	}
	iEvent = (int32)asEvents.NumElements();
	aiBlockEvents.Add(&iEvent);
	for (ui = 0; ui < asLoadPairs.NumElements(); ui++)
	{
		psLoadPair = asLoadPairs.Get(ui);
		if (!psLoadPair->bWide || cRanges.IsPaired((int32)psLoadPair->sResult.Index(), (int32)psLoadPair->sPointer.Index()))
		{
			cRanges.AddPair(psLoadPair->sResult, psLoadPair->sPointer, psLoadPair->iPosition, psLoadPair->iDepth);
		}
	}

	//Each block's uses and definitions, and from them what is live into and out of each block.
	cSets.Init(CIR816Backend::GetScratch());
	cSets.Start(mpcFunction);
	for (ui = 0; ui < uiBlocks; ui++)
	{
		for (iEvent = *aiBlockEvents.Get(ui); iEvent < *aiBlockEvents.Get(ui + 1); iEvent++)
		{
			psEvent = asEvents.Get(iEvent);
			if (psEvent->iKind == IR816_EVENT_USE)
			{
				cSets.Use(ui, IRReference(IRRK_Temporary, (uint32)psEvent->iTemporary));
			}
			else if (psEvent->iKind == IR816_EVENT_DEFINE)
			{
				cSets.Define(ui, IRReference(IRRK_Temporary, (uint32)psEvent->iTemporary));
			}
		}
	}
	cSets.Solve();

	//The ranges: in each block from the start (live in) or the definition, to the end (live out) or the last use.
	aiOpen.Init(CIR816Backend::GetScratch());
	aiLastUse.Init(CIR816Backend::GetScratch());
	aiOpened.Init(CIR816Backend::GetScratch());
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		aiOpen.Add(&iNone);
		aiLastUse.Add(&iNone);
	}
	for (ui = 0; ui < uiBlocks; ui++)
	{
		for (uj = 0; uj < uiTemporaries; uj++)
		{
			if (cSets.IsLiveIn(ui, uj))
			{
				iTemporary = (int32)uj;
				*aiOpen.Get(uj) = *aiBlockStart.Get(ui);
				*aiLastUse.Get(uj) = *aiBlockStart.Get(ui);
				aiOpened.Add(&iTemporary);
			}
		}
		for (iEvent = *aiBlockEvents.Get(ui); iEvent < *aiBlockEvents.Get(ui + 1); iEvent++)
		{
			psEvent = asEvents.Get(iEvent);
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
			iEnd = cSets.IsLiveOut(ui, (size)iTemporary) ? *aiBlockEnd.Get(ui) : *aiLastUse.Get(iTemporary);
			cRanges.Add(iTemporary, *aiOpen.Get(iTemporary), iEnd);
			*aiOpen.Get(iTemporary) = -1;
			*aiLastUse.Get(iTemporary) = -1;
		}
		aiOpened.FakeSetUsedElements(0);
		for (iEvent = *aiBlockEvents.Get(ui); iEvent < *aiBlockEvents.Get(ui + 1); iEvent++)
		{
			psEvent = asEvents.Get(iEvent);
			if (psEvent->iKind == IR816_EVENT_WRITE)
			{
				cRanges.Add(psEvent->iTemporary, psEvent->iPosition, psEvent->iPosition);
			}
		}
	}

	//Placed in the order they start (but the parameters).
	abParameter.Init(CIR816Backend::GetScratch());
	abParameter.GrowToAtLeastNumElements(uiTemporaries + 1, true, 0);
	for (ui = 0; ui < mpcFunction->masParameters.NumElements(); ui++)
	{
		*abParameter.Get(mpcFunction->masParameters.Get(ui)->Index()) = true;
	}
	aiOrder.Init(CIR816Backend::GetScratch());
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		iTemporary = (int32)ui;
		if ((cRanges.Start(iTemporary) >= 0) && (ValueBytes(mpcFunction->GetTemporaryType(IRReference(IRRK_Temporary, (uint32)ui))) != 0) && !*abParameter.Get(ui))
		{
			aiOrder.Add(&iTemporary);
		}
	}
	for (ui = 1; ui < aiOrder.NumElements(); ui++)
	{
		iTemporary = *aiOrder.Get(ui);
		for (uj = ui; (uj > 0) && (cRanges.Start(*aiOrder.Get(uj - 1)) > cRanges.Start(iTemporary)); uj--)
		{
			*aiOrder.Get(uj) = *aiOrder.Get(uj - 1);
		}
		*aiOrder.Get(uj) = iTemporary;
	}

	AddRegions(&cRanges);

	iNext = IR816_FIRST_TEMPORARY;
	for (ui = 0; ui < aiOrder.NumElements(); ui++)
	{
		iTemporary = *aiOrder.Get(ui);
		iBytes = (ValueBytes(mpcFunction->GetTemporaryType(IRReference(IRRK_Temporary, (uint32)iTemporary))) + 1) & ~1;

		//A pair's place (the one where they meet in the deepest loop: the copy saved is done most), then the first
		//place free.  IsFree keeps it inside its region: past the pushed arguments is the caller's.
		bFree = false;
		iBestDepth = -1;
		iBestOffset = -1;
		for (iPair = *cRanges.maiPairFirst.Get(iTemporary); iPair != -1; iPair = *cRanges.maiPairNext.Get(iPair))
		{
			iOffset = *maiOffsets.Get(*cRanges.maiPairOther.Get(iPair));
			if (cRanges.IsFree(iTemporary, iOffset, iBytes, &maiOffsets))
			{
				iDepth = *cRanges.maiPairWeight.Get(iPair);
				if (iDepth > iBestDepth)
				{
					iBestDepth = iDepth;
					iBestOffset = iOffset;
				}
			}
		}
		if (iBestOffset >= 0)
		{
			iOffset = iBestOffset;
			bFree = true;
		}
		if (!bFree)
		{
			iOffset = IR816_FIRST_TEMPORARY;
			while (!cRanges.IsFree(iTemporary, iOffset, iBytes, &maiOffsets))
			{
				iOffset += 2;
			}
		}
		cRanges.Take(iTemporary, iOffset, iBytes);
		*maiOffsets.Get(iTemporary) = iOffset;
		if ((iOffset < IR816_ARGUMENT_STANDIN) && (iOffset + iBytes > iNext))
		{
			iNext = iOffset + iBytes;
		}
	}

	//A temporary with no definition or use still has a place (it is never live).
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		if ((cRanges.Start((int32)ui) == -1) && (ValueBytes(mpcFunction->GetTemporaryType(IRReference(IRRK_Temporary, (uint32)ui))) != 0))
		{
			*maiOffsets.Get(ui) = IR816_FIRST_TEMPORARY;
		}
	}

	AllocateX(&cRanges, &abParameter);
	asLoadPairs.Kill();
	aiDepths.Kill();
	abParameter.Kill();
	aiOrder.Kill();
	aiOpened.Kill();
	aiLastUse.Kill();
	aiOpen.Kill();
	cSets.Kill();
	aiBlockEnd.Kill();
	aiBlockStart.Kill();
	aiBlockEvents.Kill();
	asEvents.Kill();
	cRanges.Kill();
	return iNext;
}


#define IR816_X_BAD		-100000		//A use X can't serve: the temporary isn't kept in X.


//////////////////////////////////////////////////////////////////////////
//
// What keeping argument iArgument of an instruction in X saves (in about
// cycles) where it is read: txa for an lda, phx for a push, the operands
// of a commutative instruction swapped; a second operand read through a
// scratch word costs.  IR816_X_BAD for a use that can't read X.
//
//////////////////////////////////////////////////////////////////////////
static int32 IR816XUse(uint16 uiOpcode, int iArgument)
{
	switch (uiOpcode)
	{
	case IRO_Arg:
	case IRO_Copy:
	case IRO_Shl:
	case IRO_LShr:
	case IRO_AShr:
	case IRO_Mul:
	case IRO_UDiv:
	case IRO_SDiv:
	case IRO_URem:
	case IRO_SRem:
	case IRO_Add:
	case IRO_And:
	case IRO_Or:
	case IRO_Xor:
	case IRO_Not:
		return 3;
	case IRO_Sub:
	case IRO_Eq:
	case IRO_Ne:
	case IRO_ULt:
	case IRO_ULe:
	case IRO_UGt:
	case IRO_UGe:
		return (iArgument == 0) ? 3 : -5;
	case IRO_SLt:
	case IRO_SLe:
	case IRO_SGt:
	case IRO_SGe:
		return 3;
	case IRO_Neg:
		return -5;
	case IRO_SExt:
	case IRO_ZExt:
	case IRO_Trunc:
	case IRO_PtrToI:
	case IRO_IToPtr:
		return 0;
	case IRO_Store:
		return (iArgument == 0) ? 3 : IR816_X_BAD;
	default:
		return IR816_X_BAD;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an instruction's result goes to its place through Store (which
// can put it in X), or can be worked out in A and put in X (a copy, an
// extension or trunc, a 16 bit shift by a constant).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816XResult(SIRInstruction* psInstruction)
{
	switch (psInstruction->uiOpcode)
	{
	case IRO_Copy:
	case IRO_SExt:
	case IRO_ZExt:
	case IRO_Trunc:
	case IRO_PtrToI:
		return true;
	case IRO_Shl:
	case IRO_LShr:
	case IRO_AShr:
		return psInstruction->asArguments[1].IsConstant() && psInstruction->sType.Is(IRTK_I16);
	case IRO_Add:
	case IRO_Sub:
	case IRO_And:
	case IRO_Or:
	case IRO_Xor:
	case IRO_Not:
	case IRO_Neg:
	case IRO_Load:
	case IRO_Call:
	case IRO_Mul:
	case IRO_UDiv:
	case IRO_SDiv:
	case IRO_URem:
	case IRO_SRem:
		return true;
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether writing an instruction changes X: calls (and the runtime and soft
// float routines), and shifts by a count only known at run time.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816ClobbersX(SIRInstruction* psInstruction)
{
	switch (psInstruction->uiOpcode)
	{
	case IRO_Call:
	case IRO_Mul:
	case IRO_UDiv:
	case IRO_SDiv:
	case IRO_URem:
	case IRO_SRem:
	case IRO_FAdd:
	case IRO_FSub:
	case IRO_FMul:
	case IRO_FDiv:
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
		return true;
	case IRO_Shl:
	case IRO_LShr:
	case IRO_AShr:
		return !psInstruction->asArguments[1].IsConstant();
	default:
		return false;
	}
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
void CIR816Backend::AllocateX(CIR816LiveRanges* pcRanges, CArrayTemplate<bool>* pabParameter)
{
	CArrayTemplate<int32>	aiScore;		//Per temporary.
	CArrayTemplate<uint8>	abCandidate;	//Per temporary.
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
	aiScore.Init(CIR816Backend::GetScratch());
	aiScore.GrowToAtLeastNumElements(uiTemporaries + 1, true, 0);
	abCandidate.Init(CIR816Backend::GetScratch());
	abCandidate.GrowToAtLeastNumElements(uiTemporaries + 1, true, 0);
	aiClobbers.Init(CIR816Backend::GetScratch());

	//The scores and the positions that change X, numbered as PackTemporaries numbers them.
	iPosition = 0;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		iDepth = mcLoops.Depth(pcBlock);
		iWeight = 1 << ((iDepth > 4) ? 12 : 3 * iDepth);
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
					*aiScore.Get(sValue.Index()) += IR816_X_BAD;
				}
			}
			for (iArgument = 0; iArgument < 2; iArgument++)
			{
				sValue = psInstruction->asArguments[iArgument];
				if (sValue.IsTemporary())
				{
					iScore = IR816XUse(psInstruction->uiOpcode, iArgument);
					if (!FoldedBase(sValue).Equals(sValue) || (psInstruction->uiOpcode == IRO_Call) || (psInstruction->uiOpcode == IRO_Load))
					{
						iScore = IR816_X_BAD;
					}
					*aiScore.Get(sValue.Index()) += (iScore == IR816_X_BAD) ? IR816_X_BAD : iScore * iWeight;
				}
			}
			sValue = psInstruction->sResult;
			if (sValue.IsTemporary() && FoldedBase(sValue).Equals(sValue) && IR816XResult(psInstruction))
			{
				*abCandidate.Get(sValue.Index()) = true;
				*aiScore.Get(sValue.Index()) += 3 * iWeight;
				if (((psInstruction->uiOpcode == IRO_Add) || (psInstruction->uiOpcode == IRO_Sub)) && psInstruction->asArguments[0].IsTemporary() &&
					psInstruction->asArguments[1].IsConstant() && (mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits <= 2))
				{
					//inx or dex when both are in X.
					*aiScore.Get(sValue.Index()) += 6 * iWeight;
					*aiScore.Get(psInstruction->asArguments[0].Index()) += 6 * iWeight;
				}
			}
			iPosition++;
		}
		if (pcBlock->msJump.sArgument.IsTemporary())
		{
			*aiScore.Get(pcBlock->msJump.sArgument.Index()) += 3 * iWeight;
		}
		for (uj = 0; uj < pcBlock->NumSuccessors(); uj++)
		{
			pcSuccessor = pcBlock->GetSuccessor(uj);
			for (uk = 0; uk < pcSuccessor->masPhis.NumElements(); uk++)
			{
				psPhi = pcSuccessor->masPhis.Get(uk);
				psArgument = pcSuccessor->FindPhiArgument(psPhi, pcBlock);
				*aiScore.Get(psPhi->sResult.Index()) += 5 * iWeight;
				if (psArgument && psArgument->sValue.IsTemporary())
				{
					*aiScore.Get(psArgument->sValue.Index()) += 3 * iWeight;
				}
			}
		}
		iPosition++;
	}

	//The candidates, best first.
	aiOrder.Init(CIR816Backend::GetScratch());
	for (ui = 0; ui < uiTemporaries; ui++)
	{
		iTemporary = (int32)ui;
		if (*abCandidate.Get(ui) && !*pabParameter->Get(ui) && (*aiScore.Get(ui) > 0) && (pcRanges->Start(iTemporary) >= 0) &&
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

	aiInX.Init(CIR816Backend::GetScratch());
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
	abCandidate.Kill();
	aiScore.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::InX(SIRReference sValue)
{
	return sValue.IsTemporary() && (sValue.Index() < mabInX.NumElements()) && *mabInX.Get(sValue.Index());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CIR816Backend::OffsetOf(SIRReference sTemporary)
{
	return *maiOffsets.Get(sTemporary.Index());
}

