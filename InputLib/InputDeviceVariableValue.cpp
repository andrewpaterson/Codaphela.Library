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
#include "InputDeviceVariable.h"
#include "InputDeviceVariableValue.h"
#include "InputDeviceVariableDesc.h"
#include "InputDeviceCopyContext.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariableValue::Init(CInputDeviceVariable* pcVariable, CInputDeviceVariableValueDesc* pcDesc)
{
	mcCondition.Init();
	mpcDesc = pcDesc;
	mpcVariable = pcVariable;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariableValue::Kill(void)
{
	mcCondition.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariableValue::AddValueCondition(CInputDeviceVariableValue* pcValue)
{
	mcCondition.AddValueCondition(pcValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariableValue::SetChordCondition(CInputChord* pcChord)
{
	mcCondition.SetChordCondition(pcChord);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDeviceVariableValue::Equals(char* szName)
{
	return mpcDesc->Equals(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDeviceVariableValue::ContainsChord(CInputChord* pcChord)
{
	return mcCondition.ContainsChord(pcChord);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDeviceVariableValue::MakeCurrent(void)
{
	if (mpcVariable->GetCurrent() != this)
	{
		mpcVariable->SetCurrent(this);
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDeviceVariableValue::MatchCondition(CInputChord* pcChord)
{
	return mcCondition.MatchCondition(pcChord);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDeviceVariableValue::MatchNonChordCondition(void)
{
	return mcCondition.MatchNonChordCondition();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDeviceVariableValueDesc* CInputDeviceVariableValue::GetDesc(void)
{
	return mpcDesc;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDeviceVariableValue* CInputDeviceVariableValue::GetCurrentVariableValue(void)
{
	return mpcVariable->GetCurrent();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariableValue::ToString(CChars* psz)
{
	psz->Append(" --- CInputDeviceVariableValue ---\n");

	psz->Append("Name: ");
	psz->Append(mpcDesc->GetName());
	psz->AppendNewLine();

	mcCondition.ToString(psz);
}

