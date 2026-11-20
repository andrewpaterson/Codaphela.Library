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
#ifndef __BANK_MALLOCATOR_H__
#define __BANK_MALLOCATOR_H__
#include "PrimitiveTypes.h"
#include "TrueMallocator.h"


//A heap over banks of memory that come and go as it needs them.  On the W65C816 a bank is the 64KB an index register
//reaches all of from its start, and banks are mapped into a process (Libc65's heap under the HCP65 kernal) or taken
//from main memory (the kernal's main memory heap).  What fits in a bank is allocated by a CTrueMallocator with a
//region for each bank the heap has, so no block crosses a bank; what doesn't fit has a run of whole neighbouring
//banks of its own.  A bank is asked of the CBankSource before it is used and given back to it when it empties, but
//for the spares, which are kept for what is allocated next.  Without a source every bank is there already.
//
//The banks are the bank sized pieces of the memory given to Init, each starting at a multiple of the bank size: the
//first and last can be parts of a bank.  Blocks of up to MaxSmall come from the banks lowest first (or highest first);
//larger ones from the highest run of whole free banks that holds them.  Like CTrueMallocator it uses nothing else from
//BaseLib, so it can be linked into programs that only use Libc65.  It isn't safe between threads.

#define BANK_MALLOCATOR_MAX_BANKS		256
#define BANK_MALLOCATOR_OVERHEAD		64		//The most a bank's region keeps for itself: its region's and a block's headers.

//What a bank is.
#define BANK_MALLOCATOR_FREE			0		//Not the heap's: asked of the source when it is needed.
#define BANK_MALLOCATOR_SMALL			1		//A region of the CTrueMallocator with blocks in it.
#define BANK_MALLOCATOR_SPARE			2		//A region of the CTrueMallocator with none, kept for the next.
#define BANK_MALLOCATOR_LARGE			3		//The first of a large allocation's banks.
#define BANK_MALLOCATOR_MORE			4		//One of a large allocation's banks after its first.
#define BANK_MALLOCATOR_RESERVED		5		//Never the heap's (Reserve).


//Where a CBankMallocator's banks come from.  Acquire is asked for uiCount neighbouring banks from pvBank: true if
//they are the heap's now, false if not (another bank is asked for instead).  Release gives them back.
class CBankSource
{
public:
	virtual bool	Acquire(void* pvBank, size uiCount) =0;
	virtual void	Release(void* pvBank, size uiCount) =0;
};


class CBankMallocator
{
protected:
	CTrueMallocator	mcSmall;
	CBankSource*	mpcSource;
	lsize			muiBankSize;
	int				miBankShift;		//muiBankSize is 1 << miBankShift.
	lsize			muiFirst;			//Where the first bank starts: a multiple of muiBankSize, at or before muiStart.
	lsize			muiStart;			//The memory given to Init.
	lsize			muiEnd;
	size			muiNumBanks;
	size			muiSpares;			//How many empty banks are kept.
	size			muiNumSpare;		//How many are.
	bool			mbHighFirst;		//Small blocks' banks are taken highest first.
	lsize			muiNumLarge;		//Large allocations.
	lsize			muiLargeBytes;		//Their banks' sizes.
	uint8			mauiBanks[BANK_MALLOCATOR_MAX_BANKS];	//BANK_MALLOCATOR_*.
	uint16			mauiBlocks[BANK_MALLOCATOR_MAX_BANKS];	//How many of mcSmall's blocks each bank has.

public:
	bool			Init(void* pvMemory, lsize uiBytes, lsize uiBankSize, CBankSource* pcSource, size uiSpares, bool bHighFirst);
	void			Kill(void);
	bool			Reserve(void* pvMemory, lsize uiBytes);

	void*			Malloc(lsize uiBytes);
	void*			Realloc(void* pv, lsize uiBytes);
	void			Free(void* pv);
	lsize			Size(void* pv);

	lsize			MaxSmall(void) const;
	size			NumBanks(void) const;
	size			NumHeld(void) const;
	uint8			GetBank(size uiBank) const;
	lsize			NumAllocations(void) const;
	lsize			AllocatedBytes(void);
	bool			Contains(void* pv) const;
	bool			IsValid(void);

protected:
	size			BankOf(void* pv) const;
	lsize			BankAddress(size uiBank) const;
	lsize			BankStart(size uiBank) const;
	lsize			BankEnd(size uiBank) const;
	bool			IsWhole(size uiBank) const;
	bool			Acquire(size uiBank, size uiCount);
	void			Release(size uiBank, size uiCount);
	bool			TakeSmall(size uiBank);
	void			BlockCame(void* pv);
	void			BlockGone(size uiBank);
	void*			MallocSmall(lsize uiBytes);
	void*			MallocLarge(lsize uiBytes);
	lsize			LargeBytes(size uiBank) const;
	void			FreeLarge(size uiBank);
};


#endif // __BANK_MALLOCATOR_H__

