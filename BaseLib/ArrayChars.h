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
	bool			mbFaked;

	void 		_Init(void);
	void 		Init(void);
	void 		Init(const char* szFirst, ...);
	void 		Init(CArrayChars* pasz);
	void 		Fake(void);
	void 		ReInit(void);
	void 		Kill(void);

	size		NumElements(void);
	CChars*	 	Add(CChars cChars);
	CChars*		Add(CChars* pcChars);
	CChars*	 	Add(char* szString);
	CChars*	 	Add(const char* szString);
	CChars*		Add(void);
	CChars*		Add(char* szString, size iStartInclusive, size iEndExclusive);
	CChars*		Add(char* szText, char* szLastCharInclusive);
	CChars*		AddIfUnique(char* szString);
	CChars*		AddIfUnique(CChars cChars);
	void		AddList(char* sz, ...);
	CChars*		InsertIntoSorted(char* szText, char* szLastCharInclusive);
	CChars*		InsertIntoSorted(CChars* psz);
	void 		Remove(CChars* pcChars);
	void 		Remove(char* szString);
	void 		Remove(size iIndex);
	bool		RemoveTail(void);
	CChars* 	Get(size iIndex);
	CChars* 	Tail(void);
	char*		GetText(size iIndex);
	size		GetIndex(char* szStart, size iLen);
	size		GetIndex(char* szStart);
	size		GetIndex(CChars* psz);
	size		GetSubStringIndex(char* szStart);
	size		FindInSorted(char* szString, bool bCaseSensitive = true);
	size		FindInSorted(CChars* szString, bool bCaseSensitive = true);
	void		Copy(CArrayChars* pcSource);
	bool		Equals(CArrayChars* pcOther);
	bool		Contains(char* szText);
	bool		Contains(CChars* psz);
	bool		ContainsSubString(char* szText);
	void		QuickSort(bool bCaseSensitive = true);
	void		BubbleSort(bool bCaseSensitive = true);
	void		Shuffle(CRandom* pcRandom = NULL);
	bool		Split(char* szString, char cSplitter);
	void		Finalise(void);
	void		Print(CChars* psz);
	void		Dump(void);
};


#endif // __ARRAY_CHARS_H__

