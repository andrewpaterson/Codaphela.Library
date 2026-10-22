#ifndef __X11_WINDOW_H__
#define __X11_WINDOW_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "WindowLib/NativeWindow.h"
#include "X11WindowFactory.h"
#include "X11Canvas.h"


struct _XGC;
union _XEvent;
class CX11Window : public CNativeWindow
{
CONSTRUCTABLE(CX11Window);
protected:
	X11ID			mulWindow;
	_XGC*			mpsGC;
	int				miWidth;
	int				miHeight;
	bool			mbPresentPending;  //Painted since the last present, or exposed.
	bool			mbPointerKnown;  //miPointerX and miPointerY hold where the pointer last moved to.
	int				miPointerX;
	int				miPointerY;

public:
	void	Init(CWindow* pcWindow, CNativeWindowFactory* pcWindowFactory);
	void	Kill(void);

	using	CNativeWindow::Present;
	void	Present(CNativeCanvas* pcNativeCanvas, int32 iWidth, int32 iHeight) override;

	bool	CreateNativeWindow(void) override;
	bool	ExecuteNativeWindow(void) override;

	void	GetRectangle(CInt32Rect* pcDest) override;
	void	SignalPresent(void) override;

protected:
	bool	HandleEvent(_XEvent* puEvent);
	void	MouseMoved(int x, int y);
	void	MouseButton(unsigned int uiButton, unsigned int uiState, int x, int y, bool bPressed);
	void	MouseLeft(unsigned int uiState);
	void	Activated(void);
	void	Deactivated(void);
	void	DestroyNativeWindow(void);
};


#endif // __X11_WINDOW_H__

