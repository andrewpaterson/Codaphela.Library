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


//How SHashOfFoldedText sees a character: SFoldCase sees an ASCII letter in either case as the same (as
//StringInsensitiveCompare does), SFoldFileName that and either slash as the same.
struct SFoldCase
{
	static char Fold(char c)
	{
		return ((c >= 'A') && (c <= 'Z')) ? (char)(c + 'a' - 'A') : c;
	}
};


struct SFoldFileName
{
	static char Fold(char c)
	{
		return (c == '\\') ? '/' : SFoldCase::Fold(c);
	}
};


//SHashText's HASH with each character seen as FOLD folds it: texts are equal when their folded characters are.
template<class FOLD>
struct SHashOfFoldedText
{
	static uint32		Hash(const SHashText* psKey);
	static HashEquals	Equals(void);
	static bool			TextEquals(const void* pvKey, const void* pvEntryKey);
};


typedef SHashOfFoldedText<SFoldCase>		SHashOfTextIgnoreCase;
typedef SHashOfFoldedText<SFoldFileName>	SHashOfFileName;


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


//////////////////////////////////////////////////////////////////////////
//
// FNV-1a of the folded characters.
//
//////////////////////////////////////////////////////////////////////////
template<class FOLD>
uint32 SHashOfFoldedText<FOLD>::Hash(const SHashText* psKey)
{
	uint32	uiHash;
	size	i;

	uiHash = FNV_HASH32_START;
	for (i = 0; i < psKey->uiLength; i++)
	{
		uiHash = (uiHash ^ (uint8)FOLD::Fold(psKey->sz[i])) * FNV_HASH32_PRIME;
	}
	return HashFNVFinish(uiHash);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class FOLD>
HashEquals SHashOfFoldedText<FOLD>::Equals(void)
{
	return &SHashOfFoldedText<FOLD>::TextEquals;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class FOLD>
bool SHashOfFoldedText<FOLD>::TextEquals(const void* pvKey, const void* pvEntryKey)
{
	const SHashText*	psKey;
	const SHashText*	psEntryKey;
	size				i;

	psKey = (const SHashText*)pvKey;
	psEntryKey = (const SHashText*)pvEntryKey;
	if (psKey->uiLength != psEntryKey->uiLength)
	{
		return false;
	}
	for (i = 0; i < psKey->uiLength; i++)
	{
		if (FOLD::Fold(psKey->sz[i]) != FOLD::Fold(psEntryKey->sz[i]))
		{
			return false;
		}
	}
	return true;
}


#endif // __HASH_TEXT_H__

