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
	miFileNumber = 0;
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
	int						i;
	CNamedIndexesBlock*		pcBlock;
	OIndex					oiExisiting;
	CNamedIndexesBlock*		pcNotFullBlock1;
	CNamedIndexesBlock*		pcNotFullBlock2;
	CNamedIndexesBlock*		pcNotFullBlock;

	pcNotFullBlock1 = NULL;
	pcNotFullBlock2 = NULL;
	pcNotFullBlock = NULL;

	for (i = 0; i < macBlocks.NumElements(); i++)
	{
		pcBlock = macBlocks.Get(i);
		if (pcBlock->CouldContain(szName))
		{
			if (!pcBlock->IsCached())
			{
				Cache(pcBlock);
			}

			oiExisiting = pcBlock->GetIndex(szName);
			if (oiExisiting != INVALID_OBJECT_IDENTIFIER)
			{
				//Already exists;
				return !bFailOnExisting;
			}

			if (pcNotFullBlock1 == NULL)
			{
				if (pcBlock->IsNotFull())
				{
					pcNotFullBlock1 = pcBlock;
				}
			}
		}
		else
		{
			if (pcNotFullBlock2 == NULL)
			{
				if (pcBlock->IsNotFull())
				{
					pcNotFullBlock2 = pcBlock;
				}
			}
		}
	}

	if ((pcNotFullBlock1 == NULL) && (pcNotFullBlock2 != NULL))
	{
		pcNotFullBlock = pcNotFullBlock2;
	}
	else if (pcNotFullBlock1 != NULL)
	{
		pcNotFullBlock = pcNotFullBlock1;
	}

	if (pcNotFullBlock == NULL)
	{
		pcNotFullBlock = macBlocks.Add();
		pcNotFullBlock->Init(miBlockWidth, miNewNumBlocks);
		Cache(pcNotFullBlock);
	}
	return pcNotFullBlock->AddUnsafe(oi, szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlocks::AddNewBlock(int iBlockWidth, void* pvBlocks, int iNumBlocks, filePos uiFilePos)
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

	pcBlock->Init(miBlockWidth, pvBlocks, iNumBlocks, uiFilePos, pvCache);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlocks::Cache(CNamedIndexesBlock* pcBlock)
{
	if (pcBlock->IsInFile())
	{
		return FALSE;
	}
	else
	{
		return pcBlock->SetCache(AllocateInCache(pcBlock->GetAllocatedByteSize()));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CNamedIndexesBlocks::AllocateInCache(int iSize)
{
	void*			pvData;
	CArrayPointer	apEvicted;
	int				i;

	apEvicted.Init(128);
	mpcCache->PreAllocate(iSize, &apEvicted);

	for (i = 0; i < apEvicted.NumElements(); i++)
	{
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
	int						i;
	CNamedIndexesBlock*		pcBlock;
	OIndex					oiExisiting;

	for (i = 0; i < macBlocks.NumElements(); i++)
	{
		pcBlock = macBlocks.Get(i);
		if (pcBlock->CouldContain(szName))
		{
			if (!pcBlock->IsCached())
			{
				Cache(pcBlock);
			}

			oiExisiting = pcBlock->GetIndex(szName);
			if (oiExisiting != INVALID_OBJECT_IDENTIFIER)
			{
				return oiExisiting;
			}
		}
	}

	return INVALID_OBJECT_IDENTIFIER;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlocks::Remove(CChars* szName)
{
	int						i;
	CNamedIndexesBlock*		pcBlock;
	CNamedIndexesBlock*		pcNotFullBlock;
	BOOL					bResult;

	pcNotFullBlock = NULL;

	for (i = 0; i < macBlocks.NumElements(); i++)
	{
		pcBlock = macBlocks.Get(i);
		if (pcBlock->CouldContain(szName))
		{
			if (!pcBlock->IsCached())
			{
				Cache(pcBlock);
			}

			bResult = pcBlock->Remove(szName);
			if (bResult)
			{
				if (pcBlock->IsEmpty())
				{
					pcBlock->Kill();
					macBlocks.RemoveAt(i, FALSE);
					pcBlock = macBlocks.SafeGet(i);
					if (pcBlock)
					{
						pcBlock->Dirty();
					}
				}
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

