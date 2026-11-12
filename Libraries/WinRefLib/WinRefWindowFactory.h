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
#ifndef __WIN_REF_WINDOW_FACTORY_H__
#define __WIN_REF_WINDOW_FACTORY_H__
#include "BaseLib/Define.h"
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/Chars.h"
#include "SupportLib/ColourFormat.h"
#include "WindowLib/NativeWindowFactory.h"


class CWinRefWindowFactory : public CNativeWindowFactory
{
CONSTRUCTABLE(CWinRefWindowFactory);
protected:
	CInt32Rect	mcWindowBounds;
	CChars		mszImagePath;
	CChars		mszClipboardText;  //The reference system's clipboard, shared by every window the factory makes.
	CInt32Rect	mcScreenArea;  //The reference screen, empty until set: its size isn't known.

public:
	void			Init(CMallocator* pcMallocator, int32 iWidth, int32 iHeight, char* szImagePath);
	void			Kill(void);
	void			SetScreenArea(CInt32Rect* pcArea);
	bool			GetScreenArea(CInt32Rect* pcDest);

	CNativeWindow*	CreateNativeWindow(CWindow* pcWindow) override;
	CNativeCanvas*	CreateNativeCanvas(CCanvas* pcCanvas) override;
	CNativeClipboard*	CreateNativeClipboard(CClipboard* pcClipboard) override;

	int32			GetWidth(void);
	int32			GetHeight(void);
	CChars*			GetClipboardText(void);

protected:

};


#endif // __WIN_REF_WINDOW_FACTORY_H__

