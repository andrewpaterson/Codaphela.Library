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
BOOL CNamedIndexes::Init(CDurableFileController* pcController, int iCacheSize, int iNewNumBlocks)
{
	macBlocks.Init(2);
	mcCache.Init(iCacheSize);
	mcFiles.Init(pcController, "NAM");

	AddBlock(  32,    1,   23, iNewNumBlocks);
	AddBlock(  64,   23,   55, iNewNumBlocks);
	AddBlock(  96,   55,   87, iNewNumBlocks);
	AddBlock( 128,   87,  119, iNewNumBlocks);
	AddBlock( 192,  119,  183, iNewNumBlocks);
	AddBlock( 256,  183,  247, iNewNumBlocks);
	AddBlock( 512,  247,  503, iNewNumBlocks);
	AddBlock(1024,  503, 1015, iNewNumBlocks);
	AddBlock(4096, 1015, 4087, iNewNumBlocks);

	return TRUE;
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
void CNamedIndexes::Close(void)
{
	int						i;
	CNamedIndexesBlocks*	pcBlock;

	for (i = 0; i < macBlocks.NumElements(); i++)
	{
		pcBlock = macBlocks.Get(i);
		pcBlock->Save();
	}

	mcFiles.Close();
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

	pcBlock = GetBlock(szName->Length());
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
		return INVALID_OBJECT_IDENTIFIER;
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
		pcBlock = GetBlock(szName->Length());
		if (pcBlock)
		{
			return pcBlock->GetIndex(szName);
		}
		else
		{
			return INVALID_OBJECT_IDENTIFIER;
		}
	}
	else
	{
		return INVALID_OBJECT_IDENTIFIER;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Remove(CChars* szName)
{
	CNamedIndexesBlocks*	pcBlock;

	pcBlock = GetBlock(szName->Length());
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
BOOL CNamedIndexes::Flush(void)
{
	int						i;
	CNamedIndexesBlocks*	pcBlock;
	BOOL					bResult;

	bResult = TRUE;
	for (i = 0; i < macBlocks.NumElements(); i++)
	{
		pcBlock = macBlocks.Get(i);
		bResult &= pcBlock->Flush();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNamedIndexesBlocks* CNamedIndexes::GetBlock(int iNameLength)
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
CNamedIndexesBlocks* CNamedIndexes::AddBlock(int iBlockSize, int iMinNameLength, int iMaxNameLength, int iNewNumBlocks)
{
	CNamedIndexesBlocks*	pcBlock;

	pcBlock = macBlocks.Add();
	pcBlock->Init(iBlockSize, iMinNameLength, iMaxNameLength, iNewNumBlocks, this);
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
		pcNamedIndexes = pcBlocks->GetNamedIndexesBlock(pvCacheMem);
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
		if (pcIndexedFile->mbNew)
		{
			pcIndexedFile->mbNew = FALSE;
			mcFiles.WriteIndexedFileDescriptors();
		}
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNamedIndexesBlocks* CNamedIndexes::TestGetBlock(int iNameLength)
{
	return GetBlock(iNameLength);
}

