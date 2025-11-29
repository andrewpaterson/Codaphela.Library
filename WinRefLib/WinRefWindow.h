#ifndef __WIN_REF_WINDOW_H__
#define __WIN_REF_WINDOW_H__
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
#include "WinRefCanvas.h"


class CWinRefWindow : public CNativeWindow
{
CONSTRUCTABLE(CWinRefWindow);
protected:
	CRectangle	mcBounds;
	bool		mbCreated;
	char*		mszImagePath;
	uint64		muiFrame;

public:
	void	Init(CWindow* pcWindow, CNativeWindowFactory* pcWindowFactory, CRectangle* pcBounds, char* szImagePath);
	void	Kill(void);

	void	Present(CNativeCanvas* pcNativeCanvas, int32 iWidth, int32 iHeight) override;

	bool	CreateNativeWindow(void) override;
	bool	ExecuteNativeWindow(void) override;

	void	GetRectangle(CRectangle* pcDest) override;
	void	SignalPresent(void) override;
};


#endif // __WIN_REF_WINDOW_H__

