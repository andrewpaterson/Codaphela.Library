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


//The keys that mean something with control, shift or both held.  Each is a chord named for its modifiers and key,
//like "Control+C" or "Control+Shift+Cursor Left", made for every left and right modifier the keyboard has.
struct SComponentInputChordRow
{
	bool					bControl;
	bool					bShift;
	char*					szKey;
	EComponentInputCommand	ePressed;
	EComponentInputCommand	eReleased;  //When the chord's key is let go, CIC_Ignored for nothing.
};


static SComponentInputChordRow	gasKeyboardChordRows[] =
{
	{ true,  false, "Cursor Left",	CIC_WordLeft,					CIC_Ignored },
	{ true,  false, "Cursor Right",	CIC_WordRight,					CIC_Ignored },
	{ true,  false, "Home",			CIC_DocumentHome,				CIC_Ignored },
	{ true,  false, "End",			CIC_DocumentEnd,				CIC_Ignored },
	{ true,  false, "Cursor Up",	CIC_LineUp,						CIC_Ignored },
	{ true,  false, "Cursor Down",	CIC_LineDown,					CIC_Ignored },
	{ false, true,  "Cursor Up",	CIC_SelectUp,					CIC_Ignored },
	{ false, true,  "Cursor Down",	CIC_SelectDown,					CIC_Ignored },
	{ false, true,  "Cursor Left",	CIC_SelectLeft,					CIC_Ignored },
	{ false, true,  "Cursor Right",	CIC_SelectRight,				CIC_Ignored },
	{ true,  true,  "Cursor Left",	CIC_SelectWordLeft,				CIC_Ignored },
	{ true,  true,  "Cursor Right",	CIC_SelectWordRight,			CIC_Ignored },
	{ false, true,  "Home",			CIC_SelectHome,					CIC_Ignored },
	{ false, true,  "End",			CIC_SelectEnd,					CIC_Ignored },
	{ true,  true,  "Home",			CIC_SelectDocumentHome,			CIC_Ignored },
	{ true,  true,  "End",			CIC_SelectDocumentEnd,			CIC_Ignored },
	{ false, true,  "Page Up",		CIC_SelectPageUp,				CIC_Ignored },
	{ false, true,  "Page Down",	CIC_SelectPageDown,				CIC_Ignored },
	{ true,  false, "W",			CIC_SelectWholeWord,			CIC_Ignored },
	{ true,  false, "A",			CIC_SelectAll,					CIC_Ignored },
	{ false, true,  "Space",		CIC_Space,						CIC_Ignored },
	{ false, true,  "Tab",			CIC_BackTab,					CIC_Ignored },
	{ true,  true,  "Enter",		CIC_HomeEnter,					CIC_Ignored },
	{ true,  true,  "Numpad Enter",	CIC_HomeEnter,					CIC_Ignored },
	{ false, true,  "Enter",		CIC_EndEnter,					CIC_Ignored },
	{ false, true,  "Numpad Enter",	CIC_EndEnter,					CIC_Ignored },
	{ true,  false, "X",			CIC_Cut,						CIC_Ignored },
	{ false, true,  "Delete",		CIC_Cut,						CIC_Ignored },
	{ true,  false, "C",			CIC_Copy,						CIC_Ignored },
	{ true,  false, "Insert",		CIC_Copy,						CIC_Ignored },
	{ true,  false, "V",			CIC_Paste,						CIC_Ignored },
	{ false, true,  "Insert",		CIC_Paste,						CIC_Ignored },
	{ true,  false, "D",			CIC_Duplicate,					CIC_Ignored },
	{ true,  false, "Backspace",	CIC_DeleteWordRemainingLeft,	CIC_Ignored },
	{ true,  false, "Delete",		CIC_DeleteWordRemainingRight,	CIC_Ignored },
	{ true,  false, "Z",			CIC_Undo,						CIC_Ignored },
	{ true,  true,  "Z",			CIC_StepUndo,					CIC_Ignored },
	{ true,  false, "Y",			CIC_Redo,						CIC_Ignored },
	{ true,  true,  "Y",			CIC_StepRedo,					CIC_Ignored },
};


//The keyboard's modifiers with the mouse's buttons.
static SComponentInputChordRow	gasMouseChordRows[] =
{
	{ false, true,  "Button 1",			CIC_StartDragSelect,	CIC_StopDragSelect },
	{ true,  false, "Button 1",			CIC_StartDragSelect,	CIC_StopDragSelect },
	{ true,  true,  "Button 1",			CIC_StartDragSelect,	CIC_StopDragSelect },
	{ false, true,  "Wheel Forward",	CIC_WheelLeft,			CIC_Ignored },
	{ false, true,  "Wheel Backward",	CIC_WheelRight,			CIC_Ignored },
};


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
	masChordCommands.Init();

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

	uiNumElements = masChordCommands.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		masChordCommands.Get(i)->szName.Kill();
	}
	masChordCommands.Kill();

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
// The keyboard's chords are made with it, and those of its modifiers with
// the buttons of every mouse added so far.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::AddKeyboard(CInputDevice* pcDevice)
{
	CComponentInputDevice*	pcInputDevice;
	CComponentInputDevice*	pcMouse;
	size					i;
	size					uiNumElements;

	pcInputDevice = AddDevice(pcDevice);
	if (!pcInputDevice)
	{
		return false;
	}

	pcInputDevice->InitKeyboard();
	AddKeyboardChords(pcInputDevice);

	//The keyboard was added last, the array may have moved.
	uiNumElements = macDevices.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcMouse = macDevices.Get(i);
		if (pcMouse->IsMouse())
		{
			AddMouseChords(macDevices.Get(uiNumElements - 1), pcMouse);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::AddMouse(CInputDevice* pcDevice)
{
	CComponentInputDevice*	pcInputDevice;
	CComponentInputDevice*	pcKeyboard;
	size					i;
	size					uiNumElements;

	pcInputDevice = AddDevice(pcDevice);
	if (!pcInputDevice)
	{
		return false;
	}

	pcInputDevice->InitMouse();

	uiNumElements = macDevices.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcKeyboard = macDevices.Get(i);
		if (!pcKeyboard->IsMouse())
		{
			AddMouseChords(pcKeyboard, macDevices.Get(uiNumElements - 1));
		}
	}
	return true;
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
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::AddKeyboardChords(CComponentInputDevice* pcKeyboard)
{
	size						i;
	SComponentInputChordRow*	psRow;

	for (i = 0; i < sizeof(gasKeyboardChordRows) / sizeof(SComponentInputChordRow); i++)
	{
		psRow = &gasKeyboardChordRows[i];
		AddChordCommands(pcKeyboard, pcKeyboard, psRow->bControl, psRow->bShift, psRow->szKey, psRow->ePressed, psRow->eReleased);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::AddMouseChords(CComponentInputDevice* pcKeyboard, CComponentInputDevice* pcMouse)
{
	size						i;
	SComponentInputChordRow*	psRow;

	for (i = 0; i < sizeof(gasMouseChordRows) / sizeof(SComponentInputChordRow); i++)
	{
		psRow = &gasMouseChordRows[i];
		AddChordCommands(pcKeyboard, pcMouse, psRow->bControl, psRow->bShift, psRow->szKey, psRow->ePressed, psRow->eReleased);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A chord for each of the keyboard's left and right controls and shifts
// that the row wants, then szKey on pcKeyDevice.  They share one name and
// command.  A modifier the keyboard doesn't have makes no chord; without
// its shift a control and shift chord would be its control chord too.
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::AddChordCommands(CComponentInputDevice* pcKeyboard, CComponentInputDevice* pcKeyDevice, bool bControl, bool bShift, char* szKey, EComponentInputCommand ePressed, EComponentInputCommand eReleased)
{
	CInputSourceDesc*				apcControls[2];
	CInputSourceDesc*				apcShifts[2];
	CInputSourceDesc*				pcKey;
	CChars							szName;
	SComponentInputChordCommand*	psCommand;
	size							i;
	size							j;

	pcKey = pcKeyDevice->GetSource(szKey);
	if (!pcKey)
	{
		return;
	}

	apcControls[0] = pcKeyboard->GetSource("Left Control");
	apcControls[1] = pcKeyboard->GetSource("Right Control");
	apcShifts[0] = pcKeyboard->GetSource("Left Shift");
	apcShifts[1] = pcKeyboard->GetSource("Right Shift");

	szName.Init();
	if (bControl)
	{
		szName.Append("Control+");
	}
	if (bShift)
	{
		szName.Append("Shift+");
	}
	szName.Append(szKey);

	for (i = 0; i < 2; i++)
	{
		if (bControl && bShift)
		{
			for (j = 0; j < 2; j++)
			{
				if (apcControls[i] && apcShifts[j])
				{
					mpcSource->AddKeyChord(szName.Text(), pcKeyboard->GetDevice(), apcControls[i], apcShifts[j], pcKeyDevice->GetDevice(), pcKey);
				}
			}
		}
		else if (bControl)
		{
			mpcSource->AddKeyChord(szName.Text(), pcKeyboard->GetDevice(), apcControls[i], NULL, pcKeyDevice->GetDevice(), pcKey);
		}
		else
		{
			mpcSource->AddKeyChord(szName.Text(), pcKeyboard->GetDevice(), apcShifts[i], NULL, pcKeyDevice->GetDevice(), pcKey);
		}
	}

	if (!GetChordCommand(szName.Text()))
	{
		psCommand = masChordCommands.Add();
		psCommand->szName.Init(szName.Text());
		psCommand->ePressed = ePressed;
		psCommand->eReleased = eReleased;
	}
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SComponentInputChordCommand* CComponentInput::GetChordCommand(char* szName)
{
	SComponentInputChordCommand*	psCommand;
	size							i;
	size							uiNumElements;

	uiNumElements = masChordCommands.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		psCommand = masChordCommands.Get(i);
		if (psCommand->szName.Equals(szName))
		{
			return psCommand;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Updates the source and gives each event it queued to the window, whose
// focus translates (see Translate) and dispatches it.
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::Update(void)
{
	size	i;

	mpcSource->Update();

	for (i = 0; i < mpcSource->NumEvents(); i++)
	{
		mpcWindow->Input(mpcSource->GetEvent(i), this);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::InitEvent(SComponentInputEvent* psEvent, EComponentInputEventType eType, SWindowInputEvent* psWindowEvent)
{
	memset(psEvent, 0, sizeof(SComponentInputEvent));
	psEvent->eType = eType;
	psEvent->eCommand = CIC_Ignored;
	psEvent->bInsert = mbInsert;
	psEvent->bShift = IsShift();
	psEvent->bCapsLock = mbCapsLock;
	psEvent->bNumLock = mbNumLock;
	psEvent->uiTime = psWindowEvent->uiTime;
	psEvent->psWindowEvent = psWindowEvent;
}


//////////////////////////////////////////////////////////////////////////
//
// Writes into asDest (COMPONENT_INPUT_MAX_EVENTS long) what the queued
// event means, in the order they are given to the components:
//   A key or button of a keyboard or mouse added here: its key event, then
//     the command or printable it means alone, if any.
//   A chord: its chord event, then its command if it is a standard chord.
//   Movement of a mouse added here: a movement event.
//   A key or button of another device: its key event.
// Returns how many were written.  The pointer position is set by the focus.
//
//////////////////////////////////////////////////////////////////////////
size CComponentInput::Translate(SWindowInputEvent* psWindowEvent, SComponentInputEvent* asDest)
{
	SComponentInputChordCommand*	psChordCommand;
	CComponentInputDevice*			pcDevice;
	char*							szName;
	bool							bDown;

	pcDevice = GetDevice(psWindowEvent->pcDevice);
	if (psWindowEvent->eType == WIET_Chord)
	{
		InitEvent(&asDest[0], CIET_Chord, psWindowEvent);
		asDest[0].szName = psWindowEvent->szChord;
		psChordCommand = GetChordCommand(psWindowEvent->szChord);
		if (!psChordCommand)
		{
			return 1;
		}

		//The chord's key was used up so its release only means something if the chord said so.
		if (psChordCommand->eReleased != CIC_Ignored)
		{
			AddPressed(psWindowEvent->pcSource, psChordCommand->eReleased);
		}
		InitEvent(&asDest[1], CIET_Command, psWindowEvent);
		asDest[1].eCommand = psChordCommand->ePressed;
		return 2;
	}
	else if (psWindowEvent->eType == WIET_Delta)
	{
		if (!pcDevice || !pcDevice->IsMouse())
		{
			return 0;
		}

		InitEvent(&asDest[0], CIET_Movement, psWindowEvent);
		szName = mpcSource->GetName(psWindowEvent->pcSource);
		if (strcmp(szName, "Left") == 0)
		{
			asDest[0].iRight = -(int)psWindowEvent->fValue;
		}
		else if (strcmp(szName, "Right") == 0)
		{
			asDest[0].iRight = (int)psWindowEvent->fValue;
		}
		else if (strcmp(szName, "Up") == 0)
		{
			asDest[0].iDown = -(int)psWindowEvent->fValue;
		}
		else if (strcmp(szName, "Down") == 0)
		{
			asDest[0].iDown = (int)psWindowEvent->fValue;
		}
		return 1;
	}

	bDown = psWindowEvent->fValue >= COMPONENT_INPUT_PRESSED;
	InitEvent(&asDest[0], bDown ? CIET_KeyDown : CIET_KeyUp, psWindowEvent);
	asDest[0].szName = mpcSource->GetName(psWindowEvent->pcSource);
	asDest[0].fValue = psWindowEvent->fValue;
	if (!pcDevice)
	{
		return 1;
	}

	if (bDown)
	{
		return Pressed(psWindowEvent, &asDest[1]) ? 2 : 1;
	}
	else
	{
		return Released(psWindowEvent, &asDest[1]) ? 2 : 1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A pressed source is, in order: a lock or insert, a numpad key standing
// in for another source, then nothing if control or alt is held (it would
// have been a chord), a command or a printable character.  Returns true if
// psDest is set.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::Pressed(SWindowInputEvent* psWindowEvent, SComponentInputEvent* psDest)
{
	CComponentInputKey*			psKey;
	CComponentInputNumKey*		psNumKey;
	CComponentInputCommand*		psCommand;
	CComponentInputCommand*		psReleased;
	CComponentInputPrintable*	psPrintable;
	CInputSourceDesc*			pcSource;
	CInputSourceDesc*			pcRemapped;

	pcSource = psWindowEvent->pcSource;
	psKey = GetKey(pcSource);
	if (psKey)
	{
		if (PressedKey(psKey))
		{
			return false;
		}
	}

	//With num lock off the numpad keys move.
	pcRemapped = pcSource;
	psNumKey = GetNumKeyRemap(pcSource);
	if (psNumKey)
	{
		pcRemapped = mbNumLock ? psNumKey->pcYesNumLockRemap : psNumKey->pcNoNumLockRemap;
		if (!pcRemapped)
		{
			return false;
		}
	}

	//With control or alt held a key is a chord or nothing.
	if (IsControl() || IsAlt())
	{
		return false;
	}

	psReleased = GetCommand(pcRemapped, false);
	AddPressed(pcSource, psReleased ? psReleased->eCommand : CIC_Ignored);

	psCommand = GetCommand(pcRemapped, true);
	if (psCommand)
	{
		if (psCommand->eCommand == CIC_Ignored)
		{
			return false;
		}
		InitEvent(psDest, CIET_Command, psWindowEvent);
		psDest->eCommand = psCommand->eCommand;
		return true;
	}

	psPrintable = GetPrintable(pcRemapped);
	if (psPrintable)
	{
		InitEvent(psDest, CIET_Printable, psWindowEvent);
		psDest->cPrintable = psPrintable->Get(IsShift(), mbCapsLock);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Returns true if psDest is set to the command letting go gives.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::Released(SWindowInputEvent* psWindowEvent, SComponentInputEvent* psDest)
{
	SComponentInputPressed		sPressed;

	if (!RemovePressed(psWindowEvent->pcSource, &sPressed))
	{
		//Pressed before this was listening, used up by a chord, or a lock.
		return false;
	}
	if (sPressed.eReleased == CIC_Ignored)
	{
		return false;
	}

	InitEvent(psDest, CIET_Command, psWindowEvent);
	psDest->eCommand = sPressed.eReleased;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Returns true if the key has been used up.  Shift, control and alt are
// read from the keyboard when they are needed.  Insert with control or
// shift is a chord so it never gets here.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::PressedKey(CComponentInputKey* psKey)
{
	switch (psKey->eKey)
	{
	case CIK_Shift:
	case CIK_Control:
	case CIK_Alt:
		return true;
	case CIK_CapsLock:
		mbCapsLock = !mbCapsLock;
		return true;
	case CIK_NumLock:
		mbNumLock = !mbNumLock;
		return true;
	case CIK_Insert:
		mbInsert = !mbInsert;
		return true;
	}
	return false;
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
	masPressed.ReInit();
	mpcSource->ForgetDown();
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
void CComponentInput::AddPressed(CInputSourceDesc* pcSource, EComponentInputCommand eReleased)
{
	SComponentInputPressed*	psPressed;
	SComponentInputPressed	sIgnored;

	//A source pressed twice without being released, keep the latest.
	RemovePressed(pcSource, &sIgnored);

	psPressed = masPressed.Add();
	psPressed->pcSource = pcSource;
	psPressed->eReleased = eReleased;
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
CComponentInputCommand* CComponentInput::GetCommand(CInputSourceDesc* pcSource, bool bPressed)
{
	CComponentInputCommand*		psCommand;
	size						i;
	size						uiNumElements;

	uiNumElements = macDevices.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		psCommand = macDevices.Get(i)->GetCommand(pcSource, bPressed);
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
// True if any keyboard has a key of this kind down.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::IsKeyDown(EComponentInputKey eKey)
{
	size	i;
	size	uiNumElements;

	uiNumElements = macDevices.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		if (macDevices.Get(i)->IsKeyDown(eKey))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the key or button with the friendly name, like "Left Shift" or
// "Button 1", is down on any keyboard or mouse added.  It is read from the
// devices, not the events, so it is down even while the chords hold back
// its event, as they do a modifier that could start a chord.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::IsSourceDown(const char* szFriendlyName)
{
	CComponentInputDevice*	pcDevice;
	CInputSourceDesc*		pcSource;
	size					i;
	size					uiNumElements;

	uiNumElements = macDevices.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcDevice = macDevices.Get(i);
		pcSource = pcDevice->GetSource((char*)szFriendlyName);
		if (pcSource && mpcSource->IsDown(pcDevice->GetDevice(), pcSource))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool		CComponentInput::IsShift(void) { return IsKeyDown(CIK_Shift); }
bool		CComponentInput::IsControl(void) { return IsKeyDown(CIK_Control); }
bool		CComponentInput::IsAlt(void) { return IsKeyDown(CIK_Alt); }
bool		CComponentInput::IsCapsLock(void) { return mbCapsLock; }
bool		CComponentInput::IsNumLock(void) { return mbNumLock; }
bool		CComponentInput::IsInsert(void) { return mbInsert; }
CWindowInputSource*	CComponentInput::GetSource(void) { return mpcSource; }


//////////////////////////////////////////////////////////////////////////
//
// The window the events go to, as when a modal dialog takes them from its
// owner.  Release everything first.
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
