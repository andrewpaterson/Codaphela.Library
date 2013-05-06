/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __FILE_COMPARE_H__
#define __FILE_COMPARE_H__
#include "NaiveFile.h"


class CFileCompare
{
protected:	
	void	ErrorNames(char* szName1, char* szName2, CChars* psz1, CChars* psz2);
	void	Difference(CChars* psz1, filePos iPosition, char c);
	BOOL	Compare(char* szName1, char* szName2, CChars* psz1, CChars* psz2, void* pvMem1, void* pvMem2, filePos iLength1, filePos iLength2);

public:
	BOOL	Compare(char* szName1, char* szName2, CChars* psz1, CChars* psz2);
	BOOL	Compare(char* szName1, void* pvMemory, filePos iLength, CChars* psz1, CChars* psz2);
};


#endif // __FILE_COMPARE_H__

