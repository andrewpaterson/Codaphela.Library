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
#ifndef __HASH_MAP_TEMPLATE_H__
#define __HASH_MAP_TEMPLATE_H__
#include "ConstructorCall.h"
#include "SystemAllocator.h"
#include "HashBlock.h"


//Where a hash table entry's key goes: after the hash (a uint32), at its own alignment.
template<class K>
struct SHashSetLayout
{
	static size Align(size uiOffset, size uiAlignment)
	{
		return (uiOffset + uiAlignment - 1) & ~(uiAlignment - 1);
	}

	static size KeyOffset(void)
	{
		return Align(sizeof(uint32), alignof(K));
	}

	static size EntrySize(void)
	{
		size	uiAlignment;

		uiAlignment = alignof(K) > alignof(uint32) ? alignof(K) : alignof(uint32);
		return Align(KeyOffset() + sizeof(K), uiAlignment);
	}
};


//And its value: after the key, at its own alignment.
template<class K, class V>
struct SHashMapLayout
{
	static size Align(size uiOffset, size uiAlignment)
	{
		return SHashSetLayout<K>::Align(uiOffset, uiAlignment);
	}

	static size KeyOffset(void)
	{
		return SHashSetLayout<K>::KeyOffset();
	}

	static size ValueOffset(void)
	{
		return Align(KeyOffset() + sizeof(K), alignof(V));
	}

	static size EntrySize(void)
	{
		size	uiAlignment;

		uiAlignment = alignof(K) > alignof(V) ? alignof(K) : alignof(V);
		if (uiAlignment < alignof(uint32))
		{
			uiAlignment = alignof(uint32);
		}
		return Align(ValueOffset() + sizeof(V), uiAlignment);
	}
};


//A map from K to V by a hash of the key (HASH: see SHashOf), the entries kept in the order they were added (see
//CHashBlockBase for what that means after a Remove).  A pointer to a key or a value lasts only until the next Put or
//Remove: something that must last is kept as a pointer.  A new value is zeroed (and given its virtual function table
//if it has one); nothing is constructed or destructed otherwise.  SIZE is SArrayNaturalSize (counted in size) or
//SArrayLargeSize (lsize).
template<class K, class V, class HASH = SHashOf<K>, class SIZE = SArrayNaturalSize>
class CHashMapTemplate : protected CPostMalloc<V>
{
public:
	typedef typename SIZE::Count	Count;

protected:
	CHashBlockBase<SIZE>	mcBlock;

public:
	void	Init(void);
	void	Init(CMallocator* pcMalloc);
	void	Kill(void);
	void	ReInit(void);

	V*		Get(const K* pKey);
	V*		Put(const K* pKey);
	V*		FindOrAdd(const K* pKey, bool* pbAdded);
	bool	Put(const K* pKey, const V* pValue);
	bool	HasKey(const K* pKey);
	bool	Remove(const K* pKey);
	void	RemoveAt(Count uiIndex);
	Count	Find(const K* pKey);
	Count	FindOrAddIndex(const K* pKey, bool* pbAdded = NULL);
	bool	Reserve(Count uiNumElements);

	Count	NumElements(void) const;
	bool	IsEmpty(void) const;
	K*		GetKey(Count uiIndex);
	V*		GetValue(Count uiIndex);

	void	SetDataFreeCallback(CDataFree* pcDataFree);

	bool	Write(CFileWriter* pcFileWriter);
	bool	Read(CFileReader* pcFileReader);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class K, class V, class HASH, class SIZE>
void CHashMapTemplate<K, V, HASH, SIZE>::Init(void)
{
	Init(&gcSystemAllocator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class K, class V, class HASH, class SIZE>
void CHashMapTemplate<K, V, HASH, SIZE>::Init(CMallocator* pcMalloc)
{
	typedef SHashMapLayout<K, V>	SLayout;

	mcBlock.Init(pcMalloc, (Count)SLayout::EntrySize(), (Count)SLayout::KeyOffset(), (Count)sizeof(K), (Count)SLayout::ValueOffset(), (Count)sizeof(V), HASH::Equals());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class K, class V, class HASH, class SIZE>
void CHashMapTemplate<K, V, HASH, SIZE>::Kill(void)
{
	mcBlock.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class K, class V, class HASH, class SIZE>
void CHashMapTemplate<K, V, HASH, SIZE>::ReInit(void)
{
	mcBlock.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// The value keyed pKey, or NULL.
//
//////////////////////////////////////////////////////////////////////////
template<class K, class V, class HASH, class SIZE>
V* CHashMapTemplate<K, V, HASH, SIZE>::Get(const K* pKey)
{
	return (V*)mcBlock.Get(HASH::Hash(pKey), pKey);
}


//////////////////////////////////////////////////////////////////////////
//
// The value keyed pKey, added (zeroed) if there isn't one.  NULL if there
// is no memory for it.
//
//////////////////////////////////////////////////////////////////////////
template<class K, class V, class HASH, class SIZE>
V* CHashMapTemplate<K, V, HASH, SIZE>::Put(const K* pKey)
{
	return FindOrAdd(pKey, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// As Put(pKey), *pbAdded saying whether the value is new.  (Not a Put:
// with a bool value, Put(pKey, &bValue) would be this.)
//
//////////////////////////////////////////////////////////////////////////
template<class K, class V, class HASH, class SIZE>
V* CHashMapTemplate<K, V, HASH, SIZE>::FindOrAdd(const K* pKey, bool* pbAdded)
{
	Count	uiIndex;
	bool	bAdded;

	uiIndex = mcBlock.Put(HASH::Hash(pKey), pKey, &bAdded);
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
		return this->PostMalloc((V*)mcBlock.GetValue(uiIndex));
	}
	return (V*)mcBlock.GetValue(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// pKey's value is a copy of *pValue, whether it had one or not.
//
//////////////////////////////////////////////////////////////////////////
template<class K, class V, class HASH, class SIZE>
bool CHashMapTemplate<K, V, HASH, SIZE>::Put(const K* pKey, const V* pValue)
{
	V*	pDest;

	pDest = FindOrAdd(pKey, NULL);
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
template<class K, class V, class HASH, class SIZE>
bool CHashMapTemplate<K, V, HASH, SIZE>::HasKey(const K* pKey)
{
	return mcBlock.Find(HASH::Hash(pKey), pKey) != SIZE::NotFound;
}


//////////////////////////////////////////////////////////////////////////
//
// False if there was no entry keyed pKey.
//
//////////////////////////////////////////////////////////////////////////
template<class K, class V, class HASH, class SIZE>
bool CHashMapTemplate<K, V, HASH, SIZE>::Remove(const K* pKey)
{
	return mcBlock.Remove(HASH::Hash(pKey), pKey);
}


//////////////////////////////////////////////////////////////////////////
//
// The last entry takes uiIndex.
//
//////////////////////////////////////////////////////////////////////////
template<class K, class V, class HASH, class SIZE>
void CHashMapTemplate<K, V, HASH, SIZE>::RemoveAt(Count uiIndex)
{
	mcBlock.RemoveAt(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// The index of the entry keyed pKey, or SIZE::NotFound.
//
//////////////////////////////////////////////////////////////////////////
template<class K, class V, class HASH, class SIZE>
typename SIZE::Count CHashMapTemplate<K, V, HASH, SIZE>::Find(const K* pKey)
{
	return mcBlock.Find(HASH::Hash(pKey), pKey);
}


//////////////////////////////////////////////////////////////////////////
//
// As FindOrAdd, giving the entry's index (SIZE::NotFound if there is no
// memory).
//
//////////////////////////////////////////////////////////////////////////
template<class K, class V, class HASH, class SIZE>
typename SIZE::Count CHashMapTemplate<K, V, HASH, SIZE>::FindOrAddIndex(const K* pKey, bool* pbAdded)
{
	Count	uiIndex;
	bool	bAdded;

	uiIndex = mcBlock.Put(HASH::Hash(pKey), pKey, &bAdded);
	if (pbAdded)
	{
		*pbAdded = bAdded;
	}
	if ((uiIndex != SIZE::NotFound) && bAdded)
	{
		this->PostMalloc((V*)mcBlock.GetValue(uiIndex));
	}
	return uiIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class K, class V, class HASH, class SIZE>
bool CHashMapTemplate<K, V, HASH, SIZE>::Reserve(Count uiNumElements)
{
	return mcBlock.Reserve(uiNumElements);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class K, class V, class HASH, class SIZE>
typename SIZE::Count CHashMapTemplate<K, V, HASH, SIZE>::NumElements(void) const
{
	return mcBlock.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class K, class V, class HASH, class SIZE>
bool CHashMapTemplate<K, V, HASH, SIZE>::IsEmpty(void) const
{
	return mcBlock.IsEmpty();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class K, class V, class HASH, class SIZE>
K* CHashMapTemplate<K, V, HASH, SIZE>::GetKey(Count uiIndex)
{
	return (K*)mcBlock.GetKey(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class K, class V, class HASH, class SIZE>
V* CHashMapTemplate<K, V, HASH, SIZE>::GetValue(Count uiIndex)
{
	return (V*)mcBlock.GetValue(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// pcDataFree is given each value as it is removed, and each left when the
// map is killed or re-initialised.
//
//////////////////////////////////////////////////////////////////////////
template<class K, class V, class HASH, class SIZE>
void CHashMapTemplate<K, V, HASH, SIZE>::SetDataFreeCallback(CDataFree* pcDataFree)
{
	mcBlock.SetDataFreeCallback(pcDataFree);
}


//////////////////////////////////////////////////////////////////////////
//
// Only for keys and values with no pointers in them.
//
//////////////////////////////////////////////////////////////////////////
template<class K, class V, class HASH, class SIZE>
bool CHashMapTemplate<K, V, HASH, SIZE>::Write(CFileWriter* pcFileWriter)
{
	return mcBlock.Write(pcFileWriter);
}


//////////////////////////////////////////////////////////////////////////
//
// Initialises the map (don't call Init first) with what Write wrote.
//
//////////////////////////////////////////////////////////////////////////
template<class K, class V, class HASH, class SIZE>
bool CHashMapTemplate<K, V, HASH, SIZE>::Read(CFileReader* pcFileReader)
{
	Init();
	return mcBlock.Read(pcFileReader);
}


#endif // __HASH_MAP_TEMPLATE_H__

