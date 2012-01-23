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
#ifndef __TEXT_ELEMENT_H__
#define  __TEXT_ELEMENT_H__
#include "ElementTypes.h"


struct STextElement
{
	EElement	eET_Text;  //This int must always have the value ET_Text.
	int			iNumIntsInString;
	char		szStringCharacters[1];

	void Init(char* szText)
	{
		int len;

		eET_Text = ET_Text;
		len = (int)strlen(szText)+1;
		iNumIntsInString = (int)ceil((float)len/4);
		strcpy(szStringCharacters, szText);
	}

	int Size(void)
	{
		return iNumIntsInString+2;
	}

	int Size(char* szText)
	{
		int len;

		len = (int)strlen(szText)+1;
		return ((int)ceil((float)len/4))+2;
	}

	void String(CChars* szString)
	{
		szString->Init();
		szString->Append("TEXT(");
		szString->Append(szStringCharacters);
		szString->Append(")");
	}
};


#endif //__TEXT_ELEMENT_H__
