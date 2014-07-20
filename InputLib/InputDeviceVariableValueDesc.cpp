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
#include "InputDeviceCopyContext.h"
#include "InputDeviceVariableDesc.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariableValueDesc::Init(char* szName, CInputDeviceVariableDesc* pcVariableDesc)
{
	mszName.Init(szName);
	mpcVariableDesc = pcVariableDesc;
	mapcConditions.Init();
	mbInitial = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariableValueDesc::Kill(void)
{
	mapcConditions.Kill();
	mszName.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariableValueDesc::Copy(CInputDeviceVariableValueDesc* pcSource, CInputDeviceCopyContext* pcContext)
{
	CInputDeviceVariableValueConditionDesc*		pcSourceConditionDesc;
	int											i;
	CInputDeviceVariableValueDesc*				pcValueDesc;
	CInputChordDesc*							pcChordDesc;

	mbInitial = pcSource->mbInitial;
	for (i = 0; i < pcSource->mapcConditions.NumElements(); i++)
	{
		pcSourceConditionDesc = (CInputDeviceVariableValueConditionDesc*)pcSource->mapcConditions.Get(i);
		if (pcSourceConditionDesc->IsChord())
		{
			pcChordDesc = (CInputChordDesc*)pcContext->mmppChords.Get(((CInputDeviceVariableValueConditionChordDesc*)pcSourceConditionDesc)->mpcChordDesc);
			AddCondition(pcChordDesc);
		}
		else if (pcSourceConditionDesc->IsVariable())
		{
			pcValueDesc = (CInputDeviceVariableValueDesc*)pcContext->mmppValues.Get(((CInputDeviceVariableValueConditionVariableDesc*)pcSourceConditionDesc)->mpcVariableValueDesc);
			AddCondition(pcValueDesc);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDeviceVariableValueDesc::Equals(char* szName)
{
	return mszName.Equals(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariableValueDesc::AddCondition(CInputChordDesc* pcChordDesc)
{
	CInputDeviceVariableValueConditionChordDesc*	pcConditionDesc;

	pcConditionDesc = mapcConditions.Add<CInputDeviceVariableValueConditionChordDesc>();
	pcConditionDesc->Init(pcChordDesc);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariableValueDesc::AddCondition(CInputDeviceVariableValueDesc* pcVariableValueDesc)
{
	CInputDeviceVariableValueConditionVariableDesc*	pcConditionDesc;

	pcConditionDesc = mapcConditions.Add<CInputDeviceVariableValueConditionVariableDesc>();
	pcConditionDesc->Init(pcVariableValueDesc);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CInputDeviceVariableValueDesc::GetName(void)
{
	return mszName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDeviceVariableValueDesc::IsInitial(void)
{
	return mbInitial;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayInputDeviceVariableValueConditionDesc* CInputDeviceVariableValueDesc::GetConditions(void) { return &mapcConditions; }
CInputDeviceVariableDesc* CInputDeviceVariableValueDesc::GetVariableDesc(void) { return mpcVariableDesc; }
void CInputDeviceVariableValueDesc::SetInitial(BOOL bInitial) { mbInitial = bInitial; }

