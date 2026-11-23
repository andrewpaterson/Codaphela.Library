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
#ifndef __HASH_MAP_STRING_H__
#define __HASH_MAP_STRING_H__
#include "MemoryStackExtended.h"
#include "HashText.h"
#include "HashMapTemplate.h"


#define HASH_MAP_STRING_CHUNK	2048


//A map from text to V by a hash of the text, which keeps its own copy of each key (NULL terminated, so GetKey gives a
//C string).  A key's text lasts until the map is killed or re-initialised, even after the key is removed.  Otherwise
//as CHashMapTemplate: the order added, pointers to values lasting until the next Put or Remove.
template<class V, class SIZE = SArrayNaturalSize>
class CHashMapString
{
public:
	typedef typename SIZE::Count	Count;

protected:
	CHashMapTemplate<SHashText, V, SHashOfText, SIZE>	mcMap;
	CMemoryStackExtended								mcText;

public:
	void		Init(void);
	void		Init(CMallocator* pcMalloc, lsize uiTextChunkSize = HASH_MAP_STRING_CHUNK);
	void		Kill(void);
	void		ReInit(void);

	V*			Get(const char* szKey);
	V*			Get(const char* szKey, size uiLength);
	V*			Put(const char* szKey);
	V*			FindOrAdd(const char* szKey, size uiLength, bool* pbAdded = NULL);
	bool		Put(const char* szKey, const V* pValue);
	bool		HasKey(const char* szKey);
	bool		HasKey(const char* szKey, size uiLength);
	bool		Remove(const char* szKey);
	bool		Remove(const char* szKey, size uiLength);
	void		RemoveAt(Count uiIndex);
	Count		Find(const char* szKey, size uiLength);
	bool		Reserve(Count uiNumElements);

	Count		NumElements(void) const;
	bool		IsEmpty(void) const;
	const char*	GetKey(Count uiIndex);
	size		GetKeyLength(Count uiIndex);
	V*			GetValue(Count uiIndex);

	void		SetDataFreeCallback(CDataFree* pcDataFree);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE>
void CHashMapString<V, SIZE>::Init(void)
{
	Init(&gcSystemAllocator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE>
void CHashMapString<V, SIZE>::Init(CMallocator* pcMalloc, lsize uiTextChunkSize)
{
	mcMap.Init(pcMalloc);
	mcText.Init(uiTextChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE>
void CHashMapString<V, SIZE>::Kill(void)
{
	mcMap.Kill();
	mcText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE>
void CHashMapString<V, SIZE>::ReInit(void)
{
	mcMap.ReInit();
	mcText.Clear();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE>
V* CHashMapString<V, SIZE>::Get(const char* szKey)
{
	SHashText	sKey;

	sKey.Init(szKey);
	return mcMap.Get(&sKey);
}


//////////////////////////////////////////////////////////////////////////
//
// szKey needn't be NULL terminated.
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE>
V* CHashMapString<V, SIZE>::Get(const char* szKey, size uiLength)
{
	SHashText	sKey;

	sKey.Init(szKey, uiLength);
	return mcMap.Get(&sKey);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE>
V* CHashMapString<V, SIZE>::Put(const char* szKey)
{
	return FindOrAdd(szKey, (size)strlen(szKey), NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// The value keyed szKey (needn't be NULL terminated), added (zeroed, the
// text copied) if there isn't one.  NULL if there is no memory for it.
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE>
V* CHashMapString<V, SIZE>::FindOrAdd(const char* szKey, size uiLength, bool* pbAdded)
{
	SHashText	sKey;
	Count		uiIndex;
	bool		bAdded;
	char*		szCopy;
	SHashText*	psKept;

	sKey.Init(szKey, uiLength);
	uiIndex = mcMap.FindOrAddIndex(&sKey, &bAdded);
	if (pbAdded)
	{
		*pbAdded = bAdded;
	}
	if (uiIndex == SIZE::NotFound)
	{
		return NULL;
	}
	if (bAdded)
	{
		szCopy = (char*)mcText.Add((lsize)uiLength + 1);
		if (szCopy == NULL)
		{
			mcMap.RemoveAt(uiIndex);
			return NULL;
		}
		memcpy(szCopy, szKey, uiLength);
		szCopy[uiLength] = '\0';
		psKept = mcMap.GetKey(uiIndex);
		psKept->sz = szCopy;
	}
	return mcMap.GetValue(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// szKey's value is a copy of *pValue, whether it had one or not.
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE>
bool CHashMapString<V, SIZE>::Put(const char* szKey, const V* pValue)
{
	V*	pDest;

	pDest = FindOrAdd(szKey, (size)strlen(szKey), NULL);
	if (pDest == NULL)
	{
		return false;
	}
	memcpy((void*)pDest, (const void*)pValue, sizeof(V));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE>
bool CHashMapString<V, SIZE>::HasKey(const char* szKey)
{
	SHashText	sKey;

	sKey.Init(szKey);
	return mcMap.HasKey(&sKey);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE>
bool CHashMapString<V, SIZE>::HasKey(const char* szKey, size uiLength)
{
	SHashText	sKey;

	sKey.Init(szKey, uiLength);
	return mcMap.HasKey(&sKey);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE>
bool CHashMapString<V, SIZE>::Remove(const char* szKey)
{
	SHashText	sKey;

	sKey.Init(szKey);
	return mcMap.Remove(&sKey);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE>
bool CHashMapString<V, SIZE>::Remove(const char* szKey, size uiLength)
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
template<class V, class SIZE>
void CHashMapString<V, SIZE>::RemoveAt(Count uiIndex)
{
	mcMap.RemoveAt(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE>
typename SIZE::Count CHashMapString<V, SIZE>::Find(const char* szKey, size uiLength)
{
	SHashText	sKey;

	sKey.Init(szKey, uiLength);
	return mcMap.Find(&sKey);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE>
bool CHashMapString<V, SIZE>::Reserve(Count uiNumElements)
{
	return mcMap.Reserve(uiNumElements);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE>
typename SIZE::Count CHashMapString<V, SIZE>::NumElements(void) const
{
	return mcMap.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE>
bool CHashMapString<V, SIZE>::IsEmpty(void) const
{
	return mcMap.IsEmpty();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE>
const char* CHashMapString<V, SIZE>::GetKey(Count uiIndex)
{
	return mcMap.GetKey(uiIndex)->sz;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE>
size CHashMapString<V, SIZE>::GetKeyLength(Count uiIndex)
{
	return mcMap.GetKey(uiIndex)->uiLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE>
V* CHashMapString<V, SIZE>::GetValue(Count uiIndex)
{
	return mcMap.GetValue(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE>
void CHashMapString<V, SIZE>::SetDataFreeCallback(CDataFree* pcDataFree)
{
	mcMap.SetDataFreeCallback(pcDataFree);
}


#endif // __HASH_MAP_STRING_H__

