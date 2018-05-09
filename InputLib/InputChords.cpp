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
	mlcChords.Init();
	mscEvaluators.Init();
	macHistory.Init();
	muiEvictionAge = 300;  // Milliseconds.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChords::Kill(void)
{
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

	muiEvictionAge = pcSource->muiEvictionAge;

	pcSourceInputSourceEvaluator = pcSource->mscEvaluators.StartIteration(&sEvaluatorIter);
	while (pcSourceInputSourceEvaluator)
	{
		pcDestInputSourceEvaluator = mscEvaluators.Add();
		pcDestInputSourceEvaluator->Copy(pcSourceInputSourceEvaluator, NULL);  //NULL used to be pcOverwriteDevice
		pcContext->mmppEvaluators.Put(pcSourceInputSourceEvaluator, pcDestInputSourceEvaluator);
		pcSourceInputSourceEvaluator = pcSource->mscEvaluators.Iterate(&sEvaluatorIter);
	}

	pcSourceInputChord = pcSource->mlcChords.StartIteration(&sActionIter);
	while (pcSourceInputChord)
	{
		pcDestInputChord = mlcChords.Add();
		pcContext->mmppChords.Put(pcSourceInputChord, pcDestInputChord);
		pcDestInputChord->Copy(pcSourceInputChord, pcContext, this);
		pcSourceInputChord = pcSource->mlcChords.Iterate(&sActionIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChords::InputEvent(CUnknown* pcSource, void* pvContext)
{
	CInputDeviceValue*		psEvent;
	SSetIterator			sIter;
	CInputChord*			pcChord;
	SInputDeviceTimeValue*	psHistory;
	SMatchResult			sResult;
	SMatchingAction			sMatch;
	int						iLowestIndex;
	SChordInputEvent		sChordInputEvent;
	CInputVirtualDevice*	pcVirtual;

	psEvent = (CInputDeviceValue*)pvContext;
	pcVirtual = (CInputVirtualDevice*)pcSource;

	if (macHistory.NumElements() == MAX_INPUT_HISTORY_EVENTS)
	{
		macHistory.RemoveAt(0, TRUE);
	}

	psHistory = macHistory.Add();
	psHistory->fValue = psEvent->fValue;
	psHistory->pcDevice = psEvent->pcDevice;
	psHistory->pcSource = psEvent->pcSource;
	psHistory->uiTime = 0;

	sMatch.iIndex = MAX_INT;
	sMatch.iLength = 0;
	sMatch.bTotalMatch = FALSE;
	sMatch.bPotentialMatch = FALSE;
	iLowestIndex = MAX_INT;
	pcChord = mlcChords.StartIteration(&sIter);
	while (pcChord)
	{
		//You need to look at this logic.  It's out of date.
		sResult = pcChord->Match(&macHistory);
		pcChord->SetMatchedCriteria(sResult.iLength);
		if (sResult.iIndex >= 0)
		{
			if (sResult.iIndex < iLowestIndex)
			{
				iLowestIndex = sResult.iIndex;
			}

			if (sResult.iLength > sMatch.iLength)
			{
				sMatch.iIndex = sResult.iIndex;
				sMatch.bPotentialMatch = TRUE;
				sMatch.iLength = sResult.iLength;
			}
			else if (sResult.iLength == sMatch.iLength)
			{
				if (sResult.iIndex < sMatch.iIndex)
				{
					sMatch.iIndex = sResult.iIndex;
					sMatch.bPotentialMatch = TRUE;
					sMatch.iLength = sResult.iLength;
				}
			}
		}
		pcChord = mlcChords.Iterate(&sIter);
	}

	pcChord = mlcChords.StartIteration(&sIter);
	while (pcChord)
	{
		if ((pcChord->GetMatchedCriteria() == sMatch.iLength) && (pcChord->GetTotalCriteria() == sMatch.iLength))
		{
			sChordInputEvent.pcValue = (CInputDeviceValue*)pvContext;
			sChordInputEvent.pcChord = pcChord;
			pcChord->Call(&sChordInputEvent);
			sMatch.bTotalMatch = TRUE;
		}
		pcChord = mlcChords.Iterate(&sIter);
	}

	if (sMatch.bPotentialMatch)
	{
		if (sMatch.bTotalMatch)
		{
			macHistory.RemoveRange(0, sMatch.iIndex + sMatch.iLength, TRUE);
		}
		else
		{
			macHistory.RemoveRange(0, iLowestIndex, TRUE);
		}
	}
	else
	{
		macHistory.ReInit();
	}
}


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
//
//////////////////////////////////////////////////////////////////////////
CInputChord* CInputChords::StartChordsIteration(SSetIterator* psIter)
{
	return mlcChords.StartIteration(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputChord* CInputChords::IterateChords(SSetIterator* psIter)
{
	return mlcChords.Iterate(psIter);
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

	psz->Append(" --- CInputChords ---\n");

	psz->Append("Eviction Age: ");
	psz->Append(muiEvictionAge);
	psz->AppendNewLine();

	psz->Append(" - CInputSourceEvaluator : mscEvaluators -\n");
	pcEvaluator = mscEvaluators.StartIteration(&sIter);
	while (pcEvaluator)
	{
		pcEvaluator->ToString(psz);
		pcEvaluator = mscEvaluators.Iterate(&sIter);
	}

	psz->Append(" - CInputChord : mlcChords -\n");
	pcChord = mlcChords.StartIteration(&sIter);
	while (pcChord)
	{
		pcChord->ToString(psz);
		pcChord = mlcChords.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChords::DumpHistory(void)
{
	int						i;
	SInputDeviceTimeValue*	psValue;
	CChars					sz;

	sz.Init();
	for (i = 0; i < macHistory.NumElements(); i++)
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

