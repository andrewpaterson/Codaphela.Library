/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela InputLib

Codaphela InputLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela InputLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela InputLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "BaseLib/ArraySize.h"
#include "InputSourceDesc.h"
#include "InputChords.h"
#include "InputChord.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size MatchesGroupEvent(CArrayActionInputChordCriteria* pasChordCriteria, CArrayInputDeviceTimeValue* pcHistory, size iStartPos)
{
	size							i;
	SInputDeviceTimeValue*			psHistory;
	size							j;
	CActionInputChordCriteria*		psChordCriteria;
	size							iCount;
	bool							abMatched[MAX_GROUP_SIZE];
	float							fValue;
	bool							bMatch;
	bool							bAnyMatch;
	size							iEnd;
	bool							bResult;
	size							uiChordNumElements;

	memset(abMatched, 0, MAX_GROUP_SIZE * sizeof(bool));
	iCount = 0;
	uiChordNumElements = pasChordCriteria->NumElements();
	iEnd = uiChordNumElements+ iStartPos;
	if (iEnd > pcHistory->NumElements())
	{
		iEnd = pcHistory->NumElements();
	}

	for (i = iStartPos; i < iEnd; i++)
	{
		psHistory = pcHistory->Get(i);

		bAnyMatch = false;
		for (j = 0; j < uiChordNumElements; j++)
		{
			if (!abMatched[j])
			{
				psChordCriteria = pasChordCriteria->Get(j);
				bMatch = psChordCriteria->mpcEvaluator->Evaluate(psHistory, &fValue);
				if (bMatch)
				{
					bResult =	(((psChordCriteria->mpcEvaluator->GetChordType() == ICT_Toggle) &&
								(((psChordCriteria->eAction == BAA_Active) && (fValue == 1.0f)) ||
								((psChordCriteria->eAction == BAA_Inactive) && (fValue == 0.0f)))) 
							||
								((psChordCriteria->mpcEvaluator->GetChordType() == ICT_Range) &&
								(((psChordCriteria->eAction == BAA_Active) && (fValue > 0.0f)) ||
								((psChordCriteria->eAction == BAA_Inactive) && (fValue == 0.0f)))));
					if (bResult)
					{
						abMatched[j] = true;
						iCount++;
						bAnyMatch = true;
						break;
					}
				}
			}
		}

		if (!bAnyMatch)
		{
			return 0;
		}
	}

	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChord::Init(CAction* pcAction, CInputChords* pcInputChords, CInputChordDesc* pcDesc)
{
	mpcInputChords = pcInputChords;
	mpcAction = pcAction;
	memset(&muActionCriteria, 0, sizeof(UInputChordCriteria));
	muActionCriteria.eType.eAction = BAA_Unknown;
	miTotalCriteria = 0;
	miUpdateCriteria = 0;
	masStartDeviceValue.Init();
	masScratchPadDeviceValue.Init();
	mpcDesc = pcDesc;
	miMatchedCriteria = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChord::Kill(void)
{
	masScratchPadDeviceValue.Kill();
	masStartDeviceValue.Kill();
	KillActionCriteria(&muActionCriteria);
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Returns false if another chord of the same chords could match the same
// inputs, both would be called.  This chord is then removed and killed
// and *ppcMatching, if given, is the other chord (NULL when true).
//
//////////////////////////////////////////////////////////////////////////
bool CInputChord::Done(CInputChord** ppcMatching)
{
	CInputChord*	pcMatching;

	Calculate();

	pcMatching = mpcInputChords->FindMatchingChord(this);
	if (ppcMatching)
	{
		*ppcMatching = pcMatching;
	}
	if (pcMatching)
	{
		//This is killed, nothing of it can be used after.
		mpcInputChords->RemoveChord(this);
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChord::Calculate(void)
{
	CalculateLongestAction();
	CalculateTotalCriteria();
	CalculatePotentialStart();
}


//////////////////////////////////////////////////////////////////////////
//
// True if the input could be the first of this chord.
//
//////////////////////////////////////////////////////////////////////////
bool CInputChord::IsStart(SInputDeviceValue* psValue)
{
	CActionInputChordCriteria*	psCriteria;
	float						fValue;
	size						i;
	size						uiNumElements;

	uiNumElements = masStartDeviceValue.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		psCriteria = masStartDeviceValue.Get(i);
		if (psCriteria->mpcEvaluator->Evaluate(psValue, &fValue))
		{
			if (((psCriteria->eAction == BAA_Active) && (fValue > 0.0f)) || ((psCriteria->eAction == BAA_Inactive) && (fValue == 0.0f)))
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The criteria as blocks of inputs that follow each other; the inputs in
// a block are in any order.  An active or inactive criteria is a block of
// one, a group is a block of its criteria and an ordered is its children's
// blocks one after another.
//
//////////////////////////////////////////////////////////////////////////
void FlattenCriteria(UInputChordCriteria* puCriteria, CArrayTemplate<CActionInputChordCriteria>* pasInputs, CArraySize* paiBlockSizes)
{
	size					i;
	size					uiNumElements;
	UInputChordCriteria*	puChild;

	switch (puCriteria->eType.eAction)
	{
	case BAA_Active:
		pasInputs->Add(&puCriteria->cActive);
		paiBlockSizes->Add(1);
		break;
	case BAA_Inactive:
		pasInputs->Add(&puCriteria->cInactive);
		paiBlockSizes->Add(1);
		break;
	case BAA_Group:
		uiNumElements = puCriteria->cGroup.mausBasicActionCriteria.NumElements();
		for (i = 0; i < uiNumElements; i++)
		{
			puChild = (UInputChordCriteria*)puCriteria->cGroup.mausBasicActionCriteria.Get(i);
			if (puChild->eType.eAction == BAA_Active)
			{
				pasInputs->Add(&puChild->cActive);
			}
			else if (puChild->eType.eAction == BAA_Inactive)
			{
				pasInputs->Add(&puChild->cInactive);
			}
		}
		paiBlockSizes->Add((int)uiNumElements);
		break;
	case BAA_Ordered:
		uiNumElements = puCriteria->cOrdered.mausBasicActionCriteria.NumElements();
		for (i = 0; i < uiNumElements; i++)
		{
			puChild = (UInputChordCriteria*)puCriteria->cOrdered.mausBasicActionCriteria.Get(i);
			FlattenCriteria(puChild, pasInputs, paiBlockSizes);
		}
		break;
	case BAA_Unknown:
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if some input could satisfy both.  Two generic sources are only
// known to overlap when they are the same generic.
//
//////////////////////////////////////////////////////////////////////////
bool CriteriaOverlap(CActionInputChordCriteria* psA, CActionInputChordCriteria* psB)
{
	CInputSourceEvaluator*			pcA;
	CInputSourceEvaluator*			pcB;
	CInputSourceEvaluator*			pcSpecific;
	CInputSourceEvaluator*			pcGeneric;
	SInputConfigSpecificSource*		psSpecific;

	if (psA->eAction != psB->eAction)
	{
		return false;
	}

	pcA = psA->mpcEvaluator;
	pcB = psB->mpcEvaluator;
	if ((pcA->GetEvaluatorType() == IET_AnyToggleSource) || (pcB->GetEvaluatorType() == IET_AnyToggleSource))
	{
		return true;
	}

	if ((pcA->GetEvaluatorType() == IET_SpecificSource) && (pcB->GetEvaluatorType() == IET_SpecificSource))
	{
		return (pcA->GetDeviceSource()->pcDevice == pcB->GetDeviceSource()->pcDevice) && (pcA->GetDeviceSource()->pcSource == pcB->GetDeviceSource()->pcSource);
	}

	if ((pcA->GetEvaluatorType() == IET_GenericSource) && (pcB->GetEvaluatorType() == IET_GenericSource))
	{
		return pcA->GetDeviceGeneric()->pcGeneric == pcB->GetDeviceGeneric()->pcGeneric;
	}

	if (pcA->GetEvaluatorType() == IET_SpecificSource)
	{
		pcSpecific = pcA;
		pcGeneric = pcB;
	}
	else
	{
		pcSpecific = pcB;
		pcGeneric = pcA;
	}
	psSpecific = pcSpecific->GetDeviceSource();
	return psSpecific->pcSource->GetGenerics()->Contains(&pcGeneric->GetDeviceGeneric()->pcGeneric);
}


//////////////////////////////////////////////////////////////////////////
//
// The inputs being matched, as flattened criteria, and where each chord
// is in them.  A block's used inputs are bits of its mask.
//
//////////////////////////////////////////////////////////////////////////
struct SChordOverlapSide
{
	CArrayTemplate<CActionInputChordCriteria>	asInputs;
	CArraySize									aiBlockSizes;
};


//////////////////////////////////////////////////////////////////////////
//
// Tries every pairing of the next input of A's current block with the
// next of B's, each block's inputs being in any order.  uiBlockStartA is
// the first input of A's block uiBlockA, uiUsedA which of its inputs have
// been matched so far.
//
//////////////////////////////////////////////////////////////////////////
bool BlocksOverlap(SChordOverlapSide* psA, size uiBlockA, size uiBlockStartA, uint32 uiUsedA, SChordOverlapSide* psB, size uiBlockB, size uiBlockStartB, uint32 uiUsedB)
{
	size	uiSizeA;
	size	uiSizeB;
	size	i;
	size	j;
	uint32	uiNextUsedA;
	uint32	uiNextUsedB;
	size	uiNextBlockA;
	size	uiNextBlockStartA;
	size	uiNextBlockB;
	size	uiNextBlockStartB;

	if (uiBlockA == psA->aiBlockSizes.NumElements())
	{
		return uiBlockB == psB->aiBlockSizes.NumElements();
	}
	if (uiBlockB == psB->aiBlockSizes.NumElements())
	{
		return false;
	}

	uiSizeA = (size)psA->aiBlockSizes.GetValue(uiBlockA);
	uiSizeB = (size)psB->aiBlockSizes.GetValue(uiBlockB);
	for (i = 0; i < uiSizeA; i++)
	{
		if (uiUsedA & (1 << i))
		{
			continue;
		}
		for (j = 0; j < uiSizeB; j++)
		{
			if (uiUsedB & (1 << j))
			{
				continue;
			}
			if (!CriteriaOverlap(psA->asInputs.Get(uiBlockStartA + i), psB->asInputs.Get(uiBlockStartB + j)))
			{
				continue;
			}

			uiNextUsedA = uiUsedA | (1 << i);
			uiNextBlockA = uiBlockA;
			uiNextBlockStartA = uiBlockStartA;
			if (uiNextUsedA == (uint32)((1 << uiSizeA) - 1))
			{
				uiNextUsedA = 0;
				uiNextBlockA++;
				uiNextBlockStartA += uiSizeA;
			}

			uiNextUsedB = uiUsedB | (1 << j);
			uiNextBlockB = uiBlockB;
			uiNextBlockStartB = uiBlockStartB;
			if (uiNextUsedB == (uint32)((1 << uiSizeB) - 1))
			{
				uiNextUsedB = 0;
				uiNextBlockB++;
				uiNextBlockStartB += uiSizeB;
			}

			if (BlocksOverlap(psA, uiNextBlockA, uiNextBlockStartA, uiNextUsedA, psB, uiNextBlockB, uiNextBlockStartB, uiNextUsedB))
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// True if some inputs would match all of this and all of pcOther, so
// both would be called for them.
//
//////////////////////////////////////////////////////////////////////////
bool CInputChord::MatchesSameAs(CInputChord* pcOther)
{
	SChordOverlapSide	sThis;
	SChordOverlapSide	sOther;
	bool				bResult;

	if (miTotalCriteria != pcOther->miTotalCriteria)
	{
		return false;
	}

	sThis.asInputs.Init();
	sThis.aiBlockSizes.Init();
	sOther.asInputs.Init();
	sOther.aiBlockSizes.Init();

	FlattenCriteria(&muActionCriteria, &sThis.asInputs, &sThis.aiBlockSizes);
	FlattenCriteria(&pcOther->muActionCriteria, &sOther.asInputs, &sOther.aiBlockSizes);
	bResult = BlocksOverlap(&sThis, 0, 0, 0, &sOther, 0, 0, 0);

	sOther.aiBlockSizes.Kill();
	sOther.asInputs.Kill();
	sThis.aiBlockSizes.Kill();
	sThis.asInputs.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CopyActionCriteria(UInputChordCriteria* puDest, UInputChordCriteria* puSource, CInputDeviceCopyContext* pcContext)
{
	size					i;
	UInputChordCriteria*	puLoopSource;
	UInputChordCriteria*	puLoopDest;
	size					uiNumElements;

	if (puSource->eType.eAction == BAA_Active)
	{
		puDest->cActive.Init((CInputSourceEvaluator*)pcContext->mmppEvaluators.Get(puSource->cActive.mpcEvaluator));
	}
	else if (puSource->eType.eAction == BAA_Inactive)
	{
		puDest->cInactive.Init((CInputSourceEvaluator*)pcContext->mmppEvaluators.Get(puSource->cInactive.mpcEvaluator));
	}
	else if (puSource->eType.eAction == BAA_Group)
	{
		puDest->cGroup.Init();
		uiNumElements = puSource->cGroup.mausBasicActionCriteria.NumElements();
		for (i = 0; i < uiNumElements; i++)
		{
			puLoopSource = (UInputChordCriteria*)puSource->cGroup.mausBasicActionCriteria.Get(i);
			puLoopDest = (UInputChordCriteria*)puDest->cGroup.mausBasicActionCriteria.Add();
			CopyActionCriteria(puLoopDest, puLoopSource, pcContext);
		}
	}
	else if (puSource->eType.eAction == BAA_Ordered)
	{
		puDest->cOrdered.Init();
		uiNumElements = puSource->cGroup.mausBasicActionCriteria.NumElements();
		for (i = 0; i < uiNumElements; i++)
		{
			puLoopSource = (UInputChordCriteria*)puDest->cOrdered.mausBasicActionCriteria.Get(i);
			puLoopDest = (UInputChordCriteria*)puDest->cOrdered.mausBasicActionCriteria.Add();
			CopyActionCriteria(puLoopDest, puLoopSource, pcContext);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChord::Copy(CInputChord* pcSource, CInputDeviceCopyContext* pcContext, CInputChords* pcInputChords)
{
	CInputChordDesc*	pcChordDesc;

	pcChordDesc = (CInputChordDesc*)pcContext->mmppChords.Get(mpcDesc);
	Init(NULL, pcInputChords, pcChordDesc);

	mpcAction = pcSource->mpcAction;
	CopyActionCriteria(&muActionCriteria, &pcSource->muActionCriteria, pcContext);

	//The chords copied from were already checked against each other.
	Calculate();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CActiveInputChordCriteria* CInputChord::AsActive(CInputSourceEvaluator* pcAction)
{
	if (muActionCriteria.eType.eAction == BAA_Unknown)
	{
		muActionCriteria.cActive.Init(pcAction);
		return &muActionCriteria.cActive;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInactiveInputChordCriteria* CInputChord::AsInactive(CInputSourceEvaluator* pcAction)
{
	if (muActionCriteria.eType.eAction == BAA_Unknown)
	{
		muActionCriteria.cInactive.Init(pcAction);
		return &muActionCriteria.cInactive;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGroupInputChordCriteria* CInputChord::AsGroup(void)
{
	if (muActionCriteria.eType.eAction == BAA_Unknown)
	{
		muActionCriteria.cGroup.Init();
		return &muActionCriteria.cGroup;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
COrderedInputChordCriteria* CInputChord::AsOrdered(void)
{
	if (muActionCriteria.eType.eAction == BAA_Unknown)
	{
		muActionCriteria.cOrdered.Init();
		return &muActionCriteria.cOrdered;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size ActionCriteriaSize(UInputChordCriteria* puCriteria)
{
	switch (puCriteria->eType.eAction)
	{
	case BAA_Active:
		return puCriteria->cActive.Size();
	case BAA_Inactive:
		return puCriteria->cInactive.Size();
	case BAA_Group:
		return puCriteria->cGroup.Size();
	case BAA_Ordered:
		return puCriteria->cOrdered.Size();
	default:
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
UInputChordCriteria* FindFirstActionCriteria(UInputChordCriteria* puCriteria)
{
	size					i;
	UInputChordCriteria*	puIterCriteria;
	UInputChordCriteria*	puFound;
	size					uiNumElements;

	switch (puCriteria->eType.eAction)
	{
	case BAA_Unknown:
		break;
	case BAA_Active:
		return puCriteria;
	case BAA_Inactive:
		return puCriteria;
	case BAA_Group:
		return puCriteria;
	case BAA_Ordered:
		uiNumElements = puCriteria->cGroup.mausBasicActionCriteria.NumElements();
		for (i = 0; i < uiNumElements; i++)
		{
			puIterCriteria = (UInputChordCriteria*)puCriteria->cGroup.mausBasicActionCriteria.Get(i);
			puFound = FindFirstActionCriteria(puIterCriteria);
			if (puFound)
			{
				return puFound;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChord::CalculatePotentialStart(void)
{
	UInputChordCriteria*	puCriteria;

	puCriteria = FindFirstActionCriteria(&muActionCriteria);
	if (puCriteria)
	{
		if (puCriteria->eType.eAction == BAA_Active)
		{
			masStartDeviceValue.Resize(puCriteria->cActive.Size());
			puCriteria->cActive.ToArray(&masStartDeviceValue);
		}
		else if (puCriteria->eType.eAction == BAA_Inactive)
		{
			masStartDeviceValue.Resize(puCriteria->cInactive.Size());
			puCriteria->cInactive.ToArray(&masStartDeviceValue);
		}
		else if (puCriteria->eType.eAction == BAA_Group)
		{
			masStartDeviceValue.Resize(puCriteria->cGroup.Size());
			puCriteria->cGroup.ToArray(&masStartDeviceValue);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size FindLongestAction(UInputChordCriteria* puCriteria)
{
	size					i;
	UInputChordCriteria*	puIterCriteria;
	size					iLongest;
	size					iCurrent;
	size					uiNumElements;

	iLongest = MAX_UINT;
	switch (puCriteria->eType.eAction)
	{
	case BAA_Unknown:
		break;
	case BAA_Active:
		return puCriteria->cActive.Size();
	case BAA_Inactive:
		return puCriteria->cInactive.Size();
	case BAA_Group:
		return puCriteria->cGroup.Size();
	case BAA_Ordered:
		uiNumElements = puCriteria->cGroup.mausBasicActionCriteria.NumElements();
		for (i = 0; i < uiNumElements; i++)
		{
			puIterCriteria = (UInputChordCriteria*)puCriteria->cGroup.mausBasicActionCriteria.Get(i);
			iCurrent = FindLongestAction(puIterCriteria);
			if ((iCurrent > iLongest) || (iLongest == MAX_UINT))
			{
				iLongest = iCurrent;
			}
		}
	}
	return iLongest;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChord::CalculateLongestAction(void)
{
	size	iLongest;

	iLongest = FindLongestAction(&muActionCriteria);
	masScratchPadDeviceValue.Allocate(iLongest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChord::CalculateTotalCriteria(void)
{
	miTotalCriteria = ActionCriteriaSize(&muActionCriteria);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMatchResult CInputChord::Match(CArrayInputDeviceTimeValue* pcHistory)
{
	size			iCount;
	size			iStartIndex;
	SMatchResult	sResult;
	size			uiNumElements;

	miUpdateCriteria = 0;


	iStartIndex = FindMatchingStart(pcHistory, 0);
	if (iStartIndex == MAX_UINT)
	{
		sResult.iIndex = MAX_UINT;
		sResult.iLength = 0;
		return sResult;
	}

	iCount = MatchActionEvent(pcHistory, iStartIndex);

	uiNumElements = pcHistory->NumElements();
	if ((uiNumElements < iStartIndex) || (iCount < uiNumElements - iStartIndex))
	{
		sResult.iIndex = MAX_UINT;
		sResult.iLength = 0;
		return sResult;
	}

	sResult.iIndex = iStartIndex;
	sResult.iLength = iCount;
	return sResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CInputChord::FindMatchingStart(CArrayInputDeviceTimeValue* pcHistory, size iStartPos)
{
	size	i;
	bool	bMatch;
	size	uiNumElements;

	uiNumElements = pcHistory->NumElements();
	for (i = iStartPos; i < uiNumElements; i++)
	{
		bMatch = MatchesGroupEvent(&masStartDeviceValue, pcHistory, i);
		if (bMatch)
		{
			return i;
		}
	}
	return MAX_UINT;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size MatchActionEvent(UInputChordCriteria* puCriteria, CArrayInputDeviceTimeValue* pcHistory, size iStartPos, CArrayActionInputChordCriteria* pasScratchPadEvent)
{
	size					i;
	UInputChordCriteria*	puIterCriteria;
	size					iCount;
	size					iTotalCount;
	size					uiNumElements;

	switch (puCriteria->eType.eAction)
	{
	case BAA_Unknown:
		break;
	case BAA_Active:
		puCriteria->cActive.ToArray(pasScratchPadEvent);
		return MatchesGroupEvent(pasScratchPadEvent, pcHistory, iStartPos);
	case BAA_Inactive:
		puCriteria->cInactive.ToArray(pasScratchPadEvent);
		return MatchesGroupEvent(pasScratchPadEvent, pcHistory, iStartPos);
	case BAA_Group:
		puCriteria->cGroup.ToArray(pasScratchPadEvent);
		return MatchesGroupEvent(pasScratchPadEvent, pcHistory, iStartPos);
	case BAA_Ordered:
		iTotalCount = 0;
		uiNumElements = puCriteria->cGroup.mausBasicActionCriteria.NumElements();
		for (i = 0; i < uiNumElements; i++)
		{
			puIterCriteria = (UInputChordCriteria*)puCriteria->cGroup.mausBasicActionCriteria.Get(i);
			iCount = MatchActionEvent(puIterCriteria, pcHistory, iStartPos + iTotalCount, pasScratchPadEvent);
			iTotalCount += iCount;

			//A child that didn't match all of itself (it ran out of history or something else came) is where the match ends.
			if (iCount < ActionCriteriaSize(puIterCriteria))
			{
				return iTotalCount;
			}
		}
		return iTotalCount;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CInputChord::MatchActionEvent(CArrayInputDeviceTimeValue* pcHistory, size iStartPos)
{
	return ::MatchActionEvent(&muActionCriteria, pcHistory, iStartPos, &masScratchPadDeviceValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChord::ToString(CChars* psz)
{
	psz->Append(" --- CInputChord ---\n");
	
	psz->Append("Total Criteria: ");
	psz->Append(miTotalCriteria);
	psz->AppendNewLine();

	psz->Append("Action: ");
	psz->AppendPointer(mpcAction);
	psz->AppendNewLine();

	psz->Append(" - UInputChordCriteria : muActionCriteria -\n");
	ToStringActionCriteria(&muActionCriteria, psz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputSourceEvaluator* CInputChord::AddEvaluatorSpecificSource(CInputVirtualDeviceSource* pcSource, EInputChordType eType)
{
	CInputSourceEvaluator*	pcEvaluator;

	pcEvaluator = mpcInputChords->AddEvaluator();
	pcEvaluator->SetSpecificSource(pcSource, eType);
	return pcEvaluator;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputSourceEvaluator* CInputChord::AddEvaluatorSpecificSource(CInputDevice* pcDevice, CInputSourceDesc* pcSource, EInputChordType eType)
{
	CInputSourceEvaluator*	pcEvaluator;

	pcEvaluator = mpcInputChords->AddEvaluator();
	pcEvaluator->SetSpecificSource(pcDevice, pcSource, eType);
	return pcEvaluator;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputSourceEvaluator* CInputChord::AddEvaluatorGenericSource(CInputCategoryGeneric* pcGeneric, EInputChordType eType)
{
	CInputSourceEvaluator*	pcEvaluator;

	pcEvaluator = mpcInputChords->AddEvaluator();
	pcEvaluator->SetGenericSource(pcGeneric, eType);
	return pcEvaluator;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputSourceEvaluator* CInputChord::AddEvaluatorAnyToggleSource(void)
{
	CInputSourceEvaluator*	pcEvaluator;

	pcEvaluator = mpcInputChords->AddEvaluator();
	pcEvaluator->SetAnyToggleSource();
	return pcEvaluator;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChord::Call(void* pvContext) 
{
	mpcAction->Call(pvContext);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputChordDesc* CInputChord::GetDesc(void) { return mpcDesc; }
UInputChordCriteria* CInputChord::GetActionCriteria(void) { return &muActionCriteria; }
CAction* CInputChord::GetAction(void) { return mpcAction; }
size CInputChord::GetTotalCriteria(void) { return miTotalCriteria; }
size CInputChord::GetUpdateCriteria(void) { return miUpdateCriteria; }
size CInputChord::GetMatchedCriteria(void) { return miMatchedCriteria; }

void CInputChord::SetMatchedCriteria(size iMatchedCriteria) { miMatchedCriteria = iMatchedCriteria; }

