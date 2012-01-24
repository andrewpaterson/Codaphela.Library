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
#include "NamedIndexesBlocks.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexesBlocks::Init(int iBlockSize, int iMinNameLength, int iMaxNameLength, int iNewNumBlocks)
{
	macBlocks.Init(1);
	miBlockWidth = iBlockSize;
	miMinNameLength = iMinNameLength;
	miMaxNameLength = iMaxNameLength;
	miNewNumBlocks = iNewNumBlocks;
	miFileNumber = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexesBlocks::Set(CMemoryCache* pcCache)
{
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
	CNamedIndexesBlock*		pcNotFullBlock;

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

			if (pcNotFullBlock == NULL)
			{
				if (pcBlock->IsNotFull())
				{
					pcNotFullBlock = pcBlock;
				}
			}
		}
	}

	if (pcNotFullBlock == NULL)
	{
		pcNotFullBlock = macBlocks.Add();
		pcNotFullBlock->Init(miBlockWidth, miNewNumBlocks);
		Cache(pcBlock);
	}
	return pcNotFullBlock->AddUnsafe(oi, szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlocks::Cache(CNamedIndexesBlock* pcBlock)
{
	if (pcBlock->IsInFile())
	{
		
	}
	else
	{
		return pcBlock->SetCache(mpcCache->Allocate(pcBlock->GetBlockSize()));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CNamedIndexesBlocks::GetIndex(CChars* szName)
{

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlocks::Remove(CChars* szName)
{

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlocks::Flush(void)
{

}

