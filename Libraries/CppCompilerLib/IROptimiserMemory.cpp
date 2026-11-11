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
#include "IROptimiserInternal.h"


//The optimiser's memory passes: constant data read, pointers' bases and aliasing, redundant loads removed, and slots'
//types unified and slots split.


//////////////////////////////////////////////////////////////////////////
//
// What a load of sType reads at sRoot + lliOffset when sRoot is a symbol
// whose contents never change (a constant global, or known data such as a
// vtable): a constant, or a symbol for a pointer to one (a vtable entry);
// IRNone when it isn't known.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRMemoryPasses::ConstantData(SIRReference sRoot, int64 lliOffset, SIRType sType)
{
	CIRGlobal*		pcData;
	SIRDataItem*	psItem;
	size			ui;
	int64			lliStart;
	int64			lliSize;
	int				iBytes;
	int				i;
	uint64			ulliBits;

	if (!sRoot.IsSymbol() || (lliOffset < 0) || sType.IsFloat())
	{
		return IRNone();
	}
	pcData = mpcModule->FindConstantData(sRoot.Index());
	if (pcData == NULL)
	{
		return IRNone();
	}
	iBytes = mpcModule->GetSize(sType);
	lliStart = 0;
	for (ui = 0; ui < pcData->masItems.NumElements(); ui++)
	{
		psItem = pcData->masItems.Get(ui);
		switch (psItem->uiKind)
		{
		case IRDK_Value:
		case IRDK_Symbol:
			lliSize = mpcModule->GetSize(psItem->sType);
			break;
		case IRDK_Bytes:
			lliSize = (int64)psItem->uiLength;
			break;
		default:
			lliSize = (int64)psItem->ulliValue;
			break;
		}
		if (lliOffset < lliStart + lliSize)
		{
			if (lliOffset + iBytes > lliStart + lliSize)
			{
				//Across two items.
				return IRNone();
			}
			switch (psItem->uiKind)
			{
			case IRDK_Value:
				return ((lliOffset == lliStart) && psItem->sType.Equals(sType)) ? mpcFunction->AddConstant(sType, psItem->ulliValue) : IRNone();
			case IRDK_Symbol:
				return ((lliOffset == lliStart) && sType.IsPointer() && (psItem->lliOffset == 0)) ? IRReference(IRRK_Symbol, (uint32)psItem->ulliValue) : IRNone();
			case IRDK_Bytes:
				if (!sType.IsInteger())
				{
					return IRNone();
				}
				ulliBits = 0;
				for (i = iBytes - 1; i >= 0; i--)
				{
					ulliBits = (ulliBits << 8) | *pcData->macBytes.Get(psItem->uiStart + (size)(lliOffset - lliStart) + i);
				}
				return mpcFunction->AddConstant(sType, ulliBits);
			default:
				return (sType.IsInteger() || sType.IsPointer()) ? mpcFunction->AddConstant(sType, 0) : IRNone();
			}
		}
		lliStart += lliSize;
	}
	return IRNone();
}


//////////////////////////////////////////////////////////////////////////
//
// An address as a root (a temporary or symbol that isn't a ptradd of a
// constant) and a constant offset from it.
//
//////////////////////////////////////////////////////////////////////////
void CIRMemoryPasses::SplitAddress(SIRReference sAddress, SIRReference* psRoot, int64* plliOffset)
{
	SIRInstruction*	psDefinition;
	int				iSteps;
	size			uiParameter;

	*psRoot = sAddress;
	*plliOffset = 0;
	for (iSteps = 0; iSteps < 16; iSteps++)
	{
		psDefinition = mpcContext->mcDefinitions.Instruction(*psRoot);
		if ((psDefinition == NULL) || (psDefinition->uiOpcode != IRO_PtrAdd) || !psDefinition->asArguments[1].IsConstant())
		{
			break;
		}
		*plliOffset += mpcFunction->GetSigned(psDefinition->asArguments[1]);
		*psRoot = psDefinition->asArguments[0];
	}

	//Every argaddr of a parameter is the same memory: the first is its root.
	psDefinition = mpcContext->mcDefinitions.Instruction(*psRoot);
	if (psDefinition && (psDefinition->uiOpcode == IRO_ArgAddress))
	{
		uiParameter = (size)mpcFunction->GetSigned(psDefinition->asArguments[0]);
		if (uiParameter < mpcContext->masArgAddresses.NumElements())
		{
			*psRoot = *mpcContext->masArgAddresses.Get(uiParameter);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if a root is a slot or an argaddr: memory no other root names.
//
//////////////////////////////////////////////////////////////////////////
bool CIRMemoryPasses::IsFrameRoot(SIRReference sRoot)
{
	SIRInstruction*	psDefinition;

	psDefinition = mpcContext->mcDefinitions.Instruction(sRoot);
	return psDefinition && ((psDefinition->uiOpcode == IRO_Slot) || (psDefinition->uiOpcode == IRO_ArgAddress));
}


//////////////////////////////////////////////////////////////////////////
//
// What a root's pointer is based on: a pointer loaded from a restrict
// symbol or a parameter, through ptradds (with any offset) and phis whose
// values are all based on the same.  Else IRPB_Unknown.
//
//////////////////////////////////////////////////////////////////////////
void CIRMemoryPasses::PointerBase(SIRReference sRoot, SIRPointerBase* psBase)
{
	SIRReference	asWork[IR_BASE_VALUES];
	SIRReference	asSeen[IR_BASE_VALUES];
	size			uiWork;
	size			uiSeen;
	size			ui;
	SIRReference	sValue;
	SIRInstruction*	psDefinition;
	SIRPhi*			psPhi;
	SIRPointerBase	sBase;
	bool			bFound;
	int				iSteps;
	int				iParameter;

	psBase->eKind = IRPB_Unknown;
	if (!mpcContext->mbRestrict)
	{
		return;
	}

	asWork[0] = sRoot;
	uiWork = 1;
	uiSeen = 0;
	bFound = false;
	while (uiWork > 0)
	{
		sValue = asWork[--uiWork];
		for (iSteps = 0; iSteps < 16; iSteps++)
		{
			psDefinition = mpcContext->mcDefinitions.Instruction(sValue);
			if ((psDefinition == NULL) || (psDefinition->uiOpcode != IRO_PtrAdd))
			{
				break;
			}
			sValue = psDefinition->asArguments[0];
		}
		for (ui = 0; ui < uiSeen; ui++)
		{
			if (asSeen[ui].Equals(sValue))
			{
				break;
			}
		}
		if (ui < uiSeen)
		{
			continue;
		}
		if (uiSeen == IR_BASE_VALUES)
		{
			psBase->eKind = IRPB_Unknown;
			return;
		}
		asSeen[uiSeen++] = sValue;

		psPhi = mpcContext->mcDefinitions.Phi(sValue);
		if (psPhi)
		{
			for (ui = 0; ui < psPhi->asArguments.NumElements(); ui++)
			{
				if (uiWork == IR_BASE_VALUES)
				{
					psBase->eKind = IRPB_Unknown;
					return;
				}
				asWork[uiWork++] = psPhi->asArguments.Get(ui)->sValue;
			}
			continue;
		}

		iParameter = mpcFunction->FindParameter(sValue);
		psDefinition = mpcContext->mcDefinitions.Instruction(sValue);
		if (iParameter >= 0)
		{
			sBase.eKind = IRPB_Parameter;
			sBase.uiIndex = (uint32)iParameter;
			sBase.bRestrict = mpcFunction->IsRestrictParameter((size)iParameter);
		}
		else if (psDefinition && (psDefinition->uiOpcode == IRO_Load) && psDefinition->asArguments[0].IsSymbol() && mpcModule->IsRestrict(psDefinition->asArguments[0].Index()))
		{
			sBase.eKind = IRPB_RestrictSymbol;
			sBase.uiIndex = psDefinition->asArguments[0].Index();
			sBase.bRestrict = true;
		}
		else
		{
			psBase->eKind = IRPB_Unknown;
			return;
		}

		if (!bFound)
		{
			*psBase = sBase;
			bFound = true;
		}
		else if ((psBase->eKind != sBase.eKind) || (psBase->uiIndex != sBase.uiIndex))
		{
			psBase->eKind = IRPB_Unknown;
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// False only if the two accesses can't touch the same bytes: the same root
// at offsets that don't overlap, different symbols, different slots or
// argaddrs, or a slot and a symbol.  What is reached through a restrict
// pointer (a restrict symbol's value or a restrict parameter, see
// PointerBase) is no symbol's or slot's, and isn't reached through a
// pointer with another base when either base is restrict and a parameter,
// or both are restrict.  A restrict symbol and a plain parameter may alias:
// the caller may have passed the symbol's value.
//
//////////////////////////////////////////////////////////////////////////
bool CIRMemoryPasses::MayAlias(SIRAvailable* psAvailable, SIRReference sRoot, int64 lliOffset, int iBytes)
{
	bool			bFrame1;
	bool			bFrame2;
	SIRPointerBase	sBase1;
	SIRPointerBase	sBase2;
	bool			bRestrict1;
	bool			bRestrict2;

	if (psAvailable->sRoot.Equals(sRoot))
	{
		return (psAvailable->lliOffset < lliOffset + iBytes) && (lliOffset < psAvailable->lliOffset + mpcModule->GetSize(psAvailable->sType));
	}
	bFrame1 = IsFrameRoot(psAvailable->sRoot) || psAvailable->sRoot.IsSymbol();
	bFrame2 = IsFrameRoot(sRoot) || sRoot.IsSymbol();
	if (bFrame1 && bFrame2)
	{
		return false;
	}

	sBase1.eKind = IRPB_Unknown;
	sBase2.eKind = IRPB_Unknown;
	if (!bFrame1)
	{
		PointerBase(psAvailable->sRoot, &sBase1);
	}
	if (!bFrame2)
	{
		PointerBase(sRoot, &sBase2);
	}
	bRestrict1 = (sBase1.eKind != IRPB_Unknown) && sBase1.bRestrict;
	bRestrict2 = (sBase2.eKind != IRPB_Unknown) && sBase2.bRestrict;
	if ((bRestrict1 && bFrame2) || (bRestrict2 && bFrame1))
	{
		return false;
	}
	if ((sBase1.eKind == IRPB_Unknown) || (sBase2.eKind == IRPB_Unknown))
	{
		return true;
	}
	if ((sBase1.eKind == sBase2.eKind) && (sBase1.uiIndex == sBase2.uiIndex))
	{
		return true;
	}
	if ((bRestrict1 && bRestrict2) || (bRestrict1 && (sBase1.eKind == IRPB_Parameter)) || (bRestrict2 && (sBase2.eKind == IRPB_Parameter)))
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Notes that sValue is in memory at sRoot + lliOffset, unless too much is
// noted already.
//
//////////////////////////////////////////////////////////////////////////
void CIRMemoryPasses::AddAvailable(CArrayIRAvailables* pasAvailable, SIRReference sRoot, int64 lliOffset, SIRType sType, SIRReference sValue)
{
	SIRAvailable*	psAvailable;

	if (pasAvailable->NumElements() < IR_AVAILABLE_LIMIT)
	{
		psAvailable = pasAvailable->Add();
		psAvailable->sRoot = sRoot;
		psAvailable->lliOffset = lliOffset;
		psAvailable->sType = sType;
		psAvailable->sValue = sValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What a block entered from several blocks knows is in memory: what all
// of them leave known, as the same value (none if one is a loop's way back,
// not walked yet).
//
//////////////////////////////////////////////////////////////////////////
void CIRMemoryPasses::MergeAvailable(CIRBlock* pcBlock, CArrayIRAvailables* pasAvailable, CArrayIRAvailables* pasEnds, CArrayTemplate<SIRBlockEnd>* pasBlockEnds)
{
	size			uiPredecessor;
	size			uiFirst;
	size			uiOther;
	SIRBlockEnd*	psFirstEnd;
	SIRBlockEnd*	psEnd;
	SIRAvailable*	psCandidate;
	SIRAvailable*	psOther;
	bool			bEverywhere;
	bool			bFound;

	if (pcBlock->mapcPredecessors.NumElements() < 2)
	{
		return;
	}
	for (uiPredecessor = 0; uiPredecessor < pcBlock->mapcPredecessors.NumElements(); uiPredecessor++)
	{
		if (pasBlockEnds->Get(pcBlock->mapcPredecessors.GetPtr(uiPredecessor)->muiIndex)->iStart < 0)
		{
			return;
		}
	}
	psFirstEnd = pasBlockEnds->Get(pcBlock->mapcPredecessors.GetPtr(0)->muiIndex);
	for (uiFirst = 0; uiFirst < (size)psFirstEnd->iCount; uiFirst++)
	{
		psCandidate = pasEnds->Get(psFirstEnd->iStart + uiFirst);
		bEverywhere = true;
		for (uiPredecessor = 1; bEverywhere && (uiPredecessor < pcBlock->mapcPredecessors.NumElements()); uiPredecessor++)
		{
			psEnd = pasBlockEnds->Get(pcBlock->mapcPredecessors.GetPtr(uiPredecessor)->muiIndex);
			bFound = false;
			for (uiOther = 0; uiOther < (size)psEnd->iCount; uiOther++)
			{
				psOther = pasEnds->Get(psEnd->iStart + uiOther);
				if (psOther->sRoot.Equals(psCandidate->sRoot) && (psOther->lliOffset == psCandidate->lliOffset) && psOther->sType.Equals(psCandidate->sType) && Replacement(psOther->sValue).Equals(Replacement(psCandidate->sValue)))
				{
					bFound = true;
					break;
				}
			}
			bEverywhere = bFound;
		}
		if (bEverywhere)
		{
			AddAvailable(pasAvailable, psCandidate->sRoot, psCandidate->lliOffset, psCandidate->sType, Replacement(psCandidate->sValue));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A load of what is known to be in memory is that value: what an earlier
// load read or a store wrote, in its block or in the blocks before it (all
// the blocks that enter it agreeing), with no call, volatile load or store
// that may touch it between.
//
//////////////////////////////////////////////////////////////////////////
bool CIRMemoryPasses::RemoveRedundantLoads(void)
{
	CArrayIRAvailables				asAvailable;
	CArrayIRAvailables				asEnds;			//What each block leaves known, from asBlockEnds.
	CArrayTemplate<SIRBlockEnd>		asBlockEnds;	//Per block index.
	size							ui;
	size							uiInstruction;
	size							uiAvailable;
	CIRBlock*						pcBlock;
	SIRInstruction*					psInstruction;
	SIRAvailable*					psAvailable;
	SIRBlockEnd*					psEnd;
	SIRReference					sRoot;
	SIRReference					sConstant;
	int64							lliOffset;
	bool							bChanged;
	bool							bFound;

	Renumber();
	mpcFunction->ComputePredecessors();
	ComputeDominators();
	ClearReplacements();
	FindDefinitions();

	asAvailable.Init();
	asEnds.Init();
	asBlockEnds.Init();
	asBlockEnds.GrowToAtLeastNumElements(mpcFunction->NumBlocks(), true, 0xFF);
	bChanged = false;
	for (ui = 0; ui < mpcContext->mcDominators.NumReachable(); ui++)
	{
		pcBlock = mpcContext->mcDominators.GetReversePostOrder(ui);
		asAvailable.FakeSetUsedElements(0);
		if (pcBlock->mapcPredecessors.NumElements() == 1)
		{
			psEnd = asBlockEnds.Get(pcBlock->mapcPredecessors.GetPtr(0)->muiIndex);
			if (psEnd->iStart >= 0)
			{
				asAvailable.InsertBlockAt(asEnds.Get(psEnd->iStart), psEnd->iCount, 0);
			}
		}
		else
		{
			MergeAvailable(pcBlock, &asAvailable, &asEnds, &asBlockEnds);
		}

		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			psInstruction->asArguments[0] = Replacement(psInstruction->asArguments[0]);
			psInstruction->asArguments[1] = Replacement(psInstruction->asArguments[1]);
			if ((psInstruction->uiOpcode == IRO_Load) && !(psInstruction->uiFlags & IRF_Volatile) && psInstruction->sResult.IsTemporary())
			{
				SplitAddress(psInstruction->asArguments[0], &sRoot, &lliOffset);
				sConstant = ConstantData(sRoot, lliOffset, psInstruction->sType);
				if (!sConstant.IsNone())
				{
					Replace(psInstruction->sResult, sConstant);
					bChanged = true;
					continue;
				}
				bFound = false;
				for (uiAvailable = 0; uiAvailable < asAvailable.NumElements(); uiAvailable++)
				{
					psAvailable = asAvailable.Get(uiAvailable);
					if (psAvailable->sRoot.Equals(sRoot) && (psAvailable->lliOffset == lliOffset) && psAvailable->sType.Equals(psInstruction->sType))
					{
						Replace(psInstruction->sResult, psAvailable->sValue);
						bChanged = true;
						bFound = true;
						break;
					}
				}
				if (!bFound)
				{
					AddAvailable(&asAvailable, sRoot, lliOffset, psInstruction->sType, psInstruction->sResult);
				}
			}
			else if (psInstruction->uiOpcode == IRO_Store)
			{
				SplitAddress(psInstruction->asArguments[1], &sRoot, &lliOffset);
				for (uiAvailable = asAvailable.NumElements(); uiAvailable > 0; uiAvailable--)
				{
					if (MayAlias(asAvailable.Get(uiAvailable - 1), sRoot, lliOffset, mpcModule->GetSize(psInstruction->sType)))
					{
						asAvailable.RemoveAt(uiAvailable - 1, true);
					}
				}
				if (!(psInstruction->uiFlags & IRF_Volatile))
				{
					AddAvailable(&asAvailable, sRoot, lliOffset, psInstruction->sType, psInstruction->asArguments[0]);
				}
			}
			else if (MayWriteMemory(psInstruction) && (psInstruction->uiOpcode != IRO_Arg))
			{
				asAvailable.FakeSetUsedElements(0);
			}
		}

		psEnd = asBlockEnds.Get(pcBlock->muiIndex);
		psEnd->iStart = (int32)asEnds.NumElements();
		psEnd->iCount = (int32)asAvailable.NumElements();
		if (asAvailable.NumElements() > 0)
		{
			asEnds.InsertBlockAt(asAvailable.Get(0), asAvailable.NumElements(), asEnds.NumElements());
		}
	}
	asBlockEnds.Kill();
	asEnds.Kill();
	asAvailable.Kill();
	if (bChanged)
	{
		ApplyReplacements();
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// The slot and constant offset an address is (a slot, or a ptradd of one
// and a constant); false if it is neither.
//
//////////////////////////////////////////////////////////////////////////
bool CIRMemoryPasses::SlotOffset(SIRReference sAddress, SIRReference* psSlot, int64* plliOffset)
{
	SIRInstruction*	psDefinition;

	psDefinition = mpcContext->mcDefinitions.Instruction(sAddress);
	if (psDefinition && (psDefinition->uiOpcode == IRO_PtrAdd) && psDefinition->asArguments[1].IsConstant())
	{
		*plliOffset = mpcFunction->GetSigned(psDefinition->asArguments[1]);
		psDefinition = mpcContext->mcDefinitions.Instruction(psDefinition->asArguments[0]);
		if (psDefinition && (psDefinition->uiOpcode == IRO_Slot))
		{
			*psSlot = psDefinition->sResult;
			return true;
		}
		return false;
	}
	if (psDefinition && (psDefinition->uiOpcode == IRO_Slot))
	{
		*psSlot = sAddress;
		*plliOffset = 0;
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The type a slot's bytes at lliOffset are loaded and stored at when it is
// one type, or a pointer and an integer of a pointer's size (a pointer
// field copied as an integer: an object returned by value): the pointer.
// IRTK_None for anything else (or a slot with an access that isn't a load
// or store).  mpcContext->mcDefinitions must be found.
//
//////////////////////////////////////////////////////////////////////////
SIRType CIRMemoryPasses::SlotOffsetType(SIRReference sSlot, int64 lliOffset)
{
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRReference	sFoundSlot;
	SIRType			sType;
	SIRType			sInteger;
	int64			lliFound;
	size			ui;
	size			uj;
	bool			bPointer;

	bPointer = false;
	sInteger = IRType(IRTK_None);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uj = 0; uj < pcBlock->masInstructions.NumElements(); uj++)
		{
			psInstruction = pcBlock->masInstructions.Get(uj);
			if (((psInstruction->uiOpcode != IRO_Load) && (psInstruction->uiOpcode != IRO_Store)) ||
				!SlotOffset(psInstruction->asArguments[(psInstruction->uiOpcode == IRO_Load) ? 0 : 1], &sFoundSlot, &lliFound) ||
				!sFoundSlot.Equals(sSlot) || (lliFound != lliOffset))
			{
				continue;
			}
			sType = psInstruction->sType;
			if (sType.IsPointer())
			{
				bPointer = true;
			}
			else if (sType.IsInteger() && (mpcModule->GetSize(sType) == mpcModule->GetSize(IRType(IRTK_Pointer))) &&
					 (sInteger.Is(IRTK_None) || sInteger.Equals(sType)))
			{
				sInteger = sType;
			}
			else
			{
				return IRType(IRTK_None);
			}
		}
	}
	return (bPointer && !sInteger.Is(IRTK_None)) ? IRType(IRTK_Pointer) : IRType(IRTK_None);
}


//////////////////////////////////////////////////////////////////////////
//
// A slot's pointer that is also loaded or stored as an integer of a
// pointer's size (an object returned by value is copied a word at a time)
// is only loaded and stored as a pointer: the integer is converted
// (ptrtoi after a load, itoptr before a store), so the slot can be split
// and promoted.
//
//////////////////////////////////////////////////////////////////////////
bool CIRMemoryPasses::UnifySlotTypes(void)
{
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRReference	sSlot;
	SIRReference	sValue;
	SIRType			sType;
	int64			lliOffset;
	size			ui;
	size			uj;
	bool			bChanged;

	NeedDefinitions();
	bChanged = false;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uj = pcBlock->masInstructions.NumElements(); uj > 0; uj--)
		{
			psInstruction = pcBlock->masInstructions.Get(uj - 1);
			if (((psInstruction->uiOpcode != IRO_Load) && (psInstruction->uiOpcode != IRO_Store)) || (psInstruction->uiFlags & IRF_Volatile) ||
				!psInstruction->sType.IsInteger() || ((psInstruction->uiOpcode == IRO_Load) && !psInstruction->sResult.IsTemporary()) ||
				!SlotOffset(psInstruction->asArguments[(psInstruction->uiOpcode == IRO_Load) ? 0 : 1], &sSlot, &lliOffset))
			{
				continue;
			}
			sType = SlotOffsetType(sSlot, lliOffset);
			if (!sType.IsPointer())
			{
				continue;
			}
			if (psInstruction->uiOpcode == IRO_Load)
			{
				//x = load iN p is t = load ptr p; x = ptrtoi t.
				sValue = InsertInstruction(pcBlock, uj - 1, IRO_Load, IRType(IRTK_Pointer), psInstruction->asArguments[0], IRNone());
				pcBlock->masInstructions.Get(uj - 1)->uiFlags = pcBlock->masInstructions.Get(uj)->uiFlags;
				psInstruction = pcBlock->masInstructions.Get(uj);
				psInstruction->uiOpcode = IRO_PtrToI;
				psInstruction->asArguments[0] = sValue;
				psInstruction->asArguments[1] = IRNone();
			}
			else
			{
				//store iN x, p is t = itoptr x; store ptr t, p.
				sValue = InsertInstruction(pcBlock, uj - 1, IRO_IToPtr, IRType(IRTK_Pointer), psInstruction->asArguments[0], IRNone());
				psInstruction = pcBlock->masInstructions.Get(uj);
				psInstruction->sType = IRType(IRTK_Pointer);
				psInstruction->asArguments[0] = sValue;
			}
			bChanged = true;
			FindDefinitions();
		}
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// Slots go to the start of the entry block (inlining leaves them in other
// blocks), and a slot only loaded and stored, at constant offsets that
// don't overlap and each at one type (an object's fields), becomes a slot
// per offset, which slot promotion can make temporaries.
//
//////////////////////////////////////////////////////////////////////////
bool CIRMemoryPasses::SplitSlots(void)
{
	CArrayTemplate<SIRSlotAccess>	asAccesses;
	CArrayTemplate<uint8>			auiInvalid;		//Per temporary (a slot's): 1 if it can't be split.
	size							ui;
	size							uiInstruction;
	size							uiArgument;
	size							uiAccess;
	size							uiOther;
	CIRBlock*						pcBlock;
	CIRBlock*						pcEntry;
	SIRInstruction*					psInstruction;
	SIRInstruction					sMoved;
	SIRSlotAccess*					psAccess;
	SIRSlotAccess*					psOther;
	SIRSlotAccess					sAccess;
	SIRReference					sSlot;
	SIRReference					sValue;
	SIRReference					sNew;
	int64							lliOffset;
	bool							bAccess;
	bool							bChanged;

	//Every slot to the start of the entry block.
	bChanged = false;
	pcEntry = mpcFunction->GetEntry();
	for (ui = 1; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = pcBlock->masInstructions.NumElements(); uiInstruction > 0; uiInstruction--)
		{
			if (pcBlock->masInstructions.Get(uiInstruction - 1)->uiOpcode == IRO_Slot)
			{
				sMoved = *pcBlock->masInstructions.Get(uiInstruction - 1);
				pcBlock->masInstructions.RemoveAt(uiInstruction - 1, true);
				pcEntry->masInstructions.InsertAt(&sMoved, 0);
				bChanged = true;
			}
		}
	}

	FindDefinitions();
	asAccesses.Init();
	auiInvalid.Init();
	auiInvalid.GrowToAtLeastNumElements(mpcFunction->NumTemporaries(), true, 0);

	//Each use of a slot (or of a ptradd of it and a constant) must be a load's or store's address.
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masPhis.NumElements(); uiInstruction++)
		{
			for (uiArgument = 0; uiArgument < pcBlock->masPhis.Get(uiInstruction)->asArguments.NumElements(); uiArgument++)
			{
				if (SlotOffset(pcBlock->masPhis.Get(uiInstruction)->asArguments.Get(uiArgument)->sValue, &sSlot, &lliOffset))
				{
					*auiInvalid.Get(sSlot.Index()) = 1;
				}
			}
		}
		if (SlotOffset(pcBlock->msJump.sArgument, &sSlot, &lliOffset))
		{
			*auiInvalid.Get(sSlot.Index()) = 1;
		}
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			for (uiArgument = 0; uiArgument < 2; uiArgument++)
			{
				sValue = psInstruction->asArguments[uiArgument];
				if (!SlotOffset(sValue, &sSlot, &lliOffset))
				{
					continue;
				}
				//The ptradd that makes an offset address isn't a use of the slot's memory.
				if ((psInstruction->uiOpcode == IRO_PtrAdd) && (uiArgument == 0) && sValue.Equals(sSlot) && psInstruction->asArguments[1].IsConstant())
				{
					continue;
				}
				bAccess = !(psInstruction->uiFlags & IRF_Volatile) &&
						  (((psInstruction->uiOpcode == IRO_Load) && (uiArgument == 0)) || ((psInstruction->uiOpcode == IRO_Store) && (uiArgument == 1)));
				if (!bAccess)
				{
					*auiInvalid.Get(sSlot.Index()) = 1;
					continue;
				}
				sAccess.sSlot = sSlot;
				sAccess.lliOffset = lliOffset;
				sAccess.sType = psInstruction->sType;
				sAccess.sNew = IRNone();
				asAccesses.Add(&sAccess);
			}
		}
	}

	//One type at each offset, offsets that don't overlap, inside the slot; more than one offset, or not 0.
	for (uiAccess = 0; uiAccess < asAccesses.NumElements(); uiAccess++)
	{
		psAccess = asAccesses.Get(uiAccess);
		if (*auiInvalid.Get(psAccess->sSlot.Index()))
		{
			continue;
		}
		if ((psAccess->lliOffset < 0) || (psAccess->lliOffset + mpcModule->GetSize(psAccess->sType) > mpcFunction->GetSigned(mpcContext->mcDefinitions.Instruction(psAccess->sSlot)->asArguments[0])))
		{
			*auiInvalid.Get(psAccess->sSlot.Index()) = 1;
			continue;
		}
		for (uiOther = 0; uiOther < asAccesses.NumElements(); uiOther++)
		{
			psOther = asAccesses.Get(uiOther);
			if (!psOther->sSlot.Equals(psAccess->sSlot))
			{
				continue;
			}
			if ((psOther->lliOffset == psAccess->lliOffset) ? !psOther->sType.Equals(psAccess->sType) :
				((psOther->lliOffset < psAccess->lliOffset + mpcModule->GetSize(psAccess->sType)) && (psAccess->lliOffset < psOther->lliOffset + mpcModule->GetSize(psOther->sType))))
			{
				*auiInvalid.Get(psAccess->sSlot.Index()) = 1;
				break;
			}
		}
	}

	//A slot per offset; the old slot and its ptradds are replaced by them (and die).
	for (uiAccess = 0; uiAccess < asAccesses.NumElements(); uiAccess++)
	{
		psAccess = asAccesses.Get(uiAccess);
		if (*auiInvalid.Get(psAccess->sSlot.Index()) || !psAccess->sNew.IsNone())
		{
			continue;
		}
		if (psAccess->lliOffset == 0)
		{
			for (uiOther = 0; (uiOther < asAccesses.NumElements()) && (asAccesses.Get(uiOther)->lliOffset == 0 || !asAccesses.Get(uiOther)->sSlot.Equals(psAccess->sSlot)); uiOther++)
			{
			}
			if (uiOther == asAccesses.NumElements())
			{
				//Only ever at 0: already a slot slot promotion takes.
				continue;
			}
		}
		sNew = mpcFunction->AddTemporary(IRType(IRTK_Pointer));
		pcEntry->masInstructions.InsertAt(0);
		psInstruction = pcEntry->masInstructions.Get(0);
		psInstruction->uiOpcode = IRO_Slot;
		psInstruction->uiFlags = 0;
		psInstruction->uiNamed = 0;
		psInstruction->uiSource = 0;
		psInstruction->sType = IRType(IRTK_Pointer);
		psInstruction->sResult = sNew;
		psInstruction->asArguments[0] = mpcFunction->AddInteger(IRType(IRTK_I32), mpcModule->GetSize(psAccess->sType));
		psInstruction->asArguments[1] = mpcFunction->AddInteger(IRType(IRTK_I32), 4);
		for (uiOther = uiAccess; uiOther < asAccesses.NumElements(); uiOther++)
		{
			psOther = asAccesses.Get(uiOther);
			if (psOther->sSlot.Equals(psAccess->sSlot) && (psOther->lliOffset == psAccess->lliOffset))
			{
				psOther->sNew = sNew;
			}
		}
		bChanged = true;
	}

	//Loads and stores of a split slot use the new slots.
	if (bChanged)
	{
		FindDefinitions();
		for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
		{
			pcBlock = mpcFunction->GetBlock(ui);
			for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
			{
				psInstruction = pcBlock->masInstructions.Get(uiInstruction);
				uiArgument = (psInstruction->uiOpcode == IRO_Load) ? 0 : 1;
				if (((psInstruction->uiOpcode != IRO_Load) && (psInstruction->uiOpcode != IRO_Store)) || !SlotOffset(psInstruction->asArguments[uiArgument], &sSlot, &lliOffset))
				{
					continue;
				}
				for (uiAccess = 0; uiAccess < asAccesses.NumElements(); uiAccess++)
				{
					psAccess = asAccesses.Get(uiAccess);
					if (psAccess->sSlot.Equals(sSlot) && (psAccess->lliOffset == lliOffset) && !psAccess->sNew.IsNone())
					{
						psInstruction->asArguments[uiArgument] = psAccess->sNew;
						break;
					}
				}
			}
		}
	}
	auiInvalid.Kill();
	asAccesses.Kill();
	return bChanged;
}
