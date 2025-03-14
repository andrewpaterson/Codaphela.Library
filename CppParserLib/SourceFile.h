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
#ifndef __SOURCE_FILE_H__
#define __SOURCE_FILE_H__
#include "BaseLib/Chars.h"
#include "PPBlockSetArray.h"
#include "PPTokenMemory.h"


class CLibrary;
class CSourceFile
{
protected:
	CChars				mszContents;
	CChars				mszFullName;
	bool				mbLoaded;
	CPPTokenMemory		mcTokenMemory;
	CPPBlockSetArray	macBlockSets;
	bool				mbBlockSetsKilled;

public:
			void				Init(char* pszFullName);
	virtual void				Kill(void);

			void				KillBlockSets(void);
			void				Load(void);
			void				SetContents(const char* sz);
			bool				IsLoaded(void);
			bool				IsNamed(char* szName);
	virtual bool				IsTranslationUnit(void) =0;
	virtual bool				IsHeader(void) =0;
			char*				ShortName(void);
			char*				FullName(void);
	virtual bool				IsPragmaOnced(void) =0;
	virtual bool				IsSystemFile(void) =0;
			void				Path(CChars* pszPath);
			char*				GetContents(void);
			size					GetContentsLength(void);
			size_t				GetTokenMemoryUsed(void);
			CPPBlockSetArray*	GetBlockSets(void);
			CPPTokenMemory*		GetTokenMemory(void);
			void				DumpBlockSets(void);
};


#endif // !__SOURCE_FILE_H__

