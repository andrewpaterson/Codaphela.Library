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
#include "InputDevice.h"
#include "InputDeviceValue.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDevice::Init(CInputDeviceDesc* pcDesc, int iDeviceID, EInputDevice iDescriptionID, CInputActions* pcInputActions)
{
	mszFriendlyName.Init(pcDesc->GetFriendlyName());
	mpcDesc = pcDesc;
	mcState.Init();
	miDeviceID = iDeviceID;
	meDescriptionID = iDescriptionID;
	mbResting = TRUE;
	mcInputValues.Init();
	mcVariables.Init();
	mpcDefaultVirtual = CreateDefaultVirtualDeviceFromThis();
	mpcVariableVirtual = CreateVariableVirtualDeviceFromThis(pcInputActions);
	mcVariables.AddVariablesFromDesc(mpcDesc);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDevice::Kill(void)
{
	mcInputValues.Kill();
	mcVariables.Kill();
	mcState.Kill();
	mszFriendlyName.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDevices* CInputDevice::GetInputDevices(void)
{
	return mpcDesc->GetInputDevices();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDevice::GenerateRestState(void)
{
	mcState.Rest(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDevice::Process(void* pvInput, unsigned int uiSequence)
{
	CInputSourceDesc*	pcSourceDesc;
	SSetIterator		sIter;

	pcSourceDesc = mpcDesc->StartInputsIteration(&sIter);
	while (pcSourceDesc)
	{
		pcSourceDesc->Process(pvInput, &mcState, &mcInputValues, uiSequence);
		pcSourceDesc = mpcDesc->IterateInputs(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDevice::ClearStateFlags(void)
{
	mcState.ClearFlags();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDevice::UpdateRestEvents(unsigned int uiSequence)
{
	mcState.UpdateRestEvents(&mcInputValues, uiSequence);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDevice::GetSources(CArrayInputSourceDescPtr* pcSources, CInputCategoryGeneric* pcGeneric)
{
	CInputSourceDesc*	pcSource;
	SSetIterator		sIter;

	pcSource = mpcDesc->StartInputsIteration(&sIter);
	while (pcSource)
	{
		if (pcSource->GetGenerics()->Contains(&pcGeneric))
		{
			pcSources->Add(&pcSource);
		}
		pcSource = mpcDesc->IterateInputs(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDevice::ClearEvents(void)
{
	mcInputValues.Clear();
	mcVariables.ClearEvents();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDevice::SortEvents(void)
{
	mcInputValues.Sort();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CInputDevice::GetDeviceID(void)
{
	return miDeviceID;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputVirtualDevice* CInputDevice::GetDefaultVirtualDevice(void)
{
	return mpcDefaultVirtual;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputVirtualDevice* CInputDevice::CreateDefaultVirtualDeviceFromThis(char* szName)
{
	CInputVirtualDevice*	pcVirtual;
	SSetIterator			sIter;
	CInputSourceDesc*		pcSource;
	CInputDevices*			pcInputDevices;
	CChars					szTemp;

	pcInputDevices = GetInputDevices();

	if (szName)
	{
		szTemp.Init(szName);
	}
	else
	{
		szTemp.Init(mszFriendlyName.Text());
		szTemp.Append(" Default");
	}
	pcVirtual = pcInputDevices->CreateVirtualDevice(szTemp.Text());
	szTemp.Kill();

	pcSource = mpcDesc->StartInputsIteration(&sIter);
	while (pcSource)
	{
		pcVirtual->AddSource(this, pcSource);
		pcSource = mpcDesc->IterateInputs(&sIter);
	}
	pcVirtual->DoneAddingSources();
	pcInputDevices->UpdateCommonality();
	return pcVirtual;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//CInputVirtualDevice* CInputDevice::CreateDefaultVirtualDeviceFromVirtualDesc(CInputActions* pcInputActions)
//{
//	CInputDevices*					pcInputDevices;
//	CArrayInputDeviceVirtualDevice	asCreatedVirtualDevices;
//	BOOL							bLegit;
//	int								i;
//	SInputDeviceVirtualDevicePair*	psPair;
//
//	//I don't think this is needed anymore.
//	pcInputDevices = GetInputDevices();
//	asCreatedVirtualDevices.Init(4);
//	bLegit = pcInputDevices->CreateVirtualDevices(&asCreatedVirtualDevices, mpcDesc->mpcDefaultVirtualDesc);
//	if (bLegit)
//	{
//		for (i = 0; i < asCreatedVirtualDevices.NumElements(); i++)
//		{
//			psPair = asCreatedVirtualDevices.Get(i);
//			psPair->pcDevice->SetDefaultVirtualDevice(psPair->pcVirtual);
//		}
//	}
//	asCreatedVirtualDevices.Kill();
//	return mpcDefaultVirtual;
//}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputVirtualDevice* CInputDevice::GetVariableVirtualDevice(CInputActions* pcInputActions)
{
	return mpcVariableVirtual;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputVirtualDevice* CInputDevice::CreateVariableVirtualDeviceFromThis(CInputActions* pcInputActions)
{
	CInputVirtualDevice*	pcVirtual;
	CInputSourceDesc*		pcSource;
	CInputDevices*			pcInputDevices;
	CChars					szTemp;
	CArrayPointer			apcSourceDescs;
	int						i;

	pcInputDevices = GetInputDevices();

	szTemp.Init(mszFriendlyName.Text());
	szTemp.Append(" Variable");
	pcVirtual = pcInputDevices->CreateVirtualDevice(szTemp.Text());
	szTemp.Kill();

	mcVariables.SetChords(pcVirtual->GetChords());

	apcSourceDescs.Init(8);
	mpcDesc->GetVariableChordDescs()->GetInputSourceDescs(&apcSourceDescs, mpcDesc);
	for (i = 0; i < apcSourceDescs.NumElements(); i++)
	{
		pcSource = (CInputSourceDesc*)apcSourceDescs.GetPtr(i);
		pcVirtual->AddSource(this, pcSource);
	}
	apcSourceDescs.Kill();
	pcVirtual->DoneAddingSources();
	GetInputDevices()->AddChords(pcVirtual, mpcDesc->GetVariableChordDescs(), this);
	pcInputDevices->UpdateCommonality();
	pcVirtual->Unstoppable();
	return pcVirtual;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputSourceDesc* CInputDevice::GetSource(char* szFriendlyName)
{
	return mpcDesc->GetSource(szFriendlyName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDevice::SetDefaultVirtualDevice(CInputVirtualDevice* pcVirtual)
{
	if (!mpcDefaultVirtual)
	{
		mpcDefaultVirtual = pcVirtual;
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
CInputCategoryGeneric* CInputDevice::GetCategoryGeneric(char* szGeneric)
{
	return mpcDesc->GetCategory()->GetGeneric(szGeneric);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDevice::VariableAction(CInputChord* pcChord)
{
	mcVariables.VariableAction(pcChord);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EInputDevice CInputDevice::GetDescriptionID(void)
{
	return meDescriptionID;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDeviceValues* CInputDevice::GetInputValues(void)
{
	return &mcInputValues;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDeviceDesc* CInputDevice::GetDesc(void)
{
	return mpcDesc;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDevice::DumpEvents(void)
{
	mcInputValues.Dump();
}

