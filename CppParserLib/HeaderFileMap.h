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
#ifndef __HEADER_FILE_MAP_H__
#define __HEADER_FILE_MAP_H__
#include "BaseLib/MapStringTemplate.h"
#include "BaseLib/LinkedListTemplate.h"
#include "HeaderFile.h"


typedef CMapStringTemplate<CHeaderFile*>	CHeaderFilePtrMap;
typedef CLinkedListTemplate<CHeaderFile>	CHeadFileList;

class CHeaderFileMap
{
protected:
	CHeaderFilePtrMap	mcFileMap;
	CHeadFileList		mcFileList;

public:
	void 			Init(void);
	void 			Kill(void);

	CHeaderFile*	AddFile(char* szAbsoluteFileName, BOOL bSystem);
	CHeaderFile*	GetFile(char* szRelativeName);
	CHeaderFile*	StartIteration(SMapIterator* psIter);
	CHeaderFile*	Iterate(SMapIterator* psIter);
};


#endif // !__HEADER_FILE_MAP_H__

