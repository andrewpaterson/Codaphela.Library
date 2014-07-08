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
#include "CoreLib/IndexedFile.h"
#include "BaseObject.h"
#include "NamedIndexedBlock.h"
#include "NamedIndexesBlock.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexesBlock::Init(int iBlockWidth, filePos iBlockChunkSize)
{
	miDataIndex = -1;
	mszFirst.Init();
	mszLast.Init();

	mpvCachePos = NULL;
	miBlockWidth = iBlockWidth;
	miBlockChunkSize = iBlockChunkSize;
	miUsedBlocks = 0;
	mbDirty = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexesBlock::Init(int iBlockWidth, void* pvBlocks, filePos iBlockChunkSize, filePos iDataIndex, void* pvCache)
{
	CNamedIndexedBlock*		pcBlock;
	filePos					i;

	miDataIndex = iDataIndex;

	mpvCachePos = pvCache;
	miBlockWidth = iBlockWidth;
	miBlockChunkSize = iBlockChunkSize;
	mbDirty = FALSE;
	miUsedBlocks = iBlockChunkSize;

	memcpy_fast(mpvCachePos, pvBlocks, (size_t)(miUsedBlocks * miBlockWidth));

	for (i = 0; i < iBlockChunkSize; i++)
	{
		pcBlock = GetUnsafe(i);
		if (pcBlock->IsEmpty())
		{
			miUsedBlocks = i;
			break;;
		}
	}

	if (miUsedBlocks == 0)
	{
		mszFirst.Init();
		mszLast.Init();
	}
	else
	{
		pcBlock = GetUnsafe(0);
		mszFirst.Init(pcBlock->Name());

		pcBlock = GetUnsafe(miUsedBlocks - 1);
		mszLast.Init(pcBlock->Name());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexesBlock::Kill(void)
{
	mszLast.Kill();
	mszFirst.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlock::CouldContain(CChars* szName)
{
	if ((szName->CompareIgnoreCase(&mszFirst) >= 0) && (szName->CompareIgnoreCase(&mszLast) <= 0))
	{
		return TRUE;
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
BOOL CNamedIndexesBlock::IsNotFull(void)
{
	return miUsedBlocks < miBlockChunkSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlock::IsFull(void)
{
	return miUsedBlocks == miBlockChunkSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlock::IsInFile(void)
{
	return miDataIndex != -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlock::IsCached(void)
{
	return mpvCachePos != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlock::IsNotCached(void)
{
	return mpvCachePos == NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlock::AddUnsafe(OIndex oi, CChars* szName)
{
	CArrayBase			avFakeBlock;
	CNamedIndexedBlock	sBlock;
	int					iIndex;

	if (!mpvCachePos)
	{
		return FALSE;
	}

	memset_fast(&sBlock, 0, miBlockWidth);
	sBlock.Set(szName->Text(), oi);
	avFakeBlock.Fake(miBlockWidth, mpvCachePos, (int)miUsedBlocks, (int)miBlockChunkSize);
	
	//It's safe to insert into a faked array because we know there is at least one free element in the chunk
	//That is: miBlockChunkSize - miUsedBlocks >= 1
	iIndex = avFakeBlock.InsertIntoSorted(&CompareNamedIndexedBlock, &sBlock, FALSE);

	if (miUsedBlocks == 0)
	{
		mszFirst.Set(szName);
		mszLast.Set(szName);
	}
	else
	{
		if (iIndex == 0)
		{
			mszFirst.Set(szName);
		}
		else if (iIndex == miUsedBlocks)
		{
			mszLast.Set(szName);
		}
	}

	miUsedBlocks++;
	Dirty();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CNamedIndexesBlock::GetIndex(CChars* szName)
{
	CArrayBase				avFakeBlock;
	CNamedIndexedBlock		sBlock;
	CNamedIndexedBlock*		psBlock;
	int						iIndex;
	BOOL					bResult;

	strcpy(sBlock.Name(), szName->Text());

	avFakeBlock.Fake(miBlockWidth, mpvCachePos, (int)miUsedBlocks, (int)miBlockChunkSize);

	bResult = avFakeBlock.FindInSorted(&sBlock, &CompareNamedIndexedBlock, &iIndex);
	if (bResult)
	{
		if (iIndex != -1)
		{
			psBlock = (CNamedIndexedBlock*)avFakeBlock.Get(iIndex);
			return psBlock->Id();
		}
	}
	return INVALID_O_INDEX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlock::Remove(CChars* szName)
{
	CArrayBase				avFakeBlock;
	CNamedIndexedBlock		sBlock;
	int						iIndex;
	CNamedIndexedBlock*		pcBlock;
	
	strcpy(sBlock.Name(), szName->Text());

	avFakeBlock.Fake(miBlockWidth, mpvCachePos, (int)miUsedBlocks, (int)miBlockChunkSize);

	if (avFakeBlock.FindInSorted(&sBlock, &CompareNamedIndexedBlock, &iIndex))
	{
		if (iIndex != -1)
		{
			if (miUsedBlocks > 1)
			{
				avFakeBlock.RemoveAt(iIndex, TRUE);
				miUsedBlocks--;

				pcBlock = GetUnsafe(0);
				mszFirst.Set(pcBlock->Name());

				pcBlock = GetUnsafe(miUsedBlocks-1);
				mszLast.Set(pcBlock->Name());
				return TRUE;
			}
			else
			{
				miUsedBlocks--;
				return TRUE;
			}
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CNamedIndexesBlock::Write(CIndexedFile* pcFile)
{
	filePos		iIndex;

	if (!IsInFile())
	{
		iIndex = pcFile->Write(mpvCachePos, miBlockChunkSize);
		miDataIndex = iIndex;
		mbDirty = FALSE;
		return iIndex;
	}
	else
	{
		mbDirty = FALSE;
		return pcFile->Write(miDataIndex, mpvCachePos, miBlockChunkSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlock::Uncache(CIndexedFile* pcFile)
{
	filePos		iIndex;

	if (mbDirty)
	{
		iIndex = Write(pcFile);
		if (iIndex != -1)
		{
			mpvCachePos = NULL;
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		mpvCachePos = NULL;
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlock::Cache(CIndexedFile* pcFile, void* pvCache)
{
	BOOL	bResult;
	void*	pvClear;

	if (IsInFile())
	{
		mpvCachePos = pvCache;
		bResult = pcFile->Read(miDataIndex, pvCache, miUsedBlocks);

		if (IsNotFull())
		{
			pvClear = RemapSinglePointer(pvCache, (size_t)(miBlockWidth * miUsedBlocks));
			memset_fast(pvClear, 0, (size_t)((miBlockChunkSize - miUsedBlocks) * miBlockWidth));
		}

		mbDirty = FALSE;
		return bResult;
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
BOOL CNamedIndexesBlock::IsEmpty(void)
{
	return miUsedBlocks == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlock::IsCache(void* pvCachePos)
{
	return mpvCachePos == pvCachePos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexesBlock::Dirty(void)
{
	mbDirty = TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlock::Cache(void* pvCache)
{
	if (pvCache)
	{
		mpvCachePos = pvCache;
		memset_fast(pvCache, 0, (size_t)(miBlockChunkSize * miBlockWidth));
		return TRUE;
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
size_t CNamedIndexesBlock::GetUsedByteSize(void)
{
	return (size_t)(miBlockWidth * miUsedBlocks);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CNamedIndexesBlock::GetAllocatedByteSize(void)
{
	return (size_t)(miBlockWidth * miBlockChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNamedIndexedBlock* CNamedIndexesBlock::GetUnsafe(filePos iIndex)
{
	return (CNamedIndexedBlock*)RemapSinglePointer(mpvCachePos, (size_t)(iIndex * miBlockWidth));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexesBlock::Dump(void)
{
	CArrayBase				avFakeBlock;
	CChars					szText;

	if (IsCached())
	{
		avFakeBlock.Fake(miBlockWidth, mpvCachePos, (int)miUsedBlocks, (int)miBlockChunkSize);
		Dump(&avFakeBlock);
	}
	else
	{
		szText.Init("--- Block(Not Cached) ---\n");
		szText.Append("(");
		szText.Append(mszFirst.Text());
		szText.Append(") -> (");
		szText.Append(mszLast.Text());
		szText.Append(")\n\n");
		szText.Dump();
		szText.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexesBlock::Dump(CArrayBase* pavFakeBlock)
{
	int						i;
	CNamedIndexedBlock*		pvcBlock;
	CChars					szText;

	szText.Init("--- Block(");
	szText.Append(pavFakeBlock->NumElements());
	szText.Append(") ---\n");
	szText.Append("(");
	szText.Append(mszFirst.Text());
	szText.Append(") -> (");
	szText.Append(mszLast.Text());
	szText.Append(")\n");
	for (i = 0; i < pavFakeBlock->NumElements(); i++)
	{
		pvcBlock = (CNamedIndexedBlock*)pavFakeBlock->Get(i);
		
		szText.Append(pvcBlock->Name());
		szText.AppendNewLine();
	}
	szText.AppendNewLine();
	szText.Dump();
	szText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CNamedIndexesBlock::UsedNames(void) 
{ 
	return miUsedBlocks; 
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CNamedIndexesBlock::GetBlockWidth(void) { return miBlockWidth; }
filePos CNamedIndexesBlock::GetNumBlocks(void) { return miBlockChunkSize; }
filePos CNamedIndexesBlock::GetUsedBlocks(void) { return miUsedBlocks; }
char* CNamedIndexesBlock::GetFirst(void) { return mszFirst.Text(); }
char* CNamedIndexesBlock::GetLast(void) { return mszLast.Text(); }
BOOL CNamedIndexesBlock::IsDirty(void) { return mbDirty; }

