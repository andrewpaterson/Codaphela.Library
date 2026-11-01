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
	masForgotten.Init();

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
	U_Data(CArrayTemplate<SInputDeviceValueSource>, masForgotten);
}


//////////////////////////////////////////////////////////////////////////
//
// The virtual device is the CInput's, it goes when the input is killed.
//
//////////////////////////////////////////////////////////////////////////
void CWindowInputSource::Free(void)
{
	masForgotten.Kill();
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
	RememberLetGo();

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
// A chord of one or two modifiers (in either order) then the key, all
// held down, like control then c.  pcSecondModifier can be NULL.  The
// modifiers are on one device, the key can be on another (shift and a
// mouse button).  Returns false if the chord couldn't be made, or another
// chord would match the same inputs.
//
//////////////////////////////////////////////////////////////////////////
bool CWindowInputSource::AddKeyChord(char* szName, CInputDevice* pcModifierDevice, CInputSourceDesc* pcFirstModifier, CInputSourceDesc* pcSecondModifier, CInputDevice* pcKeyDevice, CInputSourceDesc* pcKey)
{
	CInputChord*				pcChord;
	COrderedInputChordCriteria*	pcOrdered;
	CGroupInputChordCriteria*	pcGroup;

	if (!pcFirstModifier || !pcKey)
	{
		return false;
	}

	pcChord = AddChord(szName);
	pcOrdered = pcChord->AsOrdered();
	if (pcSecondModifier)
	{
		pcGroup = pcOrdered->AddGroupAction();
		pcGroup->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcModifierDevice, pcFirstModifier, ICT_Toggle));
		pcGroup->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcModifierDevice, pcSecondModifier, ICT_Toggle));
	}
	else
	{
		pcOrdered->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcModifierDevice, pcFirstModifier, ICT_Toggle));
	}
	pcOrdered->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcKeyDevice, pcKey, ICT_Toggle));
	return pcChord->Done();
}


//////////////////////////////////////////////////////////////////////////
//
// A chord of two keys one after the other with two modifiers, like
// control and shift (in either order) and f, then h.  The first key is
// let go before the second is pressed, and so are the modifiers flagged
// to be (in any order with the first key); the others are still held.  A
// chord for each set of modifiers let go makes letting go of them
// optional.  All the keys are on the keyboard.  Once everything is let
// go the second key must come within uiEvictionMilliseconds (0 for the
// usual chord time).  Returns false as AddKeyChord.
//
//////////////////////////////////////////////////////////////////////////
bool CWindowInputSource::AddKeySequenceChord(char* szName, CInputDevice* pcKeyboard, CInputSourceDesc* pcFirstModifier, CInputSourceDesc* pcSecondModifier, CInputSourceDesc* pcFirstKey, CInputSourceDesc* pcSecondKey, bool bFirstModifierLetGo, bool bSecondModifierLetGo, uint32 uiEvictionMilliseconds)
{
	CInputChord*				pcChord;
	COrderedInputChordCriteria*	pcOrdered;
	CGroupInputChordCriteria*	pcGroup;

	if (!pcFirstModifier || !pcSecondModifier || !pcFirstKey || !pcSecondKey)
	{
		return false;
	}

	pcChord = AddChord(szName);
	pcOrdered = pcChord->AsOrdered();
	pcGroup = pcOrdered->AddGroupAction();
	pcGroup->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcKeyboard, pcFirstModifier, ICT_Toggle));
	pcGroup->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcKeyboard, pcSecondModifier, ICT_Toggle));
	pcOrdered->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcKeyboard, pcFirstKey, ICT_Toggle));
	if (bFirstModifierLetGo || bSecondModifierLetGo)
	{
		pcGroup = pcOrdered->AddGroupAction();
		pcGroup->AddInactiveAction(pcChord->AddEvaluatorSpecificSource(pcKeyboard, pcFirstKey, ICT_Toggle));
		if (bFirstModifierLetGo)
		{
			pcGroup->AddInactiveAction(pcChord->AddEvaluatorSpecificSource(pcKeyboard, pcFirstModifier, ICT_Toggle));
		}
		if (bSecondModifierLetGo)
		{
			pcGroup->AddInactiveAction(pcChord->AddEvaluatorSpecificSource(pcKeyboard, pcSecondModifier, ICT_Toggle));
		}
	}
	else
	{
		pcOrdered->AddInactiveAction(pcChord->AddEvaluatorSpecificSource(pcKeyboard, pcFirstKey, ICT_Toggle));
	}
	pcOrdered->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcKeyboard, pcSecondKey, ICT_Toggle));
	pcChord->SetEvictionAge(uiEvictionMilliseconds);
	return pcChord->Done();
}


//////////////////////////////////////////////////////////////////////////
//
// True if the key or button is down, as the device last said, unless it
// was down when ForgetDown was called and hasn't been let go since.
//
//////////////////////////////////////////////////////////////////////////
bool CWindowInputSource::IsDown(CInputDevice* pcDevice, CInputSourceDesc* pcSource)
{
	float					fValue;
	SInputDeviceValueSource*	psForgotten;
	size					i;
	size					uiNumElements;

	if (!pcDevice || !pcSource)
	{
		return false;
	}
	if (!pcDevice->GetValue(pcSource, &fValue) || (fValue < INPUT_CHORDS_DOWN))
	{
		return false;
	}

	uiNumElements = masForgotten.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		psForgotten = masForgotten.Get(i);
		if ((psForgotten->pcDevice == pcDevice) && (psForgotten->pcSource == pcSource))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// For when the window loses focus.  Keys let go while another window has
// focus are never heard of, so those down now aren't down to IsDown until
// they have been let go.
//
//////////////////////////////////////////////////////////////////////////
void CWindowInputSource::ForgetDown(void)
{
	CInputDevice*				pcDevice;
	CInputSourceDesc*			pcSource;
	SInputDeviceValueSource*	psForgotten;
	SSetIterator				sIter;
	size						i;
	size						uiNumDevices;
	bool						bExists;

	masForgotten.ReInit();
	uiNumDevices = mapcDevices.NumElements();
	for (i = 0; i < uiNumDevices; i++)
	{
		pcDevice = *mapcDevices.Get(i);
		bExists = pcDevice->GetDesc()->StartInputsIteration(&sIter, &pcSource);
		while (bExists)
		{
			if (IsDown(pcDevice, pcSource))
			{
				psForgotten = masForgotten.Add();
				psForgotten->pcDevice = pcDevice;
				psForgotten->pcSource = pcSource;
			}
			bExists = pcDevice->GetDesc()->IterateInputs(&sIter, &pcSource);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A forgotten key that is let go is remembered again.
//
//////////////////////////////////////////////////////////////////////////
void CWindowInputSource::RememberLetGo(void)
{
	SInputDeviceValueSource*	psForgotten;
	float						fValue;
	size						i;

	for (i = masForgotten.NumElements(); i > 0; i--)
	{
		psForgotten = masForgotten.Get(i - 1);
		if (!psForgotten->pcDevice->GetValue(psForgotten->pcSource, &fValue) || (fValue < INPUT_CHORDS_DOWN))
		{
			masForgotten.RemoveAt(i - 1, true);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CWindowInputSource::GetName(CInputSourceDesc* pcSource) { return pcSource->GetFriendlyName(); }
bool CWindowInputSource::IsDelta(CInputSourceDesc* pcSource) { return pcSource->GetType() == ISET_Delta; }
size CWindowInputSource::NumEvents(void) { return masEvents.NumElements(); }
SWindowInputEvent* CWindowInputSource::GetEvent(size uiIndex) { return masEvents.Get(uiIndex); }
CInputVirtualDevice* CWindowInputSource::GetVirtualDevice(void) { return mpcVirtual; }
