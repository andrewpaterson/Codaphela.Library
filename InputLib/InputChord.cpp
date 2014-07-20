/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "InputChords.h"
#include "InputChord.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int MatchesGroupEvent(CArrayActionInputChordCriteria* pasChordCriteria, CArrayInputDeviceTimeValue* pcHistory, int iStartPos)
{
	//int								iRemain;
	int								i;
	SInputDeviceTimeValue*			psHistory;
	int								j;
	CActionInputChordCriteria*		psChordCriteria;
	int								iCount;
	BOOL							abMatched[MAX_GROUP_SIZE];
	float							fValue;
	BOOL							bMatch;
	BOOL							bAnyMatch;
	int								iEnd;
	BOOL							bResult;

	memset(abMatched, 0, MAX_GROUP_SIZE * sizeof(BOOL));
	iCount = 0;
	iEnd = pasChordCriteria->NumElements() + iStartPos;
	if (iEnd > pcHistory->NumElements())
	{
		iEnd = pcHistory->NumElements();
	}

	for (i = iStartPos; i < iEnd; i++)
	{
		psHistory = pcHistory->Get(i);

		bAnyMatch = FALSE;
		for (j = 0; j < pasChordCriteria->NumElements(); j++)
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
						abMatched[j] = TRUE;
						iCount++;
						bAnyMatch = TRUE;
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
//
//////////////////////////////////////////////////////////////////////////
void CInputChord::Done(void)
{
	CalculateLongestAction();
	CalculateTotalCriteria();
	CalculatePotentialStart();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CopyActionCriteria(UInputChordCriteria* puDest, UInputChordCriteria* puSource, CInputDeviceCopyContext* pcContext)
{
	int						i;
	UInputChordCriteria*	puLoopSource;
	UInputChordCriteria*	puLoopDest;

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
		for (i = 0; i < puSource->cGroup.mausBasicActionCriteria.NumElements(); i++)
		{
			puLoopSource = (UInputChordCriteria*)puSource->cGroup.mausBasicActionCriteria.Get(i);
			puLoopDest = (UInputChordCriteria*)puDest->cGroup.mausBasicActionCriteria.Add();
			CopyActionCriteria(puLoopDest, puLoopSource, pcContext);
		}
	}
	else if (puSource->eType.eAction == BAA_Ordered)
	{
		puDest->cOrdered.Init();
		for (i = 0; i < puSource->cOrdered.mausBasicActionCriteria.NumElements(); i++)
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
	
	Done();
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
int ActionCriteriaSize(UInputChordCriteria* puCriteria)
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
	int						i;
	UInputChordCriteria*	puIterCriteria;
	UInputChordCriteria*	puFound;

	switch (puCriteria->eType.eAction)
	{
	case BAA_Active:
		return puCriteria;
	case BAA_Inactive:
		return puCriteria;
	case BAA_Group:
		return puCriteria;
	case BAA_Ordered:
		for (i = 0; i < puCriteria->cGroup.mausBasicActionCriteria.NumElements(); i++)
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
			masStartDeviceValue.GrowToNumElements(puCriteria->cActive.Size());
			puCriteria->cActive.ToArray(&masStartDeviceValue);
		}
		else if (puCriteria->eType.eAction == BAA_Inactive)
		{
			masStartDeviceValue.GrowToNumElements(puCriteria->cInactive.Size());
			puCriteria->cInactive.ToArray(&masStartDeviceValue);
		}
		else if (puCriteria->eType.eAction == BAA_Group)
		{
			masStartDeviceValue.GrowToNumElements(puCriteria->cGroup.Size());
			puCriteria->cGroup.ToArray(&masStartDeviceValue);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int FindLongestAction(UInputChordCriteria* puCriteria)
{
	int						i;
	UInputChordCriteria*	puIterCriteria;
	int						iLongest;
	int						iCurrent;

	iLongest = -1;
	switch (puCriteria->eType.eAction)
	{
	case BAA_Active:
		return puCriteria->cActive.Size();
	case BAA_Inactive:
		return puCriteria->cInactive.Size();
	case BAA_Group:
		return puCriteria->cGroup.Size();
	case BAA_Ordered:
		for (i = 0; i < puCriteria->cGroup.mausBasicActionCriteria.NumElements(); i++)
		{
			puIterCriteria = (UInputChordCriteria*)puCriteria->cGroup.mausBasicActionCriteria.Get(i);
			iCurrent = FindLongestAction(puIterCriteria);
			if (iCurrent > iLongest)
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
	int		iLongest;

	iLongest =  FindLongestAction(&muActionCriteria);
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
	int				iLess;
	int				iCount;
	int				iLongestCount;
	int				iStartIndex;
	int				iLongestIndex;
	SMatchResult	sResult;

	miUpdateCriteria = 0;

	iLess = masStartDeviceValue.NumElements();
	iLongestCount = 0;
	iLongestIndex = -1;

	iStartIndex = FindMatchingStart(pcHistory, 0);
	if (iStartIndex == -1)
	{
		sResult.iIndex = -1;
		sResult.iLength = 0;
		return sResult;
	}

	iCount = MatchActionEvent(pcHistory, iStartIndex);

	if (iCount < pcHistory->NumElements() - iStartIndex)
	{
		sResult.iIndex = -1;
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
int CInputChord::FindMatchingStart(CArrayInputDeviceTimeValue* pcHistory, int iStartPos)
{
	int						i;
	BOOL					bMatch;
	//int						iLess;

	//iLess = masStartDeviceValue.NumElements();
	for (i = iStartPos; i < pcHistory->NumElements(); i++)
	{
		bMatch = MatchesGroupEvent(&masStartDeviceValue, pcHistory, i);
		if (bMatch)
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int MatchActionEvent(UInputChordCriteria* puCriteria, CArrayInputDeviceTimeValue* pcHistory, int iStartPos, CArrayActionInputChordCriteria* pasScratchPadEvent)
{
	int						i;
	UInputChordCriteria*	puIterCriteria;
	int						iCount;
	int						iTotalCount;

	switch (puCriteria->eType.eAction)
	{
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
		for (i = 0; i < puCriteria->cGroup.mausBasicActionCriteria.NumElements(); i++)
		{
			puIterCriteria = (UInputChordCriteria*)puCriteria->cGroup.mausBasicActionCriteria.Get(i);
			iCount = MatchActionEvent(puIterCriteria, pcHistory, iStartPos + iTotalCount, pasScratchPadEvent);
			if (iCount == 0)
			{
				return iTotalCount;
			}
			iTotalCount += iCount;
		}
		return iTotalCount;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CInputChord::MatchActionEvent(CArrayInputDeviceTimeValue* pcHistory, int iStartPos)
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
int CInputChord::GetTotalCriteria(void) { return miTotalCriteria; }
int CInputChord::GetUpdateCriteria(void) { return miUpdateCriteria; }
int CInputChord::GetMatchedCriteria(void) { return miMatchedCriteria; }

void CInputChord::SetMatchedCriteria(int iMatchedCriteria) { miMatchedCriteria = iMatchedCriteria; }