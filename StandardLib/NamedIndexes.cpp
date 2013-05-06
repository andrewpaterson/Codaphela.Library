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
#include "NamedIndexes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexes::Init(CDurableFileController* pcController, int iCacheSize, int iBlockChunkSize)
{
	macBlocks.Init(2);
	mcCache.Init(iCacheSize);
	mcFiles.Init(pcController, "NAM");

	AddBlock(  32,    1,   23, iBlockChunkSize);
	AddBlock(  64,   23,   55, iBlockChunkSize);
	AddBlock(  96,   55,   87, iBlockChunkSize);
	AddBlock( 128,   87,  119, iBlockChunkSize);
	AddBlock( 192,  119,  183, iBlockChunkSize);
	AddBlock( 256,  183,  247, iBlockChunkSize);
	AddBlock( 512,  247,  503, iBlockChunkSize);
	AddBlock(1024,  503, 1015, iBlockChunkSize);
	AddBlock(4096, 1015, 4087, iBlockChunkSize);

	//mcFiles.Open();
	Open();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexes::Kill(void)
{
	int						i;
	CNamedIndexesBlocks*	pcBlock;
	
	for (i = 0; i < macBlocks.NumElements(); i++)
	{
		pcBlock = macBlocks.Get(i);
		pcBlock->Kill();
	}
	macBlocks.Kill();

	mcCache.Kill();
	mcFiles.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Save(void)
{
	int						i;
	CNamedIndexesBlocks*	pcBlock;
	BOOL					bResult;

	bResult = TRUE;
	for (i = 0; i < macBlocks.NumElements(); i++)
	{
		pcBlock = macBlocks.Get(i);
		bResult &= pcBlock->Save();
	}

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Close(void)
{
	return mcFiles.Close();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Open(void)
{
	int						i;
	CNamedIndexesBlocks*	pcBlock;
	BOOL					bResult;
	int						iFileNumber;

	bResult = mcFiles.Open();
	if (!bResult)
	{
		return FALSE;
	}

	for (i = 0; i < macBlocks.NumElements(); i++)
	{
		pcBlock = macBlocks.Get(i);
		iFileNumber = mcFiles.GetUniqueFileNumber(pcBlock->GetDataSize());

		bResult = pcBlock->Load(iFileNumber);
		if (!bResult)
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Flush(void)
{
	return Save();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Add(OIndex oi, char* szName, BOOL bFailOnExisting)
{
	CChars	szFake;
	BOOL	bResult;

	szFake.Fake(szName);
	bResult = Add(oi, &szFake, bFailOnExisting);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Add(OIndex oi, CChars* szName, BOOL bFailOnExisting)
{
	CNamedIndexesBlocks*	pcBlock;

	pcBlock = GetBlockFittingLength(szName->Length());
	if (pcBlock)
	{
		return pcBlock->Add(oi, szName, bFailOnExisting);
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CNamedIndexes::GetIndex(char* szName)
{
	CChars	szFake;

	if (szName)
	{
		szFake.Fake(szName);
		return GetIndex(&szFake);
	}
	else
	{
		return INVALID_O_INDEX;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CNamedIndexes::GetIndex(CChars* szName)
{
	CNamedIndexesBlocks*	pcBlock;

	if ((szName) && !szName->Empty())
	{
		pcBlock = GetBlockFittingLength(szName->Length());
		if (pcBlock)
		{
			return pcBlock->GetIndex(szName);
		}
		else
		{
			return INVALID_O_INDEX;
		}
	}
	else
	{
		return INVALID_O_INDEX;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Remove(CChars* szName)
{
	CNamedIndexesBlocks*	pcBlock;

	pcBlock = GetBlockFittingLength(szName->Length());
	if (pcBlock)
	{
		return pcBlock->Remove(szName);
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Remove(char* szName)
{
	CChars	szFake;

	szFake.Fake(szName);
	return Remove(&szFake);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNamedIndexesBlocks* CNamedIndexes::GetBlockFittingLength(int iNameLength)
{
	int						i;
	CNamedIndexesBlocks*	pcBlock;

	for (i = 0; i < macBlocks.NumElements(); i++)
	{
		pcBlock = macBlocks.Get(i);
		if (pcBlock->FitsLength(iNameLength))
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
CNamedIndexesBlocks* CNamedIndexes::GetBlockWithDataSize(int iDataSize)
{
	int						i;
	CNamedIndexesBlocks*	pcBlock;

	for (i = 0; i < macBlocks.NumElements(); i++)
	{
		pcBlock = macBlocks.Get(i);
		if (pcBlock->GetDataSize() == iDataSize)
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
CNamedIndexesBlocks* CNamedIndexes::AddBlock(int iBlockSize, int iMinNameLength, int iMaxNameLength, int iBlockChunkSize)
{
	CNamedIndexesBlocks*	pcBlock;

	pcBlock = macBlocks.Add();
	pcBlock->Init(iBlockSize, iMinNameLength, iMaxNameLength, iBlockChunkSize, this);
	return pcBlock;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CNamedIndexes::NumNames(void)
{
	int						i;
	CNamedIndexesBlocks*	pcBlock;
	filePos						iNames;

	iNames = 0;
	for (i = 0; i < macBlocks.NumElements(); i++)
	{
		pcBlock = macBlocks.Get(i);
		iNames += pcBlock->NumNames();
	}
	return iNames;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CNamedIndexes::AllocateInCache(size_t iSize)
{
	CMemoryCacheAllocation		cPreAllocated;
	void*						pvData;
	int							i;
	SMemoryCacheDescriptor*		psMemoryDesc;
	int							iCacheDescriptorSize;
	CNamedIndexesBlock*			pcNamedIndexes;
	void*						pvCacheMem;
	CIndexedFile*				pcFile;
	BOOL						bResult;
	CNamedIndexesBlocks*		pcBlocks;

	cPreAllocated.Init(iSize);
	if (!mcCache.PreAllocate(&cPreAllocated))
	{
		cPreAllocated.Kill();
		return NULL;
	}

	for (i = 0; i < cPreAllocated.NumElements(); i++)
	{
		psMemoryDesc = cPreAllocated.Get(i);
		iCacheDescriptorSize = psMemoryDesc->iDataSize;
		pcBlocks = GetBlockForCacheDescriptorSize(iCacheDescriptorSize);
		if (!pcBlocks)
		{
			return NULL;
		}

		pvCacheMem = RemapSinglePointer(psMemoryDesc, sizeof(SMemoryCacheDescriptor));
		pcNamedIndexes = pcBlocks->GetBlock(pvCacheMem);
		if (!pcNamedIndexes)
		{
			return NULL;
		}

		pcFile = GetOrCreateFile(pcBlocks->GetDataSize(), pcBlocks->GetFileNumber());
		pcBlocks->SetFileNumber(pcFile->miFileNumber);
		if (!pcFile)
		{
			return NULL;
		}

		bResult = pcNamedIndexes->Uncache(pcFile);
		if (!bResult)
		{
			cPreAllocated.Kill();
			return NULL;
		}
	}

	pvData = mcCache.Allocate(&cPreAllocated);

	cPreAllocated.Kill();

	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexedFile* CNamedIndexes::GetOrCreateFile(int iDataSize, int iFileNumber)
{
	CIndexedFile*	pcIndexedFile;

	if (iFileNumber != -1)
	{
		pcIndexedFile = GetFile(iDataSize, iFileNumber);
	}
	else
	{
		pcIndexedFile = mcFiles.GetOrCreateFile(iDataSize);
	}

	return pcIndexedFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexedFile* CNamedIndexes::GetFile(int iDataSize, int iFileNumber)
{
	return mcFiles.GetFile(iDataSize, iFileNumber);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNamedIndexesBlocks* CNamedIndexes::GetBlockForCacheDescriptorSize(int iCacheDescriptorSize)
{
	int						i;
	CNamedIndexesBlocks*	pcBlock;
	CNamedIndexesBlocks*	pcFoundBlock;
	int						iNumFound;

	iNumFound = 0;
	for (i = 0; i < macBlocks.NumElements(); i++)
	{
		pcBlock = macBlocks.Get(i);
		if (pcBlock->GetCacheDescriptorSize() == iCacheDescriptorSize)
		{
			iNumFound++;
			pcFoundBlock =  pcBlock;
		}
	}

	if (iNumFound == 1)
	{
		return pcFoundBlock;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexes::TestGetPotentialContainingBlocks(char* szName, CArrayNamedIndexesBlockPtr* pcDest)
{
	int						i;
	CNamedIndexesBlocks*	pcBlock;
	int						iNameLength;
	CChars	szFake;

	if (!szName)
	{
		return;
	}

	szFake.Fake(szName);
	if (szFake.Empty())
	{
		return;
	}
	
	iNameLength = (int)strlen(szName);
	for (i = 0; i < macBlocks.NumElements(); i++)
	{
		pcBlock = macBlocks.Get(i);
		if (pcBlock->FitsLength(iNameLength))
		{
			pcBlock->GetPotentialContainingBlocks(&szFake, pcDest);
		}
	}
}


CArrayNamedIndexesBlocks* CNamedIndexes::GetBlocks(void) { return &macBlocks; }
CMemoryCache* CNamedIndexes::GetCache(void) { return &mcCache; }
CIndexedFiles* CNamedIndexes::GetFiles(void) { return &mcFiles; }
