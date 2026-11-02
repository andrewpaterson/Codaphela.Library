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
#include "SystemAllocator.h"
#include "ReuseAllocator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CReuseAllocator::Init(lsize uiKeepLimit)
{
	mpvBlock = NULL;
	muiBlockSize = 0;
	muiKeepLimit = uiKeepLimit;
	mbInUse = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CReuseAllocator::Kill(void)
{
	FreeBlock();
}


//////////////////////////////////////////////////////////////////////////
//
// The kept block if it is free and big enough, else a new one (which is kept).
//
//////////////////////////////////////////////////////////////////////////
void* CReuseAllocator::Malloc(lsize uiSize)
{
	if (mbInUse)
	{
		return gcSystemAllocator.Malloc(uiSize);
	}

	if (uiSize > muiBlockSize)
	{
		FreeBlock();
		mpvBlock = gcSystemAllocator.Malloc(uiSize);
		if (mpvBlock == NULL)
		{
			return NULL;
		}
		muiBlockSize = uiSize;
	}
	mbInUse = true;
	return mpvBlock;
}


//////////////////////////////////////////////////////////////////////////
//
// The kept block only grows, unless it is bigger than the keep limit and uiSize is not (a CChars made with this
// allocator is emptied with a Realloc, not a Free).
//
//////////////////////////////////////////////////////////////////////////
void* CReuseAllocator::Realloc(void* pv, lsize uiSize)
{
	void*	pvNew;

	if (pv == NULL)
	{
		return Malloc(uiSize);
	}
	if (pv != mpvBlock)
	{
		return gcSystemAllocator.Realloc(pv, uiSize);
	}

	if ((uiSize > muiBlockSize) || ((muiKeepLimit != 0) && (muiBlockSize > muiKeepLimit) && (uiSize <= muiKeepLimit)))
	{
		pvNew = gcSystemAllocator.Realloc(mpvBlock, uiSize);
		if (pvNew == NULL)
		{
			//realloc leaves the block as it was.
			return NULL;
		}
		mpvBlock = pvNew;
		muiBlockSize = uiSize;
	}
	return mpvBlock;
}


//////////////////////////////////////////////////////////////////////////
//
// The kept block is kept for the next Malloc, unless it is bigger than the keep limit.
//
//////////////////////////////////////////////////////////////////////////
bool CReuseAllocator::Free(void* pv)
{
	if (pv == NULL)
	{
		return false;
	}
	if (pv != mpvBlock)
	{
		return gcSystemAllocator.Free(pv);
	}

	mbInUse = false;
	if ((muiKeepLimit != 0) && (muiBlockSize > muiKeepLimit))
	{
		FreeBlock();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CReuseAllocator::FreeBlock(void)
{
	if (mpvBlock != NULL)
	{
		gcSystemAllocator.Free(mpvBlock);
		mpvBlock = NULL;
	}
	muiBlockSize = 0;
	mbInUse = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CReuseAllocator::IsLocal(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CReuseAllocator::SizeOffset(void)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CReuseAllocator::ShortName(void)
{
	return "Reu";
}


//////////////////////////////////////////////////////////////////////////
//
// The kept block's size, 0 for none.
//
//////////////////////////////////////////////////////////////////////////
lsize CReuseAllocator::BlockSize(void)
{
	return muiBlockSize;
}
