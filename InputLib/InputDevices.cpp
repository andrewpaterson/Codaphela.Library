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
#include "BaseLib/Logger.h"
#include "InputDevicesReader.h"
#include "InputActions.h"
#include "InputDevices.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDevices::Init(CInputActions* pcInputActions)
{
	mpcInputActions = pcInputActions;
	mlcDeviceDescs.Init();
	mlcDevices.Init();
	mcCategories.Init(this);
	mcDataFormats.Init();
	mlcVirtualsDescs.Init();
	mlcVirtuals.Init();
	mbCommonalityDirty = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevices::Read(char* szDirectory)
{
	CInputDevicesReader	cReader;
	bool				bResult;

	cReader.Init(this, szDirectory);
	bResult = cReader.ReadInputDevicesXML();
	cReader.Kill();
	return bResult;;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDevices::Kill(void)
{
	mlcVirtuals.Kill();
	mlcVirtualsDescs.Kill();
	mlcDevices.Kill();
	mlcDeviceDescs.Kill();

	mcDataFormats.Kill();
	mcCategories.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDataFormat* CInputDevices::AddFormat(char* szName)
{
	return mcDataFormats.Add(szName);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputCategory* CInputDevices::AddCategory(char* szName)
{
	return mcCategories.AddCategory(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputCategory* CInputDevices::GetCategory(char* szName)
{
	return mcCategories.GetCategory(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDevice* CInputDevices::CreateDevice(CInputDeviceDesc* pcDesc)
{
	CInputDevice*	pcDevice;
	int				iDescriptionID;

	pcDevice = mlcDevices.Add();
	iDescriptionID = pcDesc->GetUnusedID();
	pcDevice->Init(pcDesc, mlcDevices.NumElements()-1, (EInputDevice)iDescriptionID, mpcInputActions);
	pcDesc->AddDevice(pcDevice);
	
	return pcDevice;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputVirtualDevice* CInputDevices::CreateVirtualDevice(char* szName)
{
	CInputVirtualDevice*	pcVirtual;

	pcVirtual = mlcVirtuals.Add();
	pcVirtual->Init(szName);
	return pcVirtual;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevices::AddActiveOrInactiveCriteria(CInputChordActiveDesc* pcChordCriteriaDesc, UInputChordCriteria* puChordCriteria, CInputSourceEvaluator* pcEvaluator, CInputDevice* pcDevice)
{
	
	switch (((CInputChordActiveDesc*)pcChordCriteriaDesc)->meActiveType)
	{
	case ICCDAT_SpecificSource:
		if (!pcDevice)
		{
			pcDevice = GetDeviceForSourceDesc(pcChordCriteriaDesc->GetSourceDesc(), pcChordCriteriaDesc->GetDescriptionID());
			if (!pcDevice)
			{
				return false;
			}
		}
		pcEvaluator->SetSpecificSource(pcDevice, pcChordCriteriaDesc->GetSourceDesc(), pcChordCriteriaDesc->meChordType);
		return true;
	case ICCDAT_GenericSource:
		pcEvaluator->SetGenericSource(pcChordCriteriaDesc->mpcGeneric, pcChordCriteriaDesc->meChordType);
		return true;
	case ICCDAT_AnyToggleSource:
		pcEvaluator->SetAnyToggleSource();
		return true;
	}
	return false;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevices::AddGroupCriteria(CInputChordCollectiveDesc* pcChordCriteriaDesc, CGroupInputChordCriteria* pcGroupCriteria, CInputChords* pcChords, CInputDevice* pcDevice)
{
	size						i;
	UInputChordCriteria*		puChordCriteria;
	CInputChordCriteriaDesc*	pcCurrentChordDesc;
	bool						bResult;
	size						uiNumElements;

	uiNumElements = pcChordCriteriaDesc->macCriteria.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcCurrentChordDesc = (CInputChordCriteriaDesc*)pcChordCriteriaDesc->macCriteria.Get(i);
		puChordCriteria = (UInputChordCriteria*)pcGroupCriteria->mausBasicActionCriteria.Add();
		bResult = RecurseAddCriteria(pcCurrentChordDesc, puChordCriteria, pcChords, pcDevice);
		if (!bResult)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevices::AddOrderedCriteria(CInputChordCollectiveDesc* pcChordCriteriaDesc, COrderedInputChordCriteria* pcGroupCriteria, CInputChords* pcChords, CInputDevice* pcDevice)
{
	size						i;
	UInputChordCriteria*		puChordCriteria;
	CInputChordCriteriaDesc*	pcCurrentChordDesc;
	bool						bResult;
	size						uiNumElements;

	uiNumElements = pcChordCriteriaDesc->macCriteria.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcCurrentChordDesc = (CInputChordCriteriaDesc*)pcChordCriteriaDesc->macCriteria.Get(i);
		puChordCriteria = (UInputChordCriteria*)pcGroupCriteria->mausBasicActionCriteria.Add();
		bResult = RecurseAddCriteria(pcCurrentChordDesc, puChordCriteria, pcChords, pcDevice);
		if (!bResult)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevices::RecurseAddCriteria(CInputChordCriteriaDesc* pcChordCriteriaDesc, UInputChordCriteria* puChordCriteria, CInputChords* pcChords, CInputDevice* pcDevice)
{
	CInputSourceEvaluator*	pcEvaluator;

	switch (pcChordCriteriaDesc->GetType())
	{
	case ICCDT_Active:
		pcEvaluator = pcChords->AddEvaluator();
		puChordCriteria->cActive.Init(pcEvaluator);
		return AddActiveOrInactiveCriteria((CInputChordActiveDesc*)pcChordCriteriaDesc, puChordCriteria, pcEvaluator, pcDevice);
	case ICCDT_Inactive:
		pcEvaluator = pcChords->AddEvaluator();
		puChordCriteria->cInactive.Init(pcEvaluator);
		return AddActiveOrInactiveCriteria((CInputChordActiveDesc*)pcChordCriteriaDesc, puChordCriteria, pcEvaluator, pcDevice);
	case ICCDT_Group:
		puChordCriteria->cGroup.Init();
		return AddGroupCriteria((CInputChordCollectiveDesc*)pcChordCriteriaDesc, (CGroupInputChordCriteria*)&puChordCriteria->cGroup, pcChords, pcDevice);
	case ICCDT_Ordered:
		puChordCriteria->cOrdered.Init();
		return AddOrderedCriteria((CInputChordCollectiveDesc*)pcChordCriteriaDesc, (COrderedInputChordCriteria*)&puChordCriteria->cOrdered, pcChords, pcDevice);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevices::AddChords(CInputVirtualDevice* pcVirtual, CInputVirtualDeviceDesc* pcVirtualDesc, CInputDevice* pcDevice)
{
	return AddChords(pcVirtual, pcVirtualDesc->GetChordDescs(), pcDevice);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevices::AddChords(CInputVirtualDevice* pcVirtual, CInputChordDescs* pcChordDescs, CInputDevice* pcDevice)
{
	SSetIterator		sIter;
	CInputChordDesc*	pcChordDesc;
	CAction*			pcAction;
	bool				bResult;
	bool				bResult2;
	CInputChord*		pcChord;

	bResult = true;
	pcChordDesc = pcChordDescs->StartChordDescsIteration(&sIter);
	while (pcChordDesc)
	{
		pcAction = mpcInputActions->GetAction(pcChordDesc->GetActionName());
		if (pcAction)
		{
			if (pcChordDesc->GetRootCriteriaDesc())
			{
				pcChord = pcVirtual->AddChordAction(pcAction, pcChordDesc);
				bResult2 = RecurseAddCriteria(pcChordDesc->GetRootCriteriaDesc(), pcChord->GetActionCriteria(), pcVirtual->GetChords(), pcDevice);
				pcChord->Done();
				if (!bResult2)
				{
					bResult = false;
				}
			}

			pcChordDesc = pcChordDescs->IterateChordDescs(&sIter);
		}
		else
		{
			bResult = false;
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputVirtualDevice* CInputDevices::CreateVirtualDevice(CInputVirtualDeviceDesc* pcVirtualDesc)
{
	CInputVirtualDevice*		pcVirtual;
	SSetIterator				sIter;
	CInputVirtualSourceDesc*	pcVirtualSourceDesc;
	CInputDevice*				pcDevice;
	bool						bResult;

	if (!pcVirtualDesc->IsDeviceAgnostic())
	{
		//There is only one possible virtual device that can be created.
		//Either because it spans multiple physical devices 
		//or because the user has explicitly mapped inputs from one physical device.

		pcVirtualSourceDesc = pcVirtualDesc->StartInputSourceDescsIteration(&sIter);
		while (pcVirtualSourceDesc)
		{
			pcDevice = GetDeviceForVirtualSourceDesc(pcVirtualSourceDesc);
			if (!pcDevice)
			{
				//The physical device is not plugged in so a virtual device using it cannot be created.
				return NULL;
			}
			pcVirtualSourceDesc = pcVirtualDesc->IterateInputSourceDescs(&sIter);
		}

		pcVirtual = CreateVirtualDevice(pcVirtualDesc->GetName());
		pcVirtualSourceDesc = pcVirtualDesc->StartInputSourceDescsIteration(&sIter);
		while (pcVirtualSourceDesc)
		{
			pcDevice = GetDeviceForVirtualSourceDesc(pcVirtualSourceDesc);
			pcVirtual->AddSource(pcDevice, pcVirtualSourceDesc->GetSourceDesc());
			pcVirtualSourceDesc = pcVirtualDesc->IterateInputSourceDescs(&sIter);
		}

		pcVirtual->DoneAddingSources();
		bResult = AddChords(pcVirtual, pcVirtualDesc, NULL);
		if (!bResult)
		{
			return NULL;
		}
		UpdateCommonality();
		return pcVirtual;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDevices::CreateVirtualDevices(CArrayInputDeviceVirtualDevice* pacCreatedVirtualVevices, CInputVirtualDeviceDesc* pcVirtualDesc)
{
	CInputVirtualDevice*			pcVirtual;
	SSetIterator					sIter;
	CInputVirtualSourceDesc*		pcVirtualSourceDesc;
	CInputDevice*					pcDevice;
	CArrayInputDevicePtr			apDevices;
	size							i;
	CInputDeviceDesc*				pcDeviceDesc;
	SInputDeviceVirtualDevicePair*	psPair;
	bool							bResult;
	size							uiNumElements;

	if (pcVirtualDesc->IsDeviceAgnostic())
	{
		//Multiple virtual devices can be created because the description ID is ignored.
		//If there are 4 xbox controllers plugged in then 4 virtual devices will be created.

		pcDeviceDesc = GetDescriptionForVirtualDesc(pcVirtualDesc);
		if (!pcDeviceDesc)
		{
			return false;
		}

		apDevices.Init();
		GetDevicesForDeviceDesc(&apDevices, pcDeviceDesc);

		if (apDevices.NumElements() == 0)
		{
			apDevices.Kill();
			return false;
		}

		uiNumElements = apDevices.NumElements();
		for (i = 0; i < uiNumElements; i++)
		{
			pcDevice = *apDevices.Get(i);
			pcVirtual = CreateVirtualDevice(pcVirtualDesc->GetName());
			pcVirtualSourceDesc = pcVirtualDesc->StartInputSourceDescsIteration(&sIter);
			while (pcVirtualSourceDesc)
			{
				pcVirtual->AddSource(pcDevice, pcVirtualSourceDesc->GetSourceDesc());
				pcVirtualSourceDesc = pcVirtualDesc->IterateInputSourceDescs(&sIter);
			}

			pcVirtual->DoneAddingSources();
			bResult = AddChords(pcVirtual, pcVirtualDesc, pcDevice);
			if (!bResult)
			{
				return false;
			}

			psPair = pacCreatedVirtualVevices->Add();
			psPair->pcDevice = pcDevice;
			psPair->pcVirtual = pcVirtual;
		}
		UpdateCommonality();

		apDevices.Kill();
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
CInputVirtualDeviceDesc* CInputDevices::CreateVirtualDeviceDescription(char* szName, bool bDeviceAgnostic)
{
	CInputVirtualDeviceDesc*	pcVirtualDesc;

	pcVirtualDesc = mlcVirtualsDescs.Add();
	pcVirtualDesc->Init(szName, bDeviceAgnostic);
	return pcVirtualDesc;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDevice* CInputDevices::GetDevice(int iDeviceID)
{
	CInputDevice*	pcDevice;
	SSetIterator	sIter;

	pcDevice = mlcDevices.StartIteration(&sIter);
	while (pcDevice)
	{
		if (pcDevice->GetDeviceID() == iDeviceID)
		{
			return pcDevice;
		}
		pcDevice = mlcDevices.Iterate(&sIter);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDevices::GetDevicesForCategory(CArrayInputDevicePtr* pcDevices, char* szCategory)
{
	CInputDevice*	pcDevice;
	SSetIterator	sIter;

	pcDevice = mlcDevices.StartIteration(&sIter);
	while (pcDevice)
	{
		if (pcDevice->GetDesc()->GetCategory()->Is(szCategory))
		{
			pcDevices->Add(&pcDevice);
		}
		pcDevice = mlcDevices.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDevices::GetDevices(CArrayInputDevicePtr* pcDevices)
{
	CInputDevice*	pcDevice;
	SSetIterator	sIter;

	pcDevice = mlcDevices.StartIteration(&sIter);
	while (pcDevice)
	{
		pcDevices->Add(&pcDevice);
		pcDevice = mlcDevices.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDevices::GetDevicesForDeviceDesc(CArrayInputDevicePtr* pcDevices, CInputDeviceDesc* pcDesc)
{
	CInputDevice*	pcDevice;
	SSetIterator	sIter;

	pcDevice = mlcDevices.StartIteration(&sIter);
	while (pcDevice)
	{
		if (pcDevice->GetDesc() == pcDesc)
		{
			pcDevices->Add(&pcDevice);
		}
		pcDevice = mlcDevices.Iterate(&sIter);
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDeviceDesc* CInputDevices::GetDescriptionForVirtualDesc(CInputVirtualDeviceDesc* pcVirtualDesc)
{
	SSetIterator				sIter;
	CInputVirtualSourceDesc*	pcVirtualSourceDesc;
	CInputDeviceDesc*			pcDeviceDesc;

	if (!pcVirtualDesc->IsDeviceAgnostic())
	{
		//There is no guarantee that a device inagnostic description will use only one device description.
		return NULL;
	}
	else
	{
		pcDeviceDesc = NULL;
		pcVirtualSourceDesc = pcVirtualDesc->StartInputSourceDescsIteration(&sIter);
		while (pcVirtualSourceDesc)
		{
			if (pcDeviceDesc == NULL)
			{
				pcDeviceDesc = pcVirtualSourceDesc->GetSourceDesc()->GetDeviceDesc();
			}
			else
			{
				if (pcVirtualSourceDesc->GetSourceDesc()->GetDeviceDesc() != pcDeviceDesc)
				{
					return NULL;
				}
			}
			pcVirtualSourceDesc = pcVirtualDesc->IterateInputSourceDescs(&sIter);
		}
		return pcDeviceDesc;
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDevice* CInputDevices::GetDeviceForVirtualSourceDesc(CInputVirtualSourceDesc* pcVirtualSourceDesc)
{
	if (pcVirtualSourceDesc->GetVirtualDeviceDesc()->IsDeviceAgnostic())
	{
		//Can't get a single device for a device agnostic description, there may be more than one.
		return NULL;
	}
	else
	{
		return GetDeviceForSourceDesc(pcVirtualSourceDesc->GetSourceDesc(), pcVirtualSourceDesc->GetDescriptionID());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDevice* CInputDevices::GetDeviceForSourceDesc(CInputSourceDesc* pcSourceDesc, int iDescID)
{
	CInputDevice*	pcDevice;
	SSetIterator	sIter;


	pcDevice = mlcDevices.StartIteration(&sIter);
	while (pcDevice)
	{
		if (pcSourceDesc->GetDeviceDesc() == pcDevice->GetDesc())
		{
			if (iDescID == pcDevice->GetDescriptionID())
			{
				return pcDevice;
			}
		}
		pcDevice = mlcDevices.Iterate(&sIter);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDeviceDesc* CInputDevices::CreateDescription(char* szID, char* szFriendlyName, CInputCategory* pcCategory, bool bPhysical)
{
	CInputDeviceDesc*	pcDesc;

	pcDesc = GetDescription(szID);
	if (pcDesc)
	{
		return NULL;
	}
	else
	{
		pcDesc = mlcDeviceDescs.Add();
		pcDesc->Init(szID, szFriendlyName, pcCategory, bPhysical);
		return pcDesc;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDeviceDesc* CInputDevices::GetDescription(char* szID)
{
	CInputDeviceDesc*	psDesc;
	SSetIterator		sIter;

	psDesc = mlcDeviceDescs.StartIteration(&sIter);
	while (psDesc != NULL)
	{
		if (psDesc->Is(szID))
		{
			return psDesc;
		}
		psDesc = mlcDeviceDescs.Iterate(&sIter);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDevices::UpdateCommonality(void)
{
	SSetIterator			sIter;
	CInputVirtualDevice*	pcVirtual;
	SSetIterator			sIter2;
	CInputVirtualDevice*	pcVirtual2;

	pcVirtual = mlcVirtuals.StartIteration(&sIter);
	while (pcVirtual)
	{
		pcVirtual->ClearCommonality();
		pcVirtual2 = mlcVirtuals.StartIteration(&sIter2);
		while (pcVirtual2)
		{
			if (pcVirtual != pcVirtual2)
			{
				pcVirtual->AddCommonalityIfShared(pcVirtual2);
			}
			pcVirtual2 = mlcVirtuals.Iterate(&sIter2);
		}
		pcVirtual->SortCommonality();
		pcVirtual = mlcVirtuals.Iterate(&sIter);
	}
	mbCommonalityDirty = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDevices::DumpDevicesToFile(void)
{
	CInputDeviceDesc*	pcDeviceDesc;
	SSetIterator		sIter;

	pcDeviceDesc = mlcDeviceDescs.StartIteration(&sIter);
	while (pcDeviceDesc)
	{
		pcDeviceDesc->Dump();
		pcDeviceDesc = mlcDeviceDescs.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDevices::DirtyCommonality(void)
{
	mbCommonalityDirty = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDevices::UpdateVirtualDevicesEvents(void)
{
	CInputVirtualDevice*	pcVirtual;
	SSetIterator			sIter;

	pcVirtual = mlcVirtuals.StartIteration(&sIter);
	while (pcVirtual)
	{
		pcVirtual->ClearEvents();
		pcVirtual->UpdateEvents();
		pcVirtual->SortEvents();

		pcVirtual = mlcVirtuals.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputVirtualDevice* CInputDevices::GetFirstKeyboard(void)
{
	return GetFirstVirtualDeviceInCategoryKeyboard("Keyboard");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputVirtualDevice* CInputDevices::GetFirstVirtualDeviceInCategoryKeyboard(char* szCategory, bool bPhysical)
{
	//This method only exists to show how to get a usuable device.

	CInputCategory*			pcKeyboards;
	CInputDevice*			pcKeyboard;
	CInputVirtualDevice*	pcVirtualKeyboard;

	pcKeyboards = GetCategory(szCategory);
	pcKeyboard = pcKeyboards->GetFirstDevice(bPhysical);
	pcVirtualKeyboard = pcKeyboard->GetDefaultVirtualDevice();
	return pcVirtualKeyboard;
}

