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
#ifndef __COMPONENT_INPUT_KEYBOARD_H__
#define __COMPONENT_INPUT_KEYBOARD_H__
#include "ComponentInputDevice.h"


class CComponentInputKeyboard : public CComponentInputDevice
{
public:
	CONSTRUCTABLE(CComponentInputKeyboard);

	CInputVirtualDeviceSource*	mQ;
	CInputVirtualDeviceSource*	mW;
	CInputVirtualDeviceSource*	mE;
	CInputVirtualDeviceSource*	mR;
	CInputVirtualDeviceSource*	mT;
	CInputVirtualDeviceSource*	mY;
	CInputVirtualDeviceSource*	mU;
	CInputVirtualDeviceSource*	mI;
	CInputVirtualDeviceSource*	mO;
	CInputVirtualDeviceSource*	mP;
	CInputVirtualDeviceSource*	mA;
	CInputVirtualDeviceSource*	mS;
	CInputVirtualDeviceSource*	mD;
	CInputVirtualDeviceSource*	mF;
	CInputVirtualDeviceSource*	mG;
	CInputVirtualDeviceSource*	mH;
	CInputVirtualDeviceSource*	mJ;
	CInputVirtualDeviceSource*	mK;
	CInputVirtualDeviceSource*	mL;
	CInputVirtualDeviceSource*	mZ;
	CInputVirtualDeviceSource*	mX;
	CInputVirtualDeviceSource*	mC;
	CInputVirtualDeviceSource*	mV;
	CInputVirtualDeviceSource*	mB;
	CInputVirtualDeviceSource*	mN;
	CInputVirtualDeviceSource*	mM;

	CInputVirtualDeviceSource*	m1;
	CInputVirtualDeviceSource*	m2;
	CInputVirtualDeviceSource*	m3;
	CInputVirtualDeviceSource*	m4;
	CInputVirtualDeviceSource*	m5;
	CInputVirtualDeviceSource*	m6;
	CInputVirtualDeviceSource*	m7;
	CInputVirtualDeviceSource*	m8;
	CInputVirtualDeviceSource*	m9;
	CInputVirtualDeviceSource*	m0;

	CInputVirtualDeviceSource*	mBackTick;
	CInputVirtualDeviceSource*	mMinus;
	CInputVirtualDeviceSource*	mEquals;
	CInputVirtualDeviceSource*	mOpenSquare;
	CInputVirtualDeviceSource*	mCloseSquare;
	CInputVirtualDeviceSource*	mSemiColon;
	CInputVirtualDeviceSource*	mComma;
	CInputVirtualDeviceSource*	mPeriod;
	CInputVirtualDeviceSource*	mFrontSlash;
	CInputVirtualDeviceSource*	mBackSlash;
	CInputVirtualDeviceSource*	mSingleQuote;

	CInputVirtualDeviceSource*	mF1;
	CInputVirtualDeviceSource*	mF2;
	CInputVirtualDeviceSource*	mF3;
	CInputVirtualDeviceSource*	mF4;
	CInputVirtualDeviceSource*	mF5;
	CInputVirtualDeviceSource*	mF6;
	CInputVirtualDeviceSource*	mF7;
	CInputVirtualDeviceSource*	mF8;
	CInputVirtualDeviceSource*	mF9;
	CInputVirtualDeviceSource*	mF10;
	CInputVirtualDeviceSource*	mF11;
	CInputVirtualDeviceSource*	mF12;

	CInputVirtualDeviceSource*	mSpace;
	CInputVirtualDeviceSource*	mCursorUp;
	CInputVirtualDeviceSource*	mCursorLeft;
	CInputVirtualDeviceSource*	mCursorDown;
	CInputVirtualDeviceSource*	mCursorRight;
	CInputVirtualDeviceSource*	mEscape;
	CInputVirtualDeviceSource*	mPrintScreen;
	CInputVirtualDeviceSource*	mScrollLock;
	CInputVirtualDeviceSource*	mBreak;
	CInputVirtualDeviceSource*	mBackspace;
	CInputVirtualDeviceSource*	mInsert;
	CInputVirtualDeviceSource*	mHome;
	CInputVirtualDeviceSource*	mPageUp;
	CInputVirtualDeviceSource*	mNumLock;
	CInputVirtualDeviceSource*	mTab;
	CInputVirtualDeviceSource*	mDelete;
	CInputVirtualDeviceSource*	mEnd;
	CInputVirtualDeviceSource*	mPageDown;
	CInputVirtualDeviceSource*	mCapsLock;
	CInputVirtualDeviceSource*	mEnter;
	CInputVirtualDeviceSource*	mNumpadSlash;
	CInputVirtualDeviceSource*	mNumpadAsterisk;
	CInputVirtualDeviceSource*	mNumpadMinus;
	CInputVirtualDeviceSource*	mNumpadPlus;
	CInputVirtualDeviceSource*	mNumpadEnter;
	CInputVirtualDeviceSource*	mNumpad1;
	CInputVirtualDeviceSource*	mNumpad2;
	CInputVirtualDeviceSource*	mNumpad3;
	CInputVirtualDeviceSource*	mNumpad4;
	CInputVirtualDeviceSource*	mNumpad5;
	CInputVirtualDeviceSource*	mNumpad6;
	CInputVirtualDeviceSource*	mNumpad7;
	CInputVirtualDeviceSource*	mNumpad8;
	CInputVirtualDeviceSource*	mNumpad9;
	CInputVirtualDeviceSource*	mNumpad0;
	CInputVirtualDeviceSource*	mNumpadPeriod;

	void 						Init(CInputDevice* pcDevice, CComponentInput* pcComponentInput);
	void 						InitKeyboard(CInput* pcInput);
	void 						InitModifiers(CInput* pcInput);
	void 						InitButttons(CInput* pcInput);
	void 						InitLocks(CInput* pcInput);
	void 						InitPrintables(void);
	void 						InitNumpadRemap(void);
	void 						Kill(void);
};


typedef CSetUnknown/*Type<CComponentInputDevice>*/	CSetComponentInputDevices;


#endif // __COMPONENT_INPUT_KEYBOARD_H__

