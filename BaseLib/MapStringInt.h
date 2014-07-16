/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __MAP_STRING_INT_H__
#define __MAP_STRING_INT_H__
#include "MapStringTemplate.h"


class CMapStringInt : public CMapStringTemplate<int>
{
public:
	int*	GetWithKey(CChars* psKey);
	int*	GetWithKey(char* szKey);
	BOOL	GetAtIndex(int iIndex, CChars** ppsKey, int** ppiData);
	void	Put(CChars* psKey, int iData);
	void	Put(char* szKey, int iData);
	CChars*	GetWithValue(int iData);
};


#endif // __MAP_STRING_INT_H__

