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
#ifndef __TRANSLATION_UNIT_LIST_H__
#define __TRANSLATION_UNIT_LIST_H__
#include "BaseLib/LinkedListTemplate.h"
#include "TranslationUnit.h"


typedef CLinkedListTemplate<CTranslationUnit>	CListTranslationUnit;


class CLibrary;
class CTranslationUnitList
{
protected:
	CListTranslationUnit	mcFiles;
	CLibrary*				mpcLibrary;

public:
	void 				Init(CLibrary* pcLibrary);
	void 				Kill(void);

	void				AddAllFiles(BOOL bLogIncludes = FALSE, BOOL bLogBlocks = FALSE);
	CTranslationUnit*	AddFile(char* szRelativeFileName, BOOL bLogIncludes = FALSE, BOOL bLogBlocks = FALSE);
	CTranslationUnit*	GetFirst(void);
	CTranslationUnit*	GetNext(CTranslationUnit* pcCurrent);
};


#endif // !__TRANSLATION_UNIT_LIST_H__

