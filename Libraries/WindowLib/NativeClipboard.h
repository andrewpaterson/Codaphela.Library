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
#ifndef __NATIVE_CLIPBOARD_H__
#define __NATIVE_CLIPBOARD_H__
#include "BaseLib/Chars.h"
#include "NativeComponent.h"


//The system's clipboard as a CClipboard sees it, made by the window factory.  The text is UTF8 with line feeds; a
//native clipboard converts it to and from the system's encoding and line endings.
class CClipboard;
class CNativeClipboard : public CNativeComponent
{
CONSTRUCTABLE(CNativeClipboard);
friend class CClipboard;
protected:
	CClipboard*		mpcClipboard;

public:
			void	Init(CClipboard* pcClipboard, CNativeWindowFactory* pcWindowFactory);
			void	Kill(void);

protected:
	virtual bool	SetText(const char* szText) =0;
	virtual bool	GetText(CChars* pszDest) =0;
};


#endif // __NATIVE_CLIPBOARD_H__

