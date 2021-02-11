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
#ifndef __ARRAY_CHARS_H__
#define __ARRAY_CHARS_H__
#include "ArrayTemplate.h"
#include "Chars.h"


class CRandom;
typedef CArrayTemplate<CChars>	__CArrayChars;
class CArrayChars
{
public:
	__CArrayChars	mcArray;
	BOOL			mbFaked;

	void 		Init(void);
	void 		Init(CArrayChars* pasz);
	void 		Fake(void);
	void 		ReInit(void);
	void 		Kill(void);

	int			NumElements(void);
	CChars*	 	Add(CChars cChars);
	CChars*		Add(CChars* pcChars);
	CChars*	 	Add(char* szString);
	CChars*		Add(void);
	CChars*		Add(char* szString, int iStartInclusive, int iEndExclusive);
	CChars*		Add(char* szText, char* szLastCharInclusive);
	CChars*		AddIfUnique(char* szString);
	CChars*		AddIfUnique(CChars cChars);
	void		AddList(char* sz, ...);
	CChars*		InsertIntoSorted(char* szText, char* szLastCharInclusive);
	CChars*		InsertIntoSorted(CChars* psz);
	void 		Remove(CChars* pcChars);
	void 		Remove(char* szString);
	void 		Remove(int iIndex);
	BOOL		RemoveTail(void);
	CChars* 	Get(int iIndex);
	CChars* 	Tail(void);
	char*		GetText(int iIndex);
	int			GetIndex(char* szStart, int iLen);
	int			GetIndex(char* szStart);
	int			GetIndex(CChars* psz);
	int			GetSubStringIndex(char* szStart);
	int			FindInSorted(char* szString, BOOL bCaseSensitive = TRUE);
	int			FindInSorted(CChars* szString, BOOL bCaseSensitive = TRUE);
	void		Copy(CArrayChars* pcSource);
	BOOL		Equals(CArrayChars* pcOther);
	BOOL		Contains(char* szText);
	BOOL		Contains(CChars* psz);
	BOOL		ContainsSubString(char* szText);
	void		QuickSort(BOOL bCaseSensitive = TRUE);
	void		BubbleSort(BOOL bCaseSensitive = TRUE);
	void		Shuffle(CRandom* pcRandom = NULL);
	BOOL		Split(char* szString, char cSplitter);
	void		Finalise(void);
	void		Dump(void);
};


#endif // __ARRAY_CHARS_H__

