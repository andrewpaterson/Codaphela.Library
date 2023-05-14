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
#include "BaseLib/TextEditor.h"
#include "InputLib/Input.h"
#include "ComponentInputKeyboard.h"
#include "ComponentInputMouse.h"
#include "ComponentListener.h"
#include "SystemPointer.h"


//There's one of these per Viewport.
class CViewport;
class CComponent;
class CComponentInput : public CUnknown, public CListener
{
CONSTRUCTABLE(CComponentInput);
protected:
	CSetComponentInputDevices	mlcInputDevices;

	int							miControl;
	int							miShift;
	int							miAlt;

	BOOL						mbCapsLock;
	BOOL						mbNumLock;
	BOOL						mbInsert;

	CInput*						mpcInput;
	CViewport*					mpcViewport;

	CSystemPointer				mcPointer;

	CAction*					mpcShiftPressedAction;
	CAction*					mpcShiftReleasedAction;
	CAction*					mpcControlPressedAction;
	CAction*					mpcControlReleasedAction;
	CAction*					mpcAltPressedAction;
	CAction*					mpcAltReleasedAction;
	CAction*					mpcNumLockPressedAction;
	CAction*					mpcCapsLockPressedAction;
	CAction*					mpcInsertPressedAction;
	CAction*					mpcButtonPressedAction;
	CAction*					mpcButtonReleasedAction;
	CAction*					mpcNumpadButtonPressedAction;
	CAction*					mpcPointerMotionAction;

public:
	void Init(CInput* pcInput, CViewport* pcViewport);
	void InitActions(void);
	void Kill(void);

	BOOL						PointerInClientRect(void);
	CComponent*					GetFocussedComponent(void);
	BOOL						HasFocussedComponent(void);

	CComponentInputCommand*		GetCommand(CInputSourceDesc* pcSource, BOOL bControl, BOOL bAlt, BOOL bShift, BOOL bPressed);
	CComponentInputPrintable*	GetPrintable(CInputSourceDesc* pcSource);
	CComponentInputNumKey*		GetNumKeyRemap(CInputSourceDesc* pcSource);
	CComponentInputMotion*		GetMotion(CInputSourceDesc* pcSource);

	CSystemPointer*				GetPointer(void);
	CAction*					GetShiftPressedAction(void);
	CAction*					GetShiftReleasedAction(void);
	CAction*					GetControlPressedAction(void);
	CAction*					GetControlReleasedAction(void);
	CAction*					GetAltPressedAction(void);
	CAction*					GetAltReleasedAction(void);
	CAction*					GetNumLockPressedAction(void);
	CAction*					GetCapsLockPressedAction(void);
	CAction*					GetInsertPressedAction(void);
	CAction*					GetButtonPressedAction(void);
	CAction*					GetButtonReleasedAction(void);
	CAction*					GetNumpadButtonPressedAction(void);
	CAction*					GetPointerMotionAction(void);
	CInput*						GetInput(void);

	void ButtonPressed(CUnknown* pcSource, void* pvContext);
	void ButtonReleased(CUnknown* pcSource, void* pvContext);
	void NumpadButtonPressed(CUnknown* pcSource, void* pvContext);
	void ButtonToggle(CInputSourceDesc* pcSource, BOOL bIgnoreShift, BOOL bPressed);

	void ShiftPressed(CUnknown* pcSource, void* pvContext);
	void ShiftReleased(CUnknown* pcSource, void* pvContext);
	void ControlPressed(CUnknown* pcSource, void* pvContext);
	void ControlReleased(CUnknown* pcSource, void* pvContext);
	void AltPressed(CUnknown* pcSource, void* pvContext);
	void AltReleased(CUnknown* pcSource, void* pvContext);
	void CapsLockPressed(CUnknown* pcSource, void* pvContext);
	void NumLockPressed(CUnknown* pcSource, void* pvContext);
	void InsertPressed(CUnknown* pcSource, void* pvContext);
	void PointerMotion(CUnknown* pcSource, void* pvContext);

	//Navigation
	void Up(void);
	void Down(void);
	void Left(void);
	void Right(void);
	void WordLeft(void);
	void WordRight(void);
	void Home(void);
	void End(void);
	void DocumentHome(void);
	void DocumentEnd(void);
	void PageUp(void);
	void PageDown(void);
	void LineUp(void);
	void LineDown(void);

	//Selection
	void SelectUp(void);
	void SelectDown(void);
	void SelectLeft(void);
	void SelectRight(void);
	void SelectWordLeft(void);
	void SelectWordRight(void);
	void SelectHome(void);
	void SelectEnd(void);
	void SelectDocumentHome(void);
	void SelectDocumentEnd(void);
	void SelectPageUp(void);
	void SelectPageDown(void);
	void SelectWholeWord(void);
	void SelectAll(void);
	void ClearSelection(void);

	//Modification
	void Printable(char c);
	void Space(void);
	void Tab(void);
	void BackTab(void);
	void Enter(void);
	void HomeEnter(void);
	void EndEnter(void);

	//Modification
	void Delete(void);
	void Backspace(void);
	void Cut(void);
	void Copy(void);
	void Paste(void);
	void CutLine(void);
	void Duplicate(void);
	void DeleteWordRemainingRight(void);
	void DeleteWordRemainingLeft(void);
	void DeleteLine(void);

	//Component
	void ActivateComponent(int x, int y);
	void ExecuteComponent(int x, int y);
	void ContextMenu(int x, int y);
	void StartDragScroll(int x, int y);
	void ContDragScroll(int x, int y);
	void StopDragScroll(int x, int y);
	void StartDragSelect(int x, int y);
	void ContDragSelect(int x, int y);
	void StopDragSelect(int x, int y);
	void Hover(int x, int y);
};


#endif // __COMPONENT_INPUT_H__

