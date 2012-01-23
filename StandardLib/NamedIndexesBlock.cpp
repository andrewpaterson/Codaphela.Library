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
#include "NamedIndexesBlock.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareNamedIndexedBlock(const void* arg1, const void* arg2)
{
	SNamedIndexedBlock*	ps1;
	SNamedIndexedBlock*	ps2;

	ps1 = (SNamedIndexedBlock*)arg1;
	ps2 = (SNamedIndexedBlock*)arg2;

	return strcmp(ps1->szName, ps2->szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexesBlock::Init(int iBlockWidth, int iNumBlocks)
{
	muiFilePos = -1LL;
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
	SNamedIndexedBlock	sBlock;
	int					iIndex;

	strcpy(sBlock.szName, szName->Text());
	sBlock.oi = oi;

	avFakeBlock.Fake(mpvCachePos, GetBlockSize(), miUsedBlocks, miNumBlocks);
	
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
	CArrayBlock			avFakeBlock;
	SNamedIndexedBlock	sBlock;
	int					iIndex;

	strcpy(sBlock.szName, szName->Text());

	avFakeBlock.Fake(mpvCachePos, GetBlockSize(), miUsedBlocks, miNumBlocks);

	//It's safe to insert into a faked array because we know there is at least one free element in the chunk
	//That is: miNumBlocks - miUsedBlocks >= 1
	if (avFakeBlock.FindInSorted(&sBlock, &CompareNamedIndexedBlock, &iIndex))
	{
		return iIndex;
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
int CNamedIndexesBlock::GetBlockSize(void)
{
	return miBlockWidth * miUsedBlocks;
}

