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
#include "Logger.h"
#include "IntegerHelper.h"
#include "SlabAllocator.h"


CSlabAllocator	gcSlabAllocator;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSlabAllocator::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CSlabAllocator::Malloc(lsize uiSize)
{
	if (uiSize > SLAB_SIZE)
	{
		gcLogger.Error2(__METHOD__, " Cannot allocate [", LongToString((uint64)uiSize), "] bytes, slabs are [", LongToString((uint64)SLAB_SIZE), "] bytes.", NULL);
		return NULL;
	}

	return AllocateSlab();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CSlabAllocator::Realloc(void* pv, lsize uiSize)
{
	if (pv == NULL)
	{
		return Malloc(uiSize);
	}
	if (uiSize > SLAB_SIZE)
	{
		gcLogger.Error2(__METHOD__, " Cannot reallocate to [", LongToString((uint64)uiSize), "] bytes, slabs are [", LongToString((uint64)SLAB_SIZE), "] bytes.", NULL);
		return NULL;
	}
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSlabAllocator::Free(void* pv)
{
	if (pv)
	{
		FreeSlab(pv);
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSlabAllocator::IsLocal(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CSlabAllocator::SizeOffset(void)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CSlabAllocator::ShortName(void)
{
	return "Slab";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* GetSlab(void* pv)
{
	return (void*)((ptr)pv & ~(ptr)(SLAB_SIZE - 1));
}

