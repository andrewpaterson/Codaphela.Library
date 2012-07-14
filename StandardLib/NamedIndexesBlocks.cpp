/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseObject.h"
#include "BaseLib/MemoryCache.h"
#include "CoreLib/IndexedFiles.h"
#include "NamedIndexesBlocksLoader.h"
#include "NamedIndexesBlocks.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexesBlocks::Init(int iBlockSize, int iMinNameLength, int iMaxNameLength, int iNewNumBlocks, CMemoryCache* pcCache, CIndexedFiles* pcFiles)
{
	macBlocks.Init(1);
	miBlockWidth = iBlockSize;
	miMinNameLength = iMinNameLength;
	miMaxNameLength = iMaxNameLength;
	miNewNumBlocks = iNewNumBlocks;
	miFileNumber = -1;
	mpcFiles = pcFiles;
	mpcCache = pcCache;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexesBlocks::Kill(void)
{
	int						i;
	CNamedIndexesBlock*		pcBlock;

	for (i = 0; i < macBlocks.NumElements(); i++)
	{
		pcBlock = macBlocks.Get(i);
		pcBlock->Kill();
	}
	macBlocks.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlocks::Load(void)
{
	CNamedIndexesBlocksLoader	cLoader;
	BOOL						bResult;

	cLoader.Init(this);
	bResult = cLoader.Load();
	cLoader.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlocks::FitsLength(int iNameLength)
{
	return (miMinNameLength <= iNameLength) && (miMaxNameLength > iNameLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlocks::Add(OIndex oi, CChars* szName, BOOL bFailOnExisting)
{
	int							i;
	CNamedIndexesBlock*			pcBlock;
	OIndex						oiExisiting;
	CNamedIndexesBlock*			pcNotFullBlock;
	CIndexedFile*				pcFile;
	void*						pvCache;
	CArrayNamedIndexesBlockPtr	cArrayBlockPrts;

	cArrayBlockPrts.Init(16);
	GetPotentialContainingBlocks(szName, &cArrayBlockPrts);
	SortBlockPtrsCachedFirst(&cArrayBlockPrts);

	pcNotFullBlock = NULL;

	for (i = 0; i < cArrayBlockPrts.NumElements(); i++)
	{
		pcBlock = *cArrayBlockPrts.Get(i);
		if (!pcBlock->IsCached())
		{
			Cache(pcBlock);
		}

		oiExisiting = pcBlock->GetIndex(szName);
		if (oiExisiting != INVALID_OBJECT_IDENTIFIER)
		{
			//Already exists;
			cArrayBlockPrts.Kill();
			return !bFailOnExisting;
		}

		if (pcNotFullBlock == NULL)
		{
			if (pcBlock->IsNotFull())
			{
				pcNotFullBlock = pcBlock;
			}
		}
	}
	cArrayBlockPrts.Kill();

	if (pcNotFullBlock == NULL)
	{
		for (i = 0; i < macBlocks.NumElements(); i++)
		{
			pcBlock = macBlocks.Get(i);
			if (pcBlock->IsNotFull())
			{
				pcNotFullBlock = pcBlock;
				break;
			}
		}
	}

	if (pcNotFullBlock == NULL)
	{
		pcNotFullBlock = macBlocks.Add();
		pcNotFullBlock->Init(miBlockWidth, miNewNumBlocks);
		Cache(pcNotFullBlock);
	}
	else if (!pcNotFullBlock->IsCached())
	{
		pcFile = mpcFiles->GetFile(miBlockWidth, miFileNumber);
		pvCache = AllocateInCache(pcBlock->GetAllocatedByteSize());

		pcNotFullBlock->Cache(pcFile, pvCache);
	}

	return pcNotFullBlock->AddUnsafe(oi, szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlocks::AddNewBlock(int iBlockWidth, void* pvBlocks, int iNumBlocks, int iDataIndex)
{
	CNamedIndexesBlock*		pcBlock;
	void*					pvCache;

	pcBlock = macBlocks.Add();
	if (!pcBlock)
	{
		return FALSE;
	}

	pvCache = AllocateInCache(miNewNumBlocks * miBlockWidth);
	if (!pvCache)
	{
		return FALSE;
	}

	pcBlock->Init(miBlockWidth, pvBlocks, iNumBlocks, iDataIndex, pvCache);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlocks::Cache(CNamedIndexesBlock* pcBlock)
{
	CIndexedFile*	pcFile;
	void*			pvCache;
	
	if (pcBlock->IsInFile())
	{
		pvCache = AllocateInCache(pcBlock->GetAllocatedByteSize());
		pcFile = mpcFiles->GetFile(miBlockWidth, miFileNumber);

		return pcBlock->Cache(pcFile, pvCache);
	}
	else
	{
		pvCache = AllocateInCache(pcBlock->GetAllocatedByteSize());
		return pcBlock->SetCache(pvCache);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CNamedIndexesBlocks::AllocateInCache(int iSize)
{
	void*						pvData;
	CArrayPointer				apEvicted;
	int							i;
	SMemoryCacheDescriptor*		psMemoryDesc;
	int							iDataSize;
	CNamedIndexesBlock*			pcNamedIndexes;
	void*						pvCacheMem;
	CIndexedFile*				pcFile;
	BOOL						bResult;

	apEvicted.Init(128);
	mpcCache->PreAllocate(iSize, &apEvicted);

	for (i = 0; i < apEvicted.NumElements(); i++)
	{
		psMemoryDesc = (SMemoryCacheDescriptor *)apEvicted.GetPtr(i);
		iDataSize = psMemoryDesc->iDataSize;
		pvCacheMem = RemapSinglePointer(psMemoryDesc, sizeof(SMemoryCacheDescriptor));
		pcNamedIndexes = GetNamedIndexesBlock(pvCacheMem);
		if (pcNamedIndexes == NULL)
		{
			//Somethings broken.
			return NULL;
		}

		if (miFileNumber != -1)
		{
			pcFile = mpcFiles->GetFile(miBlockWidth, miFileNumber);
		}
		else
		{
			pcFile = mpcFiles->GetOrCreateFile(miBlockWidth);
			miFileNumber = pcFile->miFileNumber;
		}
		
		bResult = pcNamedIndexes->Uncache(pcFile);
		if (!bResult)
		{
			apEvicted.Kill();
			return FALSE;
		}
	}

	apEvicted.Kill();
	pvData = mpcCache->Allocate(iSize);
	return pvData;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CNamedIndexesBlocks::GetIndex(CChars* szName)
{
	int							i;
	CNamedIndexesBlock*			pcBlock;
	OIndex						oiExisiting;
	BOOL						bResult;
	CArrayNamedIndexesBlockPtr	cArrayBlockPrts;

	cArrayBlockPrts.Init(16);
	GetPotentialContainingBlocks(szName, &cArrayBlockPrts);
	SortBlockPtrsCachedFirst(&cArrayBlockPrts);

	for (i = 0; i < cArrayBlockPrts.NumElements(); i++)
	{
		pcBlock = *cArrayBlockPrts.Get(i);
		if (!pcBlock->IsCached())
		{
			bResult = Cache(pcBlock);
			if (!bResult)
			{
				cArrayBlockPrts.Kill();
				return INVALID_OBJECT_IDENTIFIER;
			}
		}

		oiExisiting = pcBlock->GetIndex(szName);
		if (oiExisiting != INVALID_OBJECT_IDENTIFIER)
		{
			cArrayBlockPrts.Kill();
			return oiExisiting;
		}
	}

	cArrayBlockPrts.Kill();
	return INVALID_OBJECT_IDENTIFIER;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNamedIndexesBlock* CNamedIndexesBlocks::GetNamedIndexesBlock(void* pvCacheMem)
{
	int						i;
	CNamedIndexesBlock*		pcBlock;

	for (i = 0; i < macBlocks.NumElements(); i++)
	{
		pcBlock = macBlocks.Get(i);
		if (pcBlock->IsCache(pvCacheMem))
		{
			return pcBlock;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlocks::Remove(CChars* szName)
{
	int							i;
	CNamedIndexesBlock*			pcBlock;
	BOOL						bResult;
	CArrayNamedIndexesBlockPtr	cArrayBlockPrts;
	int							iIndex;

	cArrayBlockPrts.Init(16);
	GetPotentialContainingBlocks(szName, &cArrayBlockPrts);
	SortBlockPtrsCachedFirst(&cArrayBlockPrts);

	for (i = 0; i < cArrayBlockPrts.NumElements(); i++)
	{
		pcBlock = *cArrayBlockPrts.Get(i);
		if (!pcBlock->IsCached())
		{
			bResult = Cache(pcBlock);
			if (!bResult)
			{
				cArrayBlockPrts.Kill();
				return INVALID_OBJECT_IDENTIFIER;
			}
		}

		bResult = pcBlock->Remove(szName);
		if (bResult)
		{
			if (pcBlock->IsEmpty())
			{
				pcBlock->Kill();
				iIndex = macBlocks.GetIndex(pcBlock);
				macBlocks.RemoveAt(iIndex, FALSE);
				pcBlock = macBlocks.SafeGet(i);
				if (pcBlock)
				{
					pcBlock->Dirty();
				}
			}
			cArrayBlockPrts.Kill();
			return TRUE;
		}
	}
	cArrayBlockPrts.Kill();
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CNamedIndexesBlocks::NumNames(void)
{
	int						i;
	CNamedIndexesBlock*		pcBlock;
	int						iNames;

	iNames = 0;
	for (i = 0; i < macBlocks.NumElements(); i++)
	{
		pcBlock = macBlocks.Get(i);
		iNames += pcBlock->UsedNames();
	}

	return iNames;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlocks::Flush(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexesBlocks::GetPotentialContainingBlocks(CChars* szName, CArrayNamedIndexesBlockPtr* pcDest)
{
	int						i;
	CNamedIndexesBlock*		pcBlock;

	for (i = 0; i < macBlocks.NumElements(); i++)
	{
		pcBlock = macBlocks.Get(i);
		if (pcBlock->CouldContain(szName))
		{
			pcDest->Add(&pcBlock);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexesBlocks::SortBlockPtrsCachedFirst(CArrayNamedIndexesBlockPtr* pcDest)
{
	int		iLastCached;
	int		iFirstUncached;

	if (pcDest->NumElements() <= 1)
	{
		return;
	}

	iLastCached = pcDest->NumElements()-1;
	iFirstUncached = 0;

	for (;;)
	{
		iLastCached = FindLastCachedBlock(pcDest, iLastCached);
		iFirstUncached = FindLastCachedBlock(pcDest, iFirstUncached);

		if ((iLastCached == -1) || (iFirstUncached == -1))
		{
			break;
		}

		if (iLastCached <= iFirstUncached)
		{
			break;
		}

		pcDest->Swap(iLastCached, iFirstUncached);

		iLastCached--;
		iFirstUncached++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CNamedIndexesBlocks::FindLastCachedBlock(CArrayNamedIndexesBlockPtr* pcDest, int iEnd)
{
	int						i;
	CNamedIndexesBlock*		pcBlock;
	
	for (i = iEnd; i>=0; i--)
	{
		pcBlock = *pcDest->Get(i);
		if (pcBlock->IsCached())
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CNamedIndexesBlocks::FindFirstUncachedBlock(CArrayNamedIndexesBlockPtr* pcDest, int iStart)
{
	int						i;
	CNamedIndexesBlock*		pcBlock;

	for (i = iStart; i < pcDest->NumElements(); i++)
	{
		pcBlock = *pcDest->Get(i);
		if (!pcBlock->IsCached())
		{
			return i;
		}
	}
	return -1;
}


