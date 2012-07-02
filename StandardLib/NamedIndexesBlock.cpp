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
#include "NamedIndexedBlock.h"
#include "CoreLib/IndexedFile.h"
#include "NamedIndexesBlock.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexesBlock::Init(int iBlockWidth, int iNumBlocks)
{
	miDataIndex = -1;
	mszFirst.Init();
	mszLast.Init();

	mpvCachePos = NULL;
	miBlockWidth = iBlockWidth;
	miNumBlocks = iNumBlocks;
	miUsedBlocks = 0;
	mbDirty = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexesBlock::Init(int iBlockWidth, void* pvBlocks, int iNumBlocks, int iDataIndex, void* pvCache)
{
	CNamedIndexedBlock*	pcBlock;
	int					i;

	miDataIndex = iDataIndex;

	mpvCachePos = pvCache;
	miBlockWidth = iBlockWidth;
	miNumBlocks = iNumBlocks;
	mbDirty = FALSE;
	miUsedBlocks = iNumBlocks;

	memcpy_fast(mpvCachePos, pvBlocks, miUsedBlocks * miBlockWidth);

	for (i = 0; i < iNumBlocks; i++)
	{
		pcBlock = GetUnsafe(i);
		if (pcBlock->IsEmpty())
		{
			miUsedBlocks = i-1;
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
		miUsedBlocks++;

		pcBlock = GetUnsafe(0);
		mszFirst.Init(pcBlock->Name());

		pcBlock = GetUnsafe(miUsedBlocks-1);
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
	if ((szName->Compare(&mszFirst) >= 0) && (szName->Compare(&mszLast) <= 0))
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
	return miUsedBlocks < miNumBlocks;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlock::IsFull(void)
{
	return miUsedBlocks == miNumBlocks;
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
BOOL CNamedIndexesBlock::AddUnsafe(OIndex oi, CChars* szName)
{
	CArrayBlock			avFakeBlock;
	CNamedIndexedBlock	sBlock;
	int					iIndex;

	sBlock.Set(szName->Text(), oi);

	avFakeBlock.Fake(mpvCachePos, miBlockWidth, miUsedBlocks, miNumBlocks);
	
	//It's safe to insert into a faked array because we know there is at least one free element in the chunk
	//That is: miNumBlocks - miUsedBlocks >= 1
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
	mbDirty = TRUE;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CNamedIndexesBlock::GetIndex(CChars* szName)
{
	CArrayBlock				avFakeBlock;
	CNamedIndexedBlock		sBlock;
	CNamedIndexedBlock*		psBlock;
	int						iIndex;

	strcpy(sBlock.Name(), szName->Text());

	avFakeBlock.Fake(mpvCachePos, miBlockWidth, miUsedBlocks, miNumBlocks);

	if (avFakeBlock.FindInSorted(&sBlock, &CompareNamedIndexedBlock, &iIndex))
	{
		if (iIndex != -1)
		{
			psBlock = (CNamedIndexedBlock*)avFakeBlock.Get(iIndex);
			return psBlock->Id();
		}
	}
	return INVALID_OBJECT_IDENTIFIER;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlock::Remove(CChars* szName)
{
	CArrayBlock				avFakeBlock;
	CNamedIndexedBlock		sBlock;
	int						iIndex;
	CNamedIndexedBlock*		pcBlock;
	
	strcpy(sBlock.Name(), szName->Text());

	avFakeBlock.Fake(mpvCachePos, miBlockWidth, miUsedBlocks, miNumBlocks);

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
BOOL CNamedIndexesBlock::Write(CIndexedFile* pcFile)
{
	return pcFile->Write(miDataIndex, mpvCachePos, miNumBlocks);
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
BOOL CNamedIndexesBlock::SetCache(void* pvCache)
{
	mpvCachePos = pvCache;
	return pvCache != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CNamedIndexesBlock::GetUsedByteSize(void)
{
	return miBlockWidth * miUsedBlocks;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CNamedIndexesBlock::GetAllocatedByteSize(void)
{
	return miBlockWidth * miNumBlocks;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNamedIndexedBlock* CNamedIndexesBlock::GetUnsafe(int iIndex)
{
	return (CNamedIndexedBlock*)RemapSinglePointer(mpvCachePos, iIndex * miBlockWidth);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CNamedIndexesBlock::UsedNames(void) { return miUsedBlocks; }
int CNamedIndexesBlock::GetBlockWidth(void) { return miBlockWidth; }
int CNamedIndexesBlock::GetNumBlocks(void) { return miNumBlocks; }
int CNamedIndexesBlock::GetUsedBlocks(void) { return miUsedBlocks; }
