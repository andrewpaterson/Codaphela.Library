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
#ifndef __FILE_DEFINE_H__
#define __FILE_DEFINE_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"


class CFileDefine
{
public:
	CChars			mszDefine;
	unsigned int	muiID;

	void Init(char* szName, unsigned uiID);
	void Kill(void);
};


int FileDefineCompare(const void* pv1, const void* pv2);


typedef CArrayTemplate<CFileDefine> CFileDefineArray;


#endif // __FILE_DEFINE_H__
