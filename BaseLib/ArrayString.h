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
#ifndef __ARRAY_STRING_H__
#define __ARRAY_STRING_H__
#include "ArrayTemplate.h"
#include "Chars.h"


typedef CArrayTemplate<CChars>		__CArrayString;
class CArrayString
{
public:
	__CArrayString	mcArray;
	BOOL			mbFaked;

	void 		Init(int iChunkSize, BOOL bFake = FALSE);
	void 		ReInit(void);
	void 		Kill(void);

	int			NumElements(void);
	CChars*	 	Add(CChars cChars);
	CChars*	 	Add(char* szString);
	CChars*		Add(void);
	CChars*		Add(char* szString, int iStartInclusive, int iEndExclusive);
	CChars*		Add(char* szText, char* szLastCharInclusive);
	CChars*		AddIfUnique(char* szString);
	CChars*		AddIfUnique(CChars cChars);
	CChars*		InsertIntoSorted(char* szText, char* szLastCharInclusive);
	void 		Remove(CChars* pcChars);
	void 		Remove(char* szString);
	void 		Remove(int iIndex);
	BOOL		RemoveTail(void);
	CChars* 	Get(int iIndex);
	CChars* 	Tail(void);
	char*		GetText(int iIndex);
	int			GetIndex(char* szStart, int iLen);
	int			GetIndex(char* szStart);
	int			FindInSorted(char* szString, BOOL bCaseSensitive = TRUE);
	int			FindInSorted(CChars* szString, BOOL bCaseSensitive = TRUE);
	void		Copy(CArrayString* pcSource);
	BOOL		Equals(CArrayString* pcOther);
	BOOL		Contains(char* szText);
	void		QuickSort(BOOL bIgnoreCase = FALSE);
	void		BubbleSort(BOOL bIgnoreCase = FALSE);
	BOOL		Split(char* szString, char cSplitter);
	void		Finalise(void);
	void		Dump(void);
};


#endif // __ARRAY_STRING_H__

