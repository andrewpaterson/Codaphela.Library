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
#include "InputDeviceVariableValue.h"
#include "InputDeviceVariable.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariable::Init(CInputDeviceVariableDesc* pcDesc)
{
	mpcDesc = pcDesc;
	mpcCurrent = NULL;
	mlcValues.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariable::Kill(void)
{
	mlcValues.Kill();
	mpcCurrent = NULL;
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariable::SetValue(CInputDeviceVariableValue* pcValue)
{
	mpcCurrent = pcValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDeviceVariableValue* CInputDeviceVariable::AddValue(CInputDeviceVariableValueDesc* pcVariableDesc)
{
	CInputDeviceVariableValue*	pcValue;

	pcValue = mlcValues.Add();
	pcValue->Init(this, pcVariableDesc);
	return pcValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDeviceVariableValue* CInputDeviceVariable::GetCurrent(void)
{
	return mpcCurrent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDeviceVariableValue* CInputDeviceVariable::StartValuesIteration(SSetIterator* psIter)
{
	return mlcValues.StartIteration(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDeviceVariableValue* CInputDeviceVariable::IterateValues(SSetIterator* psIter)
{
	return mlcValues.Iterate(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariable::SetCurrent(CInputDeviceVariableValue* pcCurrent) { mpcCurrent = pcCurrent; }


