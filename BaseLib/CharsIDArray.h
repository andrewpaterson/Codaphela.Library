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
#ifndef __CHARS_INDEX_ARRAY_H__
#define __CHARS_INDEX_ARRAY_H__
#include "ArrayTemplate.h"
#include "CharsID.h"


class CCharsIDArray
{
public:
	CArrayTemplate<CCharsID>	mcArray;

	void 		Init(int iChunkSize);
	void 		Kill(void);

	int			NumElements(void);
	CCharsID*	Add(CCharsID* pcChars);
	CCharsID*	Add(char* szString);
	CCharsID*	Add(void);
	CCharsID*	Add(char* szString, int iStartInclusive, int iEndExclusive);
	void 		Remove(CCharsID* pcChars);
	void 		Remove(char* szString);
	void 		Remove(int iIndex, BOOL bPreserveOrder = TRUE);
	CCharsID*	Get(int iIndex);
	char*		GetText(int iIndex);
	int			GetIndex(char* szStart, int iLen);
	void		Copy(CCharsIDArray* pcSource);
	BOOL		Equals(CCharsIDArray* pcOther);
	void		Dump(void);
};




#endif // __CHARS_INDEX_ARRAY_H__

