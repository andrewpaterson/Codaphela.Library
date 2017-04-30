/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "FreeListMaybe.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFreeListMaybe::Init(int iElementSize, int iChunkSize)
{
	miElementSize = iElementSize;
	miChunkSize = iChunkSize;
	mpcLinkList = NULL;
	mpcFreeList = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFreeListMaybe::Kill(void)
{
	if (mpcFreeList != NULL)
	{
		mpcFreeList->Kill();
		Free(mpcFreeList);
	}
	if (mpcLinkList != NULL)
	{
		mpcLinkList->Kill();
		Free(mpcLinkList);
	}
	mpcLinkList = NULL;
	mpcFreeList = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CFreeListMaybe::Malloc(size_t tSize)
{
	return malloc(tSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CFreeListMaybe::Realloc(void* pv, size_t tSize)
{
	pv = realloc(pv, tSize);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFreeListMaybe::Free(void* pv)
{
	free(pv);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CFreeListMaybe::Add(void)
{
	if (mpcFreeList)
	{
		return mpcFreeList->Add();
	}
	if (mpcLinkList)
	{
		if (mpcLinkList->NumElements() != NUM_ELEMENTS_BEFORE_FREELIST)
		{
			return mpcLinkList->InsertAfterTail(miElementSize);
		}
		else
		{
			return CreateFreeList();
		}
	}
	return CreateLinkList();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CFreeListMaybe::CreateLinkList(void)
{
	mpcLinkList = (CLinkedListBlock*)Malloc(sizeof(CLinkedListBlock));
	mpcLinkList->Init();
	return mpcLinkList->InsertAfterTail(miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CFreeListMaybe::CreateFreeList(void)
{
	mpcFreeList = (CFreeList*)Malloc(sizeof(CFreeList));
	mpcFreeList->Init(miChunkSize, miElementSize);
	return mpcFreeList->Add();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CFreeListMaybe::AddUseFreeList(int iChunkSize)
{
	if (mpcFreeList)
	{
		return mpcFreeList->Add();
	}
	return CreateFreeList();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CFreeListMaybe::AddUseLinkedList(void)
{
	if (mpcLinkList)
	{
		return mpcLinkList->InsertAfterTail(miElementSize);
	}
	return CreateLinkList();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFreeListMaybe::Remove(void* pvElement)
{
	BOOL	bResult;

	if (!mpcFreeList)
	{
		mpcLinkList->Remove(pvElement);
	}
	else
	{
		bResult = mpcFreeList->Remove(pvElement);
		if (!bResult)
		{
			mpcLinkList->Remove(pvElement);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFreeListMaybe::SafeRemove(void* pvElement)
{
	if ((mpcFreeList) && (mpcFreeList->Remove(pvElement)))
	{
		return TRUE;
	}
	if ((mpcLinkList) && (mpcLinkList->SafeRemove(pvElement)))
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CFreeListMaybe::GetElementSize(void)
{
	return miElementSize;
}

