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
#include "InputChord.h"
#include "InputDeviceVariableValue.h"
#include "InputDeviceCopyContext.h"
#include "InputDeviceVariableValue.h"
#include "InputDeviceVariable.h"
#include "InputDeviceVariableCondition.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariableCondition::Init(void)
{
	mapConditions.Init();
	mpcChord = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariableCondition::Kill(void)
{
	mapConditions.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariableCondition::Copy(CInputDeviceVariableCondition* pcSource, CInputDeviceCopyContext* pcContext)
{
	int							i;
	CInputDeviceVariableValue*	pcSourceValue;
	CInputDeviceVariableValue*	pcDestValue;

	for (i = 0; i < pcSource->mapConditions.NumElements(); i++)
	{
		pcSourceValue = *pcSource->mapConditions.Get(i);
		pcDestValue = (CInputDeviceVariableValue*)pcContext->mmppVariables.Get(pcSourceValue);
		mapConditions.Add(&pcDestValue);
	}
	if (pcSource->mpcChord != NULL)
	{
		mpcChord = (CInputChord*)pcContext->mmppChords.Get(pcSource->mpcChord);
	}
	else
	{
		mpcChord = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariableCondition::AddValueCondition(CInputDeviceVariableValue* pcValue)
{
	mapConditions.Add(&pcValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariableCondition::SetChordCondition(CInputChord* pcChord)
{
	mpcChord = pcChord;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDeviceVariableCondition::ContainsChord(CInputChord* pcChord)
{
	if (pcChord)
	{
		if (mpcChord == pcChord)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDeviceVariableCondition::MatchCondition(CInputChord* pcChord)
{
	int							i;
	CInputDeviceVariableValue*	pcConditionValue;

	if (ContainsChord(pcChord))
	{
		for (i = 0; i < mapConditions.NumElements(); i++)
		{
			pcConditionValue = *mapConditions.Get(i);
			if (pcConditionValue->GetCurrentVariableValue() != pcConditionValue)
			{
				return false;
			}
		}
		return true;		
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDeviceVariableCondition::MatchNonChordCondition(void)
{
	int							i;
	CInputDeviceVariableValue*	pcConditionValue;

	if (mpcChord == NULL)
	{
		for (i = 0; i < mapConditions.NumElements(); i++)
		{
			pcConditionValue = *mapConditions.Get(i);
			if (pcConditionValue->GetCurrentVariableValue() != pcConditionValue)
			{
				return false;
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariableCondition::ToString(CChars* psz)
{
	psz->Append(" --- CInputDeviceVariableCondition ---\n");
}

