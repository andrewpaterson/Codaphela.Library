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
#ifndef __WIN_REF_SCREEN_H__
#define __WIN_REF_SCREEN_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/FileBasic.h"
#include "SupportLib/Int32Rect.h"
#include "WindowLib/DamageArea.h"
#include "WinRefPixels.h"


//What a present's hash line covers: the areas presented, or all of the screen after them.
enum EWinRefHashArea
{
	WRHA_Presented,
	WRHA_Screen,
};


class CWinRefWindow;
//The reference screen the factory's windows are on, stacked in the order they were made or raised.  Watching, each
//window keeps what it has shown: what each present asks for is copied from its canvas, as a real screen would show it.
//Each present can be checked (what the window shows must be what it drew) and hashed to a file, a line a present.
class CWinRefScreen
{
protected:
	CInt32Rect							mcArea;  //Empty until set: the screen's size isn't known.
	CArrayTemplatePtr<CWinRefWindow>	mapcWindows;  //Stacked, the bottom one first.
	CWinRefPixels						mcPixels;  //The screen's pixels, made from the windows' when hashed.
	bool								mbCheck;
	lsize								muiMismatches;
	CFileBasic							mcHashFile;
	bool								mbHash;
	EWinRefHashArea						meHashArea;
	uint64								muiPresents;

public:
	void	Init(void);
	void	Kill(void);

	void	SetArea(CInt32Rect* pcArea);
	bool	GetArea(CInt32Rect* pcDest);
	void	SetCheck(bool bCheck);
	bool	SetHash(const char* szFileName, EWinRefHashArea eArea);
	bool	IsWatching(void);

	void	Add(CWinRefWindow* pcWindow);
	void	Remove(CWinRefWindow* pcWindow);
	void	Raise(CWinRefWindow* pcWindow);

	void	Present(CWinRefWindow* pcWindow, CWinRefPixels* pcCanvas, CDamageArea* pcDamage);

	lsize	NumMismatches(void);
	void	Compose(void);
	CWinRefPixels*	GetPixels(void);

protected:
	void	Check(CWinRefWindow* pcWindow, CWinRefPixels* pcCanvas);
	void	WriteHash(CWinRefWindow* pcWindow, CDamageArea* pcDamage);
	void	CloseHash(void);
};


#endif // __WIN_REF_SCREEN_H__

