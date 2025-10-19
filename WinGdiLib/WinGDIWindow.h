#ifndef __WIN_GDI_WINDOW_H__
#define __WIN_GDI_WINDOW_H__
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
#include "SupportLib/Rectangle.h"
#include "InputLib/Input.h"
#include "WindowLib/NativeWindow.h"


class CWinGDIWindow : public CNativeWindow
{
CONSTRUCTABLE(CWinGDIWindow);
protected:
	CChars					mszWindowClass;
	HINSTANCE				mhInstance;
	HINSTANCE 				mhPrevInstance;
	int						miCmdShow;

public:
	void	Init(HINSTANCE hInstance, HINSTANCE hPrevInstance, int nCmdShow, const char* szWindowClass, const char* szWindowTitle);
	bool	Show(void);
	void	Kill(void);

protected:
	bool	CreateNativeWindow(void);
	bool	ExecuteNativeWindow(void);
};


#endif // __WIN_GDI_WINDOW_H__

