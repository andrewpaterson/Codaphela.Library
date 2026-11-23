/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2021 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __ASCII_INDEX_H__
#define __ASCII_INDEX_H__
#include "BaseLib/HashMapString.h"
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/Numbers.h"


//Names given IDs (0, 1, 2... in the order they are added): each name's index in a hash map of them, which keeps
//their text.
class CASCIIIndex
{
protected:
	CHashMapString<uint8, SArrayLargeSize>	mcNames;	//The value isn't used.

public:
	void	Init(void);
	void	Kill(void);

	int64	Add(const char* sz, const char* szLastCharInclusive = NULL);

	char*	Get(int64 lliID, int* piLength = NULL);
	int64	Get(const char* szText, const char* szLastCharInclusive);

	int		NumElements(void) const;

protected:
	static int		Length(const char* sz, const char* szLastCharInclusive);
};


#endif // __ASCII_INDEX_H__

