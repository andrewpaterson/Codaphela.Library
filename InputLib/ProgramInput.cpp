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
#include "ProgramInput.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProgramInput::Init(void)
{
	macPDeviceDetails.Init();
	mpcFormat = NULL;
	mpcCategory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProgramInput::Kill(void)
{
	CProgramDeviceDetail*	pcDetail;
	int						i;

	for (i = 0; i < macPDeviceDetails.NumElements(); i++)
	{
		pcDetail = macPDeviceDetails.Get(i);
		pcDetail->Kill();
	}
	macPDeviceDetails.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProgramInput::AddDevicesTo(CInputDevices* pcDevices)
{
	int						i;
	CProgramDeviceDetail*	pcDetail;
	CInputDevice*			pcInputDevice;
	CInputDeviceDesc*		pcDeviceDesc;

	for (i = 0; i < macPDeviceDetails.NumElements(); i++)
	{
		pcDetail = macPDeviceDetails.Get(i);
		pcDeviceDesc = CreateStandardDesc(pcDevices, pcDetail);
		pcInputDevice = pcDevices->CreateDevice(pcDeviceDesc);
		pcInputDevice->GenerateRestState();
		pcDetail->SetUniqueID(pcInputDevice->GetDeviceID());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProgramInput::Update(CInputDevices* pcDevices, unsigned int uiSequence)
{
	int						i;
	CProgramDeviceDetail*	pcDetail;
	CInputDevice*			pcDevice;
	int						j;
	SProgramDeviceValue*	psValue;

	for (i = 0; i < macPDeviceDetails.NumElements(); i++)
	{
		pcDetail = macPDeviceDetails.Get(i);
		pcDevice = pcDevices->GetDevice(pcDetail->miUniqueID);

		for (j = 0; j < pcDetail->macValues.NumElements(); j++)
		{
			psValue = pcDetail->macValues.Get(j);
			pcDevice->Process(psValue, uiSequence);
		}
		pcDetail->ClearEvents();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDeviceDesc* CProgramInput::CreateStandardDesc(CInputDevices* pcDevices, CProgramDeviceDetail* pcDetail)
{
	CInputDeviceDesc*			pcDesc;
	int							i;
	CProgramDeviceSourceValue*	pcValue;
	CInputSourceDesc*			pcSourceDesc;
	CInputSourceValue*			pcSourceValue;

	pcDesc = pcDevices->CreateDescription(pcDetail->mszID.Text(), pcDetail->GetFriendlyName(), mpcCategory, FALSE);
	pcDesc->SetDataFormat(mpcFormat);

	for (i = 0; i < pcDetail->macSources.NumElements(); i++)
	{
		pcValue = pcDetail->macSources.Get(i);
		pcSourceDesc = pcDesc->AddInput(ISET_State, pcValue->mszName.Text());
		pcSourceValue = pcSourceDesc->AddValue(1);
		pcSourceValue->AddChannel(0, ISCC_Equals, &pcValue->miName);
		pcSourceValue->SetStateDetail(1.0f, 0.0f);
		pcSourceDesc->SetRest(0.0f, FALSE, TRUE);
	}

	return pcDesc;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProgramInput::AddFormat(CInputDevices* pcDevices)
{
	mpcFormat = pcDevices->AddFormat("Programmatic");
	mpcFormat->Add(PT_int, "iName");
	mpcFormat->Add(PT_float, "fValue");
	mpcFormat->Done();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProgramInput::AddCategory(CInputDevices* pcDevices)
{
	mpcCategory = pcDevices->AddCategory("Program");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CProgramDeviceDetail* CProgramInput::AddDetail(char* szID, char* szFriendlyName)
{
	CProgramDeviceDetail*	pcDetail;

	pcDetail = macPDeviceDetails.Add();
	pcDetail->Init(szID, szFriendlyName);
	
	return pcDetail;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CProgramInput::GetCategoryName(void)
{
	return mpcCategory->GetCategoryName();
}

