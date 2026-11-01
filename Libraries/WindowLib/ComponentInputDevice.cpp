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
	masPrintables.Init();
	masCommands.Init();
	masNumKeys.Init();
	masKeys.Init();
	mbMouse = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputDevice::Kill(void)
{
	masKeys.Kill();
	masNumKeys.Kill();
	masCommands.Kill();
	masPrintables.Kill();
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
	AddCommand("F1", CIC_Ignored);
	AddCommand("F2", CIC_Ignored);
	AddCommand("F3", CIC_Ignored);
	AddCommand("F4", CIC_Ignored);
	AddCommand("F5", CIC_Ignored);
	AddCommand("F6", CIC_Ignored);
	AddCommand("F7", CIC_Ignored);
	AddCommand("F8", CIC_Ignored);
	AddCommand("F9", CIC_Ignored);
	AddCommand("F10", CIC_Ignored);
	AddCommand("F11", CIC_Ignored);
	AddCommand("F12", CIC_Ignored);

	//The keys alone.  With control or shift they are chords, see CComponentInput's chord commands.
	AddCommand("Cursor Up", CIC_Up);
	AddCommand("Cursor Down", CIC_Down);
	AddCommand("Cursor Left", CIC_Left);
	AddCommand("Cursor Right", CIC_Right);
	AddCommand("Home", CIC_Home);
	AddCommand("End", CIC_End);
	AddCommand("Page Up", CIC_PageUp);
	AddCommand("Page Down", CIC_PageDown);
	AddCommand("Escape", CIC_ClearSelection);

	AddCommand("Space", CIC_Space);
	AddCommand("Tab", CIC_Tab);
	AddCommand("Enter", CIC_Enter);
	AddCommand("Numpad Enter", CIC_Enter);

	AddCommand("Delete", CIC_Delete);
	AddCommand("Backspace", CIC_Backspace);

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
// Button one with shift, control or both drags a selection; those are
// chords, see CComponentInput's chord commands.
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputDevice::InitMouse(void)
{
	mbMouse = true;
	AddCommand("Button 1", CIC_ActivateComponent, true);
	AddCommand("Button 1", CIC_ExecuteComponent, false);
	AddCommand("Button 2", CIC_ContextMenu, true);
	AddCommand("Button 3", CIC_StartDragScroll, true);
	AddCommand("Button 3", CIC_StopDragScroll, false);
	AddCommand("Wheel Forward", CIC_WheelUp, true);
	AddCommand("Wheel Backward", CIC_WheelDown, true);
}


//////////////////////////////////////////////////////////////////////////
//
// Returns NULL if the device doesn't have the source.
//
//////////////////////////////////////////////////////////////////////////
CInputSourceDesc* CComponentInputDevice::GetSource(char* szFriendlyName)
{
	return mpcComponentInput->GetSource()->GetSource(mpcDevice, szFriendlyName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputSourceDesc* CComponentInputDevice::AddCommand(char* szFriendlyName, EComponentInputCommand eCommand, bool bPressed)
{
	CInputSourceDesc*			pcSource;
	CComponentInputCommand*		psCommand;

	pcSource = GetSource(szFriendlyName);
	if (pcSource)
	{
		psCommand = masCommands.Add();
		psCommand->Init(eCommand, pcSource, bPressed);
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

	pcSource = GetSource(szFriendlyName);
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

	pcSource = GetSource(szFriendlyName);
	if (pcSource)
	{
		pcYesNumLockRemap = szYesNumLockRemap ? GetSource(szYesNumLockRemap) : NULL;
		pcNoNumLockRemap = szNoNumLockRemap ? GetSource(szNoNumLockRemap) : NULL;
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

	pcSource = GetSource(szFriendlyName);
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
CComponentInputCommand* CComponentInputDevice::GetCommand(CInputSourceDesc* pcSource, bool bPressed)
{
	CComponentInputCommand*		psCommand;
	size						i;
	size						uiNumElements;

	uiNumElements = masCommands.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		psCommand = masCommands.Get(i);
		if ((psCommand->pcSource == pcSource) && (psCommand->bPressed == bPressed))
		{
			return psCommand;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// True if a key of this kind on the device is down.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInputDevice::IsKeyDown(EComponentInputKey eKey)
{
	CComponentInputKey*		psKey;
	size					i;
	size					uiNumElements;

	uiNumElements = masKeys.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		psKey = masKeys.Get(i);
		if ((psKey->eKey == eKey) && mpcComponentInput->GetSource()->IsDown(mpcDevice, psKey->pcSource))
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
bool CComponentInputDevice::IsMouse(void) { return mbMouse; }

