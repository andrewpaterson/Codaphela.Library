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
#ifndef __MAP_STRING_STRING_H__
#define __MAP_STRING_STRING_H__
#include "MapStringTemplate.h"


class CMapStringString : public CMapStringBlock
{
public:
	char*	Get(char* szKey);
	char*	Get(uint8* szKey);
	bool	Put(char* szKey, char* szValue);
	bool	Put(uint8* szKey, char* szValue);
	void	Dump(void);
};


#endif // __MAP_STRING_STRING_H__

