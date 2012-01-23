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
#include "BaseLib/GeometricTypes.h"
#include "SupportLib/CopyPaste.h"
#include "Viewport.h"
#include "Window.h"
#include "ComponentInput.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::Init(CInput* pcInput, CViewport* pcViewport)
{
	CInputCategory*				pcKeyboards;
	CInputCategory*				pcMouses;
	CArrayInputDevicePtr		apcKeyboards;
	CArrayInputDevicePtr		apcMouses;
	int							i;
	CInputDevice*				pcKeyboard;
	CComponentInputKeyboard*	pcInputKeyboard;
	CInputDevice*				pcMouse;
	CComponentInputMouse*		pcInputMouse;

	mpcInput = pcInput;
	mpcViewport = pcViewport;

	InitActions();

	mlcInputDevices.Init();
	pcKeyboards = pcInput->GetCategory("Keyboard");
	apcKeyboards.Init();

	pcKeyboards->GetDevices(&apcKeyboards, FALSE);
	for (i = 0; i < apcKeyboards.NumElements(); i++)
	{
		pcKeyboard = *apcKeyboards.Get(i);
		pcInputKeyboard = mlcInputDevices.Add<CComponentInputKeyboard>();
		pcInputKeyboard->Init(pcKeyboard, this);
	}
	apcKeyboards.Kill();

	pcMouses = pcInput->GetCategory("Mouse");
	apcMouses.Init();

	pcMouses->GetDevices(&apcMouses, FALSE);
	for (i = 0; i < apcMouses.NumElements(); i++)
	{
		pcMouse = *apcMouses.Get(i);
		pcInputMouse = mlcInputDevices.Add<CComponentInputMouse>();
		pcInputMouse->Init(pcMouse, this);
	}
	apcMouses.Kill();


	miControl = 0;
	miAlt = 0;
	miShift = 0;
	mbCapsLock = FALSE;
	mbInsert = TRUE;
	mbNumLock = FALSE;

	mcPointer.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::InitActions(void)
{
	CInputActions*	pcActions;

	pcActions = mpcInput->GetActions();
	mpcShiftPressedAction = pcActions->AddAction("Shift Pressed", &CComponentInput::ShiftPressed, this);
	mpcShiftReleasedAction = pcActions->AddAction("Shift Released", &CComponentInput::ShiftReleased, this);
	mpcControlPressedAction = pcActions->AddAction("Control Pressed", &CComponentInput::ControlPressed, this);
	mpcControlReleasedAction = pcActions->AddAction("Control Released", &CComponentInput::ControlReleased, this);
	mpcAltPressedAction = pcActions->AddAction("Alt Pressed", &CComponentInput::AltPressed, this);
	mpcAltReleasedAction = pcActions->AddAction("Alt Released", &CComponentInput::AltReleased, this);

	mpcNumLockPressedAction = pcActions->AddAction("Num Lock Pressed", &CComponentInput::NumLockPressed, this);
	mpcCapsLockPressedAction = pcActions->AddAction("Caps Lock Pressed", &CComponentInput::CapsLockPressed, this);
	mpcInsertPressedAction = pcActions->AddAction("Insert Pressed", &CComponentInput::InsertPressed, this);

	mpcButtonPressedAction = pcActions->AddAction("Button Pressed", &CComponentInput::ButtonPressed, this);
	mpcNumpadButtonPressedAction = pcActions->AddAction("Numpad Button Pressed", &CComponentInput::NumpadButtonPressed, this);
	mpcButtonReleasedAction = pcActions->AddAction("Button Released", &CComponentInput::ButtonReleased, this);

	mpcPointerMotionAction = pcActions->AddAction("Pointer Motion", &CComponentInput::PointerMotion, this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::Kill(void)
{
	mcPointer.Kill();

	mlcInputDevices.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponentInputCommand* CComponentInput::GetCommand(CInputSourceDesc* pcSource, BOOL bControl, BOOL bAlt, BOOL bShift, BOOL bPressed)
{
	SSetIterator				sIter;
	CComponentInputDevice*		pcInputDevice;
	CComponentInputCommand*		pcCommand;

	pcInputDevice = (CComponentInputDevice*)mlcInputDevices.StartIteration(&sIter);
	while (pcInputDevice)
	{
		pcCommand = pcInputDevice->GetCommand(pcSource, bControl, bAlt, bShift, bPressed);
		if (pcCommand)
		{
			return pcCommand;
		}
		pcInputDevice = (CComponentInputDevice*)mlcInputDevices.Iterate(&sIter);	
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponentInputPrintable* CComponentInput::GetPrintable(CInputSourceDesc* pcSource)
{
	SSetIterator				sIter;
	CComponentInputDevice*	pcInputDevice;
	CComponentInputPrintable*	pcPrintable;

	pcInputDevice = (CComponentInputDevice*)mlcInputDevices.StartIteration(&sIter);
	while (pcInputDevice)
	{
		pcPrintable = pcInputDevice->GetPrintable(pcSource);
		if (pcPrintable)
		{
			return pcPrintable;
		}
		pcInputDevice = (CComponentInputDevice*)mlcInputDevices.Iterate(&sIter);	
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponentInputNumKey* CComponentInput::GetNumKeyRemap(CInputSourceDesc* pcSource)
{
	SSetIterator				sIter;
	CComponentInputDevice*	pcInputDevice;
	CComponentInputNumKey*		pcRemap;

	pcInputDevice = (CComponentInputDevice*)mlcInputDevices.StartIteration(&sIter);
	while (pcInputDevice)
	{
		pcRemap = pcInputDevice->GetNumKeyRemap(pcSource);
		if (pcRemap)
		{
			return pcRemap;
		}
		pcInputDevice = (CComponentInputDevice*)mlcInputDevices.Iterate(&sIter);	
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponentInputMotion* CComponentInput::GetMotion(CInputSourceDesc* pcSource)
{
	SSetIterator			sIter;
	CComponentInputDevice*	pcInputDevice;
	CComponentInputMotion*	pcMotion;

	pcInputDevice = (CComponentInputDevice*)mlcInputDevices.StartIteration(&sIter);
	while (pcInputDevice)
	{
		pcMotion = pcInputDevice->GetMotion(pcSource);
		if (pcMotion)
		{
			return pcMotion;
		}
		pcInputDevice = (CComponentInputDevice*)mlcInputDevices.Iterate(&sIter);	
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSystemPointer* CComponentInput::GetPointer(void) {	return &mcPointer; }
CAction* CComponentInput::GetShiftPressedAction(void){ return mpcShiftPressedAction;}
CAction* CComponentInput::GetShiftReleasedAction(void){ return mpcShiftReleasedAction;}
CAction* CComponentInput::GetControlPressedAction(void){ return mpcControlPressedAction;}
CAction* CComponentInput::GetControlReleasedAction(void){ return mpcControlReleasedAction;}
CAction* CComponentInput::GetAltPressedAction(void){ return mpcAltPressedAction;}
CAction* CComponentInput::GetAltReleasedAction(void){ return mpcAltReleasedAction;}
CAction* CComponentInput::GetNumLockPressedAction(void){ return mpcNumLockPressedAction;}
CAction* CComponentInput::GetCapsLockPressedAction(void){ return mpcCapsLockPressedAction;}
CAction* CComponentInput::GetInsertPressedAction(void){ return mpcInsertPressedAction;}
CAction* CComponentInput::GetButtonPressedAction(void){ return mpcButtonPressedAction;}
CAction* CComponentInput::GetButtonReleasedAction(void){ return mpcButtonReleasedAction;}
CAction* CComponentInput::GetNumpadButtonPressedAction(void){ return mpcNumpadButtonPressedAction;}
CAction* CComponentInput::GetPointerMotionAction(void){ return mpcPointerMotionAction;}
CInput* CComponentInput::GetInput(void) { return mpcInput; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::ButtonToggle(CInputSourceDesc* pcSource, BOOL bIgnoreShift, BOOL bPressed)
{
	CComponentInputPrintable*	psPrintable;
	BOOL						bControl;
	BOOL						bAlt;
	BOOL						bShift;
	CComponentInputCommand*		psCommand;
	int							x;
	int							y;

	if (!pcSource)
	{
		return;
	}

	bControl = miControl > 0;
	bAlt = miAlt > 0;
	if (!bIgnoreShift)
	{
		bShift = miShift > 0;
	}
	else
	{
		bShift = FALSE;
	}

	psCommand = GetCommand(pcSource, bControl, bAlt, bShift, bPressed);
	if (psCommand)
	{
		x = mcPointer.GetX();
		y = mcPointer.GetX();

		switch(psCommand->eCommand)
		{
		case CIC_Up:
			Up();
			break;
		case CIC_Down:
			Down();
			break;
		case CIC_Left:
			Left();
			break;
		case CIC_Right:
			Right();
			break;
		case CIC_WordLeft:
			WordLeft();
			break;
		case CIC_WordRight:
			WordRight();
			break;
		case CIC_Home:
			Home();
			break;
		case CIC_End:
			End();
			break;
		case CIC_DocumentHome:
			DocumentHome();
			break;
		case CIC_DocumentEnd:
			DocumentEnd();
			break;
		case CIC_PageUp:
			PageUp();
			break;
		case CIC_PageDown:
			PageDown();
			break;
		case CIC_LineUp:
			LineUp();
			break;
		case CIC_LineDown:
			LineDown();
			break;
		case CIC_SelectUp:
			SelectUp();
			break;
		case CIC_SelectDown:
			SelectDown();
			break;
		case CIC_SelectLeft:
			SelectLeft();
			break;
		case CIC_SelectRight:
			SelectRight();
			break;
		case CIC_SelectWordLeft:
			SelectWordLeft();
			break;
		case CIC_SelectWordRight:
			SelectWordRight();
			break;
		case CIC_SelectHome:
			SelectHome();
			break;
		case CIC_SelectEnd:
			SelectEnd();
			break;
		case CIC_SelectDocumentHome:
			SelectDocumentHome();
			break;
		case CIC_SelectDocumentEnd:
			SelectDocumentEnd();
			break;
		case CIC_SelectPageUp:
			SelectPageUp();
			break;
		case CIC_SelectPageDown:
			SelectPageDown();
			break;
		case CIC_SelectWholeWord:
			SelectWholeWord();
			break;
		case CIC_SelectAll:
			SelectAll();
			break;
		case CIC_ClearSelection:
			ClearSelection();
			break;
		case CIC_Space:
			Space();
			break;
		case CIC_Tab:
			Tab();
			break;
		case CIC_BackTab:
			BackTab();
			break;
		case CIC_Enter:
			Enter();
			break;
		case CIC_HomeEnter:
			HomeEnter();
			break;
		case CIC_EndEnter:
			EndEnter();
			break;
		case CIC_Delete:
			Delete();
			break;
		case CIC_Backspace:
			Backspace();
			break;
		case CIC_Cut:
			Cut();
			break;
		case CIC_Copy:
			Copy();
			break;
		case CIC_Paste:
			Paste();
			break;
		case CIC_CutLine:
			CutLine();
			break;
		case CIC_Duplicate:
			Duplicate();
			break;
		case CIC_DeleteWordRemainingLeft:
			DeleteWordRemainingLeft();
			break;
		case CIC_DeleteWordRemainingRight:
			DeleteWordRemainingRight();
			break;
		case CIC_DeleteLine:
			DeleteLine();
			break;
		case CIC_ActivateComponent:
			ActivateComponent(x, y);
			break;
		case CIC_ExecuteComponent:
			ExecuteComponent(x, y);
			break;
		case CIC_ContextMenu:
			ContextMenu(x, y);
			break;
		case CIC_StartDragScroll:
			StartDragScroll(x, y);
			break;
		case CIC_StopDragScroll:
			StopDragScroll(x, y);
			break;
		case CIC_StartDragSelect:
			StartDragSelect(x, y);
			break;
		case CIC_StopDragSelect:
			StopDragSelect(x, y);
			break;
		case CIC_Hover:
			Hover(x, y);
			break;

		case CIC_Ignored:
			break;
		}
		return;
	}

	psPrintable = GetPrintable(pcSource);
	if (psPrintable)
	{
		if ((!bAlt) && (!bControl))
		{
			if (miShift > 0)
			{
				if (mbCapsLock)
				{
					Printable(psPrintable->cYesShiftYesCaps);
				}
				else
				{
					Printable(psPrintable->cYesShiftNoCaps);
				}
			}
			else
			{
				if (mbCapsLock)
				{
					Printable(psPrintable->cNoShiftYesCaps);
				}
				else
				{
					Printable(psPrintable->cNoShiftNoCaps);
				}
			}
		}
		return;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::ButtonPressed(CUnknown* pcSource, void* pvContext)
{
	SChordInputEvent*		pcValue;

	pcValue = (SChordInputEvent*)pvContext;
	ButtonToggle(pcValue->pcValue->pcSource, FALSE, TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::ButtonReleased(CUnknown* pcSource, void* pvContext)
{
	SChordInputEvent*		pcValue;

	pcValue = (SChordInputEvent*)pvContext;
	ButtonToggle(pcValue->pcValue->pcSource, FALSE, FALSE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::PointerMotion(CUnknown* pcSource, void* pvContext)
{ 
	SChordInputEvent*		pcValue;
	CComponentInputMotion*	pcMotion;
	SFloat2					sMotion;

	pcValue = (SChordInputEvent*)pvContext;

	if (PointerInClientRect())
	{
		pcMotion = GetMotion(pcValue->pcValue->pcSource);
		if (pcMotion)
		{
			sMotion.x = pcValue->pcValue->fValue * pcMotion->mfXMultiplier * mcPointer.mfSpeed;
			sMotion.y = pcValue->pcValue->fValue * pcMotion->mfYMultiplier * mcPointer.mfSpeed;

			//mcPointer.Move(sMotion.x, sMotion.y);
		}
		mcPointer.Update();
	}
	else
	{
		mcPointer.Update();
	}

	mpcViewport->mcFocus.Update(mcPointer.GetX(), mcPointer.GetY());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CComponentInput::PointerInClientRect(void)
{
	return mpcViewport->IsPointInAbsolute(mcPointer.GetX(), mcPointer.GetY());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::NumpadButtonPressed(CUnknown* pcSource, void* pvContext)
{
	SChordInputEvent*		pcValue;
	CComponentInputNumKey*		psNumKey;

	pcValue = (SChordInputEvent*)pvContext;

	psNumKey = GetNumKeyRemap(pcValue->pcValue->pcSource);
	if (!mbNumLock)
	{
		ButtonToggle(psNumKey->pcNoNumLockRemap, TRUE, TRUE);
	}
	else
	{
		ButtonToggle(psNumKey->pcYesNumLockRemap, FALSE, TRUE);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::ShiftPressed(CUnknown* pcSource, void* pvContext)
{
	miShift++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::ShiftReleased(CUnknown* pcSource, void* pvContext)
{
	miShift--;
	if (miShift < 0)
	{
		miShift = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::ControlPressed(CUnknown* pcSource, void* pvContext)
{
	miControl++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::ControlReleased(CUnknown* pcSource, void* pvContext)
{
	miControl--;
	if (miControl < 0)
	{
		miControl = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::AltPressed(CUnknown* pcSource, void* pvContext)
{
	miAlt++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::AltReleased(CUnknown* pcSource, void* pvContext)
{
	miAlt--;
	if (miAlt < 0)
	{
		miAlt = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::CapsLockPressed(CUnknown* pcSource, void* pvContext)
{
	mbCapsLock = !mbCapsLock;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::NumLockPressed(CUnknown* pcSource, void* pvContext)
{
	mbNumLock = !mbNumLock;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::InsertPressed(CUnknown* pcSource, void* pvContext)
{
	mbInsert = !mbInsert;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponent* CComponentInput::GetFocussedComponent(void)
{
	return mpcViewport->GetFocussedComponent();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CComponentInput::HasFocussedComponent(void)
{
	return mpcViewport->GetFocussedComponent() != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInput::Up(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->Up();
	}
}

void CComponentInput::Down(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->Down();
	}
}

void CComponentInput::Left(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->Left();
	}
}

void CComponentInput::Right(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->Right();
	}
}

void CComponentInput::WordLeft(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->WordLeft();
	}
}

void CComponentInput::WordRight(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->WordRight();
	}
}

void CComponentInput::Home(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->Home();
	}
}

void CComponentInput::End(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->End();
	}
}

void CComponentInput::DocumentHome(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->DocumentHome();
	}
}

void CComponentInput::DocumentEnd(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->DocumentEnd();
	}
}

void CComponentInput::PageUp(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->PageUp();
	}
}

void CComponentInput::PageDown(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->PageDown();
	}
}

void CComponentInput::LineUp(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->LineUp();
	}
}

void CComponentInput::LineDown(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->LineDown();
	}
}

void CComponentInput::SelectUp(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->SelectUp();
	}
}

void CComponentInput::SelectDown(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->SelectDown();
	}
}

void CComponentInput::SelectLeft(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->SelectLeft();
	}
}

void CComponentInput::SelectRight(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->SelectRight();
	}
}

void CComponentInput::SelectWordLeft(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->SelectWordLeft();
	}
}

void CComponentInput::SelectWordRight(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->SelectWordRight();
	}
}

void CComponentInput::SelectHome(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->SelectHome();
	}
}

void CComponentInput::SelectEnd(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->SelectEnd();
	}
}

void CComponentInput::SelectDocumentHome(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->SelectDocumentHome();
	}
}

void CComponentInput::SelectDocumentEnd(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->SelectDocumentEnd();
	}
}

void CComponentInput::SelectPageUp(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->SelectPageUp();
	}
}

void CComponentInput::SelectPageDown(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->SelectPageDown();
	}
}

void CComponentInput::SelectWholeWord(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->SelectWholeWord();
	}
}

void CComponentInput::SelectAll(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->SelectAll();
	}
}

void CComponentInput::ClearSelection(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->ClearSelection();
	}
}

void CComponentInput::Printable(char c)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->Printable(c, mbInsert);
	}
}

void CComponentInput::Space(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->Space(mbInsert);
	}
}

void CComponentInput::Tab(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->Tab(mbInsert);
	}

}

void CComponentInput::BackTab(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->BackTab();
	}
}

void CComponentInput::Enter(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->Enter();
	}
}

void CComponentInput::HomeEnter(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->HomeEnter();
	}
}

void CComponentInput::EndEnter(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->EndEnter();
	}
}


void CComponentInput::Delete(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->Delete();
	}
}

void CComponentInput::Backspace(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->Backspace();
	}
}

void CComponentInput::Cut(void)
{
	CCopyPaste	cCopyPaste;
	CChars		szDest;

	if (HasFocussedComponent())
	{
		szDest.Init(20);
		GetFocussedComponent()->Cut(&szDest);
		cCopyPaste.Init();
		cCopyPaste.CopyText(&szDest);
		cCopyPaste.Kill();
		szDest.Kill();
	}
}

void CComponentInput::Copy(void)
{
	CCopyPaste	cCopyPaste;
	CChars		szDest;

	if (HasFocussedComponent())
	{
		szDest.Init(20);
		GetFocussedComponent()->Copy(&szDest);
		cCopyPaste.Init();
		cCopyPaste.CopyText(&szDest);
		cCopyPaste.Kill();
		szDest.Kill();
	}
}

void CComponentInput::Paste(void)
{
	CChars		szSource;
	CCopyPaste	cCopyPaste;

	if (HasFocussedComponent())
	{
		szSource.Init(20);
		cCopyPaste.Init();
		cCopyPaste.PasteText(&szSource);
		GetFocussedComponent()->Paste(szSource.Text());
		cCopyPaste.Kill();
		szSource.Kill();
	}
}

void CComponentInput::CutLine(void)
{
	CCopyPaste	cCopyPaste;
	CChars		szDest;

	if (HasFocussedComponent())
	{
		szDest.Init(20);
		GetFocussedComponent()->CutLine(&szDest);
		cCopyPaste.Init();
		cCopyPaste.CopyText(&szDest);
		cCopyPaste.Kill();
		szDest.Kill();
	}
}

void CComponentInput::Duplicate(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->Duplicate();
	}
}

void CComponentInput::DeleteWordRemainingRight(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->DeleteWordRemainingRight();
	}
}

void CComponentInput::DeleteWordRemainingLeft(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->DeleteWordRemainingLeft();
	}
}

void CComponentInput::DeleteLine(void)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->DeleteLine();
	}
}

void CComponentInput::ActivateComponent(int x, int y)
{
	mpcViewport->mcFocus.Request();
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->ActivateComponent(x, y);
	}
}

void CComponentInput::ExecuteComponent(int x, int y)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->ExecuteComponent(x, y);
	}
}

void CComponentInput::ContextMenu(int x, int y)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->ContextMenu(x, y);
	}
}

void CComponentInput::StartDragScroll(int x, int y)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->StartDragScroll(x, y);
	}
}

void CComponentInput::ContDragScroll(int x, int y)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->ContDragScroll(x, y);
	}
}

void CComponentInput::StopDragScroll(int x, int y)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->StopDragScroll(x, y);
	}
}

void CComponentInput::StartDragSelect(int x, int y)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->StartDragSelect(x, y);
	}
}

void CComponentInput::ContDragSelect(int x, int y)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->ContDragSelect(x, y);
	}
}

void CComponentInput::StopDragSelect(int x, int y)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->StopDragSelect(x, y);
	}
}

void CComponentInput::Hover(int x, int y)
{
	if (HasFocussedComponent())
	{
		GetFocussedComponent()->Hover(x, y);
	}
}
