#ifndef __WIN_GDI_CLIPBOARD_H__
#define __WIN_GDI_CLIPBOARD_H__
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
#include <windows.h>
#include "WindowLib/NativeClipboard.h"


//The Windows clipboard's text, as CF_UNICODETEXT.  The UTF8 is converted to and from UTF16 and its line feeds to and
//from carriage return and line feed pairs, with BaseLib's CUnicodeWriter; text that isn't valid is neither copied nor
//pasted.  Windows keeps the text, so it stays on the clipboard after the program has gone.
class CWinGDIClipboard : public CNativeClipboard
{
CONSTRUCTABLE(CWinGDIClipboard);
protected:
	bool	SetText(const char* szText) override;
	bool	GetText(CChars* pszDest) override;

	HWND	GetOwner(void);
	bool	Open(HWND hOwner);
};


#endif // __WIN_GDI_CLIPBOARD_H__

