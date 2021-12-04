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
#ifndef __HEADER_NAME_MAP_STACK_H__
#define __HEADER_NAME_MAP_STACK_H__
#include "BaseLib/ArrayTemplate.h"
#include "HeaderFiles.h"


class CHeaderNameMapDirectory
{
public:
	CHeaderFiles*		mpcHeaderNameMap;
	CChars				mszBaseDirectoryName;

	void Init(CHeaderFiles* pcHeaderNameMap, char* szBaseDirectoryName);
	void Kill(void);
};


typedef CArrayTemplate<CHeaderNameMapDirectory>		__CHeaderNameMapStack;

class CHeaderNameMapStack : public __CHeaderNameMapStack
{
public:
	void Kill(void);
};


#endif // !__HEADER_NAME_MAP_STACK_H__

