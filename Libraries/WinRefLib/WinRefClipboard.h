#ifndef __WIN_REF_CLIPBOARD_H__
#define __WIN_REF_CLIPBOARD_H__
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
#include "WindowLib/NativeClipboard.h"


//The reference system's clipboard: the text is kept by the factory, so every window it makes shares it as they would
//a real system's clipboard, but nothing outside the process sees it.
class CWinRefClipboard : public CNativeClipboard
{
CONSTRUCTABLE(CWinRefClipboard);
protected:
	bool	SetText(const char* szText) override;
	bool	GetText(CChars* pszDest) override;
};


#endif // __WIN_REF_CLIPBOARD_H__

