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
#ifndef __CLIPBOARD_H__
#define __CLIPBOARD_H__
#include "BaseLib/Chars.h"
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"


//The system's clipboard, for cutting, copying and pasting text.  Each window has one (see CWindow::GetClipboard), all
//of them the same system clipboard, so text copied in one window, or another program, can be pasted in any.  The text
//is UTF8 and lines end in line feeds whatever the system uses.  Its CNativeClipboard is made by the window's factory.
class CWindow;
class CNativeClipboard;
class CClipboard : public CObject
{
CONSTRUCTABLE(CClipboard);
DESTRUCTABLE(CClipboard);
protected:
	CNativeClipboard*	mpcNativeClipboard;  //NULL if the factory couldn't make one, then nothing is copied or pasted.
	Ptr<CWindow>		mpWindow;

public:
	void				Init(Ptr<CWindow> pWindow);
	void				Class(void) override;
	void				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	bool				SetText(const char* szText);
	bool				GetText(CChars* pszDest);

	Ptr<CWindow>		GetWindow(void);
	CNativeClipboard*	GetNativeClipboard(void);
};


#endif // __CLIPBOARD_H__

