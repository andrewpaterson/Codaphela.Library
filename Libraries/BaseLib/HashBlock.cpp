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
#include "SystemAllocator.h"
#include "FileReader.h"
#include "FileWriter.h"
#include "HashBlock.h"


#define HASH_BLOCK_FIRST_SLOTS		16
#define HASH_BLOCK_FIRST_ENTRIES	8


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CHashBlockBase<SIZE>::_Init(void)
{
	mpvEntries = NULL;
	muiNumEntries = 0;
	muiAllocatedEntries = 0;
	muiEntrySize = 0;
	mauiSlots = NULL;
	muiNumSlots = 0;
	muiKeyOffset = 0;
	muiKeySize = 0;
	muiValueOffset = 0;
	muiValueSize = 0;
	mfEquals = NULL;
	mpcDataFree = NULL;
	mpcMalloc = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CHashBlockBase<SIZE>::Init(Count uiEntrySize, Count uiKeyOffset, Count uiKeySize, Count uiValueOffset, Count uiValueSize, HashEquals fEquals)
{
	Init(&gcSystemAllocator, uiEntrySize, uiKeyOffset, uiKeySize, uiValueOffset, uiValueSize, fEquals);
}


//////////////////////////////////////////////////////////////////////////
//
// Entries of uiEntrySize bytes: the hash (a uint32) at 0, the key at
// uiKeyOffset and the value at uiValueOffset (uiValueSize 0 for a set).
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CHashBlockBase<SIZE>::Init(CMallocator* pcMalloc, Count uiEntrySize, Count uiKeyOffset, Count uiKeySize, Count uiValueOffset, Count uiValueSize, HashEquals fEquals)
{
	CMalloc::Init(pcMalloc);
	mpvEntries = NULL;
	muiNumEntries = 0;
	muiAllocatedEntries = 0;
	muiEntrySize = uiEntrySize;
	mauiSlots = NULL;
	muiNumSlots = 0;
	muiKeyOffset = uiKeyOffset;
	muiKeySize = uiKeySize;
	muiValueOffset = uiValueOffset;
	muiValueSize = uiValueSize;
	mfEquals = fEquals;
	mpcDataFree = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CHashBlockBase<SIZE>::Kill(void)
{
	FreeValues();
	FreeEntries();
	if (mauiSlots != NULL)
	{
		Free(mauiSlots);
		mauiSlots = NULL;
	}
	muiNumSlots = 0;
	CMalloc::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Empties the map, keeping what it was initialised with.
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CHashBlockBase<SIZE>::ReInit(void)
{
	FreeValues();
	FreeEntries();
	if (mauiSlots != NULL)
	{
		Free(mauiSlots);
		mauiSlots = NULL;
	}
	muiNumSlots = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
uint8* CHashBlockBase<SIZE>::Entry(Count uiIndex)
{
	return &mpvEntries[uiIndex * muiEntrySize];
}


//////////////////////////////////////////////////////////////////////////
//
// Room for uiNumEntries entries: twice as many as there was room for (or
// as many as fit), or just uiNumEntries if that much memory isn't free.
// The entries are copied to the new memory, not realloced, so that if
// there is none they are as they were.
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
bool CHashBlockBase<SIZE>::GrowEntries(Count uiNumEntries)
{
	Count	uiAllocate;
	uint8*	pvEntries;

	if (uiNumEntries <= muiAllocatedEntries)
	{
		return true;
	}
	if (!Fits(uiNumEntries))
	{
		return false;
	}

	uiAllocate = (muiAllocatedEntries == 0) ? HASH_BLOCK_FIRST_ENTRIES : muiAllocatedEntries;
	while (uiAllocate < uiNumEntries)
	{
		uiAllocate *= 2;
	}
	if (SIZE::CheckSize && (muiEntrySize != 0) && (uiAllocate > SIZE::Max / muiEntrySize))
	{
		uiAllocate = SIZE::Max / muiEntrySize;
	}

	pvEntries = (uint8*)Malloc((lsize)uiAllocate * muiEntrySize);
	if ((pvEntries == NULL) && (uiAllocate > uiNumEntries))
	{
		uiAllocate = uiNumEntries;
		pvEntries = (uint8*)Malloc((lsize)uiAllocate * muiEntrySize);
	}
	if (pvEntries == NULL)
	{
		return false;
	}

	if (mpvEntries != NULL)
	{
		SIZE::Copy(pvEntries, mpvEntries, muiNumEntries * muiEntrySize);
		Free(mpvEntries);
	}
	mpvEntries = pvEntries;
	muiAllocatedEntries = uiAllocate;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CHashBlockBase<SIZE>::FreeEntries(void)
{
	if (mpvEntries != NULL)
	{
		Free(mpvEntries);
		mpvEntries = NULL;
	}
	muiNumEntries = 0;
	muiAllocatedEntries = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
typename SIZE::Count CHashBlockBase<SIZE>::HomeSlot(uint32 uiHash) const
{
	return (Count)uiHash & (muiNumSlots - 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
bool CHashBlockBase<SIZE>::KeysEqual(const void* pvKey, const void* pvEntryKey)
{
	uint32	ui1;
	uint32	ui2;
	uint64	ulli1;
	uint64	ulli2;

	if (mfEquals != NULL)
	{
		return mfEquals(pvKey, pvEntryKey);
	}

	if (muiKeySize == sizeof(uint64))
	{
		memcpy(&ulli1, pvKey, sizeof(uint64));
		memcpy(&ulli2, pvEntryKey, sizeof(uint64));
		return ulli1 == ulli2;
	}
	else if (muiKeySize == sizeof(uint32))
	{
		memcpy(&ui1, pvKey, sizeof(uint32));
		memcpy(&ui2, pvEntryKey, sizeof(uint32));
		return ui1 == ui2;
	}
	return SIZE::Compare(pvKey, pvEntryKey, muiKeySize) == 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The slot holding the entry keyed pvKey, or the empty slot where it would
// go.  There must be slots.
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
typename SIZE::Count CHashBlockBase<SIZE>::FindSlot(uint32 uiHash, const void* pvKey)
{
	Count	uiMask;
	Count	uiSlot;
	Count	uiEntry;
	uint8*	pvEntry;

	uiMask = muiNumSlots - 1;
	uiSlot = (Count)uiHash & uiMask;
	for (;;)
	{
		uiEntry = mauiSlots[uiSlot];
		if (uiEntry == 0)
		{
			return uiSlot;
		}
		pvEntry = (uint8*)Entry(uiEntry - 1);
		if ((*(uint32*)pvEntry == uiHash) && KeysEqual(pvKey, &pvEntry[muiKeyOffset]))
		{
			return uiSlot;
		}
		uiSlot = (uiSlot + 1) & uiMask;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The index of the entry keyed pvKey (hashed to uiHash), or SIZE::NotFound.
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
typename SIZE::Count CHashBlockBase<SIZE>::Find(uint32 uiHash, const void* pvKey)
{
	Count	uiEntry;

	if (muiNumSlots == 0)
	{
		return SIZE::NotFound;
	}
	uiEntry = mauiSlots[FindSlot(uiHash, pvKey)];
	if (uiEntry == 0)
	{
		return SIZE::NotFound;
	}
	return uiEntry - 1;
}


//////////////////////////////////////////////////////////////////////////
//
// The value of the entry keyed pvKey, or NULL.
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void* CHashBlockBase<SIZE>::Get(uint32 uiHash, const void* pvKey)
{
	Count	uiIndex;

	uiIndex = Find(uiHash, pvKey);
	if (uiIndex == SIZE::NotFound)
	{
		return NULL;
	}
	return GetValue(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// The index of the entry keyed pvKey, added (its value zeroed) if there
// isn't one; *pbAdded says which.  SIZE::NotFound if there is no memory
// for it, or (counted in size where size is 16 bits) it would need more
// than MAX_SIZE bytes.
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
typename SIZE::Count CHashBlockBase<SIZE>::Put(uint32 uiHash, const void* pvKey, bool* pbAdded)
{
	Count	uiSlot;
	Count	uiIndex;
	uint8*	pvEntry;

	if (pbAdded)
	{
		*pbAdded = false;
	}

	if (muiNumSlots != 0)
	{
		uiSlot = FindSlot(uiHash, pvKey);
		if (mauiSlots[uiSlot] != 0)
		{
			return mauiSlots[uiSlot] - 1;
		}
	}

	uiIndex = muiNumEntries;
	if (!Fits(uiIndex + 1) || !Grow(uiIndex + 1))
	{
		return SIZE::NotFound;
	}
	uiSlot = FindSlot(uiHash, pvKey);

	if (!GrowEntries(uiIndex + 1))
	{
		return SIZE::NotFound;
	}
	pvEntry = Entry(uiIndex);
	muiNumEntries++;
	SIZE::Set(pvEntry, 0, muiEntrySize);
	*(uint32*)pvEntry = uiHash;
	SIZE::Copy(&pvEntry[muiKeyOffset], pvKey, muiKeySize);
	mauiSlots[uiSlot] = uiIndex + 1;

	if (pbAdded)
	{
		*pbAdded = true;
	}
	return uiIndex;
}


//////////////////////////////////////////////////////////////////////////
//
// Removes the entry keyed pvKey.  False if there isn't one.
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
bool CHashBlockBase<SIZE>::Remove(uint32 uiHash, const void* pvKey)
{
	Count	uiIndex;

	uiIndex = Find(uiHash, pvKey);
	if (uiIndex == SIZE::NotFound)
	{
		return false;
	}
	RemoveAt(uiIndex);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Removes the entry at uiIndex.  The last entry takes its index.
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CHashBlockBase<SIZE>::RemoveAt(Count uiIndex)
{
	Count	uiMask;
	Count	uiSlot;
	Count	uiLast;

	if (mpcDataFree && (muiValueSize != 0))
	{
		mpcDataFree->FreeData(GetValue(uiIndex));
	}

	uiMask = muiNumSlots - 1;
	uiSlot = HomeSlot(GetHash(uiIndex));
	while (mauiSlots[uiSlot] != uiIndex + 1)
	{
		uiSlot = (uiSlot + 1) & uiMask;
	}
	RemoveSlot(uiSlot);

	uiLast = muiNumEntries - 1;
	if (uiIndex != uiLast)
	{
		uiSlot = HomeSlot(GetHash(uiLast));
		while (mauiSlots[uiSlot] != uiLast + 1)
		{
			uiSlot = (uiSlot + 1) & uiMask;
		}
		mauiSlots[uiSlot] = uiIndex + 1;
		SIZE::Copy(Entry(uiIndex), Entry(uiLast), muiEntrySize);
	}
	muiNumEntries--;
}


//////////////////////////////////////////////////////////////////////////
//
// Empties uiSlot, moving the slots after it back into the gap where their
// entries' home slots allow, so that no probe stops short of its entry.
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CHashBlockBase<SIZE>::RemoveSlot(Count uiSlot)
{
	Count	uiMask;
	Count	uiGap;
	Count	uiNext;
	Count	uiHome;

	uiMask = muiNumSlots - 1;
	uiGap = uiSlot;
	for (uiNext = (uiGap + 1) & uiMask; mauiSlots[uiNext] != 0; uiNext = (uiNext + 1) & uiMask)
	{
		//An entry stays where it is if its home slot is cyclically after the gap and at or before it.
		uiHome = HomeSlot(GetHash(mauiSlots[uiNext] - 1));
		if (((uiNext > uiGap) && ((uiHome <= uiGap) || (uiHome > uiNext))) || ((uiNext < uiGap) && ((uiHome <= uiGap) && (uiHome > uiNext))))
		{
			mauiSlots[uiGap] = mauiSlots[uiNext];
			uiGap = uiNext;
		}
	}
	mauiSlots[uiGap] = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Room for uiNumElements entries, in the table and the entries' memory,
// so that neither grows again while they are added.
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
bool CHashBlockBase<SIZE>::Reserve(Count uiNumElements)
{
	return Fits(uiNumElements) && Grow(uiNumElements) && GrowEntries(uiNumElements);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether uiNumElements entries and the table for them fit in what the
// map is counted in.
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
bool CHashBlockBase<SIZE>::Fits(Count uiNumElements)
{
	Count	uiEntrySize;

	if (!SIZE::CheckSize)
	{
		return true;
	}
	uiEntrySize = muiEntrySize;
	if ((uiEntrySize != 0) && (uiNumElements > SIZE::Max / uiEntrySize))
	{
		return false;
	}
	//The table is at least twice as many slots.
	if (uiNumElements > SIZE::Max / (2 * sizeof(Count)))
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A table at most half full with uiNumElements entries in it: doubled
// (or made) as often as that takes, the entries put back in.
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
bool CHashBlockBase<SIZE>::Grow(Count uiNumElements)
{
	Count	uiNumSlots;
	Count*	auiSlots;

	if ((muiNumSlots != 0) && (uiNumElements <= muiNumSlots / 2))
	{
		return true;
	}
	if (!Fits(uiNumElements))
	{
		return false;
	}

	uiNumSlots = (muiNumSlots == 0) ? HASH_BLOCK_FIRST_SLOTS : muiNumSlots;
	while (uiNumElements > uiNumSlots / 2)
	{
		uiNumSlots *= 2;
	}

	auiSlots = (Count*)Malloc((lsize)uiNumSlots * sizeof(Count));  //In lsize: 32768 16 bit slots (from 8193 entries) are 64KB.
	if (auiSlots == NULL)
	{
		return false;
	}
	if (mauiSlots != NULL)
	{
		Free(mauiSlots);
	}
	mauiSlots = auiSlots;
	muiNumSlots = uiNumSlots;
	Rebuild();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The table made again from the entries' hashes.
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CHashBlockBase<SIZE>::Rebuild(void)
{
	Count	uiMask;
	Count	ui;
	Count	uiNumElements;
	Count	uiSlot;

	SIZE::Set(mauiSlots, 0, muiNumSlots * sizeof(Count));
	uiMask = muiNumSlots - 1;
	uiNumElements = muiNumEntries;
	for (ui = 0; ui < uiNumElements; ui++)
	{
		uiSlot = HomeSlot(GetHash(ui));
		while (mauiSlots[uiSlot] != 0)
		{
			uiSlot = (uiSlot + 1) & uiMask;
		}
		mauiSlots[uiSlot] = ui + 1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CHashBlockBase<SIZE>::FreeValues(void)
{
	Count	ui;
	Count	uiNumElements;

	if (mpcDataFree && (muiValueSize != 0))
	{
		uiNumElements = muiNumEntries;
		for (ui = 0; ui < uiNumElements; ui++)
		{
			mpcDataFree->FreeData(GetValue(ui));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
typename SIZE::Count CHashBlockBase<SIZE>::NumElements(void) const
{
	return muiNumEntries;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
bool CHashBlockBase<SIZE>::IsEmpty(void) const
{
	return muiNumEntries == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
uint32 CHashBlockBase<SIZE>::GetHash(Count uiIndex)
{
	return *(uint32*)Entry(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void* CHashBlockBase<SIZE>::GetEntry(Count uiIndex)
{
	return Entry(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void* CHashBlockBase<SIZE>::GetKey(Count uiIndex)
{
	return &((uint8*)Entry(uiIndex))[muiKeyOffset];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void* CHashBlockBase<SIZE>::GetValue(Count uiIndex)
{
	return &((uint8*)Entry(uiIndex))[muiValueOffset];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
typename SIZE::Count CHashBlockBase<SIZE>::GetEntrySize(void) const
{
	return muiEntrySize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
typename SIZE::Count CHashBlockBase<SIZE>::NumSlots(void) const
{
	return muiNumSlots;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CHashBlockBase<SIZE>::SetDataFreeCallback(CDataFree* pcDataFree)
{
	mpcDataFree = pcDataFree;
}


//////////////////////////////////////////////////////////////////////////
//
// The entries as they are, in order: only for keys and values whose bytes
// are all they are (no pointers).  The table isn't written; Read makes it.
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
bool CHashBlockBase<SIZE>::Write(CFileWriter* pcFileWriter)
{
	if (!pcFileWriter->WriteSize((lsize)muiEntrySize))
	{
		return false;
	}
	if (!pcFileWriter->WriteSize((lsize)muiKeySize))
	{
		return false;
	}
	if (!pcFileWriter->WriteSize((lsize)muiValueSize))
	{
		return false;
	}
	if (!pcFileWriter->WriteSize((lsize)muiNumEntries))
	{
		return false;
	}
	if (muiNumEntries != 0)
	{
		if (!pcFileWriter->WriteData(Entry(0), (lsize)muiNumEntries * muiEntrySize))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Replaces the entries with those Written.  Init must have been called,
// with the same sizes as the map Written.
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
bool CHashBlockBase<SIZE>::Read(CFileReader* pcFileReader)
{
	lsize	uiEntrySize;
	lsize	uiKeySize;
	lsize	uiValueSize;
	lsize	uiNumElements;

	if (!pcFileReader->ReadSize(&uiEntrySize) || !pcFileReader->ReadSize(&uiKeySize) || !pcFileReader->ReadSize(&uiValueSize) || !pcFileReader->ReadSize(&uiNumElements))
	{
		return false;
	}
	if ((uiEntrySize != (lsize)muiEntrySize) || (uiKeySize != (lsize)muiKeySize) || (uiValueSize != (lsize)muiValueSize))
	{
		return false;
	}
	if (uiNumElements > (lsize)SIZE::Max)
	{
		return false;
	}

	ReInit();
	if (uiNumElements == 0)
	{
		return true;
	}
	if (!Fits((Count)uiNumElements))
	{
		return false;
	}
	if (!GrowEntries((Count)uiNumElements))
	{
		return false;
	}
	muiNumEntries = (Count)uiNumElements;
	if (!pcFileReader->ReadData(Entry(0), (lsize)muiNumEntries * muiEntrySize))
	{
		muiNumEntries = 0;
		return false;
	}
	if (!Grow((Count)uiNumElements))
	{
		muiNumEntries = 0;
		return false;
	}
	return true;
}


template class CHashBlockBase<SArrayNaturalSize>;
template class CHashBlockBase<SArrayLargeSize>;

