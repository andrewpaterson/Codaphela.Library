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
#ifndef __END_ELEMENT_H__
#define __END_ELEMENT_H__
#include "ElementTypes.h"

struct SEndElement
{
	EElement			eET_END;  //Always: ET_End

	void Init(void)
	{
		eET_END = ET_End;
	}

	int Size(void)
	{
		return 1;
	}

	void String(CChars* szString)
	{
		szString->Init("END!");
	}
};


#endif //__END_ELEMENT_H__

