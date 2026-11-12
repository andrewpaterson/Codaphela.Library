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
#include "BaseLib/Chars.h"
#include "WindowLib/NativeWindowFactory.h"
#include "WinRefScreen.h"


//How long a reference window that waits for events sleeps at most (SetWaitForEvents).
#define WIN_REF_MAX_WAIT	50


//Makes windows drawn off screen, on a reference screen of its own (CWinRefScreen).  By default they are ticked and
//painted continuously and save every frame to the image path, if there is one.
class CWinRefWindowFactory : public CNativeWindowFactory
{
CONSTRUCTABLE(CWinRefWindowFactory);
protected:
	CInt32Rect		mcWindowBounds;
	CChars			mszImagePath;
	CChars			mszClipboardText;  //The reference system's clipboard, shared by every window the factory makes.
	CWinRefScreen	mcScreen;
	int64			miMaxWait;  //-1 for windows that don't wait for events.
	uint32			muiFrameInterval;

public:
	void				Init(CMallocator* pcMallocator, int32 iWidth, int32 iHeight, char* szImagePath);
	void				Kill(void);

	void				SetScreenArea(CInt32Rect* pcArea);
	bool				GetScreenArea(CInt32Rect* pcDest);
	void				SetWaitForEvents(bool bWait, int64 iMaxWait = WIN_REF_MAX_WAIT);
	int64				GetMaxWait(void);
	void				SetCheckPresents(bool bCheck);
	bool				SetHashPresents(const char* szFileName, EWinRefHashArea eArea);
	void				SetFrameInterval(uint32 uiInterval);
	bool				IsFrameSaved(uint64 uiFrame);
	CWinRefScreen*		GetScreen(void);

	CNativeWindow*		CreateNativeWindow(CWindow* pcWindow) override;
	CNativeCanvas*		CreateNativeCanvas(CCanvas* pcCanvas) override;
	CNativeClipboard*	CreateNativeClipboard(CClipboard* pcClipboard) override;

	int32				GetWidth(void);
	int32				GetHeight(void);
	CChars*				GetClipboardText(void);
};


#endif // __WIN_REF_WINDOW_FACTORY_H__

