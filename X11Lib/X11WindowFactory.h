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
#include "WindowLib/NativeWindowFactory.h"


//The X11 headers #define None, Bool, Status, True, False and others so they are only included by the .cpp files, after
//everything else.  The headers only use the X types that can be forward declared; a Window and an Atom are XIDs.
struct _XDisplay;
typedef unsigned long	X11ID;


class CX11Input;
class CX11WindowFactory : public CNativeWindowFactory
{
CONSTRUCTABLE(CX11WindowFactory);
protected:
	_XDisplay*		mpsDisplay;
	int				miScreen;
	X11ID			mulDeleteWindowAtom;  //WM_DELETE_WINDOW, sent when the window's close button is clicked.
	X11ID			mulProtocolsAtom;  //WM_PROTOCOLS.
	CX11Input*		mpcInput;

public:
	bool			Init(CMallocator* pcMallocator, const char* szDisplayName);
	void			Kill(void);

	CNativeWindow*	CreateNativeWindow(CWindow* pcWindow) override;
	CNativeCanvas*	CreateNativeCanvas(CCanvas* pcCanvas) override;

	_XDisplay*		GetDisplay(void);
	int				GetScreen(void);
	X11ID			GetDeleteWindowAtom(void);
	X11ID			GetProtocolsAtom(void);

	void			SetInput(CX11Input* pcInput);
	CX11Input*		GetInput(void);
};


#endif // __X11_WINDOW_FACTORY_H__

