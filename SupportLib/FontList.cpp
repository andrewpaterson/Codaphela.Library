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
#include "FontList.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFontList::Init(HWND hWnd)
{
	mscFonts.Init();
	mcWinText.Init(hWnd);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFontList::Kill(void)
{
	mcWinText.Kill();
	mscFonts.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFont* CFontList::CreateFromSystem(char* szName, int iHeight, int iWidth, int iWeight)
{
	CFont*				pcFont;
	CChars				szUniqueName;
	SWinFontInstance*	psWinFont;

	szUniqueName.Init(szName);
	if (iHeight > 0)
	{
		szUniqueName.Append(" ");
		szUniqueName.Append(iHeight);
	}
	if (iWeight != FW_DONTCARE)
	{
		switch (iWeight)
		{
		case FW_THIN:
			szUniqueName.Append(" Thin");
			break;
		case FW_EXTRALIGHT:
			szUniqueName.Append(" Extralight");
			break;
		case FW_LIGHT:
			szUniqueName.Append(" Light");
			break;
		case FW_NORMAL:
			szUniqueName.Append(" Normal");
			break;
		case FW_MEDIUM:
			szUniqueName.Append(" Medium");
			break;
		case FW_SEMIBOLD:
			szUniqueName.Append(" Semibold");
			break;
		case FW_BOLD:
			szUniqueName.Append(" Bold");
			break;
		case FW_EXTRABOLD:
			szUniqueName.Append(" Extrabold");
			break;
		case FW_HEAVY:
			szUniqueName.Append(" Heavy");
			break;
		}
	}

	pcFont = Get(szUniqueName.Text());
	if (pcFont)
	{
		szUniqueName.Kill();
		return pcFont;
	}


	psWinFont = mcWinText.Create(szName, iHeight, iWidth, iWeight);
	pcFont = mcWinText.GenerateFont(psWinFont, szUniqueName.Text());
	szUniqueName.Kill();
	mscFonts.Add(pcFont);

	return pcFont;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFont* CFontList::Get(char* szName)
{
	SSetIterator	sIter;
	CFont*			pcFont;

	pcFont = mscFonts.StartIteration(&sIter);
	while (pcFont)
	{
		if (pcFont->Is(szName))
		{
			return pcFont;
		}
		pcFont = mscFonts.Iterate(&sIter);
	}
	return NULL;
}

