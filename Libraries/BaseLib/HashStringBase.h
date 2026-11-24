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
#ifndef __HASH_STRING_BASE_H__
#define __HASH_STRING_BASE_H__
#include "SystemAllocator.h"
#include "MemoryStackExtended.h"
#include "HashText.h"


#define HASH_MAP_STRING_CHUNK	2048


//What CHashMapString and CHashSetString share: text keys, of which it keeps its own copies (NULL terminated, so GetKey
//gives a C string).  A key's text lasts until it is killed or re-initialised, even after the key is removed.  Keys
//needn't be NULL terminated where a length is given.  The order added, as CHashMapTemplate's; an entry's index is its
//own until something is removed.  MAP is a CHashMapTemplate or CHashSetTemplate keyed by SHashText.
template<class MAP, class SIZE>
class CHashStringBase
{
public:
	typedef typename SIZE::Count	Count;

protected:
	MAP						mcMap;
	CMemoryStackExtended	mcText;

public:
	void		Init(void);
	void		Init(CMallocator* pcMalloc, lsize uiTextChunkSize = HASH_MAP_STRING_CHUNK);
	void		Kill(void);
	void		ReInit(void);

	Count		FindOrAddIndex(const char* szKey, bool* pbAdded = NULL);
	Count		FindOrAddIndex(const char* szKey, size uiLength, bool* pbAdded = NULL);
	Count		Find(const char* szKey);
	Count		Find(const char* szKey, size uiLength);
	bool		HasKey(const char* szKey);
	bool		HasKey(const char* szKey, size uiLength);
	bool		Remove(const char* szKey);
	bool		Remove(const char* szKey, size uiLength);
	void		RemoveAt(Count uiIndex);
	bool		Reserve(Count uiNumElements);

	Count		NumElements(void) const;
	bool		IsEmpty(void) const;
	const char*	GetKey(Count uiIndex);
	size		GetKeyLength(Count uiIndex);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class MAP, class SIZE>
void CHashStringBase<MAP, SIZE>::Init(void)
{
	Init(&gcSystemAllocator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class MAP, class SIZE>
void CHashStringBase<MAP, SIZE>::Init(CMallocator* pcMalloc, lsize uiTextChunkSize)
{
	mcMap.Init(pcMalloc);
	mcText.Init(uiTextChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class MAP, class SIZE>
void CHashStringBase<MAP, SIZE>::Kill(void)
{
	mcMap.Kill();
	mcText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class MAP, class SIZE>
void CHashStringBase<MAP, SIZE>::ReInit(void)
{
	mcMap.ReInit();
	mcText.Clear();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class MAP, class SIZE>
typename SIZE::Count CHashStringBase<MAP, SIZE>::FindOrAddIndex(const char* szKey, bool* pbAdded)
{
	return FindOrAddIndex(szKey, (size)strlen(szKey), pbAdded);
}


//////////////////////////////////////////////////////////////////////////
//
// The index of szKey, added (its text copied, a value zeroed) if it isn't
// there.  SIZE::NotFound if there is no memory for it.
//
//////////////////////////////////////////////////////////////////////////
template<class MAP, class SIZE>
typename SIZE::Count CHashStringBase<MAP, SIZE>::FindOrAddIndex(const char* szKey, size uiLength, bool* pbAdded)
{
	SHashText	sKey;
	Count		uiIndex;
	bool		bAdded;
	char*		szCopy;

	sKey.Init(szKey, uiLength);
	uiIndex = mcMap.FindOrAddIndex(&sKey, &bAdded);
	if (pbAdded)
	{
		*pbAdded = bAdded;
	}
	if ((uiIndex == SIZE::NotFound) || !bAdded)
	{
		return uiIndex;
	}

	szCopy = (char*)mcText.Add((lsize)uiLength + 1);
	if (szCopy == NULL)
	{
		mcMap.RemoveAt(uiIndex);
		return SIZE::NotFound;
	}
	memcpy(szCopy, szKey, uiLength);
	szCopy[uiLength] = '\0';
	mcMap.GetKey(uiIndex)->sz = szCopy;
	return uiIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class MAP, class SIZE>
typename SIZE::Count CHashStringBase<MAP, SIZE>::Find(const char* szKey)
{
	return Find(szKey, (size)strlen(szKey));
}


//////////////////////////////////////////////////////////////////////////
//
// The index of szKey, SIZE::NotFound if it isn't there.
//
//////////////////////////////////////////////////////////////////////////
template<class MAP, class SIZE>
typename SIZE::Count CHashStringBase<MAP, SIZE>::Find(const char* szKey, size uiLength)
{
	SHashText	sKey;

	sKey.Init(szKey, uiLength);
	return mcMap.Find(&sKey);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class MAP, class SIZE>
bool CHashStringBase<MAP, SIZE>::HasKey(const char* szKey)
{
	return Find(szKey) != SIZE::NotFound;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class MAP, class SIZE>
bool CHashStringBase<MAP, SIZE>::HasKey(const char* szKey, size uiLength)
{
	return Find(szKey, uiLength) != SIZE::NotFound;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class MAP, class SIZE>
bool CHashStringBase<MAP, SIZE>::Remove(const char* szKey)
{
	return Remove(szKey, (size)strlen(szKey));
}


//////////////////////////////////////////////////////////////////////////
//
// The last entry takes the removed one's index.
//
//////////////////////////////////////////////////////////////////////////
template<class MAP, class SIZE>
bool CHashStringBase<MAP, SIZE>::Remove(const char* szKey, size uiLength)
{
	SHashText	sKey;

	sKey.Init(szKey, uiLength);
	return mcMap.Remove(&sKey);
}


//////////////////////////////////////////////////////////////////////////
//
// The last entry takes uiIndex.
//
//////////////////////////////////////////////////////////////////////////
template<class MAP, class SIZE>
void CHashStringBase<MAP, SIZE>::RemoveAt(Count uiIndex)
{
	mcMap.RemoveAt(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class MAP, class SIZE>
bool CHashStringBase<MAP, SIZE>::Reserve(Count uiNumElements)
{
	return mcMap.Reserve(uiNumElements);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class MAP, class SIZE>
typename SIZE::Count CHashStringBase<MAP, SIZE>::NumElements(void) const
{
	return mcMap.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class MAP, class SIZE>
bool CHashStringBase<MAP, SIZE>::IsEmpty(void) const
{
	return mcMap.IsEmpty();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class MAP, class SIZE>
const char* CHashStringBase<MAP, SIZE>::GetKey(Count uiIndex)
{
	return mcMap.GetKey(uiIndex)->sz;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class MAP, class SIZE>
size CHashStringBase<MAP, SIZE>::GetKeyLength(Count uiIndex)
{
	return mcMap.GetKey(uiIndex)->uiLength;
}


#endif // __HASH_STRING_BASE_H__
