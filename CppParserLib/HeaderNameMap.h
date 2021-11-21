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
#ifndef __HEADER_NAME_MAP_H__
#define __HEADER_NAME_MAP_H__
#include "BaseLib/MapStringTemplate.h"
#include "BaseLib/ArrayTemplate.h"
#include "HeaderFile.h"


typedef CMapStringTemplate<CHeaderFile*>	CMapStringHeaderFilePtr;


class CHeaderFileMap;
class CHeaderNameMap
{
public:
	CMapStringHeaderFilePtr		mcFileNames;
	CHeaderFileMap*				mpcFileMap;
	CChars						mszBaseDirectory;
	BOOL						mbSystem;

	BOOL Init(char* szBaseDirectory, CHeaderFileMap* pcFileMap, BOOL bSystem);
	void Kill(void);

	void AddFiles(void);
	void AddFile(char* szFile);

	void Dump(void);
};


typedef CArrayTemplate<CHeaderNameMap*> CArrayHeaderNameMapPtr;
typedef CArrayTemplate<CHeaderNameMap> CArrayHeaderNameMap;


#endif // !__HEADER_NAME_MAP_H__

