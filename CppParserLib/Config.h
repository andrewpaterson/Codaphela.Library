/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __CONIFG_H__
#define __CONIFG_H__
#include "BaseLib/LinkedListTemplate.h"
#include "BaseLib/ArrayChars.h"


class CConfig
{
public:
	CChars			mszName;
	CArrayChars	maszDefines;

	void Init(char* szName);
	void Kill(void);
	void AddDefine(char* szDefine);
	void AddDefines(char* szDefines);
};


typedef CLinkedListTemplate<CConfig>	CListConfigs;


#endif // !__CONIFG_H__

