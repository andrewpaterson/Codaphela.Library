/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#ifndef __EXTERNAL_STRING_H__
#define __EXTERNAL_STRING_H__
#include "PrimitiveTypes.h"


class CExternalString
{
public:
	char*	msz;
	size	miLen;

	void	Init(void);
	void	Init(char* sz, size iLen);
	void	Init(char* szStart, char* szLastCharInclusive);
	void	Clear(void);
	char*	EndInclusive(void);
	void	Dump(void);
	bool	Equals(CExternalString* pcOther);
	bool	Equals(char* sz);
};


#endif // __EXTERNAL_STRING_H__

