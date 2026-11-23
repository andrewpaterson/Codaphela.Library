/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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

** ------------------------------------------------------------------------ **/
#ifndef __HASH_TEXT_H__
#define __HASH_TEXT_H__
#include "HashOf.h"


//A key that is text kept somewhere else: the map keeps only where it is, so the text must last as long as the map.
//It needn't be NULL terminated.
struct SHashText
{
	const char*		sz;
	size			uiLength;

	void Init(const char* sz, size uiLength);
	void Init(const char* sz);
};


//SHashText's HASH: FNV-1a of the text, equal when the texts are.
struct SHashOfText
{
	static uint32		Hash(const SHashText* psKey);
	static HashEquals	Equals(void);
	static bool			TextEquals(const void* pvKey, const void* pvEntryKey);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline void SHashText::Init(const char* szText, size uiTextLength)
{
	sz = szText;
	uiLength = uiTextLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline void SHashText::Init(const char* szText)
{
	sz = szText;
	uiLength = (size)strlen(szText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline uint32 SHashOfText::Hash(const SHashText* psKey)
{
	return HashBytes(psKey->sz, psKey->uiLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline HashEquals SHashOfText::Equals(void)
{
	return &SHashOfText::TextEquals;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline bool SHashOfText::TextEquals(const void* pvKey, const void* pvEntryKey)
{
	const SHashText*	psKey;
	const SHashText*	psEntryKey;

	psKey = (const SHashText*)pvKey;
	psEntryKey = (const SHashText*)pvEntryKey;
	return (psKey->uiLength == psEntryKey->uiLength) && (memcmp(psKey->sz, psEntryKey->sz, psKey->uiLength) == 0);
}


#endif // __HASH_TEXT_H__

