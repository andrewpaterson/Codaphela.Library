#ifndef __X11_WINDOW_FACTORY_H__
#define __X11_WINDOW_FACTORY_H__
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
#include "BaseLib/Define.h"
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "WindowLib/NativeWindowFactory.h"
#include "X11Clipboard.h"


//The X11 headers #define None, Bool, Status, True, False and others so they are only included by the .cpp files, after
//everything else.  The headers only use the X types that can be forward declared; a Window and an Atom are XIDs.
struct _XDisplay;
typedef unsigned long	X11ID;


class CX11Input;
class CX11Window;
typedef CArrayTemplatePtr<CX11Window>	CArrayX11WindowPtr;


//Every window shares the factory's display, so the factory reads the display's events and gives each to its window.
class CX11WindowFactory : public CNativeWindowFactory
{
CONSTRUCTABLE(CX11WindowFactory);
protected:
	_XDisplay*		mpsDisplay;
	int				miScreen;
	X11ID			mulDeleteWindowAtom;  //WM_DELETE_WINDOW, sent when the window's close button is clicked.
	X11ID			mulProtocolsAtom;  //WM_PROTOCOLS.
	CX11Input*		mpcInput;
	CArrayX11WindowPtr	mapcWindows;  //The windows that are on the screen.
	CX11Selection	mcSelection;  //The clipboard every window shares; it must outlive any one of them.
	bool			mbSharedMemoryAvailable;  //The X server has MIT-SHM, so windows can present from memory it shares.
	bool			mbSharedMemory;  //Windows present from shared memory when it is available, see SetSharedMemory.
	int				miSharedCompletionEvent;  //The event type the X server sends when it has read a window's shared pixels.

public:
	bool			Init(CMallocator* pcMallocator, const char* szDisplayName);
	void			Kill(void);

	CNativeWindow*	CreateNativeWindow(CWindow* pcWindow) override;
	CNativeCanvas*	CreateNativeCanvas(CCanvas* pcCanvas) override;
	CNativeClipboard*	CreateNativeClipboard(CClipboard* pcClipboard) override;

	_XDisplay*		GetDisplay(void);
	CX11Selection*	GetSelection(void);
	int				GetScreen(void);
	X11ID			GetDeleteWindowAtom(void);
	X11ID			GetProtocolsAtom(void);

	void			SetInput(CX11Input* pcInput);
	CX11Input*		GetInput(void);
	void			SetSharedMemory(bool bSharedMemory);
	bool			UsesSharedMemory(void);
	int				GetSharedCompletionEvent(void);

	void			AddWindow(CX11Window* pcWindow);
	void			RemoveWindow(CX11Window* pcWindow);
	CX11Window*		GetWindow(X11ID ulWindow);
	void			ExecuteEvents(void);
	void			WaitForEvents(int64 iMilliseconds);
	X11ID			InternAtom(const char* szName);
};


#endif // __X11_WINDOW_FACTORY_H__

