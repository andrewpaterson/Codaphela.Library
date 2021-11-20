/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#ifndef __DEFINE_MAP_H__
#define __DEFINE_MAP_H__
#include "BaseLib/IndexPrimitiveTemplate.h"
#include "ASCIITree.h"
#include "Define.h"


typedef CIndexPrimitiveTemplate<int64, CDefine>	CDefinesIndex;


class CDefineMap
{
protected:
	CASCIITree			mcNameToIDIndex;
	CDefinesIndex		mcIDToDefineIndex;

public:
	void 		Init(void);
	void 		Kill(void);
	CDefine*	AddDefine(CExternalString* pcName);
	CDefine*	AddDefine(char* szName);
	CDefine*	AddDefine(CExternalString* pcName, CDefine* pcDefine);
	void		RemoveDefine(CExternalString* pcName);
	void		RemoveDefine(char* szName);
	CDefine*	GetDefine(CExternalString* pcName);
	CDefine*	GetDefine(CChars* pszName);
	CDefine*	GetDefine(char* szName);
	void		Dump(void);
};


#endif // !__DEFINE_MAP_H__

