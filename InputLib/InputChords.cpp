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
#include "BaseLib/DataTypes.h"
#include "StandardLib/Unknown.h"
#include "InputChords.h"
#include "InputSourceDesc.h"
#include "InputVirtualDevice.h"
#include "InputDeviceValue.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChords::Init(void)
{
	mlcChords.Init(false);
	mscEvaluators.Init(false);
	macHistory.Init();
	muiEvictionAge = 300;  // Milliseconds.
	muiTime = 0;
	masEvents.Init();
	mpcPending = NULL;
	muiPendingStart = 0;
	muiPendingLength = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChords::Kill(void)
{
	masEvents.Kill();
	macHistory.Kill();
	mscEvaluators.Kill();
	mlcChords.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChords::Copy(CInputChords* pcSource, CInputDeviceCopyContext* pcContext)
{
	//This assumes Init has already been called.

	SSetIterator			sActionIter;
	CInputChord*			pcSourceInputChord;
	CInputChord*			pcDestInputChord;
	SSetIterator			sEvaluatorIter;
	CInputSourceEvaluator*	pcSourceInputSourceEvaluator;
	CInputSourceEvaluator*	pcDestInputSourceEvaluator;
	bool					bExists;

	muiEvictionAge = pcSource->muiEvictionAge;

	bExists = pcSource->mscEvaluators.StartIteration(&sEvaluatorIter, &pcSourceInputSourceEvaluator);
	while (bExists)
	{
		pcDestInputSourceEvaluator = mscEvaluators.Add();
		pcDestInputSourceEvaluator->Copy(pcSourceInputSourceEvaluator, NULL);  //NULL used to be pcOverwriteDevice
		pcContext->mmppEvaluators.Put(pcSourceInputSourceEvaluator, pcDestInputSourceEvaluator);
		bExists = pcSource->mscEvaluators.Iterate(&sEvaluatorIter, &pcSourceInputSourceEvaluator);
	}

	bExists = pcSource->mlcChords.StartIteration(&sActionIter, &pcSourceInputChord);
	while (bExists)
	{
		pcDestInputChord = mlcChords.Add();
		pcContext->mmppChords.Put(pcSourceInputChord, pcDestInputChord);
		pcDestInputChord->Copy(pcSourceInputChord, pcContext, this);
		bExists = pcSource->mlcChords.Iterate(&sActionIter, &pcSourceInputChord);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChords::InputEvent(CUnknown* pcSource, void* pvContext)
{
	CInputDeviceValue*		psEvent;
	SInputDeviceTimeValue*	psHistory;
	SInputDeviceTimeValue	sValue;

	psEvent = (CInputDeviceValue*)pvContext;

	if ((psEvent->pcSource->GetType() == ISET_Delta) && !IsChordSource(psEvent))
	{
		sValue.fValue = psEvent->fValue;
		sValue.pcDevice = psEvent->pcDevice;
		sValue.pcSource = psEvent->pcSource;
		sValue.uiTime = muiTime;
		AddEvent(NULL, &sValue);
		return;
	}

	if (macHistory.NumElements() == MAX_INPUT_HISTORY_EVENTS)
	{
		if (mpcPending && (muiPendingStart == 0))
		{
			CallPending();
		}
		else
		{
			Release(1);
		}
	}

	psHistory = macHistory.Add();
	psHistory->fValue = psEvent->fValue;
	psHistory->pcDevice = psEvent->pcDevice;
	psHistory->pcSource = psEvent->pcSource;
	psHistory->uiTime = muiTime;

	Match(psEvent);
}


//////////////////////////////////////////////////////////////////////////
//
// A chord matches the history from its start to the newest input or not
// at all, so a chord's matched criteria are the number of inputs it
// covers and the chords with the same number start at the same input.
//
// Calls the chords the history matches all of, using up their inputs,
// unless a longer chord matching the same inputs could still go on; then
// the complete chord is pending until the longer one completes (and is
// called instead), stops matching or is evicted.  Lets through the inputs
// before the earliest chord that could still match, or all of them if
// none could.  psValue is the input just added, NULL when matching what
// is left after an eviction or a pending chord.
//
//////////////////////////////////////////////////////////////////////////
void CInputChords::Match(CInputDeviceValue* psValue)
{
	SSetIterator			sIter;
	CInputChord*			pcChord;
	CInputChord*			pcComplete;
	SMatchResult			sResult;
	SMatchingAction			sMatch;
	size					iLowestIndex;
	SChordInputEvent		sChordInputEvent;
	bool					bExists;
	bool					bLonger;
	bool					bPendingGoesOn;

	sMatch.iIndex = MAX_UINT;
	sMatch.iLength = 0;
	sMatch.bTotalMatch = false;
	sMatch.bPotentialMatch = false;
	iLowestIndex = MAX_UINT;
	bPendingGoesOn = false;
	bExists = mlcChords.StartIteration(&sIter, &pcChord);
	while (bExists)
	{
		sResult = pcChord->Match(&macHistory);
		pcChord->SetMatchedCriteria(sResult.iLength);
		if (sResult.iIndex != MAX_UINT)
		{
			if (sResult.iIndex < iLowestIndex)
			{
				iLowestIndex = sResult.iIndex;
			}
			if (mpcPending && (sResult.iIndex == muiPendingStart))
			{
				bPendingGoesOn = true;
			}

			if (sResult.iLength > sMatch.iLength)
			{
				sMatch.iIndex = sResult.iIndex;
				sMatch.bPotentialMatch = true;
				sMatch.iLength = sResult.iLength;
			}
			else if (sResult.iLength == sMatch.iLength)
			{
				if (sResult.iIndex < sMatch.iIndex)
				{
					sMatch.iIndex = sResult.iIndex;
					sMatch.bPotentialMatch = true;
					sMatch.iLength = sResult.iLength;
				}
			}
		}
		bExists = mlcChords.Iterate(&sIter, &pcChord);
	}

	//Nothing longer can follow the pending chord now; it is called and what came after it is matched again.
	if (mpcPending && !bPendingGoesOn)
	{
		CallPending();
		Match(NULL);
		return;
	}

	if (!sMatch.bPotentialMatch)
	{
		Release(macHistory.NumElements());
		return;
	}

	pcComplete = NULL;
	bLonger = false;
	bExists = mlcChords.StartIteration(&sIter, &pcChord);
	while (bExists)
	{
		if (pcChord->GetMatchedCriteria() == sMatch.iLength)
		{
			if (pcChord->GetTotalCriteria() == sMatch.iLength)
			{
				if (!pcComplete)
				{
					pcComplete = pcChord;
				}
				sMatch.bTotalMatch = true;
			}
			else
			{
				bLonger = true;
			}
		}
		bExists = mlcChords.Iterate(&sIter, &pcChord);
	}

	if (sMatch.bTotalMatch && bLonger)
	{
		Release(iLowestIndex);
		mpcPending = pcComplete;
		muiPendingStart = sMatch.iIndex - iLowestIndex;
		muiPendingLength = sMatch.iLength;
	}
	else if (sMatch.bTotalMatch)
	{
		//The inputs before the chord are let through before it.  The chord's are used up.  A pending chord was the start of it.
		mpcPending = NULL;
		Release(sMatch.iIndex);
		bExists = mlcChords.StartIteration(&sIter, &pcChord);
		while (bExists)
		{
			if ((pcChord->GetMatchedCriteria() == sMatch.iLength) &&
				(pcChord->GetTotalCriteria() == sMatch.iLength))
			{
				sChordInputEvent.pcValue = psValue;
				sChordInputEvent.pcChord = pcChord;
				pcChord->Call(&sChordInputEvent);
				AddEvent(pcChord, macHistory.Get(sMatch.iLength - 1));
			}
			bExists = mlcChords.Iterate(&sIter, &pcChord);
		}
		macHistory.RemoveRange(0, sMatch.iLength, true);
	}
	else
	{
		Release(iLowestIndex);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Lets through the inputs before the pending chord, then calls it, using
// up its inputs.
//
//////////////////////////////////////////////////////////////////////////
void CInputChords::CallPending(void)
{
	CInputChord*		pcChord;
	SChordInputEvent	sChordInputEvent;

	Release(muiPendingStart);
	pcChord = mpcPending;
	mpcPending = NULL;

	sChordInputEvent.pcValue = NULL;
	sChordInputEvent.pcChord = pcChord;
	pcChord->Call(&sChordInputEvent);
	AddEvent(pcChord, macHistory.Get(muiPendingLength - 1));
	macHistory.RemoveRange(0, muiPendingLength, true);
}


//////////////////////////////////////////////////////////////////////////
//
// Lets through the first uiNumHistory inputs of the history.  They are
// all before the pending chord, if there is one.
//
//////////////////////////////////////////////////////////////////////////
void CInputChords::Release(size uiNumHistory)
{
	size	i;

	for (i = 0; i < uiNumHistory; i++)
	{
		AddEvent(NULL, macHistory.Get(i));
	}
	macHistory.RemoveRange(0, uiNumHistory, true);
	if (mpcPending)
	{
		muiPendingStart -= uiNumHistory;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChords::AddEvent(CInputChord* pcChord, SInputDeviceTimeValue* psValue)
{
	SInputChordsEvent*	psEvent;

	psEvent = masEvents.Add();
	psEvent->pcChord = pcChord;
	memcpy(&psEvent->sValue, psValue, sizeof(SInputDeviceTimeValue));
}


//////////////////////////////////////////////////////////////////////////
//
// The partial chord the oldest held input starts is too old to finish.
// If the input is the start of a pending chord the pending chord is
// called, otherwise the input is let through.  What is left is matched
// again, which lets through the rest unless a later chord could still
// match.
//
//////////////////////////////////////////////////////////////////////////
void CInputChords::Evict(void)
{
	SInputDeviceTimeValue*	psOldest;

	while (macHistory.NumElements() > 0)
	{
		psOldest = macHistory.Get(0);
		if (muiTime - psOldest->uiTime <= muiEvictionAge)
		{
			break;
		}
		if (mpcPending && (muiPendingStart == 0))
		{
			CallPending();
		}
		else
		{
			Release(1);
		}
		Match(NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Called before the update's inputs.  What was let through by the last
// update is cleared.  Partial chords too old by now are let through
// before the new inputs can join them.
//
//////////////////////////////////////////////////////////////////////////
void CInputChords::BeginUpdate(uint32 uiTime)
{
	muiTime = uiTime;
	masEvents.ReInit();
	Evict();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChords::EndUpdate(void)
{
	Evict();
}


//////////////////////////////////////////////////////////////////////////
//
// True if any chord has an evaluator for the input's source.
//
//////////////////////////////////////////////////////////////////////////
bool CInputChords::IsChordSource(SInputDeviceValue* psValue)
{
	SSetIterator			sIter;
	CInputSourceEvaluator*	pcEvaluator;
	float					fValue;
	bool					bExists;

	bExists = mscEvaluators.StartIteration(&sIter, &pcEvaluator);
	while (bExists)
	{
		if (pcEvaluator->Evaluate(psValue, &fValue))
		{
			return true;
		}
		bExists = mscEvaluators.Iterate(&sIter, &pcEvaluator);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayInputChordsEvent* CInputChords::GetEvents(void) { return &masEvents; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputSourceEvaluator* CInputChords::AddEvaluator(void)
{
	CInputSourceEvaluator*	pcConfig;

	pcConfig = mscEvaluators.Add();
	pcConfig->Init();
	return pcConfig;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputChord* CInputChords::AddChord(CAction* pcAction, CInputChordDesc* pcDesc)
{
	CInputChord*	pcChord;

	pcChord = mlcChords.Add();
	pcChord->Init(pcAction, this, pcDesc);
	return pcChord;
}


//////////////////////////////////////////////////////////////////////////
//
// Another chord that some inputs would match all of as well as all of
// pcChord, or NULL if none.
//
//////////////////////////////////////////////////////////////////////////
CInputChord* CInputChords::FindMatchingChord(CInputChord* pcChord)
{
	SSetIterator	sIter;
	CInputChord*	pcOther;
	bool			bExists;

	bExists = mlcChords.StartIteration(&sIter, &pcOther);
	while (bExists)
	{
		if ((pcOther != pcChord) && pcChord->MatchesSameAs(pcOther))
		{
			return pcOther;
		}
		bExists = mlcChords.Iterate(&sIter, &pcOther);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Kills the chord.  Its evaluators aren't removed from the evaluators.
//
//////////////////////////////////////////////////////////////////////////
void CInputChords::RemoveChord(CInputChord* pcChord)
{
	if (mpcPending == pcChord)
	{
		mpcPending = NULL;
	}
	mlcChords.Remove(pcChord);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputChords::StartChordsIteration(SSetIterator* psIter, CInputChord** ppChord)
{
	return mlcChords.StartIteration(psIter, ppChord);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputChords::IterateChords(SSetIterator* psIter, CInputChord** ppChord)
{
	return mlcChords.Iterate(psIter, ppChord);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChords::ToString(CChars* psz)
{
	SSetIterator				sIter;
	CInputSourceEvaluator*		pcEvaluator;
	CInputChord*				pcChord;
	bool						bExists;

	psz->Append(" --- CInputChords ---\n");

	psz->Append("Eviction Age: ");
	psz->Append(muiEvictionAge);
	psz->AppendNewLine();

	psz->Append(" - CInputSourceEvaluator : mscEvaluators -\n");
	bExists = mscEvaluators.StartIteration(&sIter, &pcEvaluator);
	while (bExists)
	{
		pcEvaluator->ToString(psz);
		bExists = mscEvaluators.Iterate(&sIter, &pcEvaluator);
	}

	psz->Append(" - CInputChord : mlcChords -\n");
	bExists = mlcChords.StartIteration(&sIter, &pcChord);
	while (bExists)
	{
		pcChord->ToString(psz);
		bExists = mlcChords.Iterate(&sIter, &pcChord);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChords::DumpHistory(void)
{
	size					i;
	SInputDeviceTimeValue*	psValue;
	CChars					sz;
	size					uiNumElements;

	sz.Init();
	uiNumElements = macHistory.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		psValue = macHistory.Get(i);
		sz.Append(psValue->pcSource->GetFriendlyName());
		sz.Append("(");
		sz.Append(psValue->fValue, 0);
		sz.Append(") ");
	}
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}

