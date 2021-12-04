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
#ifndef __LIBRARY_H__
#define __LIBRARY_H__
#include "BaseLib/LinkedListTemplate.h"
#include "Config.h"
#include "TranslationUnitList.h"
#include "HeaderNameMap.h"


struct SCFileIter
{
	CTranslationUnit*	pcLast;
	BOOL				bValid;
};


class CLibrary
{
public:
	CTranslationUnitList	mcTranslationUnits;
	CHeaderNameMap				mcHeaderNameMap;
	CChars						mszName;
	CChars						mszBaseDir;
	CListConfigs				mcConfigs;

	void				Init(char* szName, char* szBaseDir, BOOL bIncludeSubDirectories, CHeaderFileMap* pcHeaderFileMap);
	void				Kill(void);
	CTranslationUnit*  	AddFile(char* szRelativeNameFile, BOOL bLogIncludes = FALSE, BOOL bLogBlocks = FALSE);
	void				AddAllFiles(BOOL bLogIncludes = FALSE, BOOL bLogBlocks = FALSE);
	CConfig*			AddConfiguration(char* szConfig);
	CConfig*			GetConfig(char* szConfiguration);
	CTranslationUnit*	GetFirstTranslationUnit(SCFileIter* psIter);
	CTranslationUnit*	GetNextTranslationUnit(SCFileIter* psIter);
};


typedef CLinkedListTemplate<CLibrary>	CListLibraries;


#endif // !__LIBRARY_H__

