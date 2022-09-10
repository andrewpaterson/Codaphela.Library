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
#include "InputDevice.h"
#include "InputDevices.h"
#include "InputDeviceValue.h"
#include "InputListener.h"
#include "InputVirtualDevice.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::Init(char* szName)
{
	mszName.Init(szName);
	mlcHistorySources.Init();
	mcCommonality.Init(this);
	mcListeners.Init();
	mcListeners.AddAllowedClass<CInputListener>();
	mcListeners.AddListener((CInputListener*)&mcChords);
	mapcInputValues.Init();
	mcChords.Init();
	mapcDevices.Init();
	mbEnabled = false;
	mbUnstoppable = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::Kill(void)
{
	mapcDevices.Kill();
	mcCommonality.Kill();
	mlcHistorySources.Kill();
	mszName.Kill();
	mcChords.Kill();
	mapcInputValues.Kill();
	mcListeners.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::CallListeners(void)
{
	CInputDeviceValue*	pcDeviceValue;
	int					i;

	if (mbEnabled)
	{
		for (i = 0; i < mapcInputValues.NumElements(); i++)
		{
			pcDeviceValue = *mapcInputValues.Get(i);
			mcListeners.CallListeners(&CInputListener::InputEvent, this, (void*)pcDeviceValue);
		}
	}
}	


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::UpdateEvents(void)
{
	int				i;
	CInputDevice*	pcDevice;

	if (mbEnabled)
	{
		for (i = 0; i < mapcDevices.NumElements(); i++)
		{
			pcDevice = *mapcDevices.Get(i);
			UpdateEvents(pcDevice->GetInputValues());
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::UpdateEvents(CInputDeviceValues* pcValues)
{
	CInputDeviceValue*			pcValue;

	pcValue = pcValues->StartIteration();
	while (pcValue)
	{
		if (ContainsHistorySource(pcValue))
		{
			mapcInputValues.Add(&pcValue);
		}
		pcValue = pcValues->Iterate(pcValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::ClearEvents(void)
{
	if (mbEnabled)
	{
		mapcInputValues.ReInit();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::SortEvents(void)
{
	if (mbEnabled)
	{
		mapcInputValues.BubbleSort(&SortInputEventPtrFunc);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputVirtualDevice::ContainsHistorySource(SInputDeviceValueSource* psSource)
{
	return ContainsSource(psSource, &mlcHistorySources);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputVirtualDevice::ContainsSource(SInputDeviceValueSource* psSource, CSetInputVirtualDeviceSource* pcSetSources)
{
	CInputVirtualDeviceSource*	pcTestDesc;
	SSetIterator				sIter;

	pcTestDesc = pcSetSources->StartIteration(&sIter);
	while (pcTestDesc)
	{
		if (pcTestDesc->Equals(psSource))
		{
			return true;
		}
		pcTestDesc = pcSetSources->Iterate(&sIter);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputVirtualDeviceSource* CInputVirtualDevice::StartHistorySourcesIteration(SSetIterator* psIter)
{
	return mlcHistorySources.StartIteration(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputVirtualDeviceSource* CInputVirtualDevice::IterateHistorySources(SSetIterator* psIter)
{
	return mlcHistorySources.Iterate(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::ClearCommonality(void)
{
	mcCommonality.Clear();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::SortCommonality(void)
{
	mcCommonality.Sort();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::AddCommonalityIfShared(CInputVirtualDevice* pcOtherVirtual)
{
	mcCommonality.AddIfShared(pcOtherVirtual);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputVirtualDevice::AddSource(CInputDevice* pcDevice, CInputSourceDesc* pcSource)
{
	CInputVirtualDeviceSource*	pcVirtualSource;
	CInputDevices*				pcInputDevices;

	if ((!pcSource) || (!pcDevice))
	{
		return false;
	}

	pcVirtualSource = mlcHistorySources.Add();
	pcVirtualSource->Init(pcDevice, pcSource);
	pcInputDevices = GetInputDevices();
	pcInputDevices->DirtyCommonality();

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputVirtualDevice::AddSource(CInputDevice* pcDevice, char* szFriendlyName)
{
	return AddSource(pcDevice, pcDevice->GetSource(szFriendlyName));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputVirtualDevice::AddSources(CInputDevice* pcDevice, char* szFriendlyName, ...)
{
	va_list						vaMarker;
	char*						pc;
	int							iCount;
	bool						bResult;

	iCount = 0;
	pc = szFriendlyName;
	va_start(vaMarker, szFriendlyName);
	while (pc != NULL)
	{
		bResult = AddSource(pcDevice, pc);
		if (!bResult)
		{
			va_end(vaMarker);
			return false;
		}

		iCount++;
		pc = va_arg(vaMarker, char*);
	}
	DoneAddingSources();
	GetInputDevices()->UpdateCommonality();
	va_end(vaMarker);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::DoneAddingSources(void)
{
	CInputVirtualDeviceSource*	pcSource;
	SSetIterator				sIter;
	CInputDevice*				pcDevice;

	mapcDevices.ReInit();

	pcSource = mlcHistorySources.StartIteration(&sIter);
	while (pcSource)
	{
		pcDevice = pcSource->GetSourceDevice();
		mapcDevices.AddIfUnique(&pcDevice);
		pcSource = mlcHistorySources.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::ClearSources(void)
{
	mlcHistorySources.Kill();
	mlcHistorySources.Init();

	mapcDevices.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDevices* CInputVirtualDevice::GetInputDevices(void)
{
	CInputVirtualDeviceSource*	pcSource;
	SSetIterator				sIter;

	pcSource = mlcHistorySources.StartIteration(&sIter);
	return pcSource->GetSourceDevice()->GetInputDevices();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::Enable(bool bDisableCommon)
{
	if (bDisableCommon)
	{
		mcCommonality.Disable();
	}
	mbEnabled = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::Disable(void)
{
	if (!mbUnstoppable)
	{
		mbEnabled = false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::Unstoppable(void)
{
	mbUnstoppable = true;
	mbEnabled = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::ToStringEvents(CChars* psz)
{
	int					i;
	CInputDeviceValue*	pcDeviceValue;

	for (i = 0; i < mapcInputValues.NumElements(); i++)
	{
		pcDeviceValue = *mapcInputValues.Get(i);
		pcDeviceValue->ToString(psz);
		psz->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputVirtualDeviceSource* CInputVirtualDevice::GetSource(char* szFriendlyName)
{
	CInputVirtualDeviceSource*	pcSource;
	SSetIterator				sIter;

	pcSource = mlcHistorySources.StartIteration(&sIter);
	while (pcSource)
	{
		if (pcSource->GetSourceDesc()->Is(szFriendlyName))
		{
			return pcSource;
		}
		pcSource = mlcHistorySources.Iterate(&sIter);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDevice* CInputVirtualDevice::GetDefaultDevice(void)
{
	if (mapcDevices.NumElements() == 1)
	{
		return *mapcDevices.Get(0);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputChords* CInputVirtualDevice::GetChords(void)
{
	return &mcChords;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::AddActiveAction(CAction* pcAction, CInputVirtualDeviceSource* pcSource)
{
	CInputSourceEvaluator*		pcEvaluator;

	pcEvaluator = mcChords.AddEvaluator();
	pcEvaluator->SetSpecificSource(pcSource, ICT_Toggle);
	AddSingleAction(pcAction, pcEvaluator, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::AddInactiveAction(CAction* pcAction, CInputVirtualDeviceSource* pcSource)
{
	CInputSourceEvaluator*		pcEvaluator;

	pcEvaluator = mcChords.AddEvaluator();
	pcEvaluator->SetSpecificSource(pcSource, ICT_Toggle);
	AddSingleAction(pcAction, pcEvaluator, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::AddActiveAction(CAction* pcAction, char* szFriendlyName)
{
	CInputVirtualDeviceSource*	pcSource;

	pcSource = GetSource(szFriendlyName);
	AddActiveAction(pcAction, pcSource);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::AddInactiveAction(CAction* pcAction, char* szFriendlyName)
{
	CInputVirtualDeviceSource*	pcSource;

	pcSource = GetSource(szFriendlyName);
	AddInactiveAction(pcAction, pcSource);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::AddActiveAction(CAction* pcAction, CInputCategoryGeneric* pcGeneric)
{
	CInputSourceEvaluator*		pcEvaluator;
	
	pcEvaluator = mcChords.AddEvaluator();
	pcEvaluator->SetGenericSource(pcGeneric, ICT_Toggle);
	AddSingleAction(pcAction, pcEvaluator, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::AddInactiveAction(CAction* pcAction, CInputCategoryGeneric* pcGeneric)
{
	CInputSourceEvaluator*		pcEvaluator;

	pcEvaluator = mcChords.AddEvaluator();
	pcEvaluator->SetGenericSource(pcGeneric, ICT_Toggle);
	AddSingleAction(pcAction, pcEvaluator, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::AddSingleAction(CAction* pcAction, CInputSourceEvaluator* pcEvaluator, bool bActive)
{
	CInputChord*				pcChord;

	pcChord = mcChords.AddChord(pcAction, NULL);
	if (bActive)
	{
		pcChord->AsActive(pcEvaluator);
	}
	else
	{
		pcChord->AsInactive(pcEvaluator);
	}
	pcChord->Done();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::AddOrderedAction(CAction* pcAction, char* szFriendlyName, ...)
{
	va_list						vaMarker;
	char*						pc;
	int							iCount;
	CInputChord*				pcChord;
	CInputVirtualDeviceSource*	pcSource;
	COrderedInputChordCriteria*	pcOrdered;

	pcChord = AddChordAction(pcAction, NULL);
	pcOrdered = pcChord->AsOrdered();

	iCount = 0;
	pc = szFriendlyName;
	va_start(vaMarker, szFriendlyName);
	while (pc != NULL)
	{
		pcSource = GetSource(pc);
		pcOrdered->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcSource, ICT_Toggle));
		pcOrdered->AddInactiveAction(pcChord->AddEvaluatorSpecificSource(pcSource, ICT_Toggle));

		iCount++;
		pc = va_arg(vaMarker, char*);
	}

	pcOrdered->RemoveLastAction();
	pcChord->Done();
	va_end(vaMarker);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputChord* CInputVirtualDevice::AddChordAction(CAction* pcAction, CInputChordDesc* pcChordDesc)
{
	CInputChord*				pcChord;

	pcChord = mcChords.AddChord(pcAction, pcChordDesc);
	return pcChord;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::AddRangeAction(CAction* pcAction, char* szFriendlyName)
{
	CInputVirtualDeviceSource*	pcSource;

	pcSource = GetSource(szFriendlyName);
	AddRangeAction(pcAction, pcSource);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::AddRangeAction(CAction* pcAction, CInputVirtualDeviceSource* pcSource)
{
	CInputSourceEvaluator*		pcEvaluator;

	pcEvaluator = mcChords.AddEvaluator();
	pcEvaluator->SetSpecificSource(pcSource, ICT_Range);
	AddSingleAction(pcAction, pcEvaluator, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CInputVirtualDevice::GetName(void) { return &mszName; }
CListenerCall* CInputVirtualDevice::GetListeners(void) { return &mcListeners; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDevice::Dump(void)
{
	CChars	sz;

	sz.Append("Virtual Device [");
	sz.Append(mszName.Text());
	sz.Append("]\n");

	//Finish him!

	sz.Dump();
	sz.Kill();
}

