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
#ifndef __WINDOW_H__
#define __WINDOW_H__
#include "SupportLib/Rectangle.h"
#include "InputLib/Input.h"
#include "InputLib/InputListener.h"
#include "SystemPointer.h"
#include "Container.h"
#include "Viewport.h"


//There's a one to one relationship between CViewport and CWindow.  In theory the window is OS specific and the viewport isn't.
class CWindow : public CInputListener
{
CONSTRUCTABLE(CWindow);
public:
	CViewport*		mpcViewport;
	CInput*			mpcInput;
	CRectangle		mcClientRect;

	CChars			mszMenuName;
	CChars			mszClassName;
	CChars			mszWindowName;
	HINSTANCE		mhInstance;
	HWND			hWnd;
	HCURSOR			hCursor;
	bool			mbActive;
	bool			mbLoop;

	CTimer			mcLoopTimer;

	static int			GetScreenWidth(void);
	static int			GetScreenHeight(void);

	void 				Init(HINSTANCE hInst, char* szName, CInput* pcInput);
	virtual void 		Kill(void);
	void				Layout(void);
	bool				Draw(void);
	bool 				FullScreen(int nCmdShow);
	bool 				Windowed(int nCmdShow, int xp, int yp, int xs, int ys);
	bool				Create(int nCmdShow, int xp, int yp, int xs, int ys, int iStyle);
	void				Destroy(void);
	int					WinLoop(void);
	virtual void		ActiveTick(void);
	void				UpdateRectangle(void);
	void				GetClientSize(int* px, int* py);
	int					GetClientWidth(void);
	int					GetClientHeight(void);
	void				AddComponent(CComponent* pcComponent);
	void				RemoveComponent(CComponent* pcComponent, bool bKillComponent);
	void				RemoveAllComponents(bool bKillComponents);
	CComponent*			FindComponentAtRelative(int x, int y);
	bool				IsPointInRelative(int x, int y);
	CComponent*			FindComponentAtAbsolute(int x, int y);
	bool				IsPointInAbsolute(int x, int y);
	void				Activate(void);
	void				Deactivate(void);
	void				EndingWinloop(char* szReason);

	CViewport*			GetViewport(void);
	CInput*				GetInput(void);
};


#endif // __WINDOW_H__

