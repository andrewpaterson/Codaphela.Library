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


typedef CMapStringTemplate<CHeaderFile*>	CHeaderFilePtrMap;


class CHeaderFileMap;
class CHeaderFiles
{
protected:
	CHeaderFileMap*		mpcFileMap;
	CHeaderFilePtrMap	mcFileNames;
	CChars				mszBaseDirectory;
	BOOL				mbSystem;

public:
	BOOL			Init(char* szBaseDirectory, CHeaderFileMap* pcFileMap, BOOL bIncludeSubDirectories, BOOL bSystem);
	void			Kill(void);

	void			AddFiles(BOOL bIncludeSubDirectories);
	void			AddFile(char* szRelativeFileName);

	CHeaderFile*	GetFile(char* szRelativeFileName);
	
	int				GetBaseDirectoryLength(void);

	void			Dump(void);
};


typedef CArrayTemplate<CHeaderFiles*> CArrayHeaderNameMapPtr;
typedef CArrayTemplate<CHeaderFiles> CArrayHeaderNameMap;


#endif // !__HEADER_NAME_MAP_H__

