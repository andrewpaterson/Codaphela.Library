/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#ifndef __FONT_LIST_H__
#define __FONT_LIST_H__
#include "StandardLib/SetType.h"
#include "WinText.h"
#include "Font.h"


typedef CSetType<CFont>		CSetFont;

class CFontList
{
public:
	CWinText	mcWinText;
	CSetFont	mscFonts;

	void 	Init(HWND hWnd);
	void 	Kill(void);
	void	SetHWnd(HWND hWnd);
	CFont*	CreateFromSystem(char* szName, int iHeight, int iWidth, int iWeight);
	CFont*	Get(char* szName);
};

#endif // __FONT_LIST_H__

