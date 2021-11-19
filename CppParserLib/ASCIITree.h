/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __ASCII_TREE_H__
#define __ASCII_TREE_H__
#include "BaseLib/ArrayChars.h"
#include "BaseLib/IndexStringLong.h"
#include "BaseLib/IndexLongString.h"


class CASCIITree
{
public:
	CIndexStringLong	mcIndex;

	void			Init(void);
	void			Kill(void);

	BOOL			Add(int64 lliID, char* szText, char* szLastCharInclusive = NULL);

	int64			Get(char* szText, char* szLastCharInclusive = NULL /*, BOOL bExact = TRUE */ );

	BOOL			Remove(char* szText, char* szLastCharInclusive = NULL);

	int				NumElements(void);

	int64			StartIteration(SIndexTreeMemoryUnsafeIterator* psIter);
	int64			Iterate(SIndexTreeMemoryUnsafeIterator* psIter);

	BOOL			IsEmpty(void);
	BOOL			Contains(char* szText);
};


#endif // !__ASCII_TREE_H__

