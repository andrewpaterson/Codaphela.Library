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
#include "InputLib/InputDevice.h"
#include "ComponentInput.h"
#include "ComponentInputDevice.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputDevice::Init(CInputDevice* pcDevice, CComponentInput* pcComponentInput)
{
	mpcComponentInput = pcComponentInput;
	mpcDevice = pcDevice;
	mpcVirtual = pcDevice->CreateDefaultVirtualDeviceFromThis("Component Input");
	masPrintables.Init();
	masCommands.Init();
	masNumKeys.Init();
	masKeys.Init();
	mapcHooked.Init();
}


//////////////////////////////////////////////////////////////////////////
//
// The virtual device belongs to CInput so it can't be removed, it is
// disabled instead so its actions don't call a dead CComponentInput.
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputDevice::Kill(void)
{
	Disable();
	mapcHooked.Kill();
	masKeys.Kill();
	masNumKeys.Kill();
	masCommands.Kill();
	masPrintables.Kill();
	mpcVirtual = NULL;
	mpcDevice = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputDevice::InitKeyboard(void)
{
	AddKey("Left Shift", CIK_Shift);
	AddKey("Right Shift", CIK_Shift);
	AddKey("Left Control", CIK_Control);
	AddKey("Right Control", CIK_Control);
	AddKey("Left Alt", CIK_Alt);
	AddKey("Right Alt", CIK_Alt);
	AddKey("Caps Lock", CIK_CapsLock);
	AddKey("Num Lock", CIK_NumLock);
	AddKey("Insert", CIK_Insert);

	AddPrintable("1", '1', '!', '1', '!');
	AddPrintable("2", '2', '@', '2', '@');
	AddPrintable("3", '3', '#', '3', '#');
	AddPrintable("4", '4', '$', '4', '$');
	AddPrintable("5", '5', '%', '5', '%');
	AddPrintable("6", '6', '^', '6', '^');
	AddPrintable("7", '7', '&', '7', '&');
	AddPrintable("8", '8', '*', '8', '*');
	AddPrintable("9", '9', '(', '9', '(');
	AddPrintable("0", '0', ')', '0', ')');
	AddPrintable("`", '`', '~', '`', '~');
	AddPrintable("-", '-', '_', '-', '_');
	AddPrintable("=", '=', '+', '=', '+');
	AddPrintable("[", '[', '{', '[', '{');
	AddPrintable("]", ']', '}', ']', '}');
	AddPrintable("\\", '\\', '|', '\\', '|');
	AddPrintable(";", ';', ':', ';', ':');
	AddPrintable("\'", '\'', '\"', '\'', '\"');
	AddPrintable(",", ',', '<', ',', '<');
	AddPrintable(".", '.', '>', '.', '>');
	AddPrintable("/", '/', '?', '/', '?');
	AddPrintable("Q", 'q', 'Q', 'Q', 'q');
	AddPrintable("W", 'w', 'W', 'W', 'w');
	AddPrintable("E", 'e', 'E', 'E', 'e');
	AddPrintable("R", 'r', 'R', 'R', 'r');
	AddPrintable("T", 't', 'T', 'T', 't');
	AddPrintable("Y", 'y', 'Y', 'Y', 'y');
	AddPrintable("U", 'u', 'U', 'U', 'u');
	AddPrintable("I", 'i', 'I', 'I', 'i');
	AddPrintable("O", 'o', 'O', 'O', 'o');
	AddPrintable("P", 'p', 'P', 'P', 'p');
	AddPrintable("A", 'a', 'A', 'A', 'a');
	AddPrintable("S", 's', 'S', 'S', 's');
	AddPrintable("D", 'd', 'D', 'D', 'd');
	AddPrintable("F", 'f', 'F', 'F', 'f');
	AddPrintable("G", 'g', 'G', 'G', 'g');
	AddPrintable("H", 'h', 'H', 'H', 'h');
	AddPrintable("J", 'j', 'J', 'J', 'j');
	AddPrintable("K", 'k', 'K', 'K', 'k');
	AddPrintable("L", 'l', 'L', 'L', 'l');
	AddPrintable("Z", 'z', 'Z', 'Z', 'z');
	AddPrintable("X", 'x', 'X', 'X', 'x');
	AddPrintable("C", 'c', 'C', 'C', 'c');
	AddPrintable("V", 'v', 'V', 'V', 'v');
	AddPrintable("B", 'b', 'B', 'B', 'b');
	AddPrintable("N", 'n', 'N', 'N', 'n');
	AddPrintable("M", 'm', 'M', 'M', 'm');
	AddPrintable("Numpad /", '/', '/', '/', '/');
	AddPrintable("Numpad *", '*', '*', '*', '*');
	AddPrintable("Numpad -", '-', '-', '-', '-');
	AddPrintable("Numpad +", '+', '+', '+', '+');

	//Function keys don't type anything.
	AddCommand("F1", CIC_Ignored, false, false, false);
	AddCommand("F2", CIC_Ignored, false, false, false);
	AddCommand("F3", CIC_Ignored, false, false, false);
	AddCommand("F4", CIC_Ignored, false, false, false);
	AddCommand("F5", CIC_Ignored, false, false, false);
	AddCommand("F6", CIC_Ignored, false, false, false);
	AddCommand("F7", CIC_Ignored, false, false, false);
	AddCommand("F8", CIC_Ignored, false, false, false);
	AddCommand("F9", CIC_Ignored, false, false, false);
	AddCommand("F10", CIC_Ignored, false, false, false);
	AddCommand("F11", CIC_Ignored, false, false, false);
	AddCommand("F12", CIC_Ignored, false, false, false);

	AddCommand("Cursor Up", CIC_Up, false, false, false);
	AddCommand("Cursor Down", CIC_Down, false, false, false);
	AddCommand("Cursor Left", CIC_Left, false, false, false);
	AddCommand("Cursor Right", CIC_Right, false, false, false);
	AddCommand("Cursor Left", CIC_WordLeft, true, false, false);
	AddCommand("Cursor Right", CIC_WordRight, true, false, false);
	AddCommand("Home", CIC_Home, false, false, false);
	AddCommand("End", CIC_End, false, false, false);
	AddCommand("Home", CIC_DocumentHome, true, false, false);
	AddCommand("End", CIC_DocumentEnd, true, false, false);
	AddCommand("Page Up", CIC_PageUp, false, false, false);
	AddCommand("Page Down", CIC_PageDown, false, false, false);
	AddCommand("Cursor Up", CIC_LineUp, true, false, false);
	AddCommand("Cursor Down", CIC_LineDown, true, false, false);

	AddCommand("Cursor Up", CIC_SelectUp, false, false, true);
	AddCommand("Cursor Down", CIC_SelectDown, false, false, true);
	AddCommand("Cursor Left", CIC_SelectLeft, false, false, true);
	AddCommand("Cursor Right", CIC_SelectRight, false, false, true);
	AddCommand("Cursor Left", CIC_SelectWordLeft, true, false, true);
	AddCommand("Cursor Right", CIC_SelectWordRight, true, false, true);
	AddCommand("Home", CIC_SelectHome, false, false, true);
	AddCommand("End", CIC_SelectEnd, false, false, true);
	AddCommand("Home", CIC_SelectDocumentHome, true, false, true);
	AddCommand("End", CIC_SelectDocumentEnd, true, false, true);
	AddCommand("Page Up", CIC_SelectPageUp, false, false, true);
	AddCommand("Page Down", CIC_SelectPageDown, false, false, true);
	AddCommand("W", CIC_SelectWholeWord, true, false, false);
	AddCommand("A", CIC_SelectAll, true, false, false);
	AddCommand("Escape", CIC_ClearSelection, false, false, false);

	AddCommand("Space", CIC_Space, false, false, false);
	AddCommand("Space", CIC_Space, false, false, true);
	AddCommand("Tab", CIC_Tab, false, false, false);
	AddCommand("Tab", CIC_BackTab, false, false, true);
	AddCommand("Enter", CIC_Enter, false, false, false);
	AddCommand("Numpad Enter", CIC_Enter, false, false, false);
	AddCommand("Enter", CIC_HomeEnter, true, false, true);
	AddCommand("Numpad Enter", CIC_HomeEnter, true, false, true);
	AddCommand("Enter", CIC_EndEnter, false, false, true);
	AddCommand("Numpad Enter", CIC_EndEnter, false, false, true);

	AddCommand("Delete", CIC_Delete, false, false, false);
	AddCommand("Backspace", CIC_Backspace, false, false, false);
	AddCommand("X", CIC_Cut, true, false, false);
	AddCommand("Delete", CIC_Cut, false, false, true);
	AddCommand("C", CIC_Copy, true, false, false);
	AddCommand("Insert", CIC_Copy, true, false, false);
	AddCommand("V", CIC_Paste, true, false, false);
	AddCommand("Insert", CIC_Paste, false, false, true);
	AddCommand("D", CIC_Duplicate, true, false, false);
	AddCommand("Backspace", CIC_DeleteWordRemainingLeft, true, false, false);
	AddCommand("Delete", CIC_DeleteWordRemainingRight, true, false, false);
	AddCommand("Y", CIC_DeleteLine, true, false, false);

	AddNumKeyRemap("Numpad 1", "1", "End");
	AddNumKeyRemap("Numpad 2", "2", "Cursor Down");
	AddNumKeyRemap("Numpad 3", "3", "Page Down");
	AddNumKeyRemap("Numpad 4", "4", "Cursor Left");
	AddNumKeyRemap("Numpad 5", "5", NULL);
	AddNumKeyRemap("Numpad 6", "6", "Cursor Right");
	AddNumKeyRemap("Numpad 7", "7", "Home");
	AddNumKeyRemap("Numpad 8", "8", "Cursor Up");
	AddNumKeyRemap("Numpad 9", "9", "Page Up");
	AddNumKeyRemap("Numpad 0", "0", "Delete");
	AddNumKeyRemap("Numpad .", ".", "Delete");
}


//////////////////////////////////////////////////////////////////////////
//
// Only the buttons, the pointer position comes from the native window.
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputDevice::InitMouse(void)
{
	AddCommand("Button 1", CIC_ActivateComponent, false, false, false, true);
	AddCommand("Button 1", CIC_ExecuteComponent, false, false, false, false);
	AddCommand("Button 1", CIC_StartDragSelect, false, false, true, true);
	AddCommand("Button 1", CIC_StopDragSelect, false, false, true, false);
	AddCommand("Button 2", CIC_ContextMenu, false, false, false, true);
	AddCommand("Button 3", CIC_StartDragScroll, false, false, false, true);
	AddCommand("Button 3", CIC_StopDragScroll, false, false, false, false);
	AddCommand("Wheel Forward", CIC_WheelUp, false, false, false, true);
	AddCommand("Wheel Backward", CIC_WheelDown, false, false, false, true);
	AddCommand("Wheel Forward", CIC_WheelLeft, false, false, true, true);
	AddCommand("Wheel Backward", CIC_WheelRight, false, false, true, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputDevice::Enable(void)
{
	mpcVirtual->Enable();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputDevice::Disable(void)
{
	if (mpcVirtual)
	{
		mpcVirtual->Disable();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the pressed and released actions for the source the first time it
// is used.  Returns NULL if the device doesn't have the source.
//
//////////////////////////////////////////////////////////////////////////
CInputSourceDesc* CComponentInputDevice::Hook(char* szFriendlyName)
{
	CInputVirtualDeviceSource*	pcSource;
	size						i;
	size						uiNumElements;

	pcSource = mpcVirtual->GetSource(szFriendlyName);
	if (!pcSource)
	{
		return NULL;
	}

	uiNumElements = mapcHooked.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		if (*mapcHooked.Get(i) == pcSource)
		{
			return pcSource->GetSourceDesc();
		}
	}

	mpcVirtual->AddActiveAction(mpcComponentInput->GetButtonPressedAction(), pcSource);
	mpcVirtual->AddInactiveAction(mpcComponentInput->GetButtonReleasedAction(), pcSource);
	mapcHooked.Add(pcSource);
	return pcSource->GetSourceDesc();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputSourceDesc* CComponentInputDevice::AddCommand(char* szFriendlyName, EComponentInputCommand eCommand, bool bControl, bool bAlt, bool bShift, bool bPressed)
{
	CInputSourceDesc*			pcSource;
	CComponentInputCommand*		psCommand;

	pcSource = Hook(szFriendlyName);
	if (pcSource)
	{
		psCommand = masCommands.Add();
		psCommand->Init(eCommand, pcSource, bControl, bAlt, bShift, bPressed);
	}
	return pcSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputSourceDesc* CComponentInputDevice::AddPrintable(char* szFriendlyName, char cNoShiftNoCaps, char cYesShiftNoCaps, char cNoShiftYesCaps, char cYesShiftYesCaps)
{
	CInputSourceDesc*			pcSource;
	CComponentInputPrintable*	psPrintable;

	pcSource = Hook(szFriendlyName);
	if (pcSource)
	{
		psPrintable = masPrintables.Add();
		psPrintable->Init(pcSource, cNoShiftNoCaps, cYesShiftNoCaps, cNoShiftYesCaps, cYesShiftYesCaps);
	}
	return pcSource;
}


//////////////////////////////////////////////////////////////////////////
//
// The remaps are other sources on the same device.  They only need their
// commands and printables added, the numpad key is the one pressed.
//
//////////////////////////////////////////////////////////////////////////
CInputSourceDesc* CComponentInputDevice::AddNumKeyRemap(char* szFriendlyName, char* szYesNumLockRemap, char* szNoNumLockRemap)
{
	CInputSourceDesc*			pcSource;
	CComponentInputNumKey*		psNumKey;
	CInputSourceDesc*			pcYesNumLockRemap;
	CInputSourceDesc*			pcNoNumLockRemap;

	pcSource = Hook(szFriendlyName);
	if (pcSource)
	{
		pcYesNumLockRemap = szYesNumLockRemap ? mpcDevice->GetSource(szYesNumLockRemap) : NULL;
		pcNoNumLockRemap = szNoNumLockRemap ? mpcDevice->GetSource(szNoNumLockRemap) : NULL;
		psNumKey = masNumKeys.Add();
		psNumKey->Init(pcSource, pcYesNumLockRemap, pcNoNumLockRemap);
	}
	return pcSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputSourceDesc* CComponentInputDevice::AddKey(char* szFriendlyName, EComponentInputKey eKey)
{
	CInputSourceDesc*		pcSource;
	CComponentInputKey*		psKey;

	pcSource = Hook(szFriendlyName);
	if (pcSource)
	{
		psKey = masKeys.Add();
		psKey->Init(pcSource, eKey);
	}
	return pcSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponentInputPrintable* CComponentInputDevice::GetPrintable(CInputSourceDesc* pcSource)
{
	CComponentInputPrintable*	psPrintable;
	size						i;
	size						uiNumElements;

	uiNumElements = masPrintables.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		psPrintable = masPrintables.Get(i);
		if (psPrintable->pcSource == pcSource)
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
CComponentInputCommand* CComponentInputDevice::GetCommand(CInputSourceDesc* pcSource, bool bControl, bool bAlt, bool bShift, bool bPressed)
{
	CComponentInputCommand*		psCommand;
	size						i;
	size						uiNumElements;

	uiNumElements = masCommands.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		psCommand = masCommands.Get(i);
		if (psCommand->pcSource == pcSource)
		{
			if ((psCommand->bAlt == bAlt) && (psCommand->bControl == bControl) && (psCommand->bShift == bShift) && (psCommand->bPressed == bPressed))
			{
				return psCommand;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponentInputNumKey* CComponentInputDevice::GetNumKeyRemap(CInputSourceDesc* pcSource)
{
	CComponentInputNumKey*		psNumKey;
	size						i;
	size						uiNumElements;

	uiNumElements = masNumKeys.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		psNumKey = masNumKeys.Get(i);
		if (psNumKey->pcSource == pcSource)
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
CComponentInputKey* CComponentInputDevice::GetKey(CInputSourceDesc* pcSource)
{
	CComponentInputKey*		psKey;
	size					i;
	size					uiNumElements;

	uiNumElements = masKeys.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		psKey = masKeys.Get(i);
		if (psKey->pcSource == pcSource)
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
CInputDevice* CComponentInputDevice::GetDevice(void) { return mpcDevice; }

