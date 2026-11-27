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
//after the other with control, or control and shift, held for the first ("Control+K, C", "Control+Shift+F, H").  szKeys is the binding's name, its
//modifiers in the order Control, Alt, Shift.  What the keys gave before is kept so UnbindKeys can give it back.
struct SComponentInputBinding
{
	CChars					szKeys;
	EComponentInputCommand	eCommand;
	EComponentInputCommand	eReplaced;  //What the keys gave before they were bound, once bReplacedKnown.
	bool					bReplacedKnown;  //They have been given to a keyboard, so eReplaced has been found.
	bool					bReplacedKey;  //One key with a command of its own before, not just a character it typed.
	bool					bControl;
	bool					bAlt;
	bool					bShift;
	CChars					szKey;
	CChars					szSecondKey;  //Empty unless it is two keys one after the other.
};


typedef CArrayTemplate<SComponentInputBinding>	CArrayComponentInputBinding;


//The contexts every application has: the global keys, which work anywhere nothing else binds them, and an editor's.
#define COMPONENT_INPUT_GLOBAL_CONTEXT	"Global"
#define COMPONENT_INPUT_EDITOR_CONTEXT	"Editor"


//An application's keys for a part of its window (CComponentInput::AddContext): a code editor, a circuit.  A key read
//in a context is looked for in its bindings, then its parent's, out to the global keys, and the innermost binding
//wins.  A typing context can have keys that type a character, which then give their command there instead.  Once an
//editor context has keys, the standard editor commands (caret, selection, editing) are its own: one it doesn't bind
//gives nothing.  The Editor context (a text area's) is always there.
struct SComponentInputContext
{
	CChars						szName;
	CChars						szParent;	//Empty for a context inside the global keys.
	CArrayComponentInputBinding	asBindings;
	bool						bTyping;
	bool						bEditorCommands;
};


typedef CArrayTemplate<SComponentInputContext>	CArrayComponentInputContext;


#define COMPONENT_INPUT_MAX_EVENTS	2	//Translate's most events for one queued event.
#define COMPONENT_INPUT_SEQUENCE_MILLISECONDS	500	//How long after the chord a bound sequence's second key may come.
#define COMPONENT_INPUT_REPEAT_DELAY			500	//How long a key is held before it repeats (SetRepeat), as Windows' default.
#define COMPONENT_INPUT_REPEAT_PERIOD			33	//Between its repeats: about thirty a second.


//Pumps the events a CWindowInputSource queues to the window, whose focus has them translated here and dispatches
//them.  A key or button is translated to its key event then the command or printable it means alone; modifiers only
//mean something as chords, so control or shift with a key is one of the standard chord commands (control+c is
//copy).  Shift and caps lock still choose a printable's case.  The pointer's position is not read here, the native
//window gives it to CWindow::PointerMoved.
//
//An application's keys (BindKeys) are global, given to every keyboard in place of the standard keys, or in a context
//(AddContext), read only when a key is read in that context (Translate's szContext).  The focus reads a key in the
//context of the component it's for (CComponent::GetKeyContext: a text area's is Editor) in a window that isn't a
//dialog, so what is done to a context's keys can't stop a dialog working (see CFocus::FindKeyComponent).
//
//A held key repeats here, not from the platform (InputLib gives a held key one press however often the platform sends
//it): with SetRepeat, once a key has been held for the delay the command or printable its press gave is given again to
//the component that took it, each period, until it is let go.  Only that is given again, never its key or chord event,
//so a component that takes keys as they go down and up (a game's, an emulator's) sees one press and one release, and
//what is held reads as held.  It repeats only if the press's command or printable was what was taken, the command is
//one that moves or edits (IsRepeatCommand) and the component lets it (CComponent::SetKeyRepeat).  Any other key or
//button going down or up, the keys going to another component, the window changing and ReleaseAll stop it.  Repeat
//is off until SetRepeat.
//
//Kill a CComponentInput before the CWindowInputSource it was given.
class CWindow;
class CComponent;
//Design page: Docs/libraries/windowlib/input.md
class CComponentInput
{
protected:
	CArrayComponentInputDevice			macDevices;
	CArrayComponentInputPressed			masPressed;
	CArrayComponentInputChordCommand	masChordCommands;
	CArrayComponentInputBinding			masBindings;  //An application's global keys, given every keyboard after the standard ones.
	CArrayComponentInputContext			masContexts;  //Its keys for parts of the window, looked for before the global keys.
	CWindowInputSource*					mpcSource;
	CWindow*							mpcWindow;

	bool								mbCapsLock;
	bool								mbNumLock;
	bool								mbInsert;

	uint32								muiRepeatDelay;  //0 for no repeating.
	uint32								muiRepeatPeriod;
	bool								mbRepeating;  //A held key's command or printable is to be given again.
	SComponentInputEvent				msRepeatEvent;  //What is given again.
	SWindowInputEvent					msRepeatWindowEvent;  //Its press, which msRepeatEvent points to.
	CComponent*							mpcRepeatComponent;  //Not held, only compared: what took it.
	int64								miRepeatDue;  //When it is next given (GetDebugTickCount).
	int64								miClock;  //What Now gives, -1 for the clock (SetClock).

public:
	void		Init(CWindowInputSource* pcSource, CWindow* pcWindow);
	void		Kill(void);

	bool		AddKeyboard(CInputDevice* pcDevice);
	bool		AddMouse(CInputDevice* pcDevice);
	size		AddKeyboards(const char* szCategory = "Keyboard");
	size		AddMice(const char* szCategory = "Mouse");

	void		Update(void);
	bool		GetEvictionDelay(uint32* puiMilliseconds);
	size		Translate(SWindowInputEvent* psWindowEvent, SComponentInputEvent* asDest, const char* szContext = NULL);
	bool		IsTyping(SWindowInputEvent* psWindowEvent, SComponentInputEvent* asEvents, size uiNumEvents);
	void		ReleaseAll(void);
	void		SetLocks(bool bCapsLock, bool bNumLock);
	void		SetWindow(CWindow* pcWindow);
	CWindow*	GetWindow(void) const;

	void		SetRepeat(uint32 uiDelay, uint32 uiPeriod);
	uint32		GetRepeatDelay(void) const;
	uint32		GetRepeatPeriod(void) const;
	void		StartRepeat(SComponentInputEvent* psTaken, CComponent* pcComponent);
	void		StopRepeat(void);
	bool		IsRepeating(void) const;
	int64		GetRepeatWait(void);
	bool		Repeat(void);
	void		SetClock(int64 iNow);
	int64		Now(void);

	bool				IsShift(void);
	bool				IsControl(void);
	bool				IsAlt(void);
	bool				IsCapsLock(void) const;
	bool				IsNumLock(void) const;
	bool				IsInsert(void) const;
	bool				IsSourceDown(const char* szFriendlyName);
	CWindowInputSource*	GetSource(void) const;
	bool				GetShortcutText(EComponentInputCommand eCommand, CChars* pszDest);
	bool				AddContext(const char* szName, const char* szParent, bool bTyping, bool bEditorCommands = false);
	bool				HasContext(const char* szName);
	bool				IsTypingContext(const char* szName);
	EComponentInputCommand	GetContextCommand(const char* szContext, const char* szKeys, EComponentInputCommand eStandard, SComponentInputEvent* psDest);
	bool				BindKeys(const char* szKeys, EComponentInputCommand eCommand, const char* szContext = NULL);
	bool				UnbindKeys(const char* szKeys, const char* szContext = NULL);
	void				RemoveStandardKeys(EComponentInputCommand eCommand);
	bool				IsBound(const char* szKeys, const char* szContext = NULL);
	EComponentInputCommand	GetStandardCommand(const char* szKeys);
	bool				IsTypingKeys(const char* szKeys);
	static bool			IsEditorCommand(EComponentInputCommand eCommand);
	static bool			IsWidgetCommand(EComponentInputCommand eCommand);
	static bool			IsRepeatCommand(EComponentInputCommand eCommand);
	static bool			NameKeys(const char* szKeys, CChars* pszDest);
	static void			AppendShortcutText(CChars* pszDest, const char* szKeys);

protected:
	CComponentInputDevice*			AddDevice(CInputDevice* pcDevice);
	CComponentInputDevice*			GetDevice(CInputDevice* pcDevice);
	size							AddDevices(const char* szCategory, bool bKeyboard);
	void							AddKeyboardChords(CComponentInputDevice* pcKeyboard);
	void							AddMouseChords(CComponentInputDevice* pcKeyboard, CComponentInputDevice* pcMouse);
	void							AddChordCommands(CComponentInputDevice* pcKeyboard, CComponentInputDevice* pcKeyDevice, bool bControl, bool bShift, bool bAlt, const char* szKey, EComponentInputCommand ePressed, EComponentInputCommand eReleased);
	void							AddSequenceChordCommands(CComponentInputDevice* pcKeyboard, bool bShift, const char* szFirstKey, const char* szSecondKey, EComponentInputCommand eCommand, uint32 uiEvictionMilliseconds);
	void							AddChordCommand(const char* szName, EComponentInputCommand ePressed, EComponentInputCommand eReleased);
	void							SetChordCommand(const char* szName, EComponentInputCommand ePressed);
	static bool						ParseKeys(const char* szKeys, SComponentInputBinding* psBinding);
	SComponentInputBinding*			GetBinding(const char* szKeys);
	SComponentInputContext*			GetContext(const char* szName);
	SComponentInputContext*			GetParentContext(SComponentInputContext* psContext);
	SComponentInputBinding*			GetContextBinding(SComponentInputContext* psContext, const char* szKeys);
	SComponentInputBinding*			FindContextBinding(const char* szContext, const char* szKeys, SComponentInputContext** ppsFound);
	bool							HasEditorCommands(const char* szContext);
	bool							BindGlobalKeys(const char* szKeys, EComponentInputCommand eCommand);
	bool							BindContextKeys(const char* szKeys, EComponentInputCommand eCommand, SComponentInputContext* psContext);
	bool							UnbindGlobalKeys(const char* szKeys);
	bool							IsGloballyBound(const char* szKeys);
	bool							IsTypingBinding(SComponentInputBinding* psBinding);
	void							ApplyBinding(CComponentInputDevice* pcKeyboard, SComponentInputBinding* psBinding);
	void							ApplyContextBinding(CComponentInputDevice* pcKeyboard, SComponentInputBinding* psBinding);
	void							KillBinding(SComponentInputBinding* psBinding);
	void							KillContext(SComponentInputContext* psContext);
	SComponentInputChordCommand*	GetChordCommand(const char* szName);
	bool							IsKeyDown(EComponentInputKey eKey);
	void							InitEvent(SComponentInputEvent* psEvent, EComponentInputEventType eType, SWindowInputEvent* psWindowEvent);
	bool							Pressed(SWindowInputEvent* psWindowEvent, SComponentInputEvent* psDest, const char* szContext);
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
