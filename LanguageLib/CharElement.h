/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LanguageLib

Codaphela LanguageLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LanguageLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LanguageLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __CHAR_ELEMENT_H__
#define __CHAR_ELEMENT_H__
#include "ElementTypes.h"


struct SCharElement
{
	EElement	eET_Char;  //This int must always have the value ET_Char.
	char		cChar;

	void Init(char cChar)
	{
		eET_Char = ET_Char;
		this->cChar = cChar;
	}

	int Size(void)
	{
		return 2;
	}

	void String(CChars* szString)
	{
		szString->Init("CHAR(");
		szString->Append(cChar);
		szString->Append(")");
	}
};



#endif //__CHAR_ELEMENT_H__

