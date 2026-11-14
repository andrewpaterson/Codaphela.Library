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
#include "BaseLib/StringHelper.h"
#include "Window.h"
#include "ComponentInput.h"


#define COMPONENT_INPUT_PRESSED		0.5f	//A key or button's value at or above which it is pressed.


//The keys that mean something with control, shift and alt held.  Each is a chord named for its modifiers and key,
//like "Control+C" or "Control+Alt+Shift+N", made for every left and right modifier the keyboard has.
struct SComponentInputChordRow
{
	bool					bControl;
	bool					bShift;
	bool					bAlt;
	const char*				szKey;
	EComponentInputCommand	ePressed;
	EComponentInputCommand	eReleased;  //When the chord's key is let go, CIC_Ignored for nothing.
};


static SComponentInputChordRow	gasKeyboardChordRows[] =
{
	{ true,  false, false, "Cursor Left",	CIC_WordLeft,					CIC_Ignored },
	{ true,  false, false, "Cursor Right",	CIC_WordRight,					CIC_Ignored },
	{ true,  false, false, "Home",			CIC_DocumentHome,				CIC_Ignored },
	{ true,  false, false, "End",			CIC_DocumentEnd,				CIC_Ignored },
	{ true,  false, false, "Cursor Up",	CIC_LineUp,						CIC_Ignored },
	{ true,  false, false, "Cursor Down",	CIC_LineDown,					CIC_Ignored },
	{ false, true,  false, "Cursor Up",	CIC_SelectUp,					CIC_Ignored },
	{ false, true,  false, "Cursor Down",	CIC_SelectDown,					CIC_Ignored },
	{ false, true,  false, "Cursor Left",	CIC_SelectLeft,					CIC_Ignored },
	{ false, true,  false, "Cursor Right",	CIC_SelectRight,				CIC_Ignored },
	{ true,  true,  false, "Cursor Left",	CIC_SelectWordLeft,				CIC_Ignored },
	{ true,  true,  false, "Cursor Right",	CIC_SelectWordRight,			CIC_Ignored },
	{ false, true,  false, "Home",			CIC_SelectHome,					CIC_Ignored },
	{ false, true,  false, "End",			CIC_SelectEnd,					CIC_Ignored },
	{ true,  true,  false, "Home",			CIC_SelectDocumentHome,			CIC_Ignored },
	{ true,  true,  false, "End",			CIC_SelectDocumentEnd,			CIC_Ignored },
	{ false, true,  false, "Page Up",		CIC_SelectPageUp,				CIC_Ignored },
	{ false, true,  false, "Page Down",	CIC_SelectPageDown,				CIC_Ignored },
	{ true,  false, false, "W",			CIC_ExtendSelection,			CIC_Ignored },
	{ true,  true,  false, "W",			CIC_ShrinkSelection,			CIC_Ignored },
	{ false, true,  true,  "Cursor Up",	CIC_BoxSelectUp,				CIC_Ignored },
	{ false, true,  true,  "Cursor Down",	CIC_BoxSelectDown,				CIC_Ignored },
	{ false, true,  true,  "Cursor Left",	CIC_BoxSelectLeft,				CIC_Ignored },
	{ false, true,  true,  "Cursor Right",	CIC_BoxSelectRight,				CIC_Ignored },
	{ false, true,  true,  "Home",			CIC_BoxSelectHome,				CIC_Ignored },
	{ false, true,  true,  "End",			CIC_BoxSelectEnd,				CIC_Ignored },
	{ true,  false, false, "A",			CIC_SelectAll,					CIC_Ignored },
	{ false, true,  false, "Space",		CIC_Space,						CIC_Ignored },
	{ false, true,  false, "Tab",			CIC_BackTab,					CIC_Ignored },
	{ true,  true,  false, "Enter",		CIC_HomeEnter,					CIC_Ignored },
	{ true,  true,  false, "Numpad Enter",	CIC_HomeEnter,					CIC_Ignored },
	{ false, true,  false, "Enter",		CIC_EndEnter,					CIC_Ignored },
	{ false, true,  false, "Numpad Enter",	CIC_EndEnter,					CIC_Ignored },
	{ true,  false, false, "X",			CIC_Cut,						CIC_Ignored },
	{ false, true,  false, "Delete",		CIC_Cut,						CIC_Ignored },
	{ true,  false, false, "C",			CIC_Copy,						CIC_Ignored },
	{ true,  false, false, "Insert",		CIC_Copy,						CIC_Ignored },
	{ true,  false, false, "V",			CIC_Paste,						CIC_Ignored },
	{ false, true,  false, "Insert",		CIC_Paste,						CIC_Ignored },
	{ true,  true,  false, "V",			CIC_PasteFromHistory,			CIC_Ignored },
	{ true,  false, false, "D",			CIC_Duplicate,					CIC_Ignored },
	{ true,  false, false, "Backspace",	CIC_DeleteWordRemainingLeft,	CIC_Ignored },
	{ true,  false, false, "Delete",		CIC_DeleteWordRemainingRight,	CIC_Ignored },
	{ true,  false, false, "Z",			CIC_Undo,						CIC_Ignored },
	{ true,  true,  false, "Z",			CIC_Redo,						CIC_Ignored },
	{ true,  false, false, "Y",			CIC_DeleteLine,					CIC_Ignored },
	{ false, true,  false, "F7",			CIC_PreviousDifference,			CIC_Ignored },
	{ false, true,  false, "F10",			CIC_ContextMenuKey,				CIC_Ignored },
	{ true,  false, false, "F4",			CIC_CloseDocument,				CIC_Ignored },
	{ false, true,  false, "Escape",		CIC_HideToolWindow,				CIC_Ignored },
};


//The keyboard's modifiers with the mouse's buttons.
static SComponentInputChordRow	gasMouseChordRows[] =
{
	{ false, true,  false, "Button 1",			CIC_StartDragSelect,	CIC_StopDragSelect },
	{ true,  false, false, "Button 1",			CIC_StartDragSelect,	CIC_StopDragSelect },
	{ true,  true,  false, "Button 1",			CIC_StartDragSelect,	CIC_StopDragSelect },
	{ false, true,  true,  "Button 1",			CIC_StartDragBox,		CIC_StopDragSelect },
	{ false, false, true,  "Button 1",			CIC_StartDragBox,		CIC_StopDragSelect },
	{ false, true,  false, "Wheel Forward",	CIC_WheelLeft,			CIC_Ignored },
	{ false, true,  false, "Wheel Backward",	CIC_WheelRight,			CIC_Ignored },
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
	masBindings.Init();
	masContexts.Init();
	AddContext(COMPONENT_INPUT_EDITOR_CONTEXT, NULL, false, true);

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

	uiNumElements = masBindings.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		KillBinding(masBindings.Get(i));
	}
	masBindings.Kill();

	uiNumElements = masContexts.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		KillContext(masContexts.Get(i));
	}
	masContexts.Kill();

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
	size					j;
	size					uiNumElements;

	pcInputDevice = AddDevice(pcDevice);
	if (!pcInputDevice)
	{
		return false;
	}

	pcInputDevice->InitKeyboard();
	AddKeyboardChords(pcInputDevice);
	for (i = 0; i < masBindings.NumElements(); i++)
	{
		ApplyBinding(pcInputDevice, masBindings.Get(i));
	}
	for (i = 0; i < masContexts.NumElements(); i++)
	{
		for (j = 0; j < masContexts.Get(i)->asBindings.NumElements(); j++)
		{
			ApplyContextBinding(pcInputDevice, masContexts.Get(i)->asBindings.Get(j));
		}
	}

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
size CComponentInput::AddKeyboards(const char* szCategory)
{
	return AddDevices(szCategory, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CComponentInput::AddMice(const char* szCategory)
{
	return AddDevices(szCategory, false);
}


//////////////////////////////////////////////////////////////////////////
//
// Returns how many devices were added.
//
//////////////////////////////////////////////////////////////////////////
size CComponentInput::AddDevices(const char* szCategory, bool bKeyboard)
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
		AddChordCommands(pcKeyboard, pcKeyboard, psRow->bControl, psRow->bShift, psRow->bAlt, psRow->szKey, psRow->ePressed, psRow->eReleased);
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
		AddChordCommands(pcKeyboard, pcMouse, psRow->bControl, psRow->bShift, psRow->bAlt, psRow->szKey, psRow->ePressed, psRow->eReleased);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A chord for each of the keyboard's left and right controls, shifts and
// alts that the row wants, then szKey on pcKeyDevice.  They share one
// name and command.  A modifier the keyboard doesn't have makes no
// chord; without its shift a control and shift chord would be its
// control chord too.
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::AddChordCommands(CComponentInputDevice* pcKeyboard, CComponentInputDevice* pcKeyDevice, bool bControl, bool bShift, bool bAlt, const char* szKey, EComponentInputCommand ePressed, EComponentInputCommand eReleased)
{
	CInputSourceDesc*	aapcModifiers[3][2];  //Left and right of each modifier the row wants.
	CInputSourceDesc*	apcChord[3];
	CInputSourceDesc*	pcKey;
	CChars				szName;
	size				uiNumModifiers;
	size				uiSides;
	size				i;

	pcKey = pcKeyDevice->GetSource(szKey);
	if (!pcKey)
	{
		return;
	}

	szName.Init();
	uiNumModifiers = 0;
	if (bControl)
	{
		szName.Append("Control+");
		aapcModifiers[uiNumModifiers][0] = pcKeyboard->GetSource("Left Control");
		aapcModifiers[uiNumModifiers][1] = pcKeyboard->GetSource("Right Control");
		uiNumModifiers++;
	}
	if (bAlt)
	{
		szName.Append("Alt+");
		aapcModifiers[uiNumModifiers][0] = pcKeyboard->GetSource("Left Alt");
		aapcModifiers[uiNumModifiers][1] = pcKeyboard->GetSource("Right Alt");
		uiNumModifiers++;
	}
	if (bShift)
	{
		szName.Append("Shift+");
		aapcModifiers[uiNumModifiers][0] = pcKeyboard->GetSource("Left Shift");
		aapcModifiers[uiNumModifiers][1] = pcKeyboard->GetSource("Right Shift");
		uiNumModifiers++;
	}
	szName.Append(szKey);

	//Every choice of left or right for each modifier: bit i of uiSides is the side of modifier i.  A keyboard has each
	//chord once: an application's binding on a standard chord only changes its command.
	for (uiSides = 0; (uiSides < ((size)1 << uiNumModifiers)) && !pcKeyboard->HasChord(szName.Text()); uiSides++)
	{
		for (i = 0; i < uiNumModifiers; i++)
		{
			apcChord[i] = aapcModifiers[i][(uiSides >> i) & 1];
			if (!apcChord[i])
			{
				break;
			}
		}
		if (i == uiNumModifiers)
		{
			mpcSource->AddKeyChord(szName.Text(), pcKeyboard->GetDevice(), apcChord, uiNumModifiers, pcKeyDevice->GetDevice(), pcKey);
		}
	}
	pcKeyboard->AddChord(szName.Text());

	AddChordCommand(szName.Text(), ePressed, eReleased);
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A chord, named like "Control+Shift+F, H", for each of the keyboard's
// left and right controls with each of its shifts, and for each of them
// let go or held for the second key.  They share one name and command.
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::AddSequenceChordCommands(CComponentInputDevice* pcKeyboard, const char* szFirstKey, const char* szSecondKey, EComponentInputCommand eCommand, uint32 uiEvictionMilliseconds)
{
	CInputSourceDesc*	apcControls[2];
	CInputSourceDesc*	apcShifts[2];
	CInputSourceDesc*	pcFirstKey;
	CInputSourceDesc*	pcSecondKey;
	CChars				szName;
	size				i;
	size				j;
	size				uiLetGo;

	pcFirstKey = pcKeyboard->GetSource(szFirstKey);
	pcSecondKey = pcKeyboard->GetSource(szSecondKey);
	if (!pcFirstKey || !pcSecondKey)
	{
		return;
	}

	apcControls[0] = pcKeyboard->GetSource("Left Control");
	apcControls[1] = pcKeyboard->GetSource("Right Control");
	apcShifts[0] = pcKeyboard->GetSource("Left Shift");
	apcShifts[1] = pcKeyboard->GetSource("Right Shift");

	szName.Init("Control+Shift+");
	szName.Append(szFirstKey);
	szName.Append(", ");
	szName.Append(szSecondKey);

	for (i = 0; (i < 2) && !pcKeyboard->HasChord(szName.Text()); i++)
	{
		for (j = 0; j < 2; j++)
		{
			if (apcControls[i] && apcShifts[j])
			{
				for (uiLetGo = 0; uiLetGo < 4; uiLetGo++)
				{
					mpcSource->AddKeySequenceChord(szName.Text(), pcKeyboard->GetDevice(), apcControls[i], apcShifts[j], pcFirstKey, pcSecondKey, (uiLetGo & 1) != 0, (uiLetGo & 2) != 0, uiEvictionMilliseconds);
				}
			}
		}
	}

	pcKeyboard->AddChord(szName.Text());
	AddChordCommand(szName.Text(), eCommand, CIC_Ignored);
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The command of the chords named szName, unless they have one.
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::AddChordCommand(const char* szName, EComponentInputCommand ePressed, EComponentInputCommand eReleased)
{
	SComponentInputChordCommand*	psCommand;

	if (!GetChordCommand(szName))
	{
		psCommand = masChordCommands.Add();
		psCommand->szName.Init(szName);
		psCommand->ePressed = ePressed;
		psCommand->eReleased = eReleased;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The chord gives ePressed, instead of whatever it gave before.
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::SetChordCommand(const char* szName, EComponentInputCommand ePressed)
{
	SComponentInputChordCommand*	psCommand;

	psCommand = GetChordCommand(szName);
	if (psCommand)
	{
		psCommand->ePressed = ePressed;
	}
	else
	{
		AddChordCommand(szName, ePressed, CIC_Ignored);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The application's keys for eCommand in szContext (NULL or "Global" for
// the global keys, instead of any standard command on the same keys): one
// key ("F5"), a chord of control, alt and shift with a key
// ("Control+Shift+B", "Alt+Backspace"), or a key after a control and
// shift chord ("Control+Shift+F, H").  Every keyboard has them, those
// added later too.  False, logged, if szKeys can't be read, the context
// isn't there, or they type a character and the context isn't a typing
// one.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::BindKeys(const char* szKeys, EComponentInputCommand eCommand, const char* szContext)
{
	SComponentInputContext*	psContext;

	if (StrEmpty(szContext) || (StringCompare(szContext, COMPONENT_INPUT_GLOBAL_CONTEXT) == 0))
	{
		return BindGlobalKeys(szKeys, eCommand);
	}
	psContext = GetContext(szContext);
	if (psContext == NULL)
	{
		return gcLogger.Error2(__METHOD__, " Keys [", szKeys, "] can't be bound: there's no context [", szContext, "].", NULL);
	}
	return BindContextKeys(szKeys, eCommand, psContext);
}


//////////////////////////////////////////////////////////////////////////
//
// See BindKeys.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::BindGlobalKeys(const char* szKeys, EComponentInputCommand eCommand)
{
	SComponentInputBinding	sParsed;
	SComponentInputBinding*	psBinding;
	CComponentInputDevice*	pcDevice;
	size					i;

	if (!ParseKeys(szKeys, &sParsed))
	{
		return gcLogger.Error2(__METHOD__, " Keys [", szKeys, "] can't be bound: give a key, a chord like Control+Shift+B, or Control+Shift+F, H.", NULL);
	}

	psBinding = GetBinding(sParsed.szKeys.Text());
	if (psBinding)
	{
		psBinding->eCommand = eCommand;
		KillBinding(&sParsed);
	}
	else
	{
		sParsed.eCommand = eCommand;
		sParsed.eReplaced = CIC_Ignored;
		sParsed.bReplacedKnown = false;
		sParsed.bReplacedKey = false;
		psBinding = masBindings.Add();
		*psBinding = sParsed;
	}

	for (i = 0; i < macDevices.NumElements(); i++)
	{
		pcDevice = macDevices.Get(i);
		if (!pcDevice->IsMouse())
		{
			ApplyBinding(pcDevice, psBinding);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The keys BindKeys bound in szContext mean nothing of their own there
// any more; global keys give what they gave before: moved off Control+D,
// a command leaves it as Duplicate again.  A chord that only existed for
// them stays on the keyboard, giving nothing.  False if they aren't bound.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::UnbindKeys(const char* szKeys, const char* szContext)
{
	CChars					szName;
	SComponentInputContext*	psContext;
	SComponentInputBinding*	psBinding;

	if (StrEmpty(szContext) || (StringCompare(szContext, COMPONENT_INPUT_GLOBAL_CONTEXT) == 0))
	{
		return UnbindGlobalKeys(szKeys);
	}
	psContext = GetContext(szContext);
	szName.Init();
	psBinding = ((psContext != NULL) && NameKeys(szKeys, &szName)) ? GetContextBinding(psContext, szName.Text()) : NULL;
	szName.Kill();
	if (!psBinding)
	{
		return false;
	}
	KillBinding(psBinding);
	psContext->asBindings.Remove(psBinding);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// See UnbindKeys.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::UnbindGlobalKeys(const char* szKeys)
{
	CChars					szName;
	SComponentInputBinding*	psBinding;
	CComponentInputDevice*	pcDevice;
	size					i;

	szName.Init();
	psBinding = NameKeys(szKeys, &szName) ? GetBinding(szName.Text()) : NULL;
	szName.Kill();
	if (!psBinding)
	{
		return false;
	}

	for (i = 0; i < macDevices.NumElements(); i++)
	{
		pcDevice = macDevices.Get(i);
		if (pcDevice->IsMouse())
		{
			continue;
		}
		if (psBinding->bControl || psBinding->bAlt || psBinding->bShift)
		{
			SetChordCommand(psBinding->szKeys.Text(), psBinding->eReplaced);
		}
		else if (psBinding->bReplacedKey)
		{
			pcDevice->SetCommand(psBinding->szKey.Text(), psBinding->eReplaced);
		}
		else
		{
			pcDevice->RemoveCommand(psBinding->szKey.Text());
		}
	}
	KillBinding(psBinding);
	masBindings.Remove(psBinding);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// szKeys mean eCommand in psContext (see BindKeys).  A chord is made on
// each keyboard that hasn't it, giving nothing elsewhere; a key that types
// (alone or with shift, in a typing context) is looked for as it's typed,
// so nothing is made and it still types everywhere else.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::BindContextKeys(const char* szKeys, EComponentInputCommand eCommand, SComponentInputContext* psContext)
{
	SComponentInputBinding	sParsed;
	SComponentInputBinding*	psBinding;
	CComponentInputDevice*	pcDevice;
	size					i;

	if (!ParseKeys(szKeys, &sParsed))
	{
		return gcLogger.Error2(__METHOD__, " Keys [", szKeys, "] can't be bound: give a key, a chord like Control+Shift+B, or Control+Shift+F, H.", NULL);
	}
	if (!psContext->bTyping && IsTypingKeys(sParsed.szKeys.Text()))
	{
		KillBinding(&sParsed);
		return gcLogger.Error2(__METHOD__, " Keys [", szKeys, "] type a character and can't be bound in context [", psContext->szName.Text(), "].", NULL);
	}

	psBinding = GetContextBinding(psContext, sParsed.szKeys.Text());
	if (psBinding)
	{
		psBinding->eCommand = eCommand;
		KillBinding(&sParsed);
	}
	else
	{
		sParsed.eCommand = eCommand;
		sParsed.eReplaced = CIC_Ignored;
		sParsed.bReplacedKnown = true;
		sParsed.bReplacedKey = false;
		psBinding = psContext->asBindings.Add();
		*psBinding = sParsed;
	}

	for (i = 0; i < macDevices.NumElements(); i++)
	{
		pcDevice = macDevices.Get(i);
		if (!pcDevice->IsMouse())
		{
			ApplyContextBinding(pcDevice, psBinding);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A part of the window with keys of its own (see SComponentInputContext),
// inside szParent (NULL or "Global" for the global keys).  Its keys are
// read when a key is read in it (Translate).  Adding one that's there
// changes how it types and its editor commands.  False, logged, if the
// parent isn't there.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::AddContext(const char* szName, const char* szParent, bool bTyping, bool bEditorCommands)
{
	SComponentInputContext*	psContext;
	bool					bGlobalParent;

	bGlobalParent = StrEmpty(szParent) || (StringCompare(szParent, COMPONENT_INPUT_GLOBAL_CONTEXT) == 0);
	if (!bGlobalParent && (GetContext(szParent) == NULL))
	{
		return gcLogger.Error2(__METHOD__, " Context [", szName, "] can't be added: there's no context [", szParent, "].", NULL);
	}
	psContext = GetContext(szName);
	if (psContext == NULL)
	{
		psContext = masContexts.Add();
		psContext->szName.Init(szName);
		psContext->szParent.Init(bGlobalParent ? "" : szParent);
		psContext->asBindings.Init();
	}
	psContext->bTyping = bTyping;
	psContext->bEditorCommands = bEditorCommands;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the context has been added (the global keys always are).
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::HasContext(const char* szName)
{
	return StrEmpty(szName) || (StringCompare(szName, COMPONENT_INPUT_GLOBAL_CONTEXT) == 0) || (GetContext(szName) != NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether keys that type can be bound in the context (not the global
// keys).
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::IsTypingContext(const char* szName)
{
	SComponentInputContext*	psContext;

	psContext = GetContext(szName);
	return (psContext != NULL) && psContext->bTyping;
}





//////////////////////////////////////////////////////////////////////////
//
// No standard key gives eCommand any more: the application's bindings
// are all that do, so one moved away really is gone.  It's for the
// keyboards added so far.
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::RemoveStandardKeys(EComponentInputCommand eCommand)
{
	CComponentInputDevice*	pcDevice;
	size					i;

	for (i = 0; i < macDevices.NumElements(); i++)
	{
		pcDevice = macDevices.Get(i);
		if (!pcDevice->IsMouse())
		{
			pcDevice->RemoveCommands(eCommand);
		}
	}
	for (i = 0; i < masChordCommands.NumElements(); i++)
	{
		if (masChordCommands.Get(i)->ePressed == eCommand)
		{
			masChordCommands.Get(i)->ePressed = CIC_Ignored;
		}
	}
	for (i = 0; i < masBindings.NumElements(); i++)
	{
		if (masBindings.Get(i)->eReplaced == eCommand)
		{
			masBindings.Get(i)->eReplaced = CIC_Ignored;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether keys bound in szContext work: a keyboard gives them (a chord,
// or a key it has).  The global keys' are IsGloballyBound's.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::IsBound(const char* szKeys, const char* szContext)
{
	CChars					szName;
	SComponentInputContext*	psContext;
	SComponentInputBinding*	psBinding;
	CComponentInputDevice*	pcDevice;
	size					i;
	bool					bBound;

	if (StrEmpty(szContext) || (StringCompare(szContext, COMPONENT_INPUT_GLOBAL_CONTEXT) == 0))
	{
		return IsGloballyBound(szKeys);
	}
	psContext = GetContext(szContext);
	szName.Init();
	psBinding = ((psContext != NULL) && NameKeys(szKeys, &szName)) ? GetContextBinding(psContext, szName.Text()) : NULL;
	bBound = false;
	for (i = 0; (psBinding != NULL) && !bBound && (i < macDevices.NumElements()); i++)
	{
		pcDevice = macDevices.Get(i);
		if (!pcDevice->IsMouse())
		{
			bBound = ((psBinding->bControl || psBinding->bAlt || psBinding->bShift) && !IsTypingBinding(psBinding)) ? pcDevice->HasChord(szName.Text()) : (pcDevice->GetSource(psBinding->szKey.Text()) != NULL);
		}
	}
	szName.Kill();
	return bBound;
}



//////////////////////////////////////////////////////////////////////////
//
// Whether eCommand is what a key does in an editor, so an editor's keys
// decide what gives it.  A character being typed isn't, and space is one;
// nor is anything that's the window's or a component's.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::IsEditorCommand(EComponentInputCommand eCommand)
{
	if (eCommand == CIC_Space)
	{
		return false;
	}
	return ((eCommand >= CIC_Up) && (eCommand <= CIC_ToggleInsert));
}


//////////////////////////////////////////////////////////////////////////
//
// What eCommand is, or a standard key gives it, for something other than
// an editor: a dialog's or a list's keys, or a text field's.  A command
// bound over its keys would stop them working there, as an editor's keys
// can't.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::IsWidgetCommand(EComponentInputCommand eCommand)
{
	switch (eCommand)
	{
	case CIC_Up:
	case CIC_Down:
	case CIC_Left:
	case CIC_Right:
	case CIC_WordLeft:
	case CIC_WordRight:
	case CIC_Home:
	case CIC_End:
	case CIC_DocumentHome:
	case CIC_DocumentEnd:
	case CIC_PageUp:
	case CIC_PageDown:
	case CIC_SelectLeft:
	case CIC_SelectRight:
	case CIC_SelectWordLeft:
	case CIC_SelectWordRight:
	case CIC_SelectHome:
	case CIC_SelectEnd:
	case CIC_SelectDocumentHome:
	case CIC_SelectDocumentEnd:
	case CIC_SelectAll:
	case CIC_ExtendSelection:
	case CIC_ShrinkSelection:
	case CIC_BoxSelectUp:
	case CIC_BoxSelectDown:
	case CIC_BoxSelectLeft:
	case CIC_BoxSelectRight:
	case CIC_BoxSelectHome:
	case CIC_BoxSelectEnd:
	case CIC_ClearSelection:
	case CIC_Space:
	case CIC_Tab:
	case CIC_BackTab:
	case CIC_Enter:
	case CIC_Delete:
	case CIC_Backspace:
	case CIC_Cut:
	case CIC_Copy:
	case CIC_Paste:
	case CIC_PasteFromHistory:
	case CIC_DeleteWordRemainingLeft:
	case CIC_DeleteWordRemainingRight:
	case CIC_Undo:
	case CIC_Redo:
		return true;
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What szKeys give when an application hasn't bound them.  CIC_Ignored
// if nothing.
//
//////////////////////////////////////////////////////////////////////////
EComponentInputCommand CComponentInput::GetStandardCommand(const char* szKeys)
{
	SComponentInputBinding			sParsed;
	SComponentInputBinding*			psBinding;
	SComponentInputChordCommand*	psChord;
	CComponentInputDevice*			pcDevice;
	CInputSourceDesc*				pcSource;
	CComponentInputCommand*			psCommand;
	EComponentInputCommand			eResult;
	size							i;

	if (StrEmpty(szKeys) || !ParseKeys(szKeys, &sParsed))
	{
		return CIC_Ignored;
	}
	eResult = CIC_Ignored;
	psBinding = GetBinding(sParsed.szKeys.Text());
	if (psBinding)
	{
		eResult = psBinding->eReplaced;
	}
	else if (sParsed.bControl || sParsed.bAlt || sParsed.bShift)
	{
		psChord = GetChordCommand(sParsed.szKeys.Text());
		eResult = psChord ? psChord->ePressed : CIC_Ignored;
	}
	else
	{
		for (i = 0; i < macDevices.NumElements(); i++)
		{
			pcDevice = macDevices.Get(i);
			if (!pcDevice->IsMouse())
			{
				pcSource = pcDevice->GetSource(sParsed.szKey.Text());
				psCommand = pcSource ? pcDevice->GetPressedCommand(pcSource) : NULL;
				if (psCommand)
				{
					eResult = psCommand->meCommand;
					break;
				}
			}
		}
	}
	KillBinding(&sParsed);
	return eResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether szKeys type a character: a key that types, alone or with shift.
// Space does, though it's a command.  Bound to anything they'd stop it
// being typed.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::IsTypingKeys(const char* szKeys)
{
	SComponentInputBinding	sParsed;
	CComponentInputDevice*	pcDevice;
	CInputSourceDesc*		pcSource;
	bool					bResult;
	size					i;

	if (StrEmpty(szKeys) || !ParseKeys(szKeys, &sParsed))
	{
		return false;
	}
	bResult = false;
	if (!sParsed.bControl && !sParsed.bAlt && sParsed.szSecondKey.Empty())
	{
		for (i = 0; !bResult && (i < macDevices.NumElements()); i++)
		{
			pcDevice = macDevices.Get(i);
			if (!pcDevice->IsMouse())
			{
				pcSource = pcDevice->GetSource(sParsed.szKey.Text());
				bResult = (pcSource != NULL) && ((pcDevice->GetPrintable(pcSource) != NULL) || sParsed.szKey.Equals("Space"));
			}
		}
	}
	KillBinding(&sParsed);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// What keys mean read in szContext: what the innermost context out from
// it binds them to (psDest is told which context); otherwise nothing, if
// what they normally mean is an editor command an editor context's keys
// have moved them off; otherwise what they normally mean (eStandard).
//
//////////////////////////////////////////////////////////////////////////
EComponentInputCommand CComponentInput::GetContextCommand(const char* szContext, const char* szKeys, EComponentInputCommand eStandard, SComponentInputEvent* psDest)
{
	SComponentInputBinding*	psBinding;
	SComponentInputContext*	psFound;

	psBinding = FindContextBinding(szContext, szKeys, &psFound);
	if (psBinding)
	{
		psDest->szContext = psFound->szName.Text();
		return psBinding->eCommand;
	}
	return (HasEditorCommands(szContext) && IsEditorCommand(eStandard)) ? CIC_Ignored : eStandard;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::KillBinding(SComponentInputBinding* psBinding)
{
	psBinding->szKeys.Kill();
	psBinding->szKey.Kill();
	psBinding->szSecondKey.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::KillContext(SComponentInputContext* psContext)
{
	size	i;

	for (i = 0; i < psContext->asBindings.NumElements(); i++)
	{
		KillBinding(psContext->asBindings.Get(i));
	}
	psContext->asBindings.Kill();
	psContext->szParent.Kill();
	psContext->szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Reads "Control+Alt+Shift+Key" (the modifiers in any order, each at most
// once) or "Control+Shift+First, Second" into psBinding, whose szKeys is
// then the keys' name with its modifiers in the order Control, Alt, Shift.
// Its strings are only initialised if it returns true.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::ParseKeys(const char* szKeys, SComponentInputBinding* psBinding)
{
	const char*	szRest;
	const char*	szComma;
	bool		bModifier;
	CChars		szLook;

	psBinding->bControl = false;
	psBinding->bAlt = false;
	psBinding->bShift = false;
	if (StrEmpty(szKeys))
	{
		return false;
	}

	szRest = szKeys;
	do
	{
		bModifier = false;
		szLook.Fake((char*)szRest);
		if (!psBinding->bControl && szLook.StartsWith("Control+"))
		{
			psBinding->bControl = true;
			szRest += 8;
			bModifier = true;
		}
		else if (!psBinding->bAlt && szLook.StartsWith("Alt+"))
		{
			psBinding->bAlt = true;
			szRest += 4;
			bModifier = true;
		}
		else if (!psBinding->bShift && szLook.StartsWith("Shift+"))
		{
			psBinding->bShift = true;
			szRest += 6;
			bModifier = true;
		}
	}
	while (bModifier);

	szComma = strstr(szRest, ", ");
	if (szComma)
	{
		if (!psBinding->bControl || !psBinding->bShift || psBinding->bAlt || (szComma == szRest) || StrEmpty(szComma + 2))
		{
			return false;
		}
		psBinding->szKey.Init(szRest, 0, (size)(szComma - szRest));
		psBinding->szSecondKey.Init(szComma + 2);
	}
	else
	{
		if (StrEmpty(szRest))
		{
			return false;
		}
		psBinding->szKey.Init(szRest);
		psBinding->szSecondKey.Init();
	}

	psBinding->szKeys.Init(psBinding->bControl ? "Control+" : "");
	psBinding->szKeys.Append(psBinding->bAlt ? "Alt+" : "");
	psBinding->szKeys.Append(psBinding->bShift ? "Shift+" : "");
	psBinding->szKeys.Append(psBinding->szKey);
	if (!psBinding->szSecondKey.Empty())
	{
		psBinding->szKeys.Append(", ");
		psBinding->szKeys.Append(psBinding->szSecondKey);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether keys bound with BindKeys work: a keyboard gives them, as their
// command.  A binding is kept for keyboards added later, but one whose
// key no keyboard has (F13, say) does nothing.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::IsGloballyBound(const char* szKeys)
{
	CChars							szName;
	SComponentInputBinding*			psBinding;
	SComponentInputChordCommand*	psChord;
	CComponentInputDevice*			pcDevice;
	CComponentInputCommand*			psCommand;
	CInputSourceDesc*				pcSource;
	size							i;
	bool							bBound;

	szName.Init();
	psBinding = NameKeys(szKeys, &szName) ? GetBinding(szName.Text()) : NULL;
	bBound = false;
	for (i = 0; (psBinding != NULL) && !bBound && (i < macDevices.NumElements()); i++)
	{
		pcDevice = macDevices.Get(i);
		if (pcDevice->IsMouse())
		{
			continue;
		}
		if (!psBinding->bControl && !psBinding->bAlt && !psBinding->bShift)
		{
			pcSource = pcDevice->GetSource(psBinding->szKey.Text());
			psCommand = pcSource ? pcDevice->GetPressedCommand(pcSource) : NULL;
			bBound = (psCommand != NULL) && (psCommand->meCommand == psBinding->eCommand);
		}
		else
		{
			psChord = GetChordCommand(szName.Text());
			bBound = pcDevice->HasChord(szName.Text()) && (psChord != NULL) && (psChord->ePressed == psBinding->eCommand);
		}
	}
	szName.Kill();
	return bBound;
}


//////////////////////////////////////////////////////////////////////////
//
// szKeys named as a binding of them is ("Shift+Control+B" is
// "Control+Shift+B"), appended to pszDest.  False if they can't be read.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::NameKeys(const char* szKeys, CChars* pszDest)
{
	SComponentInputBinding	sParsed;
	bool					bResult;

	bResult = ParseKeys(szKeys, &sParsed);
	if (bResult)
	{
		pszDest->Append(sParsed.szKeys);
		sParsed.szKeys.Kill();
		sParsed.szKey.Kill();
		sParsed.szSecondKey.Kill();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The application's binding of szKeys (a name as ParseKeys makes), NULL
// if it has none.
//
//////////////////////////////////////////////////////////////////////////
SComponentInputBinding* CComponentInput::GetBinding(const char* szKeys)
{
	size	i;

	for (i = 0; i < masBindings.NumElements(); i++)
	{
		if (masBindings.Get(i)->szKeys.Equals(szKeys))
		{
			return masBindings.Get(i);
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The application's editor binding of szKeys (a name as ParseKeys makes),
// NULL if it has none.
//
//////////////////////////////////////////////////////////////////////////
SComponentInputBinding* CComponentInput::GetContextBinding(SComponentInputContext* psContext, const char* szKeys)
{
	size	i;

	for (i = 0; i < psContext->asBindings.NumElements(); i++)
	{
		if (psContext->asBindings.Get(i)->szKeys.Equals(szKeys))
		{
			return psContext->asBindings.Get(i);
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The context named szName, NULL for the global keys or one that isn't
// there.
//
//////////////////////////////////////////////////////////////////////////
SComponentInputContext* CComponentInput::GetContext(const char* szName)
{
	size	i;

	if (StrEmpty(szName))
	{
		return NULL;
	}
	for (i = 0; i < masContexts.NumElements(); i++)
	{
		if (masContexts.Get(i)->szName.Equals(szName))
		{
			return masContexts.Get(i);
		}
	}
	return NULL;
}


SComponentInputContext* CComponentInput::GetParentContext(SComponentInputContext* psContext) { return GetContext(psContext->szParent.Text()); }


//////////////////////////////////////////////////////////////////////////
//
// The binding of szKeys (a name as ParseKeys makes) in szContext or the
// innermost context around it that has one, not the global keys', which
// the keyboards give themselves; the context it's in in ppsFound.  NULL
// if none has one.
//
//////////////////////////////////////////////////////////////////////////
SComponentInputBinding* CComponentInput::FindContextBinding(const char* szContext, const char* szKeys, SComponentInputContext** ppsFound)
{
	SComponentInputContext*	psContext;
	SComponentInputBinding*	psBinding;
	size					uiDepth;

	for (psContext = GetContext(szContext), uiDepth = 0; (psContext != NULL) && (uiDepth < masContexts.NumElements()); psContext = GetParentContext(psContext), uiDepth++)
	{
		psBinding = GetContextBinding(psContext, szKeys);
		if (psBinding)
		{
			*ppsFound = psContext;
			return psBinding;
		}
	}
	*ppsFound = NULL;
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a key read in szContext is in an editor context (it or one
// around it) with keys, which say what the editor commands are.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::HasEditorCommands(const char* szContext)
{
	SComponentInputContext*	psContext;
	size					uiDepth;

	for (psContext = GetContext(szContext), uiDepth = 0; (psContext != NULL) && (uiDepth < masContexts.NumElements()); psContext = GetParentContext(psContext), uiDepth++)
	{
		if (psContext->bEditorCommands && (psContext->asBindings.NumElements() > 0))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the binding is of a key that types a character, alone or with
// shift, so it's looked for as it's typed rather than as a chord.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::IsTypingBinding(SComponentInputBinding* psBinding)
{
	return IsTypingKeys(psBinding->szKeys.Text());
}


//////////////////////////////////////////////////////////////////////////
//
// A chord has to be there for a keyboard to say it was pressed, so one
// only a context means anything by is made, giving nothing elsewhere.
// One key, or a key that types with shift, needs nothing made: it's looked
// for as it is pressed, and still types everywhere else.
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::ApplyContextBinding(CComponentInputDevice* pcKeyboard, SComponentInputBinding* psBinding)
{
	if ((!psBinding->bControl && !psBinding->bAlt && !psBinding->bShift) || IsTypingBinding(psBinding))
	{
		return;
	}

	if (!pcKeyboard->HasChord(psBinding->szKeys.Text()))
	{
		if (psBinding->szSecondKey.Empty())
		{
			AddChordCommands(pcKeyboard, pcKeyboard, psBinding->bControl, psBinding->bShift, psBinding->bAlt, psBinding->szKey.Text(), CIC_Ignored, CIC_Ignored);
		}
		else
		{
			AddSequenceChordCommands(pcKeyboard, psBinding->szKey.Text(), psBinding->szSecondKey.Text(), CIC_Ignored, COMPONENT_INPUT_SEQUENCE_MILLISECONDS);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Gives pcKeyboard psBinding's keys.  A chord is only made once on each
// keyboard: one the keyboard already has just gives the binding's command
// now.  A keyboard without the key doesn't get it.  The first keyboard
// given them says what they gave before, as all start the same.
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::ApplyBinding(CComponentInputDevice* pcKeyboard, SComponentInputBinding* psBinding)
{
	CInputSourceDesc*				pcSource;
	CComponentInputCommand*			psCommand;
	SComponentInputChordCommand*	psChord;

	if (!psBinding->bControl && !psBinding->bAlt && !psBinding->bShift)
	{
		pcSource = pcKeyboard->GetSource(psBinding->szKey.Text());
		if (pcSource && !psBinding->bReplacedKnown)
		{
			psCommand = pcKeyboard->GetPressedCommand(pcSource);
			psBinding->bReplacedKnown = true;
			psBinding->bReplacedKey = (psCommand != NULL);
			psBinding->eReplaced = psCommand ? psCommand->meCommand : CIC_Ignored;
		}
		pcKeyboard->SetCommand(psBinding->szKey.Text(), psBinding->eCommand);
		return;
	}

	if (!psBinding->bReplacedKnown)
	{
		psChord = GetChordCommand(psBinding->szKeys.Text());
		psBinding->bReplacedKnown = true;
		psBinding->eReplaced = psChord ? psChord->ePressed : CIC_Ignored;
	}

	if (!pcKeyboard->HasChord(psBinding->szKeys.Text()))
	{
		if (psBinding->szSecondKey.Empty())
		{
			AddChordCommands(pcKeyboard, pcKeyboard, psBinding->bControl, psBinding->bShift, psBinding->bAlt, psBinding->szKey.Text(), psBinding->eCommand, CIC_Ignored);
		}
		else
		{
			AddSequenceChordCommands(pcKeyboard, psBinding->szKey.Text(), psBinding->szSecondKey.Text(), psBinding->eCommand, COMPONENT_INPUT_SEQUENCE_MILLISECONDS);
		}
	}
	SetChordCommand(psBinding->szKeys.Text(), psBinding->eCommand);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SComponentInputChordCommand* CComponentInput::GetChordCommand(const char* szName)
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
// A key's or chord's name written as IntelliJ writes it in its menus:
// "Control+Shift+Cursor Left" is "Ctrl+Shift+Left".
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::AppendShortcutText(CChars* pszDest, const char* szKeys)
{
	CChars	szLook;

	while (*szKeys != '\0')
	{
		szLook.Fake((char*)szKeys);
		if (szLook.StartsWith("Control"))
		{
			pszDest->Append("Ctrl");
			szKeys += 7;
		}
		else if (szLook.StartsWith("Cursor "))
		{
			szKeys += 7;
		}
		else
		{
			pszDest->Append(*szKeys);
			szKeys++;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The keys that give eCommand, as a menu shows them ("Ctrl+Alt+Shift+N",
// "F5", "Left"), appended to pszDest: the keys bound to it (BindKeys),
// else a key that gives it on its own, else the first chord that does.  They are whatever keys the window was given,
// so a menu can't say other keys than those that work.  False, and
// nothing appended, if no key gives it.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::GetShortcutText(EComponentInputCommand eCommand, CChars* pszDest)
{
	size				i;
	CInputSourceDesc*	pcSource;

	//An application's keys first, as they may be a standard command's other keys.
	for (i = 0; i < masBindings.NumElements(); i++)
	{
		if (masBindings.Get(i)->eCommand == eCommand)
		{
			AppendShortcutText(pszDest, masBindings.Get(i)->szKeys.Text());
			return true;
		}
	}
	for (i = 0; i < macDevices.NumElements(); i++)
	{
		if (!macDevices.Get(i)->IsMouse())
		{
			pcSource = macDevices.Get(i)->FindPressedSource(eCommand);
			if (pcSource)
			{
				AppendShortcutText(pszDest, pcSource->GetFriendlyName());
				return true;
			}
		}
	}
	for (i = 0; i < masChordCommands.NumElements(); i++)
	{
		if (masChordCommands.Get(i)->ePressed == eCommand)
		{
			AppendShortcutText(pszDest, masChordCommands.Get(i)->szName.Text());
			return true;
		}
	}
	return false;
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
// How long after this update held back chord inputs will be let through
// if nothing else happens.  The window waits no longer than this.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::GetEvictionDelay(uint32* puiMilliseconds)
{
	return mpcSource->GetEvictionDelay(puiMilliseconds);
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
	psEvent->bControl = IsControl();
	psEvent->bAlt = IsAlt();
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
size CComponentInput::Translate(SWindowInputEvent* psWindowEvent, SComponentInputEvent* asDest, const char* szContext)
{
	SComponentInputChordCommand*	psChordCommand;
	CComponentInputDevice*			pcDevice;
	EComponentInputCommand			eCommand;
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
		eCommand = GetContextCommand(szContext, psWindowEvent->szChord, psChordCommand->ePressed, &asDest[1]);
		if (eCommand == CIC_ToggleInsert)
		{
			mbInsert = !mbInsert;
			return 1;
		}
		asDest[1].szName = psWindowEvent->szChord;
		asDest[1].eCommand = eCommand;
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
		if (StringCompare(szName, "Left") == 0)
		{
			asDest[0].iRight = -(int)psWindowEvent->fValue;
		}
		else if (StringCompare(szName, "Right") == 0)
		{
			asDest[0].iRight = (int)psWindowEvent->fValue;
		}
		else if (StringCompare(szName, "Up") == 0)
		{
			asDest[0].iDown = -(int)psWindowEvent->fValue;
		}
		else if (StringCompare(szName, "Down") == 0)
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
		return Pressed(psWindowEvent, &asDest[1], szContext) ? 2 : 1;
	}
	else
	{
		return Released(psWindowEvent, &asDest[1]) ? 2 : 1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the uiNumEvents events Translate made of psWindowEvent are a key
// typing a character: its press (the key down and the printable) or the
// release of a key that types one.  A command, a chord or anything else
// isn't.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::IsTyping(SWindowInputEvent* psWindowEvent, SComponentInputEvent* asEvents, size uiNumEvents)
{
	if (uiNumEvents == 2)
	{
		return (asEvents[0].eType == CIET_KeyDown) && (asEvents[1].eType == CIET_Printable);
	}
	if (uiNumEvents == 1)
	{
		return (asEvents[0].eType == CIET_KeyUp) && (GetPrintable(psWindowEvent->pcSource) != NULL);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A pressed source is, in order: a lock or insert, a numpad key standing
// in for another source, then nothing if control or alt is held (it would
// have been a chord), the lock or insert the numpad key stands in for, a
// command, its binding in szContext (or one around it: a key that types,
// with shift if it's held, in a typing context) or a printable character.
// Returns true if psDest is set.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentInput::Pressed(SWindowInputEvent* psWindowEvent, SComponentInputEvent* psDest, const char* szContext)
{
	bool						bEditorKeys;
	CChars						szKeys;
	SComponentInputContext*		psFound;
	CComponentInputKey*			psKey;
	CComponentInputNumKey*		psNumKey;
	CComponentInputCommand*		psCommand;
	CComponentInputCommand*		psReleased;
	CComponentInputPrintable*	psPrintable;
	CInputSourceDesc*			pcSource;
	CInputSourceDesc*			pcRemapped;
	EComponentInputCommand		eCommand;
	SComponentInputBinding*		psBinding;

	bEditorKeys = HasEditorCommands(szContext);
	pcSource = psWindowEvent->pcSource;
	psKey = GetKey(pcSource);
	if (psKey && !(bEditorKeys && (psKey->meKey == CIK_Insert)))  //An editor's keys say what insert does.
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
		pcRemapped = mbNumLock ? psNumKey->mpcYesNumLockRemap : psNumKey->mpcNoNumLockRemap;
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

	//A numpad key standing in for a lock or insert, as numpad 0 does for insert, toggles it.
	if (pcRemapped != pcSource)
	{
		psKey = GetKey(pcRemapped);
		if (psKey && !(bEditorKeys && (psKey->meKey == CIK_Insert)))
		{
			PressedKey(psKey);
			return false;
		}
	}

	psReleased = GetCommand(pcRemapped, false);
	AddPressed(pcSource, psReleased ? psReleased->meCommand : CIC_Ignored);

	psCommand = GetCommand(pcRemapped, true);
	if (GetContext(szContext) != NULL)
	{
		//A key that types with shift held is bound as "Shift+A" (a typing context's); any other key by its name.
		szKeys.Init();
		if (IsShift() && GetPrintable(pcRemapped))
		{
			szKeys.Append("Shift+");
		}
		szKeys.Append(pcRemapped->GetFriendlyName());
		psBinding = FindContextBinding(szContext, szKeys.Text(), &psFound);
		szKeys.Kill();
		if (psBinding || (psCommand && bEditorKeys))
		{
			InitEvent(psDest, CIET_Command, psWindowEvent);
			psDest->szName = psBinding ? psBinding->szKeys.Text() : pcRemapped->GetFriendlyName();
			if (psBinding)
			{
				psDest->szContext = psFound->szName.Text();
				eCommand = psBinding->eCommand;
			}
			else
			{
				eCommand = IsEditorCommand(psCommand->meCommand) ? CIC_Ignored : psCommand->meCommand;
			}
			if (eCommand == CIC_ToggleInsert)
			{
				mbInsert = !mbInsert;
				return false;
			}
			if (eCommand == CIC_Ignored)
			{
				return false;
			}
			psDest->eCommand = eCommand;
			return true;
		}
		if (bEditorKeys && GetKey(pcRemapped))
		{
			return false;  //Insert, moved off: it does nothing here.
		}
	}
	if (psCommand)
	{
		if (psCommand->meCommand == CIC_Ignored)
		{
			return false;
		}
		InitEvent(psDest, CIET_Command, psWindowEvent);
		psDest->szName = pcRemapped->GetFriendlyName();
		psDest->eCommand = psCommand->meCommand;
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
	switch (psKey->meKey)
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
// What pcSource means, from the first device that gives it a meaning.
//
//////////////////////////////////////////////////////////////////////////
CComponentInputCommand* CComponentInput::GetCommand(CInputSourceDesc* pcSource, bool bPressed)
{
	return FindInDevices(bPressed ? &CComponentInputDevice::GetPressedCommand : &CComponentInputDevice::GetReleasedCommand, pcSource);
}
CComponentInputPrintable* CComponentInput::GetPrintable(CInputSourceDesc* pcSource) { return FindInDevices(&CComponentInputDevice::GetPrintable, pcSource); }
CComponentInputNumKey* CComponentInput::GetNumKeyRemap(CInputSourceDesc* pcSource) { return FindInDevices(&CComponentInputDevice::GetNumKeyRemap, pcSource); }
CComponentInputKey* CComponentInput::GetKey(CInputSourceDesc* pcSource) { return FindInDevices(&CComponentInputDevice::GetKey, pcSource); }


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
