/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "ComponentInput.h"
#include "ComponentInputDevice.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputDevice::InitDevice(CInputVirtualDevice* pcVirtual, CInputDevice* pcDevice, CComponentInput* pcComponentInput)
{
	mpcVirtual = pcVirtual;
	mpcDevice = pcDevice;
	masPrintables.Init();
	masCommands.Init();
	masNumKeys.Init();
	masMotions.Init();
	mpcComponentInput = pcComponentInput;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputDevice::Kill(void)
{
	CComponentInputPrintable*	psPrintable;
	CComponentInputCommand*		psCommand;
	CComponentInputNumKey*		psNumKey;
	CComponentInputMotion*		psMotion;
	int							i;

	for (i = 0; i < masNumKeys.NumElements(); i++)
	{
		psNumKey = masNumKeys.Get(i);
		psNumKey->Kill();
	}
	masNumKeys.Kill();

	for (i = 0; i < masPrintables.NumElements(); i++)
	{
		psPrintable = masPrintables.Get(i);
		psPrintable->Kill();
	}
	masPrintables.Kill();

	for (i = 0; i < masCommands.NumElements(); i++)
	{
		psCommand = masCommands.Get(i);
		psCommand->Kill();
	}
	masCommands.Kill();

	for (i = 0; i < masMotions.NumElements(); i++)
	{
		psMotion = masMotions.Get(i);
		psMotion->Kill();
	}
	masMotions.Kill();

	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputDevice::AddPrintable(CInputSourceDesc* pcSource, char cNoShiftNoCaps, char cYesShiftNoCaps, char cNoShiftYesCaps, char cYesShiftYesCaps)
{
	CComponentInputPrintable*	psPrintable;

	psPrintable = masPrintables.Add();
	psPrintable->Init(pcSource, cNoShiftNoCaps, cYesShiftNoCaps, cNoShiftYesCaps, cYesShiftYesCaps);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponentInputPrintable* CComponentInputDevice::GetPrintable(CInputSourceDesc* pcSource)
{
	CComponentInputPrintable*	psPrintable;
	int						i;

	for (i = 0; i < masPrintables.NumElements(); i++)
	{
		psPrintable = masPrintables.Get(i);
		if (psPrintable->pcSource == pcSource)
		{
			return psPrintable;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputDevice::AddCommand(EComponentInputCommand eCommand, CInputSourceDesc* pcSource, BOOL bControl, BOOL bAlt, BOOL bShift, BOOL bPressed)
{
	CComponentInputCommand*		psCommand;

	psCommand = masCommands.Add();
	psCommand->Init(eCommand, pcSource, bControl, bAlt, bShift, bPressed);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponentInputCommand* CComponentInputDevice::GetCommand(CInputSourceDesc* pcSource, BOOL bControl, BOOL bAlt, BOOL bShift, BOOL bPressed)
{
	CComponentInputCommand*		psCommand;
	int						i;

	for (i = 0; i < masCommands.NumElements(); i++)
	{
		psCommand = masCommands.Get(i);
		if (psCommand->pcSource == pcSource)
		{
			if ((psCommand->bAlt == bAlt) && (psCommand->bControl == bControl) && (psCommand->bShift == bShift) && (psCommand->bPressed == bPressed))
			{
				return psCommand;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputVirtualDeviceSource* CComponentInputDevice::AddButtonPressedAction(EComponentInputCommand eCommand, char* szFriendlyName, BOOL bControl, BOOL bAlt, BOOL bShift)
{
	return AddButtonPressedAction(szFriendlyName, eCommand, bControl, bAlt, bShift);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputVirtualDeviceSource* CComponentInputDevice::AddButtonPressedAction(char* szFriendlyName, EComponentInputCommand eCommand, BOOL bControl, BOOL bAlt, BOOL bShift)
{
	CInputVirtualDeviceSource*	pcSource;

	pcSource = mpcVirtual->GetSource(szFriendlyName);
	if (pcSource)
	{
		if (!HasSpecificAction(pcSource, BAA_Active))
		{
			mpcVirtual->AddActiveAction(mpcComponentInput->GetButtonPressedAction(), pcSource);
		}
		AddCommand(eCommand, pcSource->GetSourceDesc(), bControl, bAlt, bShift, TRUE);
	}
	return pcSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputVirtualDeviceSource* CComponentInputDevice::AddButtonReleasedAction(char* szFriendlyName, EComponentInputCommand eCommand, BOOL bControl, BOOL bAlt, BOOL bShift)
{
	CInputVirtualDeviceSource*	pcSource;

	pcSource = mpcVirtual->GetSource(szFriendlyName);
	if (pcSource)
	{
		if (!HasSpecificAction(pcSource, BAA_Inactive))
		{
			mpcVirtual->AddActiveAction(mpcComponentInput->GetButtonReleasedAction(), pcSource);
		}
		AddCommand(eCommand, pcSource->GetSourceDesc(), bControl, bAlt, bShift, FALSE);
	}
	return pcSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputVirtualDeviceSource* CComponentInputDevice::AddRangeAction(char* szFriendlyName)
{
	CInputVirtualDeviceSource*	pcSource;

	pcSource = mpcVirtual->GetSource(szFriendlyName);
	if (pcSource)
	{
		mpcVirtual->AddRangeAction(mpcComponentInput->GetPointerMotionAction(), pcSource);
	}
	return pcSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputVirtualDeviceSource* CComponentInputDevice::AddPrintable(char* szFriendlyName, char cNoShiftNoCaps, char cYesShiftNoCaps, char cNoShiftYesCaps, char cYesShiftYesCaps)
{
	CInputVirtualDeviceSource*	pcSource;

	pcSource = mpcVirtual->GetSource(szFriendlyName);
	if (pcSource)
	{
		if (!HasSpecificAction(pcSource, BAA_Active))
		{
			mpcVirtual->AddActiveAction(mpcComponentInput->GetButtonPressedAction(), pcSource);
		}
		AddPrintable(pcSource->GetSourceDesc(), cNoShiftNoCaps, cYesShiftNoCaps, cNoShiftYesCaps, cYesShiftYesCaps);
	}
	return pcSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputDevice::AddNumKeyRemap(CInputSourceDesc* pcSource, CInputSourceDesc* pcYesNumLockRemap, CInputSourceDesc* pcNoNumLockRemap)
{
	CComponentInputNumKey*		psNumKey;

	psNumKey = masNumKeys.Add();
	psNumKey->Init(pcSource, pcYesNumLockRemap, pcNoNumLockRemap);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponentInputNumKey* CComponentInputDevice::GetNumKeyRemap(CInputSourceDesc* pcSource)
{
	CComponentInputNumKey*		psNumKey;
	int						i;

	for (i = 0; i < masNumKeys.NumElements(); i++)
	{
		psNumKey = masNumKeys.Get(i);
		if (psNumKey->pcSource == pcSource)
		{
			return psNumKey;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputDevice::AddMotion(CInputSourceDesc* pcSource, float fXMultiplier, float fYMultiplier)
{
	CComponentInputMotion*		psMotion;

	psMotion = masMotions.Add();
	psMotion->Init(pcSource, fXMultiplier, fYMultiplier);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponentInputMotion* CComponentInputDevice::GetMotion(CInputSourceDesc* pcSource)
{
	CComponentInputMotion*	psMotion;
	int						i;

	for (i = 0; i < masMotions.NumElements(); i++)
	{
		psMotion = masMotions.Get(i);
		if (psMotion->mpcSource == pcSource)
		{
			return psMotion;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CComponentInputDevice::HasSpecificAction(CInputVirtualDeviceSource* pcSource, EBasicActionActive eActionType)
{
	SSetIterator			sIter;
	CInputChord*			pcInputChord;
	CInputChords*			pcChords;
	UInputChordCriteria*	puCriteria;

	pcChords = mpcVirtual->GetChords();
	pcInputChord = pcChords->StartChordsIteration(&sIter);
	while (pcInputChord)
	{
		puCriteria = pcInputChord->GetActionCriteria();
		if (puCriteria->eType.eAction == eActionType)
		{
			if (puCriteria->cActive.mpcEvaluator->GetEvaluatorType() == IET_SpecificSource)
			{
				if (puCriteria->cActive.mpcEvaluator->GetDeviceSource()->pcSource == pcSource->GetSourceDesc())
				{
					return TRUE;
				}
			}
		}
		pcInputChord = pcChords->IterateChords(&sIter);
	}
	return FALSE;
}

