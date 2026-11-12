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
#ifndef __WIN_REF_WINDOW_H__
#define __WIN_REF_WINDOW_H__
#include "SupportLib/Int32Rect.h"
#include "WindowLib/NativeWindow.h"
#include "WindowLib/DamageArea.h"
#include "WinRefCanvas.h"
#include "WinRefPixels.h"


class CWinRefWindowFactory;
//A window on the reference screen (see CWinRefScreen): nothing is shown anywhere, but what it presents can be saved,
//checked and hashed, and it can sleep until something is due, as the factory says.
class CWinRefWindow : public CNativeWindow
{
CONSTRUCTABLE(CWinRefWindow);
protected:
	CInt32Rect		mcWindowBounds;
	bool			mbCreated;
	char*			mszImagePath;
	uint64			muiFrame;
	CDamageArea		mcPresented;  //What was asked to be shown since ClearPresented, for tests.
	CDamageArea		mcPresenting;  //What the next present shows.
	CWinRefPixels	mcShown;  //What the window shows on the screen, kept while the screen watches.

public:
	void			Init(CWindow* pcWindow, CNativeWindowFactory* pcWindowFactory, CInt32Rect* pcBounds, char* szImagePath);
	void			Kill(void);

	void			Present(CNativeCanvas* pcNativeCanvas, int32 iWidth, int32 iHeight) override;

	bool			CreateNativeWindow(void) override;
	bool			ExecuteNativeWindow(void) override;
	void			DestroyNativeWindow(void) override;
	bool			CanWaitForEvents(void) override;
	void			WaitForEvents(int64 iMilliseconds) override;
	void			Raise(void) override;

	void			GetRectangle(CInt32Rect* pcDest) override;
	void			SignalPresent(void) override;
	void			SignalPresentArea(CInt32Rect* pcArea) override;
	void			SignalPresentDamage(CDamageArea* pcDamage) override;
	CDamageArea*	GetPresented(void);
	void			ClearPresented(void);
	void			SetScreenRectangle(CInt32Rect* pcRectangle) override;
	bool			IsCreated(void) override;
	bool			GetScreenArea(CInt32Rect* pcDest) override;

	CWinRefPixels*	GetShown(void);
	const char*		GetTitle(void);

protected:
	CWinRefWindowFactory*	GetFactory(void);
};


#endif // __WIN_REF_WINDOW_H__

