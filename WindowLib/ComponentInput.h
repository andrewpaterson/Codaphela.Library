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
#include "InputLib/Input.h"
#include "ComponentInputDevice.h"
#include "ComponentInputListener.h"


//The modifiers held when a source was pressed.  Its release is read with them
//so letting go of shift before the mouse button still stops a shift drag.
struct SComponentInputPressed
{
	CInputSourceDesc*	pcSource;
	bool				bControl;
	bool				bAlt;
	bool				bShift;
};


typedef CArrayTemplate<SComponentInputPressed>	CArrayComponentInputPressed;


//Turns keyboard and mouse button events from InputLib into commands and
//printable characters for a window's focussed component.  The pointer's
//position is not read here, the native window gives it to CWindow::PointerMoved.
//
//Kill a CComponentInput before the CInput it was given.
class CWindow;
class CComponentInput
{
protected:
	CArrayComponentInputDevice	macDevices;
	CArrayComponentInputPressed	masPressed;
	CComponentInputListener		mcListener;
	CInput*						mpcInput;
	CWindow*					mpcWindow;

	int							miShift;  //Counts so that both shift keys can be held.
	int							miControl;
	int							miAlt;
	bool						mbCapsLock;
	bool						mbNumLock;
	bool						mbInsert;

	CAction*					mpcButtonPressedAction;
	CAction*					mpcButtonReleasedAction;

public:
	void		Init(CInput* pcInput, CWindow* pcWindow);
	void		Kill(void);

	bool		AddKeyboard(CInputDevice* pcDevice);
	bool		AddMouse(CInputDevice* pcDevice);
	size		AddKeyboards(char* szCategory = "Keyboard");
	size		AddMice(char* szCategory = "Mouse");

	void		ButtonPressed(CUnknown* pcSource, void* pvContext);
	void		ButtonReleased(CUnknown* pcSource, void* pvContext);
	void		Pressed(CInputSourceDesc* pcSource);
	void		Released(CInputSourceDesc* pcSource);
	void		ReleaseAll(void);
	void		SetLocks(bool bCapsLock, bool bNumLock);

	bool		IsShift(void);
	bool		IsControl(void);
	bool		IsAlt(void);
	bool		IsCapsLock(void);
	bool		IsNumLock(void);
	bool		IsInsert(void);
	CInput*		GetInput(void);
	CAction*	GetButtonPressedAction(void);
	CAction*	GetButtonReleasedAction(void);

protected:
	CComponentInputDevice*		AddDevice(CInputDevice* pcDevice);
	size						AddDevices(char* szCategory, bool bKeyboard);
	bool						PressedKey(CComponentInputKey* psKey);
	void						ReleasedKey(CComponentInputKey* psKey);
	void						Command(EComponentInputCommand eCommand);
	CComponentInputCommand*		GetCommand(CInputSourceDesc* pcSource, bool bControl, bool bAlt, bool bShift, bool bPressed);
	CComponentInputPrintable*	GetPrintable(CInputSourceDesc* pcSource);
	CComponentInputNumKey*		GetNumKeyRemap(CInputSourceDesc* pcSource);
	CComponentInputKey*			GetKey(CInputSourceDesc* pcSource);
	void						AddPressed(CInputSourceDesc* pcSource, bool bControl, bool bAlt, bool bShift);
	bool						RemovePressed(CInputSourceDesc* pcSource, SComponentInputPressed* psDest);
};


#endif // __COMPONENT_INPUT_H__

