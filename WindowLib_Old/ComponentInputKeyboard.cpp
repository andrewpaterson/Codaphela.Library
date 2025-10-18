/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "ComponentInputCommand.h"
#include "ComponentInputKeyboard.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputKeyboard::Init(CInputDevice* pcDevice, CComponentInput* pcComponentInput)
{
	CInputVirtualDevice*		pcVirtual;

	pcVirtual = pcDevice->CreateDefaultVirtualDeviceFromThis("Text Editor Input");
	InitDevice(pcVirtual, pcDevice, pcComponentInput);
	InitKeyboard(pcComponentInput->GetInput());
	pcVirtual->Enable();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputKeyboard::InitKeyboard(CInput* pcInput)
{
	InitLocks(pcInput);
	InitModifiers(pcInput);
	InitButttons(pcInput);
	InitNumpadRemap();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputKeyboard::InitModifiers(CInput* pcInput)
{
	mpcVirtual->AddActiveAction(mpcComponentInput->GetShiftPressedAction(), mpcDevice->GetCategoryGeneric("Shift"));
	mpcVirtual->AddInactiveAction(mpcComponentInput->GetShiftReleasedAction(), mpcDevice->GetCategoryGeneric("Shift"));

	mpcVirtual->AddActiveAction(mpcComponentInput->GetControlPressedAction(), mpcDevice->GetCategoryGeneric("Control"));
	mpcVirtual->AddInactiveAction(mpcComponentInput->GetControlReleasedAction(), mpcDevice->GetCategoryGeneric("Control"));

	mpcVirtual->AddActiveAction(mpcComponentInput->GetAltPressedAction(), mpcDevice->GetCategoryGeneric("Alt"));
	mpcVirtual->AddInactiveAction(mpcComponentInput->GetAltReleasedAction(), mpcDevice->GetCategoryGeneric("Alt"));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputKeyboard::InitLocks(CInput* pcInput)
{
	mpcVirtual->AddActiveAction(mpcComponentInput->GetNumLockPressedAction(), "Num Lock");
	mpcVirtual->AddActiveAction(mpcComponentInput->GetCapsLockPressedAction(), "Caps Lock");
	mpcVirtual->AddActiveAction(mpcComponentInput->GetInsertPressedAction(), "Insert");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputKeyboard::InitButttons(CInput* pcInput)
{
	m1 = AddPrintable("1", '1', '!', '1', '!');
	m2 = AddPrintable("2", '2', '@', '2', '@');
	m3 = AddPrintable("3", '3', '#', '3', '#');
	m4 = AddPrintable("4", '4', '$', '4', '$');
	m5 = AddPrintable("5", '5', '%', '5', '%');
	m6 = AddPrintable("6", '6', '^', '6', '^');
	m7 = AddPrintable("7", '7', '&', '7', '&');
	m8 = AddPrintable("8", '8', '*', '8', '*');
	m9 = AddPrintable("9", '9', '(', '9', '(');
	m0 = AddPrintable("0", '0', ')', '0', ')');

	mBackTick =		AddPrintable("`", '`', '~', '`', '~');
	mMinus =		AddPrintable("-", '-', '_', '-', '_');
	mEquals =		AddPrintable("=", '=', '+', '=', '+');
	mOpenSquare =	AddPrintable("[", '[', '{', '[', '{');
	mCloseSquare = 	AddPrintable("]", ']', '}', ']', '}');
	mBackSlash	=	AddPrintable("\\", '\\', '|', '\\', '|');
	mSemiColon =	AddPrintable(";", ';', ':', ';', ':');
	mSingleQuote = 	AddPrintable("\'", '\'', '\"', '\'', '\"');
	mComma =		AddPrintable(",", ',', '<', ',', '<');
	mPeriod =		AddPrintable(".", '.', '>', '.', '>');
	mFrontSlash =	AddPrintable("/", '/', '?', '/', '?');

	mQ = AddPrintable("Q", 'q', 'Q', 'Q', 'q');
	mW = AddPrintable("W", 'w', 'W', 'W', 'w');
	mE = AddPrintable("E", 'e', 'E', 'E', 'e');
	mR = AddPrintable("R", 'r', 'R', 'R', 'r');
	mT = AddPrintable("T", 't', 'T', 'T', 't');
	mY = AddPrintable("Y", 'y', 'Y', 'Y', 'y');
	mU = AddPrintable("U", 'u', 'U', 'U', 'u');
	mI = AddPrintable("I", 'i', 'I', 'I', 'i');
	mO = AddPrintable("O", 'o', 'O', 'O', 'o');
	mP = AddPrintable("P", 'p', 'P', 'P', 'p');
	mA = AddPrintable("A", 'a', 'A', 'A', 'a');
	mS = AddPrintable("S", 's', 'S', 'S', 's');
	mD = AddPrintable("D", 'd', 'D', 'D', 'd');
	mF = AddPrintable("F", 'f', 'F', 'F', 'f');
	mG = AddPrintable("G", 'g', 'G', 'G', 'g');
	mH = AddPrintable("H", 'h', 'H', 'H', 'h');
	mJ = AddPrintable("J", 'j', 'J', 'J', 'j');
	mK = AddPrintable("K", 'k', 'K', 'K', 'k');
	mL = AddPrintable("L", 'l', 'L', 'L', 'l');
	mZ = AddPrintable("Z", 'z', 'Z', 'Z', 'z');
	mX = AddPrintable("X", 'x', 'X', 'X', 'x');
	mC = AddPrintable("C", 'c', 'C', 'C', 'c');
	mV = AddPrintable("V", 'v', 'V', 'V', 'v');
	mB = AddPrintable("B", 'b', 'B', 'B', 'b');
	mN = AddPrintable("N", 'n', 'N', 'N', 'n');
	mM = AddPrintable("M", 'm', 'M', 'M', 'm');

	mNumpadSlash =		AddPrintable("Numpad /", '/', '/', '/', '/');
	mNumpadAsterisk =	AddPrintable("Numpad *", '*', '*', '*', '*');
	mNumpadMinus =		AddPrintable("Numpad -", '-', '-', '-', '-');
	mNumpadPlus =		AddPrintable("Numpad +", '+', '+', '+', '+');

	mF1  = AddButtonPressedAction("F1", CIC_Ignored, false, false, false);
	mF2  = AddButtonPressedAction("F2", CIC_Ignored, false, false, false);
	mF3  = AddButtonPressedAction("F3", CIC_Ignored, false, false, false);
	mF4  = AddButtonPressedAction("F4", CIC_Ignored, false, false, false);
	mF5  = AddButtonPressedAction("F5", CIC_Ignored, false, false, false);
	mF6  = AddButtonPressedAction("F6", CIC_Ignored, false, false, false);
	mF7  = AddButtonPressedAction("F7", CIC_Ignored, false, false, false);
	mF8  = AddButtonPressedAction("F8", CIC_Ignored, false, false, false);
	mF9  = AddButtonPressedAction("F9", CIC_Ignored, false, false, false);
	mF10 = AddButtonPressedAction("F10", CIC_Ignored, false, false, false);
	mF11 = AddButtonPressedAction("F11", CIC_Ignored, false, false, false);
	mF12 = AddButtonPressedAction("F12", CIC_Ignored, false, false, false);

	mCursorUp =			AddButtonPressedAction(CIC_Up, "Cursor Up", false, false, false);
	mCursorDown =		AddButtonPressedAction(CIC_Down, "Cursor Down", false, false, false);
	mCursorLeft =		AddButtonPressedAction(CIC_Left, "Cursor Left", false, false, false);
	mCursorRight =		AddButtonPressedAction(CIC_Right, "Cursor Right", false, false, false);
	mCursorLeft =		AddButtonPressedAction(CIC_WordLeft, "Cursor Left", true, false, false);
	mCursorRight =		AddButtonPressedAction(CIC_WordRight, "Cursor Right", true, false, false);
	mHome =				AddButtonPressedAction(CIC_Home, "Home", false, false, false);
	mEnd =				AddButtonPressedAction(CIC_End, "End", false, false, false);
	mHome =				AddButtonPressedAction(CIC_DocumentHome, "Home", true, false, false);
	mEnd =				AddButtonPressedAction(CIC_DocumentEnd, "End", true, false, false);
	mPageUp =			AddButtonPressedAction(CIC_PageUp, "Page Up", false, false, false);
	mPageDown =			AddButtonPressedAction(CIC_PageDown, "Page Down", false, false, false);
	mCursorUp =			AddButtonPressedAction(CIC_LineUp, "Cursor Up", true, false, false);
	mCursorDown =		AddButtonPressedAction(CIC_LineDown, "Cursor Down", true, false, false);
	mCursorUp =			AddButtonPressedAction(CIC_SelectUp, "Cursor Up", false, false, true);
	mCursorDown =		AddButtonPressedAction(CIC_SelectDown, "Cursor Down", false, false, true);
	mCursorLeft =		AddButtonPressedAction(CIC_SelectLeft, "Cursor Left", false, false, true);
	mCursorRight =		AddButtonPressedAction(CIC_SelectRight, "Cursor Right", false, false, true);
	mCursorLeft =		AddButtonPressedAction(CIC_SelectWordLeft, "Cursor Left", true, false, true);
	mCursorRight =		AddButtonPressedAction(CIC_SelectWordRight, "Cursor Right", true, false, true);
	mHome =				AddButtonPressedAction(CIC_SelectHome, "Home", false, false, true);
	mEnd =				AddButtonPressedAction(CIC_SelectEnd, "End", false, false, true);
	mHome =				AddButtonPressedAction(CIC_SelectDocumentHome, "Home", true, false, true);
	mEnd =				AddButtonPressedAction(CIC_SelectDocumentEnd, "End", true, false, true);
	mPageUp =			AddButtonPressedAction(CIC_SelectPageUp, "Page Up", false, false, true);
	mPageDown =			AddButtonPressedAction(CIC_SelectPageDown, "Page Down", false, false, true);
	mW = 				AddButtonPressedAction(CIC_SelectWholeWord, "W", true, false, false);
	mA = 				AddButtonPressedAction(CIC_SelectAll, "A", true, false, false);
	mEscape =			AddButtonPressedAction(CIC_ClearSelection, "Escape", false, false, false);
	mSpace =			AddButtonPressedAction(CIC_Space, "Space", false, false, false);
	mSpace =			AddButtonPressedAction(CIC_Space, "Space", false, false, true);
	mTab =				AddButtonPressedAction(CIC_Tab, "Tab", false, false, false);
	mTab =				AddButtonPressedAction(CIC_BackTab, "Tab", false, false, true);
	mEnter =			AddButtonPressedAction(CIC_Enter, "Enter", false, false, false);
	mNumpadEnter =		AddButtonPressedAction(CIC_Enter, "Numpad Enter", false, false, false);
	mEnter =			AddButtonPressedAction(CIC_HomeEnter, "Enter", true, false, true);
	mNumpadEnter =		AddButtonPressedAction(CIC_HomeEnter, "Numpad Enter", true, false, true);
	mEnter =			AddButtonPressedAction(CIC_EndEnter, "Enter", false, false, true);
	mNumpadEnter =		AddButtonPressedAction(CIC_EndEnter, "Numpad Enter", false, false, true);
	mDelete =			AddButtonPressedAction(CIC_Delete, "Delete", false, false, false);
	mBackspace =		AddButtonPressedAction(CIC_Backspace, "Backspace", false, false, false);
	mX = 				AddButtonPressedAction(CIC_Cut, "X", true, false, false);
	mDelete =			AddButtonPressedAction(CIC_Cut, "Delete", false, false, true);
	mC = 				AddButtonPressedAction(CIC_Copy, "C", true, false, false);
	mInsert =			AddButtonPressedAction(CIC_Copy, "Insert", true, false, false);
	mV = 				AddButtonPressedAction(CIC_Paste, "V", true, false, false);
	mInsert =			AddButtonPressedAction(CIC_Paste, "Insert", false, false, true);
	mD = 				AddButtonPressedAction(CIC_Duplicate, "D", true, false, false);
	mBackspace =		AddButtonPressedAction(CIC_DeleteWordRemainingLeft, "Backspace", true, false, false);
	mDelete =			AddButtonPressedAction(CIC_DeleteWordRemainingRight, "Delete", true, false, false);
	mY = 				AddButtonPressedAction(CIC_DeleteLine, "Y", true, false, false);

	mPrintScreen =		mpcVirtual->GetSource("Print Screen");
	mScrollLock =		mpcVirtual->GetSource("Scroll Lock");
	mBreak =			mpcVirtual->GetSource("Pause/Break");
	mNumLock =			mpcVirtual->GetSource("Num Lock");
	mCapsLock =			mpcVirtual->GetSource("Caps Lock");
	mNumpad1 =			mpcVirtual->GetSource("Numpad 1");
	mNumpad2 =			mpcVirtual->GetSource("Numpad 2");
	mNumpad3 =			mpcVirtual->GetSource("Numpad 3");
	mNumpad4 =			mpcVirtual->GetSource("Numpad 4");
	mNumpad5 =			mpcVirtual->GetSource("Numpad 5");
	mNumpad6 =			mpcVirtual->GetSource("Numpad 6");
	mNumpad7 =			mpcVirtual->GetSource("Numpad 7");
	mNumpad8 =			mpcVirtual->GetSource("Numpad 8");
	mNumpad9 =			mpcVirtual->GetSource("Numpad 9");
	mNumpad0 =			mpcVirtual->GetSource("Numpad 0");
	mNumpadPeriod =		mpcVirtual->GetSource("Numpad .");

	mpcVirtual->AddActiveAction(mpcComponentInput->GetButtonPressedAction(), mPrintScreen);
	mpcVirtual->AddActiveAction(mpcComponentInput->GetButtonPressedAction(), mScrollLock);
	mpcVirtual->AddActiveAction(mpcComponentInput->GetButtonPressedAction(), mBreak);
	mpcVirtual->AddActiveAction(mpcComponentInput->GetButtonPressedAction(), mNumLock);
	mpcVirtual->AddActiveAction(mpcComponentInput->GetButtonPressedAction(), mCapsLock);
	mpcVirtual->AddActiveAction(mpcComponentInput->GetNumpadButtonPressedAction(), mNumpad1);
	mpcVirtual->AddActiveAction(mpcComponentInput->GetNumpadButtonPressedAction(), mNumpad2);
	mpcVirtual->AddActiveAction(mpcComponentInput->GetNumpadButtonPressedAction(), mNumpad3);
	mpcVirtual->AddActiveAction(mpcComponentInput->GetNumpadButtonPressedAction(), mNumpad4);
	mpcVirtual->AddActiveAction(mpcComponentInput->GetNumpadButtonPressedAction(), mNumpad5);
	mpcVirtual->AddActiveAction(mpcComponentInput->GetNumpadButtonPressedAction(), mNumpad6);
	mpcVirtual->AddActiveAction(mpcComponentInput->GetNumpadButtonPressedAction(), mNumpad7);
	mpcVirtual->AddActiveAction(mpcComponentInput->GetNumpadButtonPressedAction(), mNumpad8);
	mpcVirtual->AddActiveAction(mpcComponentInput->GetNumpadButtonPressedAction(), mNumpad9);
	mpcVirtual->AddActiveAction(mpcComponentInput->GetNumpadButtonPressedAction(), mNumpad0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputKeyboard::InitNumpadRemap(void)
{
	AddNumKeyRemap(mNumpad1->GetSourceDesc(), m1->GetSourceDesc(), mEnd->GetSourceDesc());
	AddNumKeyRemap(mNumpad2->GetSourceDesc(), m2->GetSourceDesc(), mCursorDown->GetSourceDesc());
	AddNumKeyRemap(mNumpad3->GetSourceDesc(), m3->GetSourceDesc(), mPageDown->GetSourceDesc());
	AddNumKeyRemap(mNumpad4->GetSourceDesc(), m4->GetSourceDesc(), mCursorLeft->GetSourceDesc());
	AddNumKeyRemap(mNumpad5->GetSourceDesc(), m5->GetSourceDesc(), NULL);
	AddNumKeyRemap(mNumpad6->GetSourceDesc(), m6->GetSourceDesc(), mCursorRight->GetSourceDesc());
	AddNumKeyRemap(mNumpad7->GetSourceDesc(), m7->GetSourceDesc(), mHome->GetSourceDesc());
	AddNumKeyRemap(mNumpad8->GetSourceDesc(), m8->GetSourceDesc(), mCursorUp->GetSourceDesc());
	AddNumKeyRemap(mNumpad9->GetSourceDesc(), m9->GetSourceDesc(), mPageUp->GetSourceDesc());
	AddNumKeyRemap(mNumpad0->GetSourceDesc(), m0->GetSourceDesc(), mDelete->GetSourceDesc());
	AddNumKeyRemap(mNumpadPeriod->GetSourceDesc(), mPeriod->GetSourceDesc(), mEnd->GetSourceDesc());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputKeyboard::Kill(void)
{
	CComponentInputDevice::Kill();
}



