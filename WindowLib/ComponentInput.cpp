/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "Window.h"
#include "ComponentInput.h"


#define COMPONENT_INPUT_PRESSED		0.5f	//A key or button's value at or above which it is pressed.


//////////////////////////////////////////////////////////////////////////
//
// No devices are added, see AddKeyboard and AddMouse.
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::Init(CWindowInputSource* pcSource, CWindow* pcWindow)
{
	mpcSource = pcSource;
	mpcWindow = pcWindow;

	macDevices.Init();
	masPressed.Init();

	miShift = 0;
	miControl = 0;
	miAlt = 0;
	mbCapsLock = false;
	mbNumLock = false;
	mbInsert = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::Kill(void)
{
	CComponentInputDevice*	pcDevice;
	size					i;
	size					uiNumElements;

	uiNumElements = macDevices.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcDevice = macDevices.Get(i);
		pcDevice->Kill();
	}
	macDevices.Kill();
	masPressed.Kill();

	mpcWindow = NULL;
	mpcSource = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponentInputDevice* CComponentInput::AddDevice(CInputDevice* pcDevice)
{
	CComponentInputDevice*	pcInputDevice;

	if (!pcDevice)
	{
		return NULL;
	}

	if (GetDevice(pcDevice))
	{
		return NULL;
	}

	//A device that isn't a keyboard or mouse, like a programmatic one, isn't in the source yet.
	mpcSource->AddDevice(pcDevice);

	pcInputDevice = macDevices.Add();
	pcInputDevice->Init(pcDevice, this);
	return pcInputDevice;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponentInputDevice* CComponentInput::GetDevice(CInputDevice* pcDevice)
{
	CComponentInputDevice*	pcInputDevice;
	size					i;
	size					uiNumElements;

	uiNumElements = macDevices.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcInputDevice = macDevices.Get(i);
		if (pcInputDevice->GetDevice() == pcDevice)
		{
			return pcInputDevice;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::AddKeyboard(CInputDevice* pcDevice)
{
	CComponentInputDevice*	pcInputDevice;

	pcInputDevice = AddDevice(pcDevice);
	if (pcInputDevice)
	{
		pcInputDevice->InitKeyboard();
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::AddMouse(CInputDevice* pcDevice)
{
	CComponentInputDevice*	pcInputDevice;

	pcInputDevice = AddDevice(pcDevice);
	if (pcInputDevice)
	{
		pcInputDevice->InitMouse();
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CComponentInput::AddKeyboards(char* szCategory)
{
	return AddDevices(szCategory, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CComponentInput::AddMice(char* szCategory)
{
	return AddDevices(szCategory, false);
}


//////////////////////////////////////////////////////////////////////////
//
// Returns how many devices were added.
//
//////////////////////////////////////////////////////////////////////////
size CComponentInput::AddDevices(char* szCategory, bool bKeyboard)
{
	CArrayInputDevicePtr	apcDevices;
	size					i;
	size					uiNumElements;
	size					uiAdded;
	bool					bAdded;
	CInputDevice*			pcDevice;

	uiAdded = 0;
	apcDevices.Init();
	uiNumElements = mpcSource->GetDevices(&apcDevices, szCategory);
	for (i = 0; i < uiNumElements; i++)
	{
		pcDevice = *apcDevices.Get(i);
		bAdded = bKeyboard ? AddKeyboard(pcDevice) : AddMouse(pcDevice);
		if (bAdded)
		{
			uiAdded++;
		}
	}
	apcDevices.Kill();
	return uiAdded;
}


//////////////////////////////////////////////////////////////////////////
//
// Updates the source and turns what it queued from this input's devices'
// keys and buttons into presses and releases.  Movement, chords and other
// devices are for someone else.
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::Update(void)
{
	SWindowInputEvent*	psEvent;
	size				i;
	size				uiNumEvents;

	mpcSource->Update();

	uiNumEvents = mpcSource->NumEvents();
	for (i = 0; i < uiNumEvents; i++)
	{
		psEvent = mpcSource->GetEvent(i);
		if ((psEvent->eType == WIET_State) && GetDevice(psEvent->pcDevice))
		{
			if (psEvent->fValue >= COMPONENT_INPUT_PRESSED)
			{
				Pressed(psEvent->pcSource);
			}
			else
			{
				Released(psEvent->pcSource);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A pressed source is, in order: a modifier or lock, a numpad key standing
// in for another source, a command for the modifiers held or a printable
// character if neither control nor alt is held.
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::Pressed(CInputSourceDesc* pcSource)
{
	CComponentInputKey*			psKey;
	CComponentInputNumKey*		psNumKey;
	CComponentInputCommand*		psCommand;
	CComponentInputPrintable*	psPrintable;
	CInputSourceDesc*			pcRemapped;
	bool						bControl;
	bool						bAlt;
	bool						bShift;

	psKey = GetKey(pcSource);
	if (psKey)
	{
		if (PressedKey(psKey))
		{
			return;
		}
	}

	bControl = miControl > 0;
	bAlt = miAlt > 0;
	bShift = miShift > 0;
	AddPressed(pcSource, bControl, bAlt, bShift);

	//With num lock off the numpad keys move, and shift doesn't select.
	pcRemapped = pcSource;
	psNumKey = GetNumKeyRemap(pcSource);
	if (psNumKey)
	{
		if (mbNumLock)
		{
			pcRemapped = psNumKey->pcYesNumLockRemap;
		}
		else
		{
			pcRemapped = psNumKey->pcNoNumLockRemap;
			bShift = false;
		}
		if (!pcRemapped)
		{
			return;
		}
	}

	psCommand = GetCommand(pcRemapped, bControl, bAlt, bShift, true);
	if (psCommand)
	{
		Command(psCommand->eCommand);
		return;
	}

	psPrintable = GetPrintable(pcRemapped);
	if (psPrintable)
	{
		if (!bAlt && !bControl)
		{
			mpcWindow->InputPrintable(psPrintable->Get(miShift > 0, mbCapsLock), mbInsert);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::Released(CInputSourceDesc* pcSource)
{
	CComponentInputKey*			psKey;
	CComponentInputCommand*		psCommand;
	SComponentInputPressed		sPressed;

	psKey = GetKey(pcSource);
	if (psKey)
	{
		ReleasedKey(psKey);
	}

	if (!RemovePressed(pcSource, &sPressed))
	{
		//Pressed before this was listening, or a modifier.
		return;
	}

	psCommand = GetCommand(pcSource, sPressed.bControl, sPressed.bAlt, sPressed.bShift, false);
	if (psCommand)
	{
		Command(psCommand->eCommand);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Returns true if the key has been used up.  Insert with control or shift
// held isn't, it gives copy or paste.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::PressedKey(CComponentInputKey* psKey)
{
	switch (psKey->eKey)
	{
	case CIK_Shift:
		miShift++;
		return true;
	case CIK_Control:
		miControl++;
		return true;
	case CIK_Alt:
		miAlt++;
		return true;
	case CIK_CapsLock:
		mbCapsLock = !mbCapsLock;
		return true;
	case CIK_NumLock:
		mbNumLock = !mbNumLock;
		return true;
	case CIK_Insert:
		if ((miShift == 0) && (miControl == 0) && (miAlt == 0))
		{
			mbInsert = !mbInsert;
			return true;
		}
		return false;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::ReleasedKey(CComponentInputKey* psKey)
{
	switch (psKey->eKey)
	{
	case CIK_Shift:
		if (miShift > 0)
		{
			miShift--;
		}
		break;
	case CIK_Control:
		if (miControl > 0)
		{
			miControl--;
		}
		break;
	case CIK_Alt:
		if (miAlt > 0)
		{
			miAlt--;
		}
		break;
	case CIK_CapsLock:
	case CIK_NumLock:
	case CIK_Insert:
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// For when the window loses focus.  Keys released while another window
// had focus are never seen so they are all treated as released now.  The
// locks are left as they are.
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::ReleaseAll(void)
{
	miShift = 0;
	miControl = 0;
	miAlt = 0;
	masPressed.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// The locks are toggled by key presses so they get out of step with the
// keyboard when they are pressed while another window has focus.  The
// native window sets them from the keyboard when it is activated.
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::SetLocks(bool bCapsLock, bool bNumLock)
{
	mbCapsLock = bCapsLock;
	mbNumLock = bNumLock;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::Command(EComponentInputCommand eCommand)
{
	if (eCommand != CIC_Ignored)
	{
		mpcWindow->InputCommand(eCommand, mbInsert);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::AddPressed(CInputSourceDesc* pcSource, bool bControl, bool bAlt, bool bShift)
{
	SComponentInputPressed*	psPressed;
	SComponentInputPressed	sIgnored;

	//A source pressed twice without being released, keep the latest.
	RemovePressed(pcSource, &sIgnored);

	psPressed = masPressed.Add();
	psPressed->pcSource = pcSource;
	psPressed->bControl = bControl;
	psPressed->bAlt = bAlt;
	psPressed->bShift = bShift;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::RemovePressed(CInputSourceDesc* pcSource, SComponentInputPressed* psDest)
{
	SComponentInputPressed*	psPressed;
	size					i;
	size					uiNumElements;

	uiNumElements = masPressed.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		psPressed = masPressed.Get(i);
		if (psPressed->pcSource == pcSource)
		{
			*psDest = *psPressed;
			masPressed.RemoveAt(i);
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponentInputCommand* CComponentInput::GetCommand(CInputSourceDesc* pcSource, bool bControl, bool bAlt, bool bShift, bool bPressed)
{
	CComponentInputCommand*		psCommand;
	size						i;
	size						uiNumElements;

	uiNumElements = macDevices.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		psCommand = macDevices.Get(i)->GetCommand(pcSource, bControl, bAlt, bShift, bPressed);
		if (psCommand)
		{
			return psCommand;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponentInputPrintable* CComponentInput::GetPrintable(CInputSourceDesc* pcSource)
{
	CComponentInputPrintable*	psPrintable;
	size						i;
	size						uiNumElements;

	uiNumElements = macDevices.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		psPrintable = macDevices.Get(i)->GetPrintable(pcSource);
		if (psPrintable)
		{
			return psPrintable;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponentInputNumKey* CComponentInput::GetNumKeyRemap(CInputSourceDesc* pcSource)
{
	CComponentInputNumKey*	psNumKey;
	size					i;
	size					uiNumElements;

	uiNumElements = macDevices.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		psNumKey = macDevices.Get(i)->GetNumKeyRemap(pcSource);
		if (psNumKey)
		{
			return psNumKey;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponentInputKey* CComponentInput::GetKey(CInputSourceDesc* pcSource)
{
	CComponentInputKey*		psKey;
	size					i;
	size					uiNumElements;

	uiNumElements = macDevices.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		psKey = macDevices.Get(i)->GetKey(pcSource);
		if (psKey)
		{
			return psKey;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool		CComponentInput::IsShift(void) { return miShift > 0; }
bool		CComponentInput::IsControl(void) { return miControl > 0; }
bool		CComponentInput::IsAlt(void) { return miAlt > 0; }
bool		CComponentInput::IsCapsLock(void) { return mbCapsLock; }
bool		CComponentInput::IsNumLock(void) { return mbNumLock; }
bool		CComponentInput::IsInsert(void) { return mbInsert; }
CWindowInputSource*	CComponentInput::GetSource(void) { return mpcSource; }


//////////////////////////////////////////////////////////////////////////
//
// The window the commands and printable characters go to, as when a
// modal dialog takes them from its owner.  Release everything first.
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::SetWindow(CWindow* pcWindow)
{
	mpcWindow = pcWindow;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CWindow* CComponentInput::GetWindow(void)
{
	return mpcWindow;
}
