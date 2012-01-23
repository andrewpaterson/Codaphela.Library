/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela InputLib

Codaphela InputLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela InputLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela InputLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __KEYBOARD_KEY_LIST_H__
#define __KEYBOARD_KEY_LIST_H__
#include "BaseLib/ArrayString.h"

struct SWinKey
{
	unsigned int uiVKey;
	unsigned int uiPressedFlags;
	unsigned int uiReleasedFlags;
};


typedef CArrayTemplate<SWinKey>	CArrayWinKey;


class CKeyboardKeyList
{
public:
	CArrayString	maszKeys;
	CArrayWinKey	masKeys;
	int				miCurrent;
	BOOL			mbFirst;

	void	Init(void);
	void	Kill(void);
	char*	GetCurrent(void);
	void	Key(unsigned int uiVKey, unsigned int uiFlags);
	void	Pressed(unsigned int uiVKey, unsigned int uiPressedFlags);
	void	Released(unsigned int uiVKey, unsigned int uiPressedFlags);
	void	Next(void);
	void	ToString(CChars* psz);
	void	Dump(void);
};


#endif // __KEYBOARD_KEY_LIST_H__

