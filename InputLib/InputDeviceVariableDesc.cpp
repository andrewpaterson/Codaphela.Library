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
#include "InputDeviceVariableDesc.h"
#include "InputDeviceCopyContext.h"
#include "InputDeviceDesc.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariableDesc::Init(char* szName, CInputDeviceDesc* pcDeviceDesc)
{
	mpcDeviceDesc = pcDeviceDesc;
	mszName.Init(szName);
	mlcValues.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariableDesc::Kill(void)
{
	mlcValues.Kill();
	mszName.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDeviceVariableDesc::Equals(char* szName)
{
	return mszName.Equals(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDeviceVariableValueDesc* CInputDeviceVariableDesc::AddValue(char* szName)
{
	CInputDeviceVariableValueDesc*	pcValue;

	pcValue = mlcValues.Add();
	pcValue->Init(szName, this);
	return pcValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDeviceVariableValueDesc* CInputDeviceVariableDesc::GetValue(char* szName)
{
	CInputDeviceVariableValueDesc*	pcValue;
	SSetIterator					sIter;

	pcValue = mlcValues.StartIteration(&sIter);
	while (pcValue)
	{
		if (pcValue->Equals(szName))
		{
			return pcValue;
		}
		pcValue = mlcValues.Iterate(&sIter);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariableDesc::Copy(CInputDeviceVariableDesc* pcSourceDesc, CInputDeviceCopyContext* pcContext)
{
	CInputDeviceVariableValueDesc*	pcDestValue;
	CInputDeviceVariableValueDesc*	pcSourceValue;
	SSetIterator					sIter;
	CInputDeviceVariableDesc*		pcVariableDesc;

	mszName.Init(pcSourceDesc->mszName.Text());
	mpcDeviceDesc = pcContext->mpcDestDeviceDesc;

	pcSourceValue = pcSourceDesc->mlcValues.StartIteration(&sIter);
	while (pcSourceValue)
	{
		pcDestValue = mlcValues.Add();
		pcContext->mmppValues.Put(pcSourceValue, pcDestValue);

		pcVariableDesc = (CInputDeviceVariableDesc*)pcContext->mmppVariables.GetWithKey(pcSourceValue->GetVariableDesc());
		pcDestValue->Init(pcSourceValue->GetName(), pcVariableDesc);

		pcSourceValue = pcSourceDesc->mlcValues.Iterate(&sIter);
	}

	pcSourceValue = pcSourceDesc->mlcValues.StartIteration(&sIter);
	while (pcSourceValue)
	{
		pcDestValue = (CInputDeviceVariableValueDesc*)pcContext->mmppValues.GetWithKey(pcSourceValue);

		pcDestValue->Copy(pcSourceValue, pcContext);
		pcSourceValue = pcSourceDesc->mlcValues.Iterate(&sIter);
	}

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDeviceVariableValueDesc* CInputDeviceVariableDesc::StartValuesIteration(SSetIterator* psIter)
{
	return mlcValues.StartIteration(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDeviceVariableValueDesc* CInputDeviceVariableDesc::IterateValues(SSetIterator* psIter)
{
	return mlcValues.Iterate(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CInputDeviceVariableDesc::GetName(void) { return mszName.Text(); }
CInputDeviceDesc* CInputDeviceVariableDesc::GetDeviceDesc(void) { return mpcDeviceDesc; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariableDesc::ToString(CChars* psz)
{
	//SSetIterator				sIter;
	//CInputDeviceVariableValue*	psValue;

	psz->Append(" --- CInputDeviceVariableDesc ---\n");

	psz->Append("Name: ");
	psz->Append(mszName.Text());
	psz->AppendNewLine();

	psz->Append(" - CInputDeviceVariableValue : mlcValues -\n");

	//psValue = mlcValues.StartIteration(&sIter);
	//while (psValue)
	//{
	//	psValue->ToString(psz);
	//	psValue = mlcValues.Iterate(&sIter);
	//}
}

