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
#ifndef __TRANSLATION_UNIT_H__
#define __TRANSLATION_UNIT_H__
#include "BaseLib/MapStringTemplate.h"
#include "CFile.h"
#include "PPBlock.h"
#include "PPHolder.h"


struct STULog
{
	CChars	szBlocksLog;
	CChars	szIncludesLog;
	BOOL	bLogBlocks;
	BOOL	bLogInlucdes;

	void Init(BOOL bLogInlucdes, BOOL bLogBlocks);
	void Kill(void);
};


class CTranslationUnit : public CCFile
{
protected:
	CPPHolder	mcTokens;  //This is really just an array of CBlock*'s but the parser only works with CPPHolder's.
	STULog*		mpcLogs;
	CLibrary*	mpcLibrary;

public:
	void			Init(char* szFullName, CLibrary* pcLibrary, BOOL bLogIncludes, BOOL bLogBlocks);
	void			Kill(void);
	char*			Print(CChars* psz);
	BOOL			IsTranslationUnit(void);
	BOOL			IsHeader(void);
	BOOL			LogBlocks(void);
	BOOL			LogInlucdes(void);
	BOOL			IsPragmaOnced(void);
	BOOL			IsSystemFile(void);
	CPPHolder*		GetTokens(void);
	STULog*			GetLogs(void);
	CLibrary*		GetLibrary(void);
};


//CTranslationUnit is used because it is largest of CTranslationUnit and CHeader
typedef CMapStringTemplate<CTranslationUnit>	CMapStringCFile;


#endif	// __TRANSLATION_UNIT_H__

