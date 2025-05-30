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
#ifndef __ARRAY_CHAR_MINIMAL_H__
#define __ARRAY_CHAR_MINIMAL_H__
#include "ArrayTemplateMinimal.h"


class CArrayCharMinimal : public CArrayTemplateMinimal<char>
{
public:
	void	Init(void);
	void	Init(char* sz);
	void	Allocate(char* sz);
	void	Allocate(size iNumElements);
	void	Add(char c);
	size	AddIfUnique(char c);
	size	Find(char c);
	char	GetValue(size iElementPos);
	char*	GetArray(void);
};


#endif // __ARRAY_CHAR_MINIMAL_H__

