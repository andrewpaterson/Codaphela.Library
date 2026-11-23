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
#ifndef __HASH_SET_TEMPLATE_H__
#define __HASH_SET_TEMPLATE_H__
#include "HashMapTemplate.h"


//A set of K by a hash of each (HASH: see SHashOf), kept in the order added: CHashMapTemplate with no value.
template<class K, class HASH = SHashOf<K>, class SIZE = SArrayNaturalSize>
class CHashSetTemplate
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

	bool	Add(const K* pKey);
	K*		Get(const K* pKey);
	bool	HasKey(const K* pKey);
	bool	Remove(const K* pKey);
	void	RemoveAt(Count uiIndex);
	Count	Find(const K* pKey);
	Count	FindOrAddIndex(const K* pKey, bool* pbAdded = NULL);
	bool	Reserve(Count uiNumElements);

	Count	NumElements(void) const;
	bool	IsEmpty(void) const;
	K*		GetKey(Count uiIndex);

	bool	Write(CFileWriter* pcFileWriter);
	bool	Read(CFileReader* pcFileReader);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class K, class HASH, class SIZE>
void CHashSetTemplate<K, HASH, SIZE>::Init(void)
{
	Init(&gcSystemAllocator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class K, class HASH, class SIZE>
void CHashSetTemplate<K, HASH, SIZE>::Init(CMallocator* pcMalloc)
{
	typedef SHashSetLayout<K>	SLayout;

	mcBlock.Init(pcMalloc, (Count)SLayout::EntrySize(), (Count)SLayout::KeyOffset(), (Count)sizeof(K), (Count)SLayout::EntrySize(), 0, HASH::Equals());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class K, class HASH, class SIZE>
void CHashSetTemplate<K, HASH, SIZE>::Kill(void)
{
	mcBlock.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class K, class HASH, class SIZE>
void CHashSetTemplate<K, HASH, SIZE>::ReInit(void)
{
	mcBlock.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// False if it was in already (or there is no memory for it).
//
//////////////////////////////////////////////////////////////////////////
template<class K, class HASH, class SIZE>
bool CHashSetTemplate<K, HASH, SIZE>::Add(const K* pKey)
{
	bool	bAdded;

	mcBlock.Put(HASH::Hash(pKey), pKey, &bAdded);
	return bAdded;
}


//////////////////////////////////////////////////////////////////////////
//
// The key kept equal to pKey, or NULL.
//
//////////////////////////////////////////////////////////////////////////
template<class K, class HASH, class SIZE>
K* CHashSetTemplate<K, HASH, SIZE>::Get(const K* pKey)
{
	Count	uiIndex;

	uiIndex = mcBlock.Find(HASH::Hash(pKey), pKey);
	if (uiIndex == SIZE::NotFound)
	{
		return NULL;
	}
	return (K*)mcBlock.GetKey(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class K, class HASH, class SIZE>
bool CHashSetTemplate<K, HASH, SIZE>::HasKey(const K* pKey)
{
	return mcBlock.Find(HASH::Hash(pKey), pKey) != SIZE::NotFound;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class K, class HASH, class SIZE>
bool CHashSetTemplate<K, HASH, SIZE>::Remove(const K* pKey)
{
	return mcBlock.Remove(HASH::Hash(pKey), pKey);
}


//////////////////////////////////////////////////////////////////////////
//
// The last key takes uiIndex.
//
//////////////////////////////////////////////////////////////////////////
template<class K, class HASH, class SIZE>
void CHashSetTemplate<K, HASH, SIZE>::RemoveAt(Count uiIndex)
{
	mcBlock.RemoveAt(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class K, class HASH, class SIZE>
typename SIZE::Count CHashSetTemplate<K, HASH, SIZE>::Find(const K* pKey)
{
	return mcBlock.Find(HASH::Hash(pKey), pKey);
}


//////////////////////////////////////////////////////////////////////////
//
// The index of the key, added if it isn't in (SIZE::NotFound if there is
// no memory for it).
//
//////////////////////////////////////////////////////////////////////////
template<class K, class HASH, class SIZE>
typename SIZE::Count CHashSetTemplate<K, HASH, SIZE>::FindOrAddIndex(const K* pKey, bool* pbAdded)
{
	return mcBlock.Put(HASH::Hash(pKey), pKey, pbAdded);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class K, class HASH, class SIZE>
bool CHashSetTemplate<K, HASH, SIZE>::Reserve(Count uiNumElements)
{
	return mcBlock.Reserve(uiNumElements);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class K, class HASH, class SIZE>
typename SIZE::Count CHashSetTemplate<K, HASH, SIZE>::NumElements(void) const
{
	return mcBlock.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class K, class HASH, class SIZE>
bool CHashSetTemplate<K, HASH, SIZE>::IsEmpty(void) const
{
	return mcBlock.IsEmpty();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class K, class HASH, class SIZE>
K* CHashSetTemplate<K, HASH, SIZE>::GetKey(Count uiIndex)
{
	return (K*)mcBlock.GetKey(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// Only for keys with no pointers in them.
//
//////////////////////////////////////////////////////////////////////////
template<class K, class HASH, class SIZE>
bool CHashSetTemplate<K, HASH, SIZE>::Write(CFileWriter* pcFileWriter)
{
	return mcBlock.Write(pcFileWriter);
}


//////////////////////////////////////////////////////////////////////////
//
// Initialises the set (don't call Init first) with what Write wrote.
//
//////////////////////////////////////////////////////////////////////////
template<class K, class HASH, class SIZE>
bool CHashSetTemplate<K, HASH, SIZE>::Read(CFileReader* pcFileReader)
{
	Init();
	return mcBlock.Read(pcFileReader);
}


#endif // __HASH_SET_TEMPLATE_H__

