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

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include <string.h>
#include "BankMallocator.h"


//////////////////////////////////////////////////////////////////////////
//
// The heap over uiBytes from pvMemory, in banks of uiBankSize (a power
// of two).  With a source none of them is the heap's until it needs it;
// without one they are all there to be used.  uiSpares empty banks are
// kept rather than given back.  False if there are no banks or more than
// BANK_MALLOCATOR_MAX_BANKS.
//
//////////////////////////////////////////////////////////////////////////
bool CBankMallocator::Init(void* pvMemory, lsize uiBytes, lsize uiBankSize, CBankSource* pcSource, size uiSpares, bool bHighFirst)
{
	lsize	uiBanks;
	size	i;

	mcSmall.Init();
	mpcSource = pcSource;
	muiBankSize = uiBankSize;
	muiStart = (lsize)pvMemory;
	muiEnd = muiStart + uiBytes;
	muiFirst = muiStart & ~(uiBankSize - 1);
	muiSpares = uiSpares;
	muiNumSpare = 0;
	mbHighFirst = bHighFirst;
	muiNumLarge = 0;
	muiLargeBytes = 0;
	muiNumBanks = 0;
	miBankShift = 0;
	if ((uiBytes == 0) || (uiBankSize == 0) || ((uiBankSize & (uiBankSize - 1)) != 0))
	{
		return false;
	}
	while (((lsize)1 << miBankShift) != uiBankSize)
	{
		miBankShift++;
	}

	uiBanks = ((muiEnd - 1 - muiFirst) >> miBankShift) + 1;
	if (uiBanks > BANK_MALLOCATOR_MAX_BANKS)
	{
		return false;
	}
	muiNumBanks = (size)uiBanks;
	for (i = 0; i < muiNumBanks; i++)
	{
		mauiBanks[i] = BANK_MALLOCATOR_FREE;
		mauiBlocks[i] = 0;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Every bank the heap has goes back to the source.
//
//////////////////////////////////////////////////////////////////////////
void CBankMallocator::Kill(void)
{
	size	i;

	for (i = 0; i < muiNumBanks; i++)
	{
		if ((mauiBanks[i] != BANK_MALLOCATOR_FREE) && (mauiBanks[i] != BANK_MALLOCATOR_RESERVED))
		{
			Release(i, 1);
		}
		mauiBanks[i] = BANK_MALLOCATOR_FREE;
		mauiBlocks[i] = 0;
	}
	mcSmall.Kill();
	muiNumBanks = 0;
	muiNumSpare = 0;
	muiNumLarge = 0;
	muiLargeBytes = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The banks uiBytes from pvMemory touch are never the heap's (HCP65's
// maths tables in a process's banks).  False, with nothing reserved, if
// any of them is the heap's already.
//
//////////////////////////////////////////////////////////////////////////
bool CBankMallocator::Reserve(void* pvMemory, lsize uiBytes)
{
	size	uiFirst;
	size	uiLast;
	size	i;

	if ((uiBytes == 0) || ((lsize)pvMemory < muiStart) || ((lsize)pvMemory >= muiEnd))
	{
		return false;
	}
	uiFirst = BankOf(pvMemory);
	uiLast = ((lsize)pvMemory + uiBytes - 1 >= muiEnd) ? (size)(muiNumBanks - 1) : BankOf((void*)((lsize)pvMemory + uiBytes - 1));
	for (i = uiFirst; i <= uiLast; i++)
	{
		if ((mauiBanks[i] != BANK_MALLOCATOR_FREE) && (mauiBanks[i] != BANK_MALLOCATOR_RESERVED))
		{
			return false;
		}
	}
	for (i = uiFirst; i <= uiLast; i++)
	{
		mauiBanks[i] = BANK_MALLOCATOR_RESERVED;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// uiBytes: up to MaxSmall in one bank, more in a run of whole banks.
// NULL if no bank (or run) the source gives holds it.
//
//////////////////////////////////////////////////////////////////////////
void* CBankMallocator::Malloc(lsize uiBytes)
{
	if (uiBytes <= MaxSmall())
	{
		return MallocSmall(uiBytes);
	}
	return MallocLarge(uiBytes);
}


//////////////////////////////////////////////////////////////////////////
//
// A small block grows or shrinks where it is, or in the banks the heap
// has, if it can; anything else is copied to a new allocation.  A large
// one that still needs more than a bank and fits its banks stays.
//
//////////////////////////////////////////////////////////////////////////
void* CBankMallocator::Realloc(void* pv, lsize uiBytes)
{
	size	uiBank;
	lsize	uiOld;
	void*	pvNew;

	if (!pv)
	{
		return Malloc(uiBytes);
	}
	uiBank = BankOf(pv);
	if (mauiBanks[uiBank] == BANK_MALLOCATOR_LARGE)
	{
		uiOld = LargeBytes(uiBank);
		if ((uiBytes > MaxSmall()) && (uiBytes <= uiOld))
		{
			return pv;
		}
	}
	else
	{
		uiOld = mcSmall.Size(pv);
		if (uiBytes <= MaxSmall())
		{
			pvNew = mcSmall.Realloc(pv, uiBytes);
			if (pvNew)
			{
				if (pvNew != pv)
				{
					BlockCame(pvNew);
					BlockGone(uiBank);
				}
				return pvNew;
			}
		}
	}

	pvNew = Malloc(uiBytes);
	if (pvNew)
	{
		memcpy(pvNew, pv, (uiOld < uiBytes) ? uiOld : uiBytes);
		Free(pv);
	}
	return pvNew;
}


//////////////////////////////////////////////////////////////////////////
//
// A bank left empty goes back to the source unless it is kept as a
// spare; a large allocation's banks go back (but one, as a spare, if
// there are too few).
//
//////////////////////////////////////////////////////////////////////////
void CBankMallocator::Free(void* pv)
{
	size	uiBank;

	if (!pv)
	{
		return;
	}
	uiBank = BankOf(pv);
	if (mauiBanks[uiBank] == BANK_MALLOCATOR_LARGE)
	{
		FreeLarge(uiBank);
		return;
	}
	mcSmall.Free(pv);
	BlockGone(uiBank);
}


//////////////////////////////////////////////////////////////////////////
//
// How many bytes pv can hold.
//
//////////////////////////////////////////////////////////////////////////
lsize CBankMallocator::Size(void* pv)
{
	size	uiBank;

	uiBank = BankOf(pv);
	if (mauiBanks[uiBank] == BANK_MALLOCATOR_LARGE)
	{
		return LargeBytes(uiBank);
	}
	return mcSmall.Size(pv);
}


//////////////////////////////////////////////////////////////////////////
//
// How many banks are the heap's: with blocks in them, spare, or a large
// allocation's.
//
//////////////////////////////////////////////////////////////////////////
size CBankMallocator::NumHeld(void) const
{
	size	uiHeld;
	size	i;

	uiHeld = 0;
	for (i = 0; i < muiNumBanks; i++)
	{
		if ((mauiBanks[i] != BANK_MALLOCATOR_FREE) && (mauiBanks[i] != BANK_MALLOCATOR_RESERVED))
		{
			uiHeld++;
		}
	}
	return uiHeld;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pv is in a bank the heap has.
//
//////////////////////////////////////////////////////////////////////////
bool CBankMallocator::Contains(void* pv) const
{
	uint8	uiKind;

	if (((lsize)pv < muiStart) || ((lsize)pv >= muiEnd))
	{
		return false;
	}
	uiKind = mauiBanks[BankOf(pv)];
	return (uiKind != BANK_MALLOCATOR_FREE) && (uiKind != BANK_MALLOCATOR_RESERVED);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the small blocks' allocator is valid and the banks agree with
// it: every block counted in a bank with blocks, the spares counted and
// empty, and every large allocation's banks after its first marked so.
//
//////////////////////////////////////////////////////////////////////////
bool CBankMallocator::IsValid(void)
{
	lsize	uiBlocks;
	lsize	uiLarge;
	size	uiSpare;
	size	uiRegions;
	size	i;

	uiBlocks = 0;
	uiLarge = 0;
	uiSpare = 0;
	for (i = 0; i < muiNumBanks; i++)
	{
		switch (mauiBanks[i])
		{
		case BANK_MALLOCATOR_SMALL:
			if (mauiBlocks[i] == 0)
			{
				return false;
			}
			uiBlocks += mauiBlocks[i];
			break;
		case BANK_MALLOCATOR_SPARE:
			if ((mauiBlocks[i] != 0) || !mcSmall.IsRegionFree((void*)BankStart(i)))
			{
				return false;
			}
			uiSpare++;
			break;
		case BANK_MALLOCATOR_LARGE:
			uiLarge++;
			break;
		case BANK_MALLOCATOR_MORE:
			if ((i == 0) || ((mauiBanks[i - 1] != BANK_MALLOCATOR_LARGE) && (mauiBanks[i - 1] != BANK_MALLOCATOR_MORE)))
			{
				return false;
			}
			break;
		}
		if ((mauiBanks[i] != BANK_MALLOCATOR_SMALL) && (mauiBanks[i] != BANK_MALLOCATOR_SPARE) && (mauiBlocks[i] != 0))
		{
			return false;
		}
	}

	//CTrueMallocator isn't valid without a region: a heap with no small blocks' banks has none.
	uiRegions = 0;
	for (i = 0; i < muiNumBanks; i++)
	{
		uiRegions += ((mauiBanks[i] == BANK_MALLOCATOR_SMALL) || (mauiBanks[i] == BANK_MALLOCATOR_SPARE)) ? 1 : 0;
	}
	if ((uiRegions > 0) ? !mcSmall.IsValid() : (mcSmall.NumAllocations() != 0))
	{
		return false;
	}
	return (uiBlocks == mcSmall.NumAllocations()) && (uiSpare == muiNumSpare) && (uiSpare <= muiSpares) && (uiLarge == muiNumLarge);
}


//////////////////////////////////////////////////////////////////////////
//
// Bank uiBank, asked of the source (uiCount from it, for a large
// allocation).  With no source every bank is there.
//
//////////////////////////////////////////////////////////////////////////
bool CBankMallocator::Acquire(size uiBank, size uiCount)
{
	if (!mpcSource)
	{
		return true;
	}
	return mpcSource->Acquire((void*)BankAddress(uiBank), uiCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBankMallocator::Release(size uiBank, size uiCount)
{
	if (mpcSource && (uiCount > 0))
	{
		mpcSource->Release((void*)BankAddress(uiBank), uiCount);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Free bank uiBank becomes a region of the small blocks' allocator (with
// no blocks yet).  False, with it given back, if the source doesn't give
// it or it is a part of a bank too small to be a region.
//
//////////////////////////////////////////////////////////////////////////
bool CBankMallocator::TakeSmall(size uiBank)
{
	if (!Acquire(uiBank, 1))
	{
		return false;
	}
	if (!mcSmall.AddRegion((void*)BankStart(uiBank), BankEnd(uiBank) - BankStart(uiBank)))
	{
		Release(uiBank, 1);
		return false;
	}
	mauiBanks[uiBank] = BANK_MALLOCATOR_SMALL;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A block was put in pv's bank: a spare has a block now.
//
//////////////////////////////////////////////////////////////////////////
void CBankMallocator::BlockCame(void* pv)
{
	size	uiBank;

	uiBank = BankOf(pv);
	if (mauiBanks[uiBank] == BANK_MALLOCATOR_SPARE)
	{
		mauiBanks[uiBank] = BANK_MALLOCATOR_SMALL;
		muiNumSpare--;
	}
	mauiBlocks[uiBank]++;
}


//////////////////////////////////////////////////////////////////////////
//
// A block left bank uiBank.  If it was the last the bank is kept as a
// spare, or given back if there are spares enough.
//
//////////////////////////////////////////////////////////////////////////
void CBankMallocator::BlockGone(size uiBank)
{
	mauiBlocks[uiBank]--;
	if ((mauiBlocks[uiBank] != 0) || (mauiBanks[uiBank] != BANK_MALLOCATOR_SMALL))
	{
		return;
	}
	if (muiNumSpare < muiSpares)
	{
		mauiBanks[uiBank] = BANK_MALLOCATOR_SPARE;
		muiNumSpare++;
	}
	else if (mcSmall.RemoveRegion((void*)BankStart(uiBank)))
	{
		mauiBanks[uiBank] = BANK_MALLOCATOR_FREE;
		Release(uiBank, 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// uiBytes in the banks the heap has, or else in the first free bank (by
// mbHighFirst) the source gives.
//
//////////////////////////////////////////////////////////////////////////
void* CBankMallocator::MallocSmall(lsize uiBytes)
{
	void*	pv;
	size	uiBank;
	size	i;

	pv = mcSmall.Malloc(uiBytes);
	for (i = 0; !pv && (i < muiNumBanks); i++)
	{
		uiBank = mbHighFirst ? (size)(muiNumBanks - 1 - i) : i;
		if ((mauiBanks[uiBank] == BANK_MALLOCATOR_FREE) && TakeSmall(uiBank))
		{
			pv = mcSmall.Malloc(uiBytes);
			if (!pv && mcSmall.RemoveRegion((void*)BankStart(uiBank)))
			{
				//A part of a bank at the heap's start or end too small for it.
				mauiBanks[uiBank] = BANK_MALLOCATOR_FREE;
				Release(uiBank, 1);
			}
		}
	}
	if (pv)
	{
		BlockCame(pv);
	}
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//
// uiBytes in the highest run of whole free banks that holds them and the
// source gives.
//
//////////////////////////////////////////////////////////////////////////
void* CBankMallocator::MallocLarge(lsize uiBytes)
{
	lsize	uiBanks;
	size	uiCount;
	size	uiRun;
	size	uiBank;
	size	i;

	//Without the wrap of uiBytes + muiBankSize - 1 near the top of lsize.
	uiBanks = (uiBytes >> miBankShift) + ((uiBytes & (muiBankSize - 1)) != 0);
	if (uiBanks > muiNumBanks)
	{
		return NULL;
	}
	uiCount = (size)uiBanks;

	uiRun = 0;
	for (uiBank = muiNumBanks; uiBank > 0; uiBank--)
	{
		if ((mauiBanks[uiBank - 1] != BANK_MALLOCATOR_FREE) || !IsWhole((size)(uiBank - 1)))
		{
			uiRun = 0;
			continue;
		}
		uiRun++;
		if (uiRun == uiCount)
		{
			if (Acquire((size)(uiBank - 1), uiCount))
			{
				mauiBanks[uiBank - 1] = BANK_MALLOCATOR_LARGE;
				for (i = 1; i < uiCount; i++)
				{
					mauiBanks[uiBank - 1 + i] = BANK_MALLOCATOR_MORE;
				}
				muiNumLarge++;
				muiLargeBytes += (lsize)uiCount << miBankShift;
				return (void*)BankAddress((size)(uiBank - 1));
			}
			//Refused: the run one bank lower is tried next.
			uiRun--;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// How many bytes the large allocation from bank uiBank has: its banks.
//
//////////////////////////////////////////////////////////////////////////
lsize CBankMallocator::LargeBytes(size uiBank) const
{
	size	uiCount;

	for (uiCount = 1; ((size)(uiBank + uiCount) < muiNumBanks) && (mauiBanks[uiBank + uiCount] == BANK_MALLOCATOR_MORE); uiCount++)
	{
	}
	return (lsize)uiCount << miBankShift;
}


//////////////////////////////////////////////////////////////////////////
//
// The large allocation from bank uiBank goes back, but its first bank
// is kept as a spare if there are too few.
//
//////////////////////////////////////////////////////////////////////////
void CBankMallocator::FreeLarge(size uiBank)
{
	size	uiCount;
	size	i;

	uiCount = (size)(LargeBytes(uiBank) >> miBankShift);
	for (i = 0; i < uiCount; i++)
	{
		mauiBanks[uiBank + i] = BANK_MALLOCATOR_FREE;
	}
	muiNumLarge--;
	muiLargeBytes -= (lsize)uiCount << miBankShift;

	if ((muiNumSpare < muiSpares) && mcSmall.AddRegion((void*)BankStart(uiBank), BankEnd(uiBank) - BankStart(uiBank)))
	{
		mauiBanks[uiBank] = BANK_MALLOCATOR_SPARE;
		muiNumSpare++;
		Release((size)(uiBank + 1), (size)(uiCount - 1));
	}
	else
	{
		Release(uiBank, uiCount);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Bank uiBank's memory: all of it, or the part of it Init was given.
//
//////////////////////////////////////////////////////////////////////////
lsize CBankMallocator::BankStart(size uiBank) const
{
	lsize	uiAddress;

	uiAddress = BankAddress(uiBank);
	return (uiAddress < muiStart) ? muiStart : uiAddress;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CBankMallocator::BankEnd(size uiBank) const
{
	lsize	uiEnd;

	uiEnd = BankAddress(uiBank) + muiBankSize;
	return (uiEnd > muiEnd) ? muiEnd : uiEnd;
}


lsize CBankMallocator::MaxSmall(void) const { return (muiBankSize > BANK_MALLOCATOR_OVERHEAD) ? muiBankSize - BANK_MALLOCATOR_OVERHEAD : 0; }
size CBankMallocator::NumBanks(void) const { return muiNumBanks; }
uint8 CBankMallocator::GetBank(size uiBank) const { return mauiBanks[uiBank]; }
lsize CBankMallocator::NumAllocations(void) const { return mcSmall.NumAllocations() + muiNumLarge; }
lsize CBankMallocator::AllocatedBytes(void) { return mcSmall.AllocatedBytes() + muiLargeBytes; }
size CBankMallocator::BankOf(void* pv) const { return (size)(((lsize)pv - muiFirst) >> miBankShift); }
lsize CBankMallocator::BankAddress(size uiBank) const { return muiFirst + ((lsize)uiBank << miBankShift); }
bool CBankMallocator::IsWhole(size uiBank) const { return (BankAddress(uiBank) >= muiStart) && (BankAddress(uiBank) + muiBankSize <= muiEnd); }

