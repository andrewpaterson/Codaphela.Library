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
#ifndef __COMPONENT_INPUT_H__
#define __COMPONENT_INPUT_H__
#include "BaseLib/Chars.h"
#include "WindowInputSource.h"
#include "ComponentInputDevice.h"
#include "ComponentInputEvent.h"


//A key or button that went down with the command its release gives, if any.
struct SComponentInputPressed
{
	CInputSourceDesc*		pcSource;
	EComponentInputCommand	eReleased;
};


typedef CArrayTemplate<SComponentInputPressed>	CArrayComponentInputPressed;


//The command a chord gives, and the command letting go of its last key gives (CIC_Ignored for none).
struct SComponentInputChordCommand
{
	CChars					szName;
	EComponentInputCommand	ePressed;
	EComponentInputCommand	eReleased;
};


typedef CArrayTemplate<SComponentInputChordCommand>	CArrayComponentInputChordCommand;


//Keys bound to a command (CComponentInput::BindKeys): one key ("F5"), a chord ("Control+Shift+B") or two keys one
//after the other with control and shift held for the first ("Control+Shift+F, H").  szKeys is the binding's name, its
//modifiers in the order Control, Alt, Shift.
struct SComponentInputBinding
{
	CChars					szKeys;
	EComponentInputCommand	eCommand;
	bool					bControl;
	bool					bAlt;
	bool					bShift;
	CChars					szKey;
	CChars					szSecondKey;  //Empty unless it is two keys one after the other.
};


typedef CArrayTemplate<SComponentInputBinding>	CArrayComponentInputBinding;


#define COMPONENT_INPUT_MAX_EVENTS	2	//Translate's most events for one queued event.
#define COMPONENT_INPUT_SEQUENCE_MILLISECONDS	500	//How long after the chord a bound sequence's second key may come.


//Pumps the events a CWindowInputSource queues to the window, whose focus has them translated here and dispatches
//them.  A key or button is translated to its key event then the command or printable it means alone; modifiers only
//mean something as chords, so control or shift with a key is one of the standard chord commands (control+c is
//copy).  Shift and caps lock still choose a printable's case.  The pointer's position is not read here, the native
//window gives it to CWindow::PointerMoved.
//
//Kill a CComponentInput before the CWindowInputSource it was given.
class CWindow;
class CComponentInput
{
protected:
	CArrayComponentInputDevice			macDevices;
	CArrayComponentInputPressed			masPressed;
	CArrayComponentInputChordCommand	masChordCommands;
	CArrayComponentInputBinding			masBindings;  //An application's keys, given every keyboard after the standard ones.
	CWindowInputSource*					mpcSource;
	CWindow*							mpcWindow;

	bool								mbCapsLock;
	bool								mbNumLock;
	bool								mbInsert;

public:
	void		Init(CWindowInputSource* pcSource, CWindow* pcWindow);
	void		Kill(void);

	bool		AddKeyboard(CInputDevice* pcDevice);
	bool		AddMouse(CInputDevice* pcDevice);
	size		AddKeyboards(char* szCategory = "Keyboard");
	size		AddMice(char* szCategory = "Mouse");

	void		Update(void);
	bool		GetEvictionDelay(uint32* puiMilliseconds);
	size		Translate(SWindowInputEvent* psWindowEvent, SComponentInputEvent* asDest);
	void		ReleaseAll(void);
	void		SetLocks(bool bCapsLock, bool bNumLock);
	void		SetWindow(CWindow* pcWindow);
	CWindow*	GetWindow(void);

	bool				IsShift(void);
	bool				IsControl(void);
	bool				IsAlt(void);
	bool				IsCapsLock(void);
	bool				IsNumLock(void);
	bool				IsInsert(void);
	bool				IsSourceDown(const char* szFriendlyName);
	CWindowInputSource*	GetSource(void);
	bool				GetShortcutText(EComponentInputCommand eCommand, CChars* pszDest);
	bool				BindKeys(const char* szKeys, EComponentInputCommand eCommand);

protected:
	CComponentInputDevice*			AddDevice(CInputDevice* pcDevice);
	CComponentInputDevice*			GetDevice(CInputDevice* pcDevice);
	size							AddDevices(char* szCategory, bool bKeyboard);
	void							AddKeyboardChords(CComponentInputDevice* pcKeyboard);
	void							AddMouseChords(CComponentInputDevice* pcKeyboard, CComponentInputDevice* pcMouse);
	void							AddChordCommands(CComponentInputDevice* pcKeyboard, CComponentInputDevice* pcKeyDevice, bool bControl, bool bShift, bool bAlt, char* szKey, EComponentInputCommand ePressed, EComponentInputCommand eReleased);
	void							AddSequenceChordCommands(CComponentInputDevice* pcKeyboard, char* szFirstKey, char* szSecondKey, EComponentInputCommand eCommand, uint32 uiEvictionMilliseconds);
	void							AddChordCommand(char* szName, EComponentInputCommand ePressed, EComponentInputCommand eReleased);
	void							SetChordCommand(char* szName, EComponentInputCommand ePressed);
	bool							ParseKeys(const char* szKeys, SComponentInputBinding* psBinding);
	SComponentInputBinding*			GetBinding(char* szKeys);
	void							ApplyBinding(CComponentInputDevice* pcKeyboard, SComponentInputBinding* psBinding);
	SComponentInputChordCommand*	GetChordCommand(char* szName);
	bool							IsKeyDown(EComponentInputKey eKey);
	void							InitEvent(SComponentInputEvent* psEvent, EComponentInputEventType eType, SWindowInputEvent* psWindowEvent);
	bool							Pressed(SWindowInputEvent* psWindowEvent, SComponentInputEvent* psDest);
	bool							Released(SWindowInputEvent* psWindowEvent, SComponentInputEvent* psDest);
	bool							PressedKey(CComponentInputKey* psKey);
	CComponentInputCommand*			GetCommand(CInputSourceDesc* pcSource, bool bPressed);
	CComponentInputPrintable*		GetPrintable(CInputSourceDesc* pcSource);
	CComponentInputNumKey*			GetNumKeyRemap(CInputSourceDesc* pcSource);
	CComponentInputKey*				GetKey(CInputSourceDesc* pcSource);
	template<class M>
	M*							FindInDevices(M* (CComponentInputDevice::*fFind)(CInputSourceDesc*), CInputSourceDesc* pcSource);
	void						AddPressed(CInputSourceDesc* pcSource, EComponentInputCommand eReleased);
	bool						RemovePressed(CInputSourceDesc* pcSource, SComponentInputPressed* psDest);
};


//////////////////////////////////////////////////////////////////////////
//
// The meaning fFind gives pcSource on the first device that has one.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CComponentInput::FindInDevices(M* (CComponentInputDevice::*fFind)(CInputSourceDesc*), CInputSourceDesc* pcSource)
{
	size	i;
	M*		psMeaning;

	for (i = 0; i < macDevices.NumElements(); i++)
	{
		psMeaning = (macDevices.Get(i)->*fFind)(pcSource);
		if (psMeaning)
		{
			return psMeaning;
		}
	}
	return NULL;
}


#endif // __COMPONENT_INPUT_H__
