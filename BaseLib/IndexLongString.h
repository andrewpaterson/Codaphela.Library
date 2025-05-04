/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#ifndef __INDEX_LONG_STRING_H__
#define __INDEX_LONG_STRING_H__
#include "PrimitiveTypes.h"
#include "IndexPrimitiveBlock.h"
#include "IntegerHelper.h"


class CIndexLongString : public CIndexPrimitiveBlock<int64>
{
public:
	bool	Get(int64 lli, char** pszData, size* piLength);
	char*	Get(int64 lli);

	bool	Put(int64 lli, char* szData, char* szLastCharInclusive = NULL);
	bool	Put(int64 lli, const char* szData, const char* szLastCharInclusive = NULL);
};


#endif // __INDEX_LONG_STRING_H__

