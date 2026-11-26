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
#ifndef __HASH_BLOCK_H__
#define __HASH_BLOCK_H__
#include "ArrayBlock.h"
#include "DataFree.h"
#include "HashOf.h"


//A hash table of entries of one size, knowing nothing of what is in them but where the key and the value are (see
//BaseLib's design page Docs/libraries/baselib/containers.md).  Each entry is its hash (a uint32, first), its key and its value.
//
//  - The entries are kept in an array in the order they were added: iterating is indexing it from 0 to
//    NumElements(), and an entry's index is its own until something is removed.
//  - The table holds only an entry's index + 1 (0 an empty slot), found by linear probing from the hash's low bits.
//    It is kept at most half full and doubles when it would be fuller; growing uses the hashes kept, so no key is
//    hashed or read again.
//  - Removing an entry shifts the table's following slots back (nothing is left behind) and moves the last entry
//    into its place: the order is the order added until something is removed, and then the last entry's index
//    changes to the removed one's.
//  - A pointer to an entry, its key or its value lasts only until the next Put or Remove (the array may move).
//  - The entries' memory doubles as it fills (no more than MAX_SIZE bytes where size is 16 bits).  When it can't, the
//    Put fails and the map is as it was.  It is given back by ReInit and Kill, not as entries are removed.
//
//The caller hashes (CHashMapTemplate and CHashSetTemplate do, inline) and gives the hash in.  Keys are compared by
//their bytes unless the block is given a HashEquals, which is called only for an entry whose hash is the same.
//Counted in size (CHashBlock, holding at most MAX_SIZE bytes of entries on the W65C816) or lsize (CHashLargeBlock).
template<class SIZE>
class CHashBlockBase : public CMalloc
{
public:
	typedef typename SIZE::Count	Count;

protected:
	uint8*					mpvEntries;		//The entries, muiEntrySize bytes each, in the order added.
	Count					muiNumEntries;
	Count					muiAllocatedEntries;
	Count					muiEntrySize;
	Count*					mauiSlots;		//An entry's index + 1, 0 for empty.  A power of two long, or NULL.
	Count					muiNumSlots;
	Count					muiKeyOffset;
	Count					muiKeySize;
	Count					muiValueOffset;
	Count					muiValueSize;
	HashEquals				mfEquals;		//NULL: keys are equal when their bytes are.
	CDataFree*				mpcDataFree;	//Given each value as it is removed or killed.

public:
	void	_Init(void);
	void	Init(Count uiEntrySize, Count uiKeyOffset, Count uiKeySize, Count uiValueOffset, Count uiValueSize, HashEquals fEquals);
	void	Init(CMallocator* pcMalloc, Count uiEntrySize, Count uiKeyOffset, Count uiKeySize, Count uiValueOffset, Count uiValueSize, HashEquals fEquals);
	void	Kill(void);
	void	ReInit(void);

	Count	Find(uint32 uiHash, const void* pvKey);
	void*	Get(uint32 uiHash, const void* pvKey);
	Count	Put(uint32 uiHash, const void* pvKey, bool* pbAdded = NULL);
	bool	Remove(uint32 uiHash, const void* pvKey);
	void	RemoveAt(Count uiIndex);
	bool	Reserve(Count uiNumElements);

	Count	NumElements(void) const;
	bool	IsEmpty(void) const;
	uint32	GetHash(Count uiIndex);
	void*	GetEntry(Count uiIndex);
	void*	GetKey(Count uiIndex);
	void*	GetValue(Count uiIndex);
	Count	GetEntrySize(void) const;
	Count	NumSlots(void) const;

	void	SetDataFreeCallback(CDataFree* pcDataFree);

	bool	Write(CFileWriter* pcFileWriter);
	bool	Read(CFileReader* pcFileReader);

protected:
	uint8*	Entry(Count uiIndex);
	bool	GrowEntries(Count uiNumEntries);
	void	FreeEntries(void);
	Count	HomeSlot(uint32 uiHash) const;
	Count	FindSlot(uint32 uiHash, const void* pvKey);
	bool	KeysEqual(const void* pvKey, const void* pvEntryKey);
	bool	Fits(Count uiNumElements);
	bool	Grow(Count uiNumElements);
	void	Rebuild(void);
	void	RemoveSlot(Count uiSlot);
	void	FreeValues(void);
};


extern template class CHashBlockBase<SArrayNaturalSize>;
extern template class CHashBlockBase<SArrayLargeSize>;


//Classes, not typedefs, so that they can be declared ahead.
class CHashBlock : public CHashBlockBase<SArrayNaturalSize> {};
class CHashLargeBlock : public CHashBlockBase<SArrayLargeSize> {};


#endif // __HASH_BLOCK_H__

