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
#include "InputDevices.h"
#include "InputCategory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputCategory::Init(char* szCategory, CInputDevices* pcInputDevices)
{
	mlcGenerics.Init();
	mszCategory.Init(szCategory);
	mpcInputDevices = pcInputDevices;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputCategory::Kill(void)
{
	mszCategory.Kill();
	mlcGenerics.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputCategoryGeneric* CInputCategory::AddGeneric(char* szAction)
{
	CInputCategoryGeneric*	pcGeneric;

	pcGeneric = mlcGenerics.Add();
	pcGeneric->Init(szAction);
	return pcGeneric;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputCategory::ContainsGeneric(CInputCategoryGeneric* pcGeneric)
{
	return mlcGenerics.Contains(pcGeneric);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputCategory::ContainsGeneric(char* szGeneric)
{
	CInputCategoryGeneric*	pcGeneric;

	pcGeneric = GetGeneric(szGeneric);
	return pcGeneric != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputCategoryGeneric* CInputCategory::GetGeneric(char* szGeneric)
{
	SSetIterator			sIter;
	CInputCategoryGeneric*	pcCurrent;

	pcCurrent = mlcGenerics.StartIteration(&sIter);
	while (pcCurrent)
	{
		if (pcCurrent->Is(szGeneric))
		{
			return pcCurrent;
		}
		pcCurrent = mlcGenerics.Iterate(&sIter);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDevice* CInputCategory::GetFirstDevice(BOOL bPhysical)
{
	CInputDevice*	pcDevice;
	SSetIterator	sIter;

	pcDevice = mpcInputDevices->mlcDevices.StartIteration(&sIter);
	while (pcDevice)
	{
		if (pcDevice->GetDesc()->GetCategory() == this)
		{
			if (((pcDevice->GetDesc()->IsPhysical()) && (bPhysical == TRUE)) || bPhysical == FALSE)
			{
				return pcDevice;
			}
		}
		pcDevice = mpcInputDevices->mlcDevices.Iterate(&sIter);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputCategory::GetDevices(CArrayInputDevicePtr* pcDevices, BOOL bPhysical)
{
	CInputDevice*	pcDevice;
	SSetIterator	sIter;

	pcDevice = mpcInputDevices->mlcDevices.StartIteration(&sIter);
	while (pcDevice)
	{
		if (pcDevice->GetDesc()->GetCategory() == this)
		{
			if (((pcDevice->GetDesc()->IsPhysical()) && (bPhysical == TRUE)) || bPhysical == FALSE)
			{
				pcDevices->Add(&pcDevice);
			}
		}
		pcDevice = mpcInputDevices->mlcDevices.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CInputCategory::GetCategoryName(void)
{
	return mszCategory.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDevices* CInputCategory::GetInputDevices(void)
{
	return mpcInputDevices;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputCategory::Is(char* szName)
{
	return mszCategory.Equals(szName);
}

