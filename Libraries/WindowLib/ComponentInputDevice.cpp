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
	masPressedCommands.Init();
	masReleasedCommands.Init();
	masNumKeys.Init();
	masKeys.Init();
	maszChords.Init();
	mbMouse = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputDevice::Kill(void)
{
	maszChords.Kill();
	masKeys.Kill();
	masNumKeys.Kill();
	masReleasedCommands.Kill();
	masPressedCommands.Kill();
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

	//Function keys don't type anything; an application binds its own (see CComponentInput::BindKeys).  As in
	//IntelliJ IDEA F7 goes to the next difference in a diff.
	AddCommand("F1", CIC_Ignored);
	AddCommand("F2", CIC_Ignored);
	AddCommand("F3", CIC_Ignored);
	AddCommand("F4", CIC_Ignored);
	AddCommand("F5", CIC_Ignored);
	AddCommand("F6", CIC_Ignored);
	AddCommand("F7", CIC_NextDifference);
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
	AddCommand("Menu", CIC_ContextMenuKey);

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
	AddNumKeyRemap("Numpad 0", "0", "Insert");
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
CInputSourceDesc* CComponentInputDevice::GetSource(const char* szFriendlyName)
{
	return mpcComponentInput->GetSource()->GetSource(mpcDevice, szFriendlyName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputSourceDesc* CComponentInputDevice::AddCommand(const char* szFriendlyName, EComponentInputCommand eCommand, bool bPressed)
{
	CInputSourceDesc*			pcSource;
	CComponentInputCommand*		psCommand;

	pcSource = GetSource(szFriendlyName);
	if (pcSource)
	{
		psCommand = bPressed ? masPressedCommands.Add() : masReleasedCommands.Add();
		psCommand->Init(eCommand, pcSource);
	}
	return pcSource;
}


//////////////////////////////////////////////////////////////////////////
//
// Pressing the key gives eCommand, instead of whatever it gave before.
// NULL if the keyboard has no such key.
//
//////////////////////////////////////////////////////////////////////////
CInputSourceDesc* CComponentInputDevice::SetCommand(const char* szFriendlyName, EComponentInputCommand eCommand)
{
	CInputSourceDesc*			pcSource;
	CComponentInputCommand*		psCommand;

	pcSource = GetSource(szFriendlyName);
	if (pcSource)
	{
		psCommand = GetPressedCommand(pcSource);
		if (psCommand)
		{
			psCommand->Init(eCommand, pcSource);
		}
		else
		{
			masPressedCommands.Add()->Init(eCommand, pcSource);
		}
	}
	return pcSource;
}


//////////////////////////////////////////////////////////////////////////
//
// Pressing the key gives no command, so it types what it types again.
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputDevice::RemoveCommand(const char* szFriendlyName)
{
	CInputSourceDesc*			pcSource;
	CComponentInputCommand*		psCommand;

	pcSource = GetSource(szFriendlyName);
	psCommand = pcSource ? GetPressedCommand(pcSource) : NULL;
	if (psCommand)
	{
		masPressedCommands.Remove(psCommand);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// No key pressed gives eCommand.  The keys stay, giving nothing.
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputDevice::RemoveCommands(EComponentInputCommand eCommand)
{
	size	i;

	for (i = 0; i < masPressedCommands.NumElements(); i++)
	{
		if (masPressedCommands.Get(i)->meCommand == eCommand)
		{
			masPressedCommands.Get(i)->Init(CIC_Ignored, masPressedCommands.Get(i)->mpcSource);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The chord named szName has been made with this keyboard's modifiers
// (CComponentInput::AddChordCommands), so it isn't made again.
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputDevice::AddChord(const char* szName)
{
	if (!HasChord(szName))
	{
		maszChords.Add(szName);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInputDevice::HasChord(const char* szName)
{
	return maszChords.Contains(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputSourceDesc* CComponentInputDevice::AddPrintable(const char* szFriendlyName, char cNoShiftNoCaps, char cYesShiftNoCaps, char cNoShiftYesCaps, char cYesShiftYesCaps)
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
CInputSourceDesc* CComponentInputDevice::AddNumKeyRemap(const char* szFriendlyName, const char* szYesNumLockRemap, const char* szNoNumLockRemap)
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
CInputSourceDesc* CComponentInputDevice::AddKey(const char* szFriendlyName, EComponentInputKey eKey)
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
		if ((psKey->meKey == eKey) && mpcComponentInput->GetSource()->IsDown(mpcDevice, psKey->mpcSource))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The first of the device's sources that gives eCommand when it is
// pressed on its own, NULL if none does.
//
//////////////////////////////////////////////////////////////////////////
CInputSourceDesc* CComponentInputDevice::FindPressedSource(EComponentInputCommand eCommand)
{
	size	i;

	for (i = 0; i < masPressedCommands.NumElements(); i++)
	{
		if (masPressedCommands.Get(i)->meCommand == eCommand)
		{
			return masPressedCommands.Get(i)->mpcSource;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponentInputPrintable* CComponentInputDevice::GetPrintable(CInputSourceDesc* pcSource) { return FindComponentInputMeaning(&masPrintables, pcSource); }
CComponentInputCommand* CComponentInputDevice::GetPressedCommand(CInputSourceDesc* pcSource) { return FindComponentInputMeaning(&masPressedCommands, pcSource); }
CComponentInputCommand* CComponentInputDevice::GetReleasedCommand(CInputSourceDesc* pcSource) { return FindComponentInputMeaning(&masReleasedCommands, pcSource); }
CComponentInputNumKey* CComponentInputDevice::GetNumKeyRemap(CInputSourceDesc* pcSource) { return FindComponentInputMeaning(&masNumKeys, pcSource); }
CComponentInputKey* CComponentInputDevice::GetKey(CInputSourceDesc* pcSource) { return FindComponentInputMeaning(&masKeys, pcSource); }
CInputDevice* CComponentInputDevice::GetDevice(void) { return mpcDevice; }
bool CComponentInputDevice::IsMouse(void) { return mbMouse; }

