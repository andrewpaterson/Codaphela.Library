/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 1998 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __WIN_INIT_H__
#define __WIN_INIT_H__
//I'm not sure what to do about this file... it really doesn't belong in BaseLib.
#ifdef _WIN32
#include "WindowsHeaders.h"
#include "Chars.h"
#include "GeometricTypes.h"


enum EShowType
{
	ST_Shown = true,
	ST_Hidden = false
};


enum EKeyType
{
	KT_Up,
	KT_Down
};


typedef void (*ActivateFunc)(bool bActive);
typedef void (*ShowFunc)(EShowType eType);
typedef void (*DestroyFunc)(void);
typedef void (*SetCursorFunc)(void);
typedef void (*KeyFunc)(int iKey, EKeyType eType);
typedef void (*SetFocusFunc)(void);
typedef void (*ChangingFunc)(WINDOWPOS* psPos);
typedef void (*SizeFunc)(int iSizeFlag, int iWidth, int iHeight);
typedef void (*MainFunc)(void);
typedef void (*RawInputFunc)(void* pvRawInputObject, void* pvRawInput);


class CWin32Window
{
public:
	HWND			hWnd;
	HINSTANCE		hInstance;
	ActivateFunc	mActivateFunc;
	ShowFunc		mShowFunc;
	DestroyFunc		mDestroyFunc;
	SetCursorFunc	mSetCursorFunc;
	KeyFunc			mKeyFunc;
	SetFocusFunc	mSetFocusFunc;
	ChangingFunc	mChangingFunc;
	MainFunc		mMainFunc;
	SizeFunc		mSizeFunc;
	RawInputFunc	mRawInputFunc;
	void*			mpvRawInputObject;
	bool			mbActive;
	LONG			clientAreaLeftOffset;
	LONG			clientAreaTopOffset;
	LONG			clientAreaRightOffset;
	LONG			clientAreaBottomOffset;
	CChars			className;
	HCURSOR			hCursor;
	bool			mbLoop;

	void		Init(HINSTANCE hInst, MainFunc Main, ActivateFunc Activate, ShowFunc Show, DestroyFunc Destroy, SetCursorFunc SetCursor, KeyFunc Key, SetFocusFunc SetFocus, ChangingFunc changing,	SizeFunc size, RawInputFunc rawInput, void* pvRawInputObject);
	bool		CreateFullScreenWindow(int nCmdShow, char* szName);
	bool		CreateWindowedWindow(int nCmdShow, char* Name, int xp, int yp, int xs, int ys);
	int			WinLoop(void);
	void		Destroy(void);

	void		GetSize(int* px, int* py);
	void		GetPosition(int* px, int* py);
	void		GetClientSize(int* px, int* py);
	static int	GetScreenWidth(void);
	static int	GetScreenHeight(void);
	void		GetCursorPositionInClientArea(int* px, int* py);
	void		GetNormalisedCursorPos(SFloat2* psPosition);
};

#endif // _WIN32
#endif // __WIN_INIT_H__

