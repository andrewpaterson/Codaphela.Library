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
#include "ProgramDeviceDetail.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProgramDeviceSourceValue::Init(char* szName, int iName)
{
	mszName.Init(szName);
	miName = iName;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProgramDeviceSourceValue::Kill(void)
{
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProgramDeviceDetail::Init(char* szID, char* szFriendlyName)
{
	mszID.Init(szID);
	mszFriendlyName.Init(szFriendlyName);
	macValues.Init(1);
	macSources.Init(8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProgramDeviceDetail::Kill(void)
{
	int							i;
	CProgramDeviceSourceValue*	pcSource;

	for (i = 0; i < macSources.NumElements(); i++)
	{
		pcSource = macSources.Get(i);
		pcSource->Kill();
	}
	macSources.Kill();

	macValues.Kill();
	mszFriendlyName.Kill();
	mszID.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProgramDeviceDetail::AddSource(char* szName)
{
	CProgramDeviceSourceValue*	pcSource;

	pcSource = macSources.Add();
	pcSource->Init(szName, macSources.NumElements()-1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProgramDeviceDetail::AddEvent(char* szName, float fValue)
{
	SProgramDeviceValue*	psValue;

	psValue = macValues.Add();
	psValue->iName = GetSourceID(szName);
	psValue->fValue = fValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CProgramDeviceDetail::GetSourceID(char* szName)
{
	int							i;
	CProgramDeviceSourceValue*	pcSource;

	for (i = 0; i < macSources.NumElements(); i++)
	{
		pcSource = macSources.Get(i);
		if (pcSource->mszName.Equals(szName))
		{
			return pcSource->miName;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProgramDeviceDetail::ClearEvents(void)
{
	macValues.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CProgramDeviceDetail::GetFriendlyName(void) { return mszFriendlyName.Text(); }
char* CProgramDeviceDetail::GetID(void) { return mszID.Text(); }
