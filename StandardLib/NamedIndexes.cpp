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
void CNamedIndexes::Init(CDurableFileController* pcController, int iCacheSize, int iNewNumBlocks)
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

	SetBlocksAfterAdding();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexes::SetBlocksAfterAdding(void)
{
	int						i;
	CNamedIndexesBlocks*	pcBlock;

	for (i = 0; i < macBlocks.NumElements(); i++)
	{
		pcBlock = macBlocks.Get(i);
		pcBlock->Set(&mcCache);
	}
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

	mcFiles.Kill();
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
OIndex CNamedIndexes::GetIndex(CChars* szName)
{
	CNamedIndexesBlocks*	pcBlock;

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
	pcBlock->Init(iBlockSize, iMinNameLength, iMaxNameLength, iNewNumBlocks);
	return pcBlock;
}


