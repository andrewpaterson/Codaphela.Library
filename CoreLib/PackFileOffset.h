/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __PACK_FILE_OFFSET_H__
#define __PACK_FILE_OFFSET_H__
#include "BaseLib/Chars.h"
#include "BaseLib/PackFiles.h"


// Hello/World/Directory.PAK
//  maps mszOffset to:
// Hello/World/[Directory/Stuff/File.txt]


class CPackFileOffset
{
public:
	CPackFiles	mcPackFiles;
	CChars		mszOffset;
	int			miFileRank;

	BOOL Init(char* szPackFileOffset, char* szPackFileDirectory, int iFileRank);
	void Kill(void);
};


#endif // !__PACK_FILE_OFFSET_H__

