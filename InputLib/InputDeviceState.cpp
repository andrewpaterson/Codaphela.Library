/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "BaseLib/SystemAllocator.h"
#include "InputDeviceState.h"
#include "InputDevice.h"
#include "InputDeviceValue.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceState::Init(void)
{
	macSourceStates.Init(4);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceState::Kill(void)
{
	macSourceStates.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDeviceState::Rest(CInputDevice* pcDevice)
{
	int					i;
	CInputSourceDesc*	pcSourceDesc;
	SInputSourceState*	psSourceState;
	SSetIterator		sIter;

	mpcDevice = pcDevice;
	macSourceStates.Kill();
	macSourceStates.Allocate(&gcSystemAllocator, pcDevice->GetDesc()->NumInputs());
	pcSourceDesc = pcDevice->GetDesc()->StartInputsIteration(&sIter);
	i = 0;
	while (pcSourceDesc)
	{
		psSourceState = macSourceStates.Get(i);
		psSourceState->pcDesc = pcSourceDesc;
		if (psSourceState->pcDesc->HasRestValue())
		{
			psSourceState->iFlags = SOURCE_STATE_RESTING | SOURCE_STATE_VALID;
			psSourceState->fValue = pcSourceDesc->GetRestValue();
		}
		else
		{
			psSourceState->iFlags = 0;
			psSourceState->fValue = 0.0f;
		}

		if (pcSourceDesc->GetStateIndex() != i)
		{
			return FALSE;
		}

		pcSourceDesc = pcDevice->GetDesc()->IterateInputs(&sIter);
		i++;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDeviceState::GetValue(CInputSourceDesc* pcSourceDesc, float* pfValue)
{
	SInputSourceState*	psSourceState;

	if (pcSourceDesc->GetStateIndex() >= 0)
	{
		psSourceState = macSourceStates.Get(pcSourceDesc->GetStateIndex());
		if (psSourceState->pcDesc == pcSourceDesc)
		{
			if (psSourceState->iFlags & SOURCE_STATE_VALID)
			{
				*pfValue = psSourceState->fValue;
				return TRUE;
			}
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDeviceState::SetValue(CInputSourceDesc* pcSourceDesc, float fValue)
{
	SInputSourceState*	psSourceState;

	if (pcSourceDesc->GetStateIndex() >= 0)
	{
		psSourceState = macSourceStates.Get(pcSourceDesc->GetStateIndex());
		if (psSourceState->pcDesc == pcSourceDesc)
		{
			return SetValue(psSourceState, fValue);
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDeviceState::SetValue(SInputSourceState* psSourceState, float fValue)
{
	if (psSourceState->fValue != fValue)
	{
		psSourceState->fValue = fValue;
		psSourceState->iFlags |= SOURCE_STATE_VALID;
		psSourceState->iFlags |= SOURCE_STATE_EMITTED_EVENT;  //This is a small cheat.  Keep an eye on it.
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceState::ClearFlags(void)
{
	int					i;
	SInputSourceState*	psState;

	for (i = 0; i < macSourceStates.NumElements(); i++)
	{
		psState = macSourceStates.Get(i);
		psState->iFlags &= SOURCE_STATE_VALID;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceState::UpdateRestEvents(CInputDeviceValues* pcEvents, unsigned int uiSequence)
{
	int					i;
	SInputSourceState*	psState;
	CInputSourceDesc*	pcSourceDesc;
	BOOL				bNewValue;
	float				fValue;

	for (i = 0; i < macSourceStates.NumElements(); i++)
	{
		psState = macSourceStates.Get(i);
		pcSourceDesc = psState->pcDesc;
		if (!(psState->iFlags & SOURCE_STATE_EMITTED_EVENT))
		{
			if (psState->pcDesc->GetEmitRestEvent())
			{
				fValue = pcSourceDesc->GetRestValue();
				bNewValue = SetValue(psState, fValue);
				if (bNewValue)
				{
					pcEvents->Add(mpcDevice, pcSourceDesc, fValue, uiSequence, TRUE, 0);
				}
			}
		}

		if (psState->fValue == pcSourceDesc->GetRestValue())
		{
			psState->iFlags |= SOURCE_STATE_RESTING;
		}
	}
}

