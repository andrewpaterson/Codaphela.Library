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
	muiPartialEvictionAge = muiEvictionAge;
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
	muiPartialEvictionAge = muiEvictionAge;

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
		sValue.bCarried = false;
		AddEvent(NULL, &sValue);
		return;
	}

	//Letting go of a carried input ends its carrying; the release is matched like any other input.
	if (!IsDown(psEvent))
	{
		RemoveCarried(psEvent);
	}

	if (macHistory.NumElements() == MAX_INPUT_HISTORY_EVENTS)
	{
		if (mpcPending && (muiPendingStart == NumCarried()))
		{
			CallPending();
		}
		else
		{
			Release(NumCarried() + 1);
		}
	}

	psHistory = macHistory.Add();
	psHistory->fValue = psEvent->fValue;
	psHistory->pcDevice = psEvent->pcDevice;
	psHistory->pcSource = psEvent->pcSource;
	psHistory->uiTime = muiTime;
	psHistory->bCarried = false;

	Match(psEvent);
}


//////////////////////////////////////////////////////////////////////////
//
// A key or button that is pressed.
//
//////////////////////////////////////////////////////////////////////////
bool CInputChords::IsDown(SInputDeviceValue* psValue)
{
	return (psValue->pcSource->GetType() == ISET_State) && (psValue->fValue >= INPUT_CHORDS_DOWN);
}


//////////////////////////////////////////////////////////////////////////
//
// True if the input at uiIndex is pressed and isn't let go later in the
// history.
//
//////////////////////////////////////////////////////////////////////////
bool CInputChords::IsStillDown(size uiIndex)
{
	SInputDeviceTimeValue*	psValue;
	SInputDeviceTimeValue*	psLater;
	size					i;
	size					uiNumElements;

	psValue = macHistory.Get(uiIndex);
	if (!IsDown(psValue))
	{
		return false;
	}

	uiNumElements = macHistory.NumElements();
	for (i = uiIndex + 1; i < uiNumElements; i++)
	{
		psLater = macHistory.Get(i);
		if ((psLater->pcDevice == psValue->pcDevice) && (psLater->pcSource == psValue->pcSource) && !IsDown(psLater))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Of the inputs after the carried ones.
//
//////////////////////////////////////////////////////////////////////////
bool CInputChords::IsAnyStillDown(void)
{
	size	i;
	size	uiNumElements;

	uiNumElements = macHistory.NumElements();
	for (i = NumCarried(); i < uiNumElements; i++)
	{
		if (IsStillDown(i))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The carried inputs are always the first of the history.
//
//////////////////////////////////////////////////////////////////////////
size CInputChords::NumCarried(void)
{
	size	i;
	size	uiNumElements;

	uiNumElements = macHistory.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		if (!macHistory.Get(i)->bCarried)
		{
			return i;
		}
	}
	return uiNumElements;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the input at uiIndex is still down and could start a chord, so
// it is carried once it has been let through or used: like shift held
// while typing for shift+tab after.
//
//////////////////////////////////////////////////////////////////////////
bool CInputChords::IsCarriable(size uiIndex)
{
	SSetIterator	sIter;
	CInputChord*	pcChord;
	bool			bExists;

	if (!IsStillDown(uiIndex))
	{
		return false;
	}

	bExists = mlcChords.StartIteration(&sIter, &pcChord);
	while (bExists)
	{
		if (pcChord->IsStart(macHistory.Get(uiIndex)))
		{
			return true;
		}
		bExists = mlcChords.Iterate(&sIter, &pcChord);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Removes the carried input of psValue's source, if it has one.
//
//////////////////////////////////////////////////////////////////////////
void CInputChords::RemoveCarried(SInputDeviceValue* psValue)
{
	SInputDeviceTimeValue*	psHistory;
	size					i;
	size					uiNumElements;

	uiNumElements = macHistory.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		psHistory = macHistory.Get(i);
		if (psHistory->bCarried && (psHistory->pcDevice == psValue->pcDevice) && (psHistory->pcSource == psValue->pcSource))
		{
			macHistory.RemoveAt(i, true);
			if (mpcPending && (i < muiPendingStart))
			{
				muiPendingStart--;
			}
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The inputs a chord used.  Those still down that could start a chord are
// carried, like ctrl held for ctrl+z, z.  The rest are used up.
//
//////////////////////////////////////////////////////////////////////////
void CInputChords::Consume(size uiStart, size uiLength)
{
	size	i;
	size	uiIndex;

	uiIndex = uiStart;
	for (i = 0; i < uiLength; i++)
	{
		if (IsCarriable(uiIndex))
		{
			macHistory.Get(uiIndex)->bCarried = true;
			uiIndex++;
		}
		else
		{
			macHistory.RemoveAt(uiIndex, true);
		}
	}
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
	size					uiKept;
	uint32					uiEvictionAge;

	sMatch.iIndex = MAX_UINT;
	sMatch.iLength = 0;
	sMatch.bTotalMatch = false;
	sMatch.bPotentialMatch = false;
	iLowestIndex = MAX_UINT;
	bPendingGoesOn = false;
	muiPartialEvictionAge = 0;
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
			uiEvictionAge = (pcChord->GetEvictionAge() != 0) ? pcChord->GetEvictionAge() : muiEvictionAge;
			if (uiEvictionAge > muiPartialEvictionAge)
			{
				muiPartialEvictionAge = uiEvictionAge;
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
		Match(psValue);
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
		uiKept = Release(iLowestIndex);
		mpcPending = pcComplete;
		muiPendingStart = sMatch.iIndex - (iLowestIndex - uiKept);
		muiPendingLength = sMatch.iLength;
	}
	else if (sMatch.bTotalMatch)
	{
		//The inputs before the chord are let through before it.  The chord's are used up.  A pending chord was the start of it.
		mpcPending = NULL;
		uiKept = Release(sMatch.iIndex);
		bExists = mlcChords.StartIteration(&sIter, &pcChord);
		while (bExists)
		{
			if ((pcChord->GetMatchedCriteria() == sMatch.iLength) &&
				(pcChord->GetTotalCriteria() == sMatch.iLength))
			{
				sChordInputEvent.pcValue = psValue;
				sChordInputEvent.pcChord = pcChord;
				pcChord->Call(&sChordInputEvent);
				AddEvent(pcChord, macHistory.Get(uiKept + sMatch.iLength - 1));
			}
			bExists = mlcChords.Iterate(&sIter, &pcChord);
		}
		Consume(uiKept, sMatch.iLength);
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
	size				uiKept;

	uiKept = Release(muiPendingStart);
	pcChord = mpcPending;
	mpcPending = NULL;

	sChordInputEvent.pcValue = NULL;
	sChordInputEvent.pcChord = pcChord;
	pcChord->Call(&sChordInputEvent);
	AddEvent(pcChord, macHistory.Get(uiKept + muiPendingLength - 1));
	Consume(uiKept, muiPendingLength);
}


//////////////////////////////////////////////////////////////////////////
//
// Lets through the inputs in the first uiNumHistory of the history.  They
// are all before the pending chord, if there is one.  Carried inputs have
// been let through or used already so they are kept, at the front, as are
// those let through now that are still down and could start a chord.
// Returns how many were kept.
//
//////////////////////////////////////////////////////////////////////////
size CInputChords::Release(size uiNumHistory)
{
	size					i;
	size					uiKept;
	size					uiRemoved;
	SInputDeviceTimeValue*	psValue;

	uiKept = 0;
	uiRemoved = 0;
	for (i = 0; i < uiNumHistory; i++)
	{
		psValue = macHistory.Get(uiKept);
		if (!psValue->bCarried)
		{
			AddEvent(NULL, psValue);
			if (IsCarriable(uiKept))
			{
				psValue->bCarried = true;
			}
		}

		if (psValue->bCarried)
		{
			uiKept++;
		}
		else
		{
			macHistory.RemoveAt(uiKept, true);
			uiRemoved++;
		}
	}

	if (mpcPending)
	{
		muiPendingStart -= uiRemoved;
	}
	return uiKept;
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
// An input that is still down never ages, so a partial chord is only too
// old to finish once everything after the carried inputs has been let go
// for longer than the eviction age of the chords that could still match
// it (worked out by Match).  If the oldest of them is the start of a
// pending chord the pending chord is called, otherwise it is let through.
// What is left is matched again, which lets through the rest unless a
// later chord could still match.  The carried inputs don't age.
//
//////////////////////////////////////////////////////////////////////////
void CInputChords::Evict(void)
{
	SInputDeviceTimeValue*	psNewest;
	size					uiNumCarried;

	for (;;)
	{
		uiNumCarried = NumCarried();
		if (uiNumCarried == macHistory.NumElements())
		{
			break;
		}
		if (IsAnyStillDown())
		{
			break;
		}
		psNewest = macHistory.Tail();
		if (muiTime - psNewest->uiTime <= muiPartialEvictionAge)
		{
			break;
		}
		if (mpcPending && (muiPendingStart == uiNumCarried))
		{
			CallPending();
		}
		else
		{
			Release(uiNumCarried + 1);
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

