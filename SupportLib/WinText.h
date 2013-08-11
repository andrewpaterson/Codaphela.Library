/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __WIN_TEXT_H__
#define __WIN_TEXT_H__
#include <Windows.h>
#include "BaseLib/ArrayTemplate.h"
#include "Font.h"


#define		TEXT_ALIGN_HCENTER	1
#define		TEXT_ALIGN_VCENTER	2
#define		TEXT_ALIGN_LEFT		4
#define		TEXT_ALIGN_RIGHT	8
#define		TEXT_ALIGN_TOP		16
#define		TEXT_ALIGN_BOTTOM	32


struct SWinFontInstance
{
	HFONT	hFont;
	int		iWidth;
	int		iHeight;
	int		iWeight;
	int		iUniqueID;
};


typedef CArrayTemplate<SWinFontInstance> CArrayWinFontInstances;


struct SWinFont
{
	char					szName[32];
	CArrayWinFontInstances	sInstances;
};


typedef CArrayTemplate<SWinFont> CArrayWinFonts;


class CWinText
{
public:
	CArrayWinFonts	mcWinFonts;
	int				miNumFonts;
			
	void Init(HWND hWnd);
	void Kill(void);
			
	SWinFontInstance*	Create(char* szName, int iHeight, int iWidth, int iWeight);
	void 				Draw(char* szString, SWinFontInstance* psInstance, HDC hDC, int iFlags, BOOL bSingleLine, RECT* psRect);
	void 				Draw(char* szString, SWinFontInstance* psInstance, HDC hDC, int x, int y);
			
	Ptr<CFont>			GenerateFont(SWinFontInstance* pcWinFont, char* szFontName);

private:
	Ptr<CImage>			DrawTextToImage(CChars*, HDC hDC);
	void				GetSourceGlyphs(CChars* pszDest);
	void				GetSourceRectangles(CArrayRectangle* pacRectangles, CChars* pszLetters, HDC hDC);
	Ptr<CImage>			PackImage(Ptr<CFont> pcFont, CImage* pcSource, CArrayRectangle* pacRectangles);
};


#endif //__WIN_TEXT_H__

