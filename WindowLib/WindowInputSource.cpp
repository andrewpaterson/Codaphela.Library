/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "StandardLib/ClassDefines.h"
#include "InputLib/InputCategory.h"
#include "WindowInputSource.h"


#define WINDOW_INPUT_CHORD_MILLISECONDS		300


//////////////////////////////////////////////////////////////////////////
//
// The devices are described by the XML so they must have been read.
//
//////////////////////////////////////////////////////////////////////////
void CWindowInputSource::Init(CInput* pcInput)
{
	PreInit();

	mpcInput = pcInput;
	mapcDevices.Init();
	mcTimer.Init();
	masEvents.Init();

	mpcVirtual = mpcInput->GetDevices()->CreateVirtualDevice("Window Input");
	AddDevices("Keyboard");
	AddDevices("Mouse");
	AddDevices("Joystick");
	AddDevices("Gamepad");
	mpcVirtual->GetChords()->muiEvictionAge = WINDOW_INPUT_CHORD_MILLISECONDS;

	//Other virtual devices on the same sources keep going.
	mpcVirtual->Enable(false);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindowInputSource::Class(void)
{
	U_Pointer(mpcInput);
	U_Pointer(mpcVirtual);
	U_Data(CArrayInputDevicePtr, mapcDevices);
	U_Data(CTimer, mcTimer);
	U_Data(CArrayWindowInputEvent, masEvents);
}


//////////////////////////////////////////////////////////////////////////
//
// The virtual device is the CInput's, it goes when the input is killed.
//
//////////////////////////////////////////////////////////////////////////
void CWindowInputSource::Free(void)
{
	masEvents.Kill();
	mcTimer.Kill();
	mapcDevices.Kill();
	mpcVirtual = NULL;
	mpcInput = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWindowInputSource::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWindowInputSource::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindowInputSource::AddDevices(char* szCategory)
{
	CArrayInputDevicePtr	apcDevices;
	size					i;
	size					uiNumElements;

	apcDevices.Init();
	GetDevices(&apcDevices, szCategory);
	uiNumElements = apcDevices.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		AddDevice(*apcDevices.Get(i));
	}
	apcDevices.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Puts all the device's sources in the virtual device.  Returns false if
// they already are.
//
//////////////////////////////////////////////////////////////////////////
bool CWindowInputSource::AddDevice(CInputDevice* pcDevice)
{
	CInputSourceDesc*	pcSource;
	SSetIterator		sIter;
	bool				bExists;

	if (mapcDevices.Find(pcDevice) != ARRAY_ELEMENT_NOT_FOUND)
	{
		return false;
	}
	mapcDevices.Add(pcDevice);

	bExists = pcDevice->GetDesc()->StartInputsIteration(&sIter, &pcSource);
	while (bExists)
	{
		mpcVirtual->AddSource(pcDevice, pcSource);
		bExists = pcDevice->GetDesc()->IterateInputs(&sIter, &pcSource);
	}
	mpcVirtual->DoneAddingSources();
	mpcInput->GetDevices()->UpdateCommonality();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Replaces the queue with what changed since the last update.
//
//////////////////////////////////////////////////////////////////////////
void CWindowInputSource::Update(void)
{
	CArrayInputChordsEvent*	pasChordsEvents;
	size					i;
	size					uiNumElements;

	mcTimer.Update();
	masEvents.ReInit();
	mpcInput->Update((uint32)mcTimer.GetTotalTimeInMillieconds());

	pasChordsEvents = mpcVirtual->GetChords()->GetEvents();
	uiNumElements = pasChordsEvents->NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		Queue(pasChordsEvents->Get(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Movement of nothing, as a delta rests at, isn't a change.
//
//////////////////////////////////////////////////////////////////////////
void CWindowInputSource::Queue(SInputChordsEvent* psChordsEvent)
{
	SWindowInputEvent*	psEvent;
	EInputSourceType	eSourceType;

	eSourceType = psChordsEvent->sValue.pcSource->GetType();
	if ((psChordsEvent->pcChord == NULL) && (eSourceType == ISET_Delta) && (psChordsEvent->sValue.fValue == 0.0f))
	{
		return;
	}

	psEvent = masEvents.Add();
	if (psChordsEvent->pcChord)
	{
		psEvent->eType = WIET_Chord;
		psEvent->szChord = psChordsEvent->pcChord->GetAction()->GetName();
	}
	else
	{
		psEvent->eType = (eSourceType == ISET_Delta) ? WIET_Delta : WIET_State;
		psEvent->szChord = NULL;
	}
	psEvent->pcDevice = psChordsEvent->sValue.pcDevice;
	psEvent->pcSource = psChordsEvent->sValue.pcSource;
	psEvent->fValue = psChordsEvent->sValue.fValue;
	psEvent->uiTime = psChordsEvent->sValue.uiTime;
}


//////////////////////////////////////////////////////////////////////////
//
// Returns how many devices are in the category.
//
//////////////////////////////////////////////////////////////////////////
size CWindowInputSource::GetDevices(CArrayInputDevicePtr* papcDevices, char* szCategory)
{
	CInputCategory*		pcCategory;

	pcCategory = mpcInput->GetCategory(szCategory);
	if (!pcCategory)
	{
		return 0;
	}
	pcCategory->GetDevices(papcDevices, false);
	return papcDevices->NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputSourceDesc* CWindowInputSource::GetSource(CInputDevice* pcDevice, char* szFriendlyName)
{
	return pcDevice->GetSource(szFriendlyName);
}


//////////////////////////////////////////////////////////////////////////
//
// Give the chord its criteria (AsGroup, AsOrdered...) from the virtual
// device's sources then call Done on it.  Done returns false, and kills
// the chord, if another chord could match the same inputs.
//
//////////////////////////////////////////////////////////////////////////
CInputChord* CWindowInputSource::AddChord(char* szName)
{
	CAction*	pcAction;

	pcAction = mpcInput->GetActions()->AddAction(szName, &CWindowInputChordListener::Chord);
	return mpcVirtual->AddChordAction(pcAction);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CWindowInputSource::NumEvents(void) { return masEvents.NumElements(); }
SWindowInputEvent* CWindowInputSource::GetEvent(size uiIndex) { return masEvents.Get(uiIndex); }
CInputVirtualDevice* CWindowInputSource::GetVirtualDevice(void) { return mpcVirtual; }
