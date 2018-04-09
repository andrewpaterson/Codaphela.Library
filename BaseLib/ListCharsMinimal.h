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
#ifndef __LIST_CHARS__H__
#define __LIST_CHARS__H__
#include "ListTemplateMinimal.h"
#include "DataTypes.h"


typedef CListTemplateMinimal<char> __CListCharsMinimal;
template class CListTemplateMinimal<char>;
class CListCharsMinimal : public __CListCharsMinimal
{
public:
	size_t		TotalSize(int iNumElements, int iRowLength);

	void		Init(int iNumElements, int iRowLength);
	
	int	 		IndexOf(char* szString);

	void		Add(char* sz);
	void		Add(char* sz, int iSize);
};


#endif // __LIST_CHARS__H__

